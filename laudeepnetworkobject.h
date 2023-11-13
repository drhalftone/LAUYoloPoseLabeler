#ifndef LAUDEEPNETWORKOBJECT_H
#define LAUDEEPNETWORKOBJECT_H

#include <QObject>

#ifdef ENABLEDEEPNETWORK
#include "opencv2/dnn/dnn.hpp"
#endif

#include "lauimage.h"
#include "laumemoryobject.h"

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUDeepNetworkObject : public QObject
{
    Q_OBJECT

public:
    struct Keypoint {
        Keypoint(float x, float y, float z) {
            this->position = cv::Point3d(x, y, z);
        }
        cv::Point3d position{};
    };

    struct Person {
        Person(cv::Rect2i _box, float _score, std::vector<Keypoint> &_kp){
            this->box = _box;
            this->score = _score;
            this->kp = _kp;
        }
        cv::Rect2i box{};
        float score{0.0};
        std::vector<Keypoint> kp{};
    };

    explicit LAUDeepNetworkObject(QString filename = QString(), QObject *parent = nullptr);

    virtual QList<LAUMemoryObject> process(LAUMemoryObject object, int frame = 0);

    bool isValid() const
    {
#ifdef ENABLEDEEPNETWORK
        return(net.empty() == false);
#else
        return(false);
#endif
    }

signals:

protected:
#ifdef ENABLEDEEPNETWORK
    cv::dnn::Net net;
    std::vector<std::string> layerNames;
#endif
    const cv::Size modelShape = cv::Size(640, 640);
    const float modelScoreThreshold{0.70};
    const float modelNMSThreshold{0.50};
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUYoloSemanticSegmentationObject : public LAUDeepNetworkObject
{
    Q_OBJECT

public:
    explicit LAUYoloSemanticSegmentationObject(QString filename = QString(), QObject *parent = nullptr);

    QList<LAUMemoryObject> process(LAUMemoryObject object, int frame = 0);

signals:

private:
    LAUMemoryObject inObject;
    LAUMemoryObject maObject;
    LAUMemoryObject otObject;
};

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
class LAUYoloPoseObject : public LAUDeepNetworkObject
{
    Q_OBJECT

public:
    explicit LAUYoloPoseObject(QString filename = QString(), QObject *parent = nullptr);

    QList<LAUMemoryObject> process(LAUMemoryObject object, int frame = 0);
    QList<LAUMemoryObject> process(LAUImage image, int frame = 0);
    QList<QVector3D> points(int index, float* confidence);

signals:

private:
    LAUMemoryObject inObject;
    LAUMemoryObject otObject;
};

#endif // LAUDEEPNETWORKOBJECT_H
