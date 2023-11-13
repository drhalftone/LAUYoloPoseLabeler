#include "laudeepnetworkobject.h"

// https://github.com/mallumoSK/yolov8/blob/master/yolo/YoloPose.cpp

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <QList>
#include <QDebug>
#include <QString>
#include <QSettings>
#include <QFileDialog>
#include <QStandardPaths>

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
inline static float clamp(float val, float min, float max) {
    return val > min ? (val < max ? val : max) : min;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUDeepNetworkObject::LAUDeepNetworkObject(QString filename, QObject *parent) : QObject(parent)
{
    if (filename.isNull()) {
        QSettings settings;
        QString directory = settings.value("LAUDeepNetworkObject::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        filename = QFileDialog::getOpenFileName(0, QString("Load model from disk (*.onnx)"), directory, QString("*.onnx"));
        if (filename.isEmpty() == false) {
            settings.setValue("LAUDeepNetworkObject::lastUsedDirectory", QFileInfo(filename).absolutePath());
        } else {
            return;
        }
    }

    try {
        net = cv::dnn::readNetFromONNX(filename.toStdString());
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    } catch (cv::Exception &e) {
        qDebug() << QString(e.msg.data());
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<LAUMemoryObject> LAUDeepNetworkObject::process(LAUMemoryObject object, int frame)
{
    Q_UNUSED(object);
    Q_UNUSED(frame);

    QList<LAUMemoryObject> objects;

    return (objects);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUYoloSemanticSegmentationObject::LAUYoloSemanticSegmentationObject(QString filename, QObject *parent) : LAUDeepNetworkObject(filename, parent)
{
    if (net.empty() == false){
        layerNames.push_back("output0");
        layerNames.push_back("output1");

        inObject = LAUMemoryObject(640, 640, 1, sizeof(float), 3);
        otObject = LAUMemoryObject(8400, 37, 1, sizeof(float));
        maObject = LAUMemoryObject(160, 160, 1, sizeof(float), 32);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<LAUMemoryObject> LAUYoloSemanticSegmentationObject::process(LAUMemoryObject object, int frame)
{
    QList<LAUMemoryObject> objects;

    if (object.depth() == sizeof(unsigned char)) {
        cv::Mat mat = cv::Mat(object.height(), object.width(), CV_8U, object.constFrame(frame), object.step());
        cv::resize(mat, mat, cv::Size(640, 640));

        memcpy(inObject.constFrame(0), mat.data, inObject.block());
        memcpy(inObject.constFrame(1), mat.data, inObject.block());
        memcpy(inObject.constFrame(2), mat.data, inObject.block());
    } else if (object.depth() == sizeof(unsigned short)) {
        cv::Mat matA = cv::Mat(object.height(), object.width(), CV_16U, object.constFrame(frame), object.step());

        cv::Mat matC;
        matA.convertTo(matC, CV_32F);
        matC = (matC - 5000.0) / 6000.0;

        cv::Mat mskC;
        cv::inRange(matC, 0.0, 1.0, mskC);

        cv::Mat matD;
        matC.copyTo(matD, mskC);

        cv::Mat matB;

        matD.convertTo(matB, CV_8U, 255.0);
        cv::resize(matD, matD, cv::Size(640, 640));

        memcpy(inObject.constFrame(0), matD.data, inObject.block());
        memcpy(inObject.constFrame(1), matD.data, inObject.block());
        memcpy(inObject.constFrame(2), matD.data, inObject.block());
    }

    std::vector<int> dims = {1, 3, 640, 640};
    cv::Mat onnxMat(dims, CV_32F, inObject.constPointer());
    //cv::Mat onnxMat = cv::dnn::blobFromImage(cv::imread("/Users/dllau/ultralytics/bus.jpg"), 1.0/255.0, cv::Size(640,640));

    net.setInput(onnxMat);
    try {
        std::vector<cv::Mat> outs;
        net.forward(outs, layerNames);

        // COPY OUTPUT TENSORS TO PRE-ALLOCATED MEMORY OBJECTS
        memcpy(otObject.constPointer(), outs[0].data, otObject.length());
        memcpy(maObject.constPointer(), outs[1].data, maObject.length());

        //otObject.save(QString("/Users/dllau/OneDrive - University of Kentucky/DeepNetworkResults/otObject%1.tif").arg(frame));
        //maObject.save(QString("/Users/dllau/OneDrive - University of Kentucky/DeepNetworkResults/maObject%1.tif").arg(frame));

        // IDENTIFY THE VALID DETECTED REGIONS OF INTEREST
        std::vector<int> indices;
        std::vector<cv::Rect> boxes(otObject.width());
        std::vector<float> scores(otObject.width());
        for (unsigned int col = 0; col < otObject.width(); col++) {
            float x = *((float *)otObject.constPixel(col, 0));
            float y = *((float *)otObject.constPixel(col, 1));
            float w = *((float *)otObject.constPixel(col, 2));
            float h = *((float *)otObject.constPixel(col, 3));

            boxes[col] = cv::Rect(x, y, w, h);
            scores[col] = *((float *)otObject.constPixel(col, 4));
        }
        cv::dnn::NMSBoxes(boxes, scores, 0.1f, 0.5f, indices);

        // ITERATE THROUGH ALL DETECTED REGIONS OF INTEREST
        for (unsigned int n = 0; n < indices.size(); n++) {
            // CREATE A LOCAL OBJECT TO ACCUMULATE MASK IMAGE
            LAUMemoryObject mask(160, 160, 1, sizeof(float));
            memset(mask.constPointer(), 0, mask.length());

            // GRAB THE CURRENT VALID REGION OF INTEREST
            int index = indices[n];

            // ITERATE THROUGH ALL MASKS
            for (unsigned int frm = 0; frm < maObject.frames(); frm++) {
                // SET THE SCALE FACTOR
                __m128 scVec = _mm_set1_ps(*((float *)otObject.constPixel(index, frm + 5)));

                // GRAB POINTERS TO THE MASK BUFFERS
                unsigned char *toBuffer = mask.constPointer();
                unsigned char *fmBuffer = maObject.constFrame(frm);
                for (int pxl = 0; pxl < mask.length(); pxl += 16) {
                    __m128 toVec = _mm_load_ps((float *)toBuffer);
                    __m128 fmVec = _mm_load_ps((float *)fmBuffer);

                    _mm_store_ps((float *)toBuffer, _mm_add_ps(toVec, _mm_mul_ps(fmVec, scVec)));

                    toBuffer += 16;
                    fmBuffer += 16;
                }
            }

            cv::Rect roi = boxes[index];
            int top = qRound(roi.y - roi.height / 2.0f) / 4;
            top = qMax(top, 0);
            int bot = qRound(roi.y + roi.height / 2.0f) / 4;
            bot = qMin(bot, (int)mask.height());
            int lef = qRound(roi.x - roi.width / 2.0f) / 4;
            lef = qMax(lef, 0);
            int rig = qRound(roi.x + roi.width / 2.0f) / 4;
            rig = qMin(rig, (int)mask.width());
            for (unsigned int row = 0; row < mask.height(); row++) {
                float *buffer = (float *)mask.constScanLine(row);
                if (row < top) {
                    memset(buffer, 0, mask.step());
                } else if (row > bot) {
                    memset(buffer, 0, mask.step());
                } else {
                    for (int col = 0; col < lef; col++) {
                        buffer[col] = 0.0f;
                    }
                    for (int col = rig + 1; col < (int)mask.width(); col++) {
                        buffer[col] = 0.0f;
                    }
                }
            }
            cv::Mat mat = cv::Mat(mask.height(), mask.width(), CV_32F, mask.constPointer(), mask.step());
            cv::resize(mat, mat, cv::Size(object.width(), object.height()));

            LAUMemoryObject newObject(object.width(), object.height(), 1, sizeof(unsigned char));
            for (unsigned int row = 0; row < newObject.height(); row++) {
                for (unsigned int col = 0; col < newObject.width(); col++) {
                    *((unsigned char *)newObject.constPixel(col, row)) = (mat.at<float>(row, col) > 0.1f) ? 255 : 0;
                }
            }
            objects << newObject;
        }
    } catch (cv::Exception &e) {
        qDebug() << QString(e.msg.data());
    }
    return (objects);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUYoloPoseObject::LAUYoloPoseObject(QString filename, QObject *parent) : LAUDeepNetworkObject(filename, parent)
{
    if (net.empty() == false){
        layerNames.push_back("output0");

        inObject = LAUMemoryObject(640, 640, 1, sizeof(float), 3);
        otObject = LAUMemoryObject(8400, 45, 1, sizeof(float));
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<LAUMemoryObject> LAUYoloPoseObject::process(LAUImage image, int frame)
{
    Q_UNUSED(frame);

    QList<LAUMemoryObject> objects;

    // MAKE SURE INPUT IMAGE IS THE CORRECT SIZE
    if (image.width() != inObject.width() || image.height() != inObject.height()){
        image = image.rescale(inObject.width(), inObject.height());
    }

    // MAKE SURE INPUT IMAGE IS RGB
    if (image.colors() != 3){
        image = image.convertToRGB();
    }

    // MAKE SURE INPUT IMAGE IS FLOATING POINT
    if (image.depth() != sizeof(float)){
        image = image.convertToFloat();
    }

    LAUImage red = image.extractChannel(0);
    LAUImage grn = image.extractChannel(1);
    LAUImage blu = image.extractChannel(2);

    cv::Mat matR = cv::Mat(red.height(), red.width(), CV_32F, red.constScanLine(0), red.step());
    cv::Mat matG = cv::Mat(grn.height(), grn.width(), CV_32F, grn.constScanLine(0), grn.step());
    cv::Mat matB = cv::Mat(blu.height(), blu.width(), CV_32F, blu.constScanLine(0), blu.step());

    memcpy(inObject.constFrame(0), matR.data, inObject.block());
    memcpy(inObject.constFrame(1), matG.data, inObject.block());
    memcpy(inObject.constFrame(2), matB.data, inObject.block());

    std::vector<int> dims = {1, 3, (int)inObject.width(), (int)inObject.height()};
    cv::Mat onnxMat(dims, CV_32F, inObject.constPointer());

    net.setInput(onnxMat);
    try {
        // RUN THE DEEP NETWORK TO FIND POSES
        std::vector<cv::Mat> outputs;
        net.forward(outputs, layerNames);

        // COPY OUTPUT TENSORS TO PRE-ALLOCATED MEMORY OBJECTS
        memcpy(otObject.constPointer(), outputs[0].data, otObject.length());

        // ADD THE CURRENT OUTPUT OBJECT TO OUR OBJECTS LIST FOR THE USER
        objects << otObject;
    } catch (cv::Exception &e) {
        qDebug() << QString(e.msg.data());
    }
    return (objects);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<LAUMemoryObject> LAUYoloPoseObject::process(LAUMemoryObject object, int frame)
{
    QList<LAUMemoryObject> objects;

    cv::Mat mat(640, 640, CV_32F);
    if (object.depth() == sizeof(unsigned char)) {
        cv::Mat matA = cv::Mat(object.height(), object.width(), CV_8U, object.constFrame(frame), object.step());
        cv::Mat matC;
        matA.convertTo(matC, CV_32F);
        cv::copyMakeBorder(matC, mat, 0, 160, 0, 0, cv::BORDER_CONSTANT, 0);
    } else if (object.depth() == sizeof(unsigned short)) {
        cv::Mat matA = cv::Mat(object.height(), object.width(), CV_16U, object.constFrame(frame), object.step());

        cv::Mat matC;
        matA.convertTo(matC, CV_32F);
        matC = 1.0f - (matC - 4000.0) / 2000.0;

        cv::Mat mskC;
        cv::inRange(matC, 0.0, 1.0, mskC);

        cv::Mat matD;
        matC.copyTo(matD, mskC);
        cv::copyMakeBorder(matD, mat, 0, 160, 0, 0, cv::BORDER_CONSTANT, 0);
    }
    memcpy(inObject.constFrame(0), mat.data, inObject.block());
    memcpy(inObject.constFrame(1), mat.data, inObject.block());
    memcpy(inObject.constFrame(2), mat.data, inObject.block());

    std::vector<int> dims = {1, 3, 640, 640};
    cv::Mat onnxMat(dims, CV_32F, inObject.constPointer());

    net.setInput(onnxMat);
    try {
        // RUN THE DEEP NETWORK TO FIND POSES
        std::vector<cv::Mat> outputs;
        net.forward(outputs, layerNames);

        // COPY OUTPUT TENSORS TO PRE-ALLOCATED MEMORY OBJECTS
        memcpy(otObject.constPointer(), outputs[0].data, otObject.length());

        // ADD THE CURRENT OUTPUT OBJECT TO OUR OBJECTS LIST FOR THE USER
        objects << otObject;
    } catch (cv::Exception &e) {
        qDebug() << QString(e.msg.data());
    }
    return (objects);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QList<QVector3D> LAUYoloPoseObject::points(int index, float *confidence)
{
    // CREATE AN EMPTY DATASTRUCTURE TO RETURN TO THE USER
    QList<QVector3D> points;

    std::vector<cv::Rect> bboxList;
    std::vector<float> scoreList;
    std::vector<int> indicesList;
    std::vector<std::vector<Keypoint>> kpList;

    for (unsigned int col = 0; col < otObject.width(); col++){
        float score = ((float*)otObject.constScanLine(4 + index))[col];
        if (score > *confidence){
            float x0 = *(float*)otObject.constPixel(col,0) - *(float*)otObject.constPixel(col,2)/2.0f;
            float y0 = *(float*)otObject.constPixel(col,1) - *(float*)otObject.constPixel(col,3)/2.0f;
            float x1 = *(float*)otObject.constPixel(col,0) + *(float*)otObject.constPixel(col,2)/2.0f;
            float y1 = *(float*)otObject.constPixel(col,1) + *(float*)otObject.constPixel(col,3)/2.0f;

            cv::Rect_<float> bbox;
            bbox.x = x0;
            bbox.y = y0;
            bbox.width = x1 - x0;
            bbox.height = y1 - y0;

            std::vector<Keypoint> kps;
            for (unsigned int f = 0; f < 13; f++){
                kps.emplace_back(*(float*)otObject.constPixel(col, 3*f + 6),
                                 *(float*)otObject.constPixel(col, 3*f + 7),
                                 *(float*)otObject.constPixel(col, 3*f + 8));
            }
            bboxList.push_back(bbox);
            scoreList.push_back(score);
            kpList.push_back(kps);
        }
    }

    // CONFIRM THAT WE FOUND AT LEAST ONE REGION OF INTEREST
    if (scoreList.size() > 0){
        // MERGE ANY OVERLAPPING REGIONS OF INTEREST INTO A SINGLE ROI AND SET OF FIDUCIALS
        cv::dnn::NMSBoxes(bboxList, scoreList, modelScoreThreshold, modelNMSThreshold, indicesList);

        // PRESERVE THE INCOMING SCORE TO CONFIDENCE
        *confidence = scoreList.at(indicesList.at(0));

        // ITERATE THROUGH ALL INDICES THAT CORRESPOND TO THE PRESERVED REGION OF INTEREST
        for (unsigned int col = 0; col < indicesList.size(); col++){
            int ind = indicesList.at(col);

            // ADD BOUNDING BOX TO OUTPUT VECTOR
            cv::Rect_<float> bbox = bboxList.at(ind);
            points << QVector3D(bbox.x, bbox.y, 1.0);
            points << QVector3D(bbox.width, bbox.height, 1.0);

            // ADD FIDUCIALS TO OUTPUT VECTOR
            std::vector<Keypoint> kps = kpList.at(col);
            for (unsigned int f = 0; f < 13; f++){
                points << QVector3D(kps.at(f).position.x, kps.at(f).position.y, kps.at(f).position.z);
            }
        }
    } else {
        // PRESERVE THE INCOMING SCORE TO CONFIDENCE
        *confidence = 0.0f;
    }

    // RETURN OUTPUT VECTOR TO USER
    return(points);
}
