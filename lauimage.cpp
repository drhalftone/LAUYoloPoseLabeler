#include "lauimage.h"

#include <QDesktopWidget>

using namespace libtiff;

int LAUImageData::instanceCounter = 0;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void myTIFFWarningHandler(const char *stringA, const char *stringB, va_list)
{
    static int warningCount = 0;
    warningCount++;

    qDebug() << QString("myTIFFWarningHandler") << QByteArray(stringA) << QByteArray(stringB);

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void myTIFFErrorHandler(const char *stringA, const char *stringB, va_list)
{
    static int errorCount = 0;
    errorCount++;

    qDebug() << QString("myTIFFErrorHandler") << QByteArray(stringA) << QByteArray(stringB);

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageData::LAUImageData()
{
    numRows = 0;
    numCols = 0;
    numChns = 0;
    numByts = 0;
    buffer = nullptr;
    stepBytes = 0;
    pProfile = nullptr;
    photometric = 0;
    xResolution = 0.0;
    yResolution = 0.0;
    numProfileBytes = 0;
    fileString = QString();
    xmlByteArray.clear();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageData::LAUImageData(const LAUImageData &other)
{
    numRows = other.numRows;
    numCols = other.numCols;
    numChns = other.numChns;
    numByts = other.numByts;
    stepBytes = other.stepBytes;
    fileString = other.fileString;

    photometric = other.photometric;
    xResolution = other.xResolution;
    yResolution = other.yResolution;

    xmlByteArray = other.xmlByteArray;

    qDebug() << QString("Performing deep copy on %1").arg(fileString);
    if (other.pProfile) {
        numProfileBytes = other.numProfileBytes;
        pProfile = (unsigned char *)malloc(numProfileBytes);
        memcpy(pProfile, other.pProfile, numProfileBytes);
    } else {
        pProfile = nullptr;
        numProfileBytes = 0;
    }
    allocateBuffer();
    if (other.buffer != nullptr) {
        memcpy(buffer, other.buffer, stepBytes * numRows);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImageData::~LAUImageData()
{
    //qDebug() << QString("LAUImageData::~LAUImageData() %1 (%2 x %3)").arg(--instanceCounter).arg(numRows).arg(numCols);
    if (buffer != nullptr) {
        _mm_free(buffer);
        buffer = nullptr;
    }
    if (pProfile != nullptr) {
        free(pProfile);
        pProfile = nullptr;
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImageData::allocateBuffer()
{
    //qDebug() << QString("LAUImageData::allocateBuffer() %1").arg(instanceCounter++) << numRows << numCols << numChns << numByts;

    // ALLOCATE SPACE FOR HOLDING PIXEL DATA BASED ON NUMBER OF CHANNELS AND BYTES PER PIXEL
    unsigned long long numBytesToAllocate;
    numBytesToAllocate  = (unsigned long long)numRows;
    numBytesToAllocate *= (unsigned long long)numCols;
    numBytesToAllocate *= (unsigned long long)numChns;
    numBytesToAllocate *= (unsigned long long)numByts;

    if (numBytesToAllocate > 0) {
        buffer = _mm_malloc(numBytesToAllocate, 16);
        stepBytes = numCols * numChns * numByts;
        if (buffer == nullptr) {
            qDebug() << QString("LAUImageData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUImageData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
            qDebug() << QString("LAUImageData::allocateBuffer() MAJOR ERROR DID NOT ALLOCATE SPACE!!!");
        }
    }
    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUImageData::setProfile(cmsHPROFILE iccProfile)
{
    if (iccProfile) {
        // MAKE SURE INCOMING PROFILE HAS THE SAME NUMBER OF CHANNELS AS CURRENT IMAGE
        if (LAUCMSWidget::howManyColorsDoesThisProfileHave(iccProfile) != numChns) {
            return (false);
        }

        // RELEASE MEMORY CURRENTLY BEING USED TO STORE OLD PROFILE
        if (pProfile) {
            free(pProfile);
        }

        // COPY PROFILE TO MEMORY AND IF SUCCESFULL, ALLOCATE SPACE FOR NEW IMAGE
        numProfileBytes = cmsSaveProfileToIOhandler(iccProfile, nullptr);
        pProfile = (unsigned char *)malloc(numProfileBytes);
        photometric = LAUCMSWidget::whatPhotometricTagMatchesThisProfile(iccProfile);
        cmsIOHANDLER *ioHandler = cmsOpenIOhandlerFromMem(cmsGetProfileContextID(iccProfile), pProfile, numProfileBytes, "w");
        bool flag = cmsSaveProfileToIOhandler(iccProfile, ioHandler);
        cmsCloseIOhandler(ioHandler);

        return (flag);
    } else {
        // SINCE INPUT IS nullptr PROFILE, MAKE SURE TO
        // DELETE THE EXISTING PROFILE IF IT EXISTS
        if (pProfile) {
            free(pProfile);
            numProfileBytes = 0;
            pProfile = nullptr;
        }
        return (true);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::LAUImage(unsigned int rows, unsigned int cols, unsigned int dpth, cmsHPROFILE iccProfile, float xRes, float yRes)
{
    data = new LAUImageData();
    data->numRows = rows;
    data->numCols = cols;
    data->numByts = dpth;
    data->xResolution = xRes;
    data->yResolution = yRes;

    // WE NEED A VALID ICC PROFILE TO CREATE A VALID IMAGE
    // SO USE A MONOCHROME PROFILE IF ONE ISN'T GIVEN
    if (!iccProfile) {
        iccProfile = cmsCreateGrayProfile(cmsD50_xyY(), cmsBuildGamma(0, 1.0));
    }

    // SET THE NUMBER OF CHANNELS BASED ON INPUT PROFILE
    data->numChns = LAUCMSWidget::howManyColorsDoesThisProfileHave(iccProfile);
    data->photometric = LAUCMSWidget::whatPhotometricTagMatchesThisProfile(iccProfile);

    // COPY PROFILE TO MEMORY AND IF SUCCESFULL, ALLOCATE SPACE FOR NEW IMAGE
    if (data->setProfile(iccProfile)) {
        data->allocateBuffer();
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::LAUImage(unsigned int rows, unsigned int cols, unsigned int chns, unsigned int dpth, float xRes, float yRes)
{
    data = new LAUImageData();
    data->numRows = rows;
    data->numCols = cols;
    data->numChns = chns;
    data->numByts = dpth;
    data->xResolution = xRes;
    data->yResolution = yRes;

    data->allocateBuffer();
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::LAUImage(QString filename)
{
    // IF WE HAVE A VALID TIFF FILE, LOAD FROM DISK
    // OTHERWISE TRY TO CONNECT TO SCANNER
    data = new LAUImageData();

    if (filename.isNull()) {
        QSettings settings;
        QString directory = settings.value("LAUImage::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        filename = QFileDialog::getOpenFileName(0, QString("Load image from disk (*.tif)"), directory, QString("*.tif;*.tiff"));
        if (filename.isEmpty() == false) {
            settings.setValue("LAUImage::lastUsedDirectory", QFileInfo(filename).absolutePath());
        } else {
            return;
        }
    }

    if (QFile::exists(filename)) {
        if (filename.toLower().endsWith(".tif") || filename.toLower().endsWith(".tiff")) {
            // OPEN INPUT TIFF FILE FROM DISK
            TIFF *inTiff = TIFFOpen(filename.toLatin1(), "r");
            if (!inTiff) {
                return;
            }
            load(inTiff);
            TIFFClose(inTiff);
        } else {
            // MAKE SURE WE HAVE A 24-BIT RGB IMAGE
            QImage image = QImage(filename).convertToFormat(QImage::Format_RGB888);

            data = new LAUImageData();
            data->numRows = image.height();
            data->numCols = image.width();
            data->numChns = 3;
            data->numByts = sizeof(unsigned char);
            data->photometric = PHOTOMETRIC_RGB;
            data->xResolution = 300.0f;
            data->yResolution = 300.0f;

            // CREATE AN SRGB COLOR PROFILE
            cmsHPROFILE rgbProfile = cmsCreate_sRGBProfile();
            data->setProfile(rgbProfile);
            cmsCloseProfile(rgbProfile);

            // ALLOCATE SPACE FOR HOLDING PIXEL DATA
            data->allocateBuffer();

            // COPY PIXELS OVER FROM SUPPLIED QIMAGE
            for (unsigned int row = 0; row < height(); row++) {
                unsigned char *buffer = (unsigned char *)scanLine(row);
                for (unsigned int col = 0; col < width(); col++) {
                    QColor pixel(image.pixel(col, row));
                    buffer[3 * col + 0] = pixel.red();
                    buffer[3 * col + 1] = pixel.green();
                    buffer[3 * col + 2] = pixel.blue();
                }
            }
        }
    }
    data->fileString = filename;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::LAUImage(const QImage image, float xRes, float yRes)
{
    // MAKE SURE WE HAVE A 24-BIT RGB IMAGE
    QImage imageP = image.convertToFormat(QImage::Format_RGB888);

    data = new LAUImageData();
    data->numRows = imageP.height();
    data->numCols = imageP.width();
    data->numChns = 3;
    data->numByts = sizeof(unsigned char);
    data->photometric = PHOTOMETRIC_RGB;
    data->xResolution = xRes;
    data->yResolution = yRes;

    // CREATE AN SRGB COLOR PROFILE
    cmsHPROFILE rgbProfile = cmsCreate_sRGBProfile();
    data->setProfile(rgbProfile);
    cmsCloseProfile(rgbProfile);

    // ALLOCATE SPACE FOR HOLDING PIXEL DATA
    data->allocateBuffer();

    // COPY PIXELS OVER FROM SUPPLIED QIMAGE
    for (unsigned int row = 0; row < height(); row++) {
        unsigned char *buffer = (unsigned char *)scanLine(row);
        for (unsigned int col = 0; col < width(); col++) {
            QColor pixel(imageP.pixel(col, row));
            buffer[3 * col + 0] = pixel.red();
            buffer[3 * col + 1] = pixel.green();
            buffer[3 * col + 2] = pixel.blue();
        }
    }

    return;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::LAUImage(TIFF *currentTiffDirectory)
{
    data = new LAUImageData();
    this->load(currentTiffDirectory);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage::~LAUImage()
{
    ;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUImage::save(QString filename)
{
    if (filename.isNull()) {
        QSettings settings;
        QString directory = settings.value("LAUImage::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        filename = QFileDialog::getSaveFileName(0, QString("Save image to disk (*.tif)"), directory, QString("*.tif;*.tiff"));
        if (!filename.isNull()) {
            settings.setValue(QString("LAUImage::lastUsedDirectory"), QFileInfo(filename).absolutePath());
            if (!filename.toLower().endsWith(QString(".tiff"))) {
                if (!filename.toLower().endsWith(QString(".tif"))) {
                    filename = QString("%1.tif").arg(filename);
                }
            }
        } else {
            return (false);
        }
    }

    // OPEN TIFF FILE FOR SAVING THE IMAGE
    TIFF *outputTiff = TIFFOpen(filename.toLatin1(), "w");
    if (!outputTiff) {
        return (false);
    }

    // WRITE IMAGE TO CURRENT DIRECTORY
    save(outputTiff);

    // CLOSE TIFF FILE
    TIFFClose(outputTiff);

    // MAKE SURE TO KEEP TRACK OF THE NEW FILENAME
    setFilename(filename);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUImage::save(TIFF *currentTiffDirectory)
{
    // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
    TIFFSetField(currentTiffDirectory, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(currentTiffDirectory, TIFFTAG_IMAGEWIDTH, static_cast<unsigned long>(width()));
    TIFFSetField(currentTiffDirectory, TIFFTAG_IMAGELENGTH, static_cast<unsigned long>(height()));
    TIFFSetField(currentTiffDirectory, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(currentTiffDirectory, TIFFTAG_XRESOLUTION, static_cast<double>(xRes()));
    TIFFSetField(currentTiffDirectory, TIFFTAG_YRESOLUTION, static_cast<double>(yRes()));
    TIFFSetField(currentTiffDirectory, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(currentTiffDirectory, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(currentTiffDirectory, TIFFTAG_SAMPLESPERPIXEL, static_cast<unsigned short>(colors()));
    TIFFSetField(currentTiffDirectory, TIFFTAG_BITSPERSAMPLE, static_cast<unsigned short>(depth() << 3));
    if (photometricInterpretation()) {
        TIFFSetField(currentTiffDirectory, TIFFTAG_PHOTOMETRIC, static_cast<unsigned short>(photometricInterpretation()));
    } else {
        switch (colors()) {
            case 3:
                TIFFSetField(currentTiffDirectory, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
                break;
            case 4:
                TIFFSetField(currentTiffDirectory, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_SEPARATED);
                break;
            default:
                TIFFSetField(currentTiffDirectory, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
                break;
        }
    }
#ifndef _TTY_WIN_
    TIFFSetField(currentTiffDirectory, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    TIFFSetField(currentTiffDirectory, TIFFTAG_PREDICTOR, 2);
    TIFFSetField(currentTiffDirectory, TIFFTAG_ROWSPERSTRIP, 1);
#endif

    // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
    // PIXELS IN 32-BIT FLOATING POINT FORMAT
    if (depth() == sizeof(float)) {
        TIFFSetField(currentTiffDirectory, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }

    // WRITE THE FILESTRING TO THE DOCUMENT NAME TIFFTAG
    TIFFSetField(currentTiffDirectory, TIFFTAG_DOCUMENTNAME, filename().toLatin1().data());
    if (!parentName().isEmpty()) {
        TIFFSetField(currentTiffDirectory, TIFFTAG_PAGENAME, parentName().toLatin1().data());
    }

    // WRITE ICC PROFILE IF IT EXISTS
    if (pProfile()) {
        TIFFSetField(currentTiffDirectory, TIFFTAG_ICCPROFILE, numProfileBytes(), pProfile());
    }

    // WRITE XML DATA IF IT EXISTS
    QByteArray xmlByteArray = xmlData();
    if (xmlByteArray.length() > 0) {
        TIFFSetField(currentTiffDirectory, TIFFTAG_XMLPACKET, xmlByteArray.length(), xmlByteArray.data());
    }

    // MAKE SURE WE HAVE PIXELS TO WRITE TO DISK
    if (isValid()) {
        // MAKE TEMPORARY BUFFER TO HOLD CURRENT ROW BECAUSE COMPRESSION DESTROYS
        // WHATS EVER INSIDE THE BUFFER
        unsigned char *tempBuffer = (unsigned char *)malloc(step());
        for (unsigned int row = 0; row < height(); row++) {
            memcpy(tempBuffer, constScanLine(row), step());
            TIFFWriteScanline(currentTiffDirectory, tempBuffer, row, 0);
        }
        free(tempBuffer);
    }

    // WRITE THE CURRENT DIRECTORY AND PREPARE FOR THE NEW ONE
    TIFFWriteDirectory(currentTiffDirectory);

    return (true);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUImage::load(TIFF *inTiff)
{
    // LOAD INPUT TIFF FILE PARAMETERS IMPORTANT TO RESAMPLING THE IMAGE
    unsigned long uLongVariable;
    unsigned short uShortVariable;
    unsigned short inChns;

    // GET THE HEIGHT AND WIDTH OF INPUT IMAGE IN PIXELS
    TIFFGetField(inTiff, TIFFTAG_IMAGEWIDTH, &uLongVariable);
    data->numCols = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_IMAGELENGTH, &uLongVariable);
    data->numRows = uLongVariable;
    TIFFGetField(inTiff, TIFFTAG_SAMPLESPERPIXEL, &uShortVariable);
    inChns = uShortVariable;
    TIFFGetField(inTiff, TIFFTAG_BITSPERSAMPLE, &uShortVariable);
    data->numByts = uShortVariable / 8;
    TIFFGetField(inTiff, TIFFTAG_PHOTOMETRIC, &uShortVariable);
    data->photometric = uShortVariable;

    // READ IN THE HORIZONTAL AND VERTICAL RESOLUTIONS
    uShortVariable = RESUNIT_INCH;
    data->xResolution = 72.0;
    data->yResolution = 72.0;
    TIFFGetField(inTiff, TIFFTAG_RESOLUTIONUNIT, &uShortVariable);    // get resolution unit (inches or centimeters)
    TIFFGetField(inTiff, TIFFTAG_XRESOLUTION, &(data->xResolution));  // get resolution in X direction
    TIFFGetField(inTiff, TIFFTAG_YRESOLUTION, &(data->yResolution));  // get resolution in X direction

    // CONVERT CENTIMETERS TO INCHES IF NECESSARY
    if (uShortVariable == RESUNIT_CENTIMETER) {
        data->xResolution *= 2.54;
        data->yResolution *= 2.54;
    }

    // LOAD THE XML FIELD OF THE TIFF FILE, IF PROVIDED
    int dataLength;
    char *dataString;
    bool dataPresent = TIFFGetField(inTiff, TIFFTAG_XMLPACKET, &dataLength, &dataString);
    if (dataPresent) {
        QByteArray byteArray(dataString, dataLength);
        int index = byteArray.lastIndexOf(QByteArray("\n"));
        if (index > -1) {
            data->xmlByteArray = byteArray.left(index + 1);
        }
    }

    // LOAD A CUSTOM FILESTRING FROM THE DOCUMENT NAME TIFFTAG
    dataPresent = TIFFGetField(inTiff, TIFFTAG_DOCUMENTNAME, &dataString);
    if (dataPresent) {
        data->fileString = QString(QByteArray(dataString));
    }

    // LOAD A CUSTOM PARENT FILESTRING FROM THE PAGE NAME TIFFTAG
    dataPresent = TIFFGetField(inTiff, TIFFTAG_PAGENAME, &dataString);
    if (dataPresent) {
        data->parentName = QByteArray(dataString);
    }

    // LOAD THE IMAGE ICC PROFILE, IF PROVIDED
    cmsHPROFILE iccProfile = nullptr;
    if (TIFFGetField(inTiff, TIFFTAG_ICCPROFILE, &dataLength, &dataString)) {
        data->numProfileBytes = dataLength;
        data->pProfile = (unsigned char *)malloc(data->numProfileBytes);
        memcpy(data->pProfile, dataString, data->numProfileBytes);
        iccProfile = profile();

        // MAKE SURE PHOTOMETRIC FLAG MATCHES ICC PROFILE
        data->photometric = LAUCMSWidget::whatPhotometricTagMatchesThisProfile(iccProfile);
    } else {
        if (data->photometric == PHOTOMETRIC_RGB) {
            iccProfile = LAUCMSWidget::profile(LAUCMSDialog::rgbProfileString);
            if (iccProfile == nullptr) {
                iccProfile = cmsCreate_sRGBProfile();
            }
        } else if (data->photometric == PHOTOMETRIC_SEPARATED) {
            iccProfile = LAUCMSWidget::profile(LAUCMSDialog::cmykProfileString);
        } else if (data->photometric == PHOTOMETRIC_MINISBLACK || data->photometric == PHOTOMETRIC_MINISWHITE) {
            iccProfile = LAUCMSWidget::profile(LAUCMSDialog::grayProfileString);
            if (iccProfile == nullptr) {
                iccProfile = cmsCreateGrayProfile(cmsD50_xyY(), cmsBuildGamma(0, 1.0));
            }
        } else {
            iccProfile = LAUCMSWidget::loadProfileFromDisk(colors(), filename());
        }
    }

    if (iccProfile == nullptr) {
        // WITHOUT AN ICC PROFILE, WE NEED TO OPEN THE FILE AS ARGB AND THEN CONVERT TO RGB
        data->numChns = 3;
        data->numByts = 1;
        data->photometric = PHOTOMETRIC_RGB;

        // ALLOCATE SPACE TO HOLD IMAGE DATA
        data->allocateBuffer();

        // CREATE TRANSFORM FROM THIS PROFILE TO RGB, IF IT ISN'T ALREADY IN AN RGB PROFILE
        iccProfile = cmsCreate_sRGBProfile();

        // READ THE IMAGE INTO MEMORY USING THE ARGB UTILITIES PROVIDED BY LIBTIFF
        int numPixels = data->numCols * data->numRows;
        unsigned char *buffer = (unsigned char *)_TIFFmalloc(numPixels * sizeof(uint32));
        if (buffer != nullptr) {
            if (TIFFReadRGBAImage(inTiff, data->numCols, data->numRows, (unsigned int *)buffer, 0)) {
                // CONVERT THE ARGB TO RGB DATA
                ippiCopy_8u_AC4C3R(buffer, data->numCols << 2, (unsigned char *)data->buffer, data->stepBytes, this->size());

                // FLIP THE IMAGE TO FIX THE FLIPPING CAUSED BY THE ARGB UTILITY
                ippiMirror_8u_C3R((unsigned char *)data->buffer, data->stepBytes, (unsigned char *)data->buffer, data->stepBytes, this->size(), ippAxsHorizontal);
            }
            _TIFFfree(buffer);
        }
    } else {
        // LET THE ICC PROFILE DEFINE HOW MANY CHANNELS IN THE OUTPUT IMAGE
        data->numChns = LAUCMSWidget::howManyColorsDoesThisProfileHave(iccProfile);

        // ALLOCATE SPACE TO HOLD IMAGE DATA
        data->allocateBuffer();

        // FIND OUT IF THE IMAGE HAS ALPHA OR SPOT CHANNELS
        bool hasAlphaChannels = (data->numChns != inChns);

        if (hasAlphaChannels == false) {
            // READ DATA AS EITHER CHUNKY OR PLANAR FORMAT
            short shortVariable;
            TIFFGetField(inTiff, TIFFTAG_PLANARCONFIG, &shortVariable);
            if (shortVariable == PLANARCONFIG_SEPARATE) {
                unsigned char *tempBuffer = new unsigned char [width()*depth()];
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    for (unsigned int row = 0; row < height(); row++) {
                        unsigned char *pBuffer = scanLine(row);
                        TIFFReadScanline(inTiff, tempBuffer, (int)row, (int)chn);
                        for (unsigned int col = 0; col < width(); col++) {
                            switch (depth()) {
                                case sizeof(unsigned char):
                                    ((unsigned char *)pBuffer)[col * colors() + chn] = ((unsigned char *)tempBuffer)[col];
                                case sizeof(unsigned short):
                                    ((unsigned short *)pBuffer)[col * colors() + chn] = ((unsigned short *)tempBuffer)[col];
                                case sizeof(float):
                                    ((float *)pBuffer)[col * colors() + chn] = ((float *)tempBuffer)[col];
                                default:
                                    break;
                            }
                        }
                    }
                }
                delete [] tempBuffer;
            } else if (shortVariable == PLANARCONFIG_CONTIG) {
                for (unsigned int row = 0; row < height(); row++) {
                    unsigned char *pBuffer = scanLine(row);
                    TIFFReadScanline(inTiff, pBuffer, (int)row);
                }
            }
        } else {
            // READ DATA AS EITHER CHUNKY OR PLANAR FORMAT
            short shortVariable;
            TIFFGetField(inTiff, TIFFTAG_PLANARCONFIG, &shortVariable);
            if (shortVariable == PLANARCONFIG_SEPARATE) {
                unsigned char *tempBuffer = new unsigned char [width()*depth()];
                for (unsigned int chn = 0; chn < colors() && chn < inChns; chn++) {
                    for (unsigned int row = 0; row < height(); row++) {
                        unsigned char *pBuffer = scanLine(row);
                        TIFFReadScanline(inTiff, tempBuffer, (int)row, (int)chn);
                        for (unsigned int col = 0; col < width(); col++) {
                            switch (depth()) {
                                case sizeof(unsigned char):
                                    ((unsigned char *)pBuffer)[col * colors() + chn] = ((unsigned char *)tempBuffer)[col];
                                case sizeof(unsigned short):
                                    ((unsigned short *)pBuffer)[col * colors() + chn] = ((unsigned short *)tempBuffer)[col];
                                case sizeof(float):
                                    ((float *)pBuffer)[col * colors() + chn] = ((float *)tempBuffer)[col];
                                default:
                                    break;
                            }
                        }
                    }
                }
                delete [] tempBuffer;
            } else if (shortVariable == PLANARCONFIG_CONTIG) {
                unsigned char *tempBuffer = new unsigned char [width()*depth()*inChns];
                for (unsigned int row = 0; row < height(); row++) {
                    unsigned char *pBuffer = scanLine(row);
                    TIFFReadScanline(inTiff, tempBuffer, (int)row);
                    for (unsigned int col = 0; col < width(); col++) {
                        switch (depth()) {
                            case sizeof(unsigned char):
                                for (unsigned int chn = 0; chn < colors() && chn < inChns; chn++) {
                                    ((unsigned char *)pBuffer)[col * colors() + chn] = ((unsigned char *)tempBuffer)[col * inChns + chn];
                                }
                                break;
                            case sizeof(unsigned short):
                                for (unsigned int chn = 0; chn < colors() && chn < inChns; chn++) {
                                    ((unsigned short *)pBuffer)[col * colors() + chn] = ((unsigned short *)tempBuffer)[col * inChns + chn];
                                }
                                break;
                            case sizeof(float):
                                for (unsigned int chn = 0; chn < colors() && chn < inChns; chn++) {
                                    ((float *)pBuffer)[col * colors() + chn] = ((float *)tempBuffer)[col * inChns + chn];
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
                delete [] tempBuffer;
            }
        }
    }

    // SAVE THE ICC PROFILE TO THE DATA OBJECT
    data->setProfile(iccProfile);
    cmsCloseProfile(iccProfile);

    return (true); //wasNotCancelledFlag);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImage::setXmlData(const QByteArray byteArray)
{
    data->xmlByteArray = byteArray;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void LAUImage::replace(const LAUImage &other)
{
    data = other.data;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHPROFILE LAUImage::profile()
{
    if (pProfile()) {
        return (cmsOpenProfileFromMem(pProfile(), numProfileBytes()));
    }
    return (nullptr);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::flip(QWidget *parent, bool *wasCanceled)
{
    LAUImage image(height(), width(), depth(), profile(), xRes(), yRes());

    // LET'S ASSUME USER WON'T CANCEL THIS OPERATION
    if (wasCanceled) {
        *wasCanceled = false;
    }

    QProgressDialog dialog(QString("Image Flip"), QString("Abort"), 0, height(), parent);
    for (unsigned int row = 0; row < height(); row++) {
        if (dialog.wasCanceled()) {
            if (wasCanceled) {
                *wasCanceled = true;
            }
            break;
        }
        qApp->processEvents();
        dialog.setValue(row);
        if (depth() == sizeof(unsigned char)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned char *)image.scanLine(row))[colP * colors() + chn] = ((unsigned char *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(unsigned short)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned short *)image.scanLine(row))[colP * colors() + chn] = ((unsigned short *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(float)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((float *)image.scanLine(row))[colP * colors() + chn] = ((float *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        }
    }
    dialog.setValue(height());
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::rotate90(QWidget *parent, bool *wasCanceled)
{
    LAUImage image(width(), height(), depth(), profile(), yRes(), xRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    // LET'S ASSUME USER WON'T CANCEL THIS OPERATION
    if (wasCanceled) {
        *wasCanceled = false;
    }

    QProgressDialog dialog(QString("Image Rotate"), QString("Abort"), 0, height(), parent);
    dialog.setModal(true);
    for (unsigned int row = 0; row < height(); row++) {
        if (dialog.wasCanceled()) {
            if (wasCanceled) {
                *wasCanceled = true;
            }
            break;
        }
        qApp->processEvents();
        dialog.setValue(row);
        if (depth() == sizeof(unsigned char)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned char *)image.scanLine(col))[row * colors() + chn] = ((unsigned char *)(this->constScanLine(row)))[colP * colors() + chn];
                }
            }
        } else if (depth() == sizeof(unsigned short)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned short *)image.scanLine(col))[row * colors() + chn] = ((unsigned short *)(this->constScanLine(row)))[colP * colors() + chn];
                }
            }
        } else if (depth() == sizeof(float)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((float *)image.scanLine(col))[row * colors() + chn] = ((float *)(this->constScanLine(row)))[colP * colors() + chn];
                }
            }
        }
    }
    dialog.setValue(height());
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::unrotate90(QWidget *parent, bool *wasCanceled)
{
    LAUImage image(width(), height(), depth(), profile(), yRes(), xRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    // LET'S ASSUME USER WON'T CANCEL THIS OPERATION
    if (wasCanceled) {
        *wasCanceled = false;
    }

    QProgressDialog dialog(QString("Image Rotate"), QString("Abort"), 0, height(), parent);
    dialog.setModal(true);
    for (unsigned int row = 0; row < height(); row++) {
        unsigned int rowP = height() - 1 - row;
        if (dialog.wasCanceled()) {
            if (wasCanceled) {
                *wasCanceled = true;
            }
            break;
        }
        qApp->processEvents();
        dialog.setValue(row);
        if (depth() == sizeof(unsigned char)) {
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned char *)image.scanLine(col))[rowP * colors() + chn] = ((unsigned char *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(unsigned short)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned short *)image.scanLine(col))[rowP * colors() + chn] = ((unsigned short *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(float)) {
            for (unsigned int col = 0; col < width(); col++) {
                unsigned int colP = width() - 1 - col;
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((float *)image.scanLine(col))[rowP * colors() + chn] = ((float *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        }
    }
    dialog.setValue(height());
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::flipAndRotate(QWidget *parent, bool *wasCanceled)
{
    LAUImage image(width(), height(), depth(), profile(), yRes(), xRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    // LET'S ASSUME USER WON'T CANCEL THIS OPERATION
    if (wasCanceled) {
        *wasCanceled = false;
    }

    QProgressDialog dialog(QString("Image Flip and Rotate"), QString("Abort"), 0, height(), parent);
    dialog.setModal(true);
    dialog.show();
    for (unsigned int row = 0; row < height(); row++) {
        if (dialog.wasCanceled()) {
            if (wasCanceled) {
                *wasCanceled = true;
            }
            break;
        }
        qApp->processEvents();
        dialog.setValue(row);
        if (depth() == sizeof(unsigned char)) {
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned char *)image.scanLine(col))[row * colors() + chn] = ((unsigned char *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(unsigned short)) {
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((unsigned short *)image.scanLine(col))[row * colors() + chn] = ((unsigned short *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        } else if (depth() == sizeof(float)) {
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    ((float *)image.scanLine(col))[row * colors() + chn] = ((float *)(this->constScanLine(row)))[col * colors() + chn];
                }
            }
        }
    }
    dialog.setValue(height());
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::invert()
{
    // MAKE A COPY OF THE SUBJECT IMAGE
    LAUImage image = *this;

    // NOW INVERT THE PIXELS BASED ON DATA TYPE
    if (depth() == sizeof(unsigned char)) {
        __m128i scalarVec = _mm_set1_epi8(0xff);
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *inBuffer = this->constScanLine(row);
            unsigned char *otBuffer = image.scanLine(row);
            for (unsigned int byte = 0; byte < step(); byte += 16) {
                __m128i inputVec = _mm_load_si128((__m128i *) & (inBuffer[byte]));
                __m128i otputVec = _mm_sub_epi8(scalarVec, inputVec);
                _mm_store_si128((__m128i *) & (otBuffer[byte]), otputVec);
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        __m128i scalarVec = _mm_set1_epi16(0xffff);
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *inBuffer = this->constScanLine(row);
            unsigned char *otBuffer = image.scanLine(row);
            for (unsigned int byte = 0; byte < step(); byte += 16) {
                __m128i inputVec = _mm_load_si128((__m128i *) & (inBuffer[byte]));
                __m128i otputVec = _mm_sub_epi16(scalarVec, inputVec);
                _mm_store_si128((__m128i *) & (otBuffer[byte]), otputVec);
            }
        }
    } else if (depth() == sizeof(float)) {
        __m128 scalarVec = _mm_set1_ps(1.0);
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *inBuffer = this->constScanLine(row);
            unsigned char *otBuffer = image.scanLine(row);
            for (unsigned int byte = 0; byte < step(); byte += 16) {
                __m128 inputVec = _mm_load_ps((float *)&inBuffer[byte]);
                __m128 otputVec = _mm_sub_ps(scalarVec, inputVec);
                _mm_store_ps((float *)&otBuffer[byte], otputVec);
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::addTitle(QString string, float hght)
{
    // MAKE A LOCAL COPY OF OURSELVES
    LAUImage image = *this;

    // MAKE A QIMAGE TO DRAW OUR LABEL INTO
    QImage label(width(), qFloor(hght * yRes()), QImage::Format_Grayscale8);
    label.fill(Qt::white);

    // DRAW THE LABEL
    QPainter painter;
    painter.begin(&label);
    painter.setPen(Qt::black);
    painter.setFont(QFont(QString("Arial"), label.height() - 2, QFont::Normal, false));
    painter.drawText(QRect(0, 0, label.width(), label.height()), (Qt::AlignCenter | Qt::AlignCenter), string);
    painter.end();

    // CREATE A LAUIMAGE VERSION OF OUR LABEL IMAGE
    LAUImage lLabel(label);

    // MAKE SURE LABEL IMAGE IS THE SAME FORMAT AND COLOR PROFILE AS THIS
    if (lLabel.depth() != depth()) {
        if (depth() == sizeof(unsigned char)) {
            lLabel = lLabel.convertToUChar();
        } else if (depth() == sizeof(unsigned short)) {
            lLabel = lLabel.convertToUShort();
        } else if (depth() == sizeof(float)) {
            lLabel = lLabel.convertToFloat();
        }
    }

    // MAKE SURE WE HAVE THE RIGHT ICC PROFILE
    if (lLabel.colors() != colors()) {
        lLabel = lLabel.convertToProfile(profile());
    }

    // NOW OVERWRITE THE OUTPUT IMAGE PIXELS WITH THE LABEL PIXELS
    unsigned int topRow = lLabel.height();
    for (unsigned int row = 0; row < lLabel.height() && (row + topRow) < height(); row++) {
        memcpy(image.scanLine(row + topRow), lLabel.constScanLine(row), qMin(image.step(), lLabel.step()));
    }

    // RETURN THE LABELED IMAGE
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::concat(LAUImage imageA, LAUImage imageB, Qt::Orientation orient, unsigned int gap)
{
    // MAKE SURE IMAGE B IS THE SAME FORMAT AND COLOR PROFILE AS IMAGE A
    if (imageA.depth() != imageB.depth()) {
        if (imageA.depth() == sizeof(unsigned char)) {
            imageB = imageB.convertToUChar();
        } else if (imageA.depth() == sizeof(unsigned short)) {
            imageB = imageB.convertToUShort();
        } else if (imageA.depth() == sizeof(float)) {
            imageB = imageB.convertToFloat();
        }
    }

    if (imageA.colors() != imageB.colors()) {
        imageB = imageB.convertToProfile(imageA.profile());
    }

    LAUImage image;
    if (orient == Qt::Vertical) {
        int cols = qMax(imageA.width(), imageB.width());
        int rows = imageA.height() + imageB.height() + gap;
        image = LAUImage(rows, cols, imageA.depth(), imageA.profile(), imageA.xRes(), imageA.yRes());

        for (unsigned int row = 0; row < imageA.height(); row++) {
            unsigned char *toBuffer = image.constScanLine(row);
            unsigned char *fmBuffer = imageA.constScanLine(row);
            memcpy(toBuffer, fmBuffer, imageA.step());
            if (imageA.step() < image.step()) {
                memset(toBuffer + imageA.step(), 255, image.step() - imageA.step());
            }
        }

        for (unsigned int row = 0; row < gap; row++) {
            unsigned char *toBuffer = image.constScanLine(row + imageA.height());
            memset(toBuffer, 255, image.step());
        }

        for (unsigned int row = 0; row < imageB.height(); row++) {
            unsigned char *toBuffer = image.constScanLine(row + imageA.height() + gap);
            unsigned char *fmBuffer = imageB.constScanLine(row);
            memcpy(toBuffer, fmBuffer, imageB.step());
            if (imageB.step() < image.step()) {
                memset(toBuffer + imageB.step(), 255, image.step() - imageB.step());
            }
        }
    } else if (orient == Qt::Horizontal) {
        int cols = imageA.width() + imageB.width() + gap;
        int rows = qMax(imageA.height(), imageB.height());
        image = LAUImage(rows, cols, imageA.depth(), imageA.profile(), imageA.xRes(), imageA.yRes());

        for (unsigned int row = 0; row < image.height(); row++) {
            unsigned char *toBuffer = image.constScanLine(row);
            if (row < imageA.height()) {
                unsigned char *fmBuffer = imageA.constScanLine(row);
                memcpy(toBuffer, fmBuffer, imageA.step());
            } else {
                memset(toBuffer, 255, imageA.step());
            }
            memset(toBuffer + imageA.step(), 255, gap * image.block());
            if (row < imageB.height()) {
                unsigned char *fmBuffer = imageB.constScanLine(row);
                memcpy(toBuffer + imageA.step() + gap * image.block(), fmBuffer, imageB.step());
            } else {
                memset(toBuffer + imageA.step() + gap * image.block(), 255, imageB.step());
            }
        }
    } else {
        return (image);
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::superimpose(LAUImage imageFG, LAUImage imageBG)
{
    // MAKE SURE IMAGE B IS THE SAME FORMAT AND COLOR PROFILE AS IMAGE A
    if (imageFG.depth() != imageBG.depth()) {
        if (imageBG.depth() == sizeof(unsigned char)) {
            imageFG = imageFG.convertToUChar();
        } else if (imageBG.depth() == sizeof(unsigned short)) {
            imageFG = imageFG.convertToUShort();
        } else if (imageBG.depth() == sizeof(float)) {
            imageFG = imageFG.convertToFloat();
        }
    }

    if (imageFG.colors() != imageBG.colors()) {
        imageFG = imageFG.convertToProfile(imageBG.profile());
    }

    // MAKE A COPY OF THE FOREGROUND IMAGE
    LAUImage image = imageBG;

    // GET THE TOP-LEFT PIXEL COORDINATES
    int topRow = ((int)image.height() - (int)imageFG.height()) / 2;
    int lefCol = ((int)image.width() - (int)imageFG.width()) / 2;

    // NOW ITERATE ROW BY ROW THROUGH THE FOREGROUND IMAGE
    for (int row = 0; row < (int)imageFG.height(); row++) {
        if ((row + topRow) > 0 && (row + topRow) < (int)image.height()) {
            if (lefCol >= 0) {
                memcpy(image.scanLine(row + topRow) + lefCol * image.block(), imageFG.constScanLine(row), imageFG.step());
            } else {
                memcpy(image.scanLine(row + topRow), imageFG.constScanLine(row) - lefCol * image.block(), image.step());
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToRGB()
{
    cmsCIEXYZ D65_XYZ = {0.95047, 1.0, 1.08883 };
    cmsCIExyY D65;
    cmsXYZ2xyY(&D65, &D65_XYZ);

    cmsToneCurve *linear = cmsBuildGamma(NULL, 1.0);
    cmsToneCurve *linrgb[3] = {linear,linear,linear};
    cmsCIExyYTRIPLE primaries = {
        {0.64, 0.33, 1.0},
        {0.30, 0.60, 1.0},
        {0.15, 0.06, 1.0}
    };

    cmsFloat64Number P[5] = { 2.4, 1. / 1.055, 0.055 / 1.055, 1. / 12.92, 0.04045 };
    cmsToneCurve *srgb = cmsBuildParametricToneCurve(NULL, 4, P);
    cmsToneCurve *srgbcurve[3] = {srgb,srgb,srgb};
    cmsHPROFILE hsRGB = cmsCreateRGBProfile(&D65, &primaries, srgbcurve);

    return(this->convertToProfile(hsRGB));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToProfile(cmsHPROFILE newProfile)
{
    // CREATE NEW SCAN IMAGE WITH THE INCOMING PROFILE
    LAUImage image(height(), width(), depth(), newProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    // CREATE A PARAMETERS FOR DEFINING TRANSFORM TO CONVERT BETWEEN PROFILES
    cmsHPROFILE inProfile = profile();
    if (inProfile) {
        cmsHPROFILE outProfile = newProfile;
        cmsUInt32Number  inFormat = COLORSPACE_SH(PT_ANY) | CHANNELS_SH(colors() % 16)       | BYTES_SH(depth() % 8);
        cmsUInt32Number outFormat = COLORSPACE_SH(PT_ANY) | CHANNELS_SH(image.colors() % 16) | BYTES_SH(image.depth() % 8);

        // SET THE COLOR SPACE BITS
        //if (photometricInterpretation() == PHOTOMETRIC_MINISBLACK) inFormat |= COLORSPACE_SH(PT_GRAY);
        //if (photometricInterpretation() == PHOTOMETRIC_MINISWHITE) inFormat |= COLORSPACE_SH(PT_GRAY);
        //if (photometricInterpretation() == PHOTOMETRIC_RGB)        inFormat |= COLORSPACE_SH(PT_RGB);
        //if (photometricInterpretation() == PHOTOMETRIC_SEPARATED)  inFormat |= COLORSPACE_SH(PT_CMYK);

        //if (image.photometricInterpretation() == PHOTOMETRIC_MINISBLACK) outFormat |= COLORSPACE_SH(PT_GRAY);
        //if (image.photometricInterpretation() == PHOTOMETRIC_MINISWHITE) outFormat |= COLORSPACE_SH(PT_GRAY);
        //if (image.photometricInterpretation() == PHOTOMETRIC_RGB)        outFormat |= COLORSPACE_SH(PT_RGB);
        //if (image.photometricInterpretation() == PHOTOMETRIC_SEPARATED)  outFormat |= COLORSPACE_SH(PT_CMYK);

        // SEE IF WE NEED TO SPECIFY FORMAT FLAVOR
        if (photometricInterpretation() == PHOTOMETRIC_MINISWHITE) {
            inFormat |= FLAVOR_SH(1);
        }
        if (image.photometricInterpretation() == PHOTOMETRIC_MINISWHITE) {
            outFormat |= FLAVOR_SH(1);
        }

        // CREATE TRANSFORM
        cmsUInt32Number intent = 0;
        if (LAUCMSDialog::cmsIntentString == QString("Perceptual")) {
            intent = INTENT_PERCEPTUAL;
        } else if (LAUCMSDialog::cmsIntentString == QString("Saturation")) {
            intent = INTENT_SATURATION;
        } else if (LAUCMSDialog::cmsIntentString == QString("Relative Colorimetric")) {
            intent = INTENT_RELATIVE_COLORIMETRIC;
        } else if (LAUCMSDialog::cmsIntentString == QString("Absolute Colorimetric")) {
            intent = INTENT_ABSOLUTE_COLORIMETRIC;
        }
        cmsHTRANSFORM transform = cmsCreateTransform(inProfile, inFormat, outProfile, outFormat, intent, 0);

        if (transform) {
            // ITERATE THROUGH EACH ROW CONVERTING COLORS FROM THIS TO THE OUTPUT IMAGE
            for (unsigned int row = 0; row < height(); row++) {
                cmsDoTransform(transform, this->constScanLine(row), image.scanLine(row), image.width());
            }
            // DELETE THE TRANSFORM AND ASSOCIATED PROFILES
            cmsDeleteTransform(transform);
        }
        cmsCloseProfile(inProfile);
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertResolution(float xResNew, float yResNew, Qt::TransformationMode transformMode)
{
    Q_UNUSED(transformMode);

    // CALCULATE NEW HEIGHT AND WIDTH
    unsigned int newWidth = (int)qRound((float)width() / xRes() * xResNew);
    unsigned int newHeight = (int)qRound((float)height() / yRes() * yResNew);

    // CREATE SCAN IMAGE THAT'S THE CORRECT SIZE
    LAUImage image(newHeight, newWidth, this->colors(), depth(), xResNew, yResNew);
    if (image.isValid()) {
        image.setProfile(profile());
        image.setParentName(parentName());
        image.setXmlData(xmlData());

        // PERFORM NEAREST NEIGHBOR INTERPOLATION TO FILL NEW IMAGE
        for (unsigned int row = 0; row < newHeight; row++) {
            unsigned int rowP = (float)row / (float)(newHeight - 1) * (height() - 1);
            unsigned char *toBuffer = image.scanLine(row);
            unsigned char *fromBuffer = constScanLine(rowP);
            for (unsigned int col = 0; col < newWidth; col++) {
                unsigned int colP = (float)col / (float)(newWidth - 1) * (width() - 1);
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    switch (depth()) {
                        case sizeof(unsigned char):
                            ((unsigned char *)toBuffer)[chn + col * colors()] = ((unsigned char *)fromBuffer)[chn + colP * colors()];
                            break;
                        case sizeof(unsigned short):
                            ((unsigned short *)toBuffer)[chn + col * colors()] = ((unsigned short *)fromBuffer)[chn + colP * colors()];
                            break;
                        case sizeof(float):
                            ((float *)toBuffer)[chn + col * colors()] = ((float *)fromBuffer)[chn + colP * colors()];
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToFloat()
{
    // CREATE NEW SCAN IMAGE THAT COPIES THIS ONE
    cmsHPROFILE iccProfile = profile();
    LAUImage image(height(), width(), sizeof(float), iccProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());
    cmsCloseProfile(iccProfile);

    // CONVERT FROM ONE IMAGE TO THE NEXT
    if (depth() == sizeof(unsigned char)) {
        for (unsigned int row = 0; row < height(); row++) {
            float *toBuffer = (float *)image.scanLine(row);
            unsigned char *fromBuffer = (unsigned char *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                toBuffer[e] = (float)fromBuffer[e] / 255.0;
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        for (unsigned int row = 0; row < height(); row++) {
            float *toBuffer = (float *)image.scanLine(row);
            unsigned short *fromBuffer = (unsigned short *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                toBuffer[e] = (float)fromBuffer[e] / 65535.0;
            }
        }
    } else if (depth() == sizeof(float)) {
        memcpy(image.scanLine(0), constScanLine(0), image.height()*image.step());
    }

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToFloatByTable(float *lookUpTable)
{
    // CREATE NEW SCAN IMAGE THAT COPIES THIS ONE
    cmsHPROFILE iccProfile = profile();
    LAUImage image(height(), width(), sizeof(float), iccProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());
    cmsCloseProfile(iccProfile);

    if (depth() == sizeof(unsigned char)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *fromBuffer = (unsigned char *)constScanLine(row);
            float *toBuffer = (float *)image.scanLine(row);
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    toBuffer[chn + col * colors()] = lookUpTable[(int)fromBuffer[chn + col * colors()]];
                }
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned short *fromBuffer = (unsigned short *)constScanLine(row);
            float *toBuffer = (float *)image.scanLine(row);
            for (unsigned int col = 0; col < width(); col++) {
                for (unsigned int chn = 0; chn < colors(); chn++) {
                    toBuffer[chn + col * colors()] = lookUpTable[(int)fromBuffer[chn + col * colors()]];
                }
            }
        }
    } else if (depth() == sizeof(float)) {
        memcpy(image.scanLine(0), constScanLine(0), image.height()*image.step());
    }

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToUChar()
{
    // CREATE NEW SCAN IMAGE THAT COPIES THIS ONE
    cmsHPROFILE iccProfile = profile();
    LAUImage image(height(), width(), sizeof(unsigned char), iccProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());
    cmsCloseProfile(iccProfile);

    // CONVERT FROM ONE IMAGE TO THE NEXT
    if (depth() == sizeof(unsigned char)) {
        memcpy(image.scanLine(0), constScanLine(0), image.height()*image.step());
    } else if (depth() == sizeof(unsigned short)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *toBuffer = (unsigned char *)image.scanLine(row);
            unsigned short *fromBuffer = (unsigned short *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                toBuffer[e] = (fromBuffer[e] >> 8);
            }
        }
    } else if (depth() == sizeof(float)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *toBuffer = (unsigned char *)image.scanLine(row);
            float *fromBuffer = (float *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                toBuffer[e] = (unsigned char)(fromBuffer[e] * 255.0);
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToUShort()
{
    // CREATE NEW SCAN IMAGE THAT COPIES THIS ONE
    cmsHPROFILE iccProfile = profile();
    LAUImage image(height(), width(), sizeof(unsigned short), iccProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());
    cmsCloseProfile(iccProfile);

    // CONVERT FROM ONE IMAGE TO THE NEXT
    if (depth() == sizeof(unsigned char)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned short *toBuffer = (unsigned short *)image.scanLine(row);
            unsigned char *fromBuffer = (unsigned char *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                toBuffer[e] = (fromBuffer[e] << 8);
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        memcpy(image.scanLine(0), constScanLine(0), image.height()*image.step());
    } else if (depth() == sizeof(float)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned short *toBuffer = (unsigned short *)image.scanLine(row);
            float *fromBuffer = (float *)constScanLine(row);
            unsigned int elements = colors() * width();
            for (unsigned int e = 0; e < elements; e++) {
                int pixel = fromBuffer[e] * 65535.0;
                if (pixel < 0) {
                    pixel = 0;
                } else if (pixel > 65535) {
                    pixel = 65535;
                }
                toBuffer[e] = (unsigned short)pixel; //(fromBuffer[e]*65535.0);
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::convertToBinary(QString method)
{
    // CREATE NEW SCAN IMAGE THAT COPIES THIS ONE
    cmsHPROFILE iccProfile = profile();
    LAUImage image(height(), width(), sizeof(unsigned char), iccProfile, xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());
    cmsCloseProfile(iccProfile);

    // CONVERT IMAGE TO UNSIGNED CHARACTERS TO HOLD BINARY DATA
    if (image.depth() != sizeof(unsigned char)) {
        image = image.convertToUChar();
    }

    // CONVERT FROM ONE IMAGE TO THE NEXT
    if (method == QString("white noise")) {
        unsigned int elements = colors() * width();
        if (depth() == sizeof(unsigned char)) {
            for (unsigned int row = 0; row < height(); row++) {
                unsigned char *toBuffer = (unsigned char *)image.scanLine(row);
                unsigned char *fromBuffer = (unsigned char *)constScanLine(row);
                for (unsigned int e = 0; e < elements; e++) {
                    unsigned char threshold = 0x000000ff & rand();
                    if (fromBuffer[e] > threshold) {
                        toBuffer[e] = 255;
                    } else {
                        toBuffer[e] = 0;
                    }
                }
            }
        } else if (depth() == sizeof(unsigned short)) {
            for (unsigned int row = 0; row < height(); row++) {
                unsigned char *toBuffer = (unsigned char *)image.scanLine(row);
                unsigned short *fromBuffer = (unsigned short *)constScanLine(row);
                for (unsigned int e = 0; e < elements; e++) {
                    unsigned char threshold = 0x0000ffff & rand();
                    if (fromBuffer[e] > threshold) {
                        toBuffer[e] = 255;
                    } else {
                        toBuffer[e] = 0;
                    }
                }
            }
        } else if (depth() == sizeof(float)) {
            for (unsigned int row = 0; row < height(); row++) {
                unsigned char *toBuffer = (unsigned char *)image.scanLine(row);
                float *fromBuffer = (float *)constScanLine(row);
                for (unsigned int e = 0; e < elements; e++) {
                    float threshold = (float)rand() / (float)RAND_MAX;
                    if (fromBuffer[e] > threshold) {
                        toBuffer[e] = 255;
                    } else {
                        toBuffer[e] = 0;
                    }
                }
            }
        }
    } else if (method == QString("error diffuion")) {
        ;
    } else if (method == QString("fm screen")) {
        ;
    } else if (method == QString("am screen")) {
        ;
    } else if (method == QString("green noise screen")) {
        ;
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::crop(unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
    LAUImage image(h, w, depth(), profile(), data->xResolution, data->yResolution);
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    if ((y + h) > height()) {
        h = height() - y;
    }
    if ((x + w) > width()) {
        w = width() - x;
    }

    unsigned char *fromBuffer = ((unsigned char *)constScanLine(0)) + y * step() + x * depth() * colors();
    if (depth() == sizeof(unsigned char)) {
        switch (colors()) {
            case 1:
                ippiCopy_8u_C1R((Ipp8u *)fromBuffer, step(), (Ipp8u *)image.scanLine(0), image.step(), image.size());
                break;
            case 2:
                for (unsigned int r = y; r < y + h; r++) {
                    unsigned char *toBuffer = image.scanLine(r - y);
                    memcpy(toBuffer, &fromBuffer[r * step()], w * depth() * 2);
                }
                break;
            case 3:
                ippiCopy_8u_C3R((Ipp8u *)fromBuffer, step(), (Ipp8u *)image.scanLine(0), image.step(), image.size());
                break;
            case 4:
                ippiCopy_8u_C4R((Ipp8u *)fromBuffer, step(), (Ipp8u *)image.scanLine(0), image.step(), image.size());
                break;
        }
    } else if (depth() == sizeof(unsigned short)) {
        switch (colors()) {
            case 1:
                ippiCopy_16u_C1R((Ipp16u *)fromBuffer, step(), (Ipp16u *)image.scanLine(0), image.step(), image.size());
                break;
            case 2:
                for (unsigned int r = y; r < y + h; r++) {
                    unsigned short *toBuffer = (Ipp16u *)image.scanLine(r - y);
                    unsigned short *fromBuffer = &(((Ipp16u *)this->scanLine(r))[x * 2]);
                    memcpy(toBuffer, &fromBuffer[r * step()], w * depth() * 2);
                }
                break;
            case 3:
                ippiCopy_16u_C3R((Ipp16u *)fromBuffer, step(), (Ipp16u *)image.scanLine(0), image.step(), image.size());
                break;
            case 4:
                ippiCopy_16u_C4R((Ipp16u *)fromBuffer, step(), (Ipp16u *)image.scanLine(0), image.step(), image.size());
                break;
        }
    } else if (depth() == sizeof(float)) {
        switch (colors()) {
            case 1:
                ippiCopy_32f_C1R((Ipp32f *)fromBuffer, step(), (Ipp32f *)image.scanLine(0), image.step(), image.size());
                break;
            case 2:
                for (unsigned int r = y; r < y + h; r++) {
                    float *toBuffer = (Ipp32f *)image.scanLine(r - y);
                    float *fromBuffer = &(((Ipp32f *)this->scanLine(r))[x * 2]);
                    memcpy(toBuffer, &fromBuffer[r * step()], w * depth() * 2);
                }
                break;
            case 3:
                ippiCopy_32f_C3R((Ipp32f *)fromBuffer, step(), (Ipp32f *)image.scanLine(0), image.step(), image.size());
                break;
            case 4:
                ippiCopy_32f_C4R((Ipp32f *)fromBuffer, step(), (Ipp32f *)image.scanLine(0), image.step(), image.size());
                break;
        }
    }

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::extractChannel(unsigned int channel)
{
    if (channel >= colors()) {
        return (LAUImage());
    }

    LAUImage image(height(), width(), 1, depth(), xRes(), yRes());
    image.setProfile(cmsCreateGrayProfile(cmsD50_xyY(), cmsBuildGamma(0, 1.0)));
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    if (depth() == sizeof(unsigned char)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned char *fromBuffer = constScanLine(row);
            unsigned char *toBuffer = image.scanLine(row);
            for (unsigned int col = 0; col < width(); col++) {
                ((unsigned char *)toBuffer)[col] = ((unsigned char *)fromBuffer)[col * colors() + channel];
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        for (unsigned int row = 0; row < height(); row++) {
            unsigned short *fromBuffer = (unsigned short *)constScanLine(row);
            unsigned short *toBuffer = (unsigned short *)image.scanLine(row);
            for (unsigned int col = 0; col < width(); col++) {
                toBuffer[col] = fromBuffer[col * colors() + channel];
            }
        }
    } else if (depth() == sizeof(float)) {
        for (unsigned int row = 0; row < height(); row++) {
            float *fromBuffer = (float *)constScanLine(row);
            float *toBuffer = (float *)image.scanLine(row);
            for (unsigned int col = 0; col < width(); col++) {
                toBuffer[col] = fromBuffer[col * colors() + channel];
            }
        }
    }
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::stretch(unsigned int rows, double cols)
{
    // CREATE NEW LAU IMAGE TO RECEIVE THE WARPED COPY
    LAUImage image(rows, (unsigned int)qCeil(cols), depth(), profile(), xRes(), yRes());
    image.setParentName(parentName());
    image.setXmlData(xmlData());

    // GENERATE DITHER PARAMETERS FOR EACH COLUMN
    double numPixelsToSubY = (double)rows - (double)height();
    double pixelStretchY = (double)rows / numPixelsToSubY;
    int pointListLengthY = floor(pixelStretchY) - 1;

    // HANDLE CASE WHERE WE ARE NEITHER ADDING OR SUBTRACTING PIXELS
    if (numPixelsToSubY == 0) {
        pointListLengthY = 1;
    }

    // GENERATE A LIST OF OFFSETS FOR DITHERING THE UPSAMPLING BOUNDARY ALONG EACH COLUMN
    QList<QPointF> pointListY;
    for (int n = 0; n < pointListLengthY; n++) {
        int inBits = n;
        int outBits = 0;
        for (int b = 0; b < 16; b++) {
            outBits = (outBits << 1) + (inBits & 0x01);
            inBits = inBits >> 1;
        }
        pointListY.append(QPointF(((double)n + 0.5) / (double)(pointListLengthY + 1), outBits));
    }

    if (pointListY.isEmpty()) {
        pixelStretchY = 0.0;
        pointListLengthY = 0;
        pointListY.append(QPointF(0.0, 0.0));
    } else {
        qSort(pointListY.begin(), pointListY.end(), pointYCoordinateLessThan);
    }

    //QProgressDialog progressDialog(QString("Stretching image..."), QString("Abort"), 0, rows, nullptr);
    int numColors = colors();
    int bytesPerSample = depth();
    int pixelStep = numColors * bytesPerSample;
    double rowError = 0.0;
    for (unsigned int row = 0; row < rows; row++) {
        unsigned char *buffer = image.scanLine(row);

        //if (progressDialog.wasCanceled()) {
        //    break;
        //}
        //progressDialog.setValue(row);
        //qApp->processEvents();

        unsigned int localCols = (unsigned int)qFloor(cols + rowError);
        rowError += cols - (double)localCols;

        // GENERATE DITHER PARAMETERS FOR EACH ROW
        double numPixelsToSubX = (double)localCols - (double)width();
        double pixelStretchX = (double)localCols / numPixelsToSubX;
        int pointListLengthX = floor(pixelStretchX) - 1;

        // HANDLE CASE WHERE WE ARE NEITHER ADDING OR SUBTRACTING PIXELS
        if (numPixelsToSubX == 0) {
            pointListLengthX = 1;
        }

        // GENERATE A LIST OF OFFSETS FOR DITHERING THE UPSAMPLING BOUNDARY ALONG EACH ROW
        QList<QPointF> pointListX;
        for (int n = 0; n < pointListLengthX; n++) {
            int inBits = n;
            int outBits = 0;
            for (int b = 0; b < 16; b++) {
                outBits = (outBits << 1) + (inBits & 0x01);
                inBits = inBits >> 1;
            }
            pointListX.append(QPointF(((double)n + 0.5) / (double)(pointListLengthX + 1), outBits));
        }

        if (pointListX.isEmpty()) {
            pixelStretchX = 0.0;
            pointListLengthX = 0;
            pointListX.append(QPointF(0.0, 0.0));
        } else {
            qSort(pointListX.begin(), pointListX.end(), pointYCoordinateLessThan);
        }

        // FILL IN BUFFER PRIOR TO UPSAMPLING
        QPointF offsetX = pointListX.at(row % pointListX.count());

        for (unsigned int col = 0; col < qMin(image.width(), localCols); col++) {
            QPointF offsetY = pointListY.at(col % pointListY.count());

            // UPSAMPLE THE ROW INDICES
            int rowP = row;
            if (pixelStretchY > 1.0) {
                double ratio = (double)row / pixelStretchY;
                double lambda = ratio - floor(ratio);

                int numPixelsToDelete = (int)floor(ratio);
                if (lambda > offsetY.x()) {
                    numPixelsToDelete++;
                }
                rowP -= numPixelsToDelete;
            }

            // UPSAMPLE THE COLUMN INDICES
            int colP = col;
            if (pixelStretchX > 1.0) {
                double ratio = (double)col / pixelStretchX;
                double lambda = ratio - floor(ratio);

                int numPixelsToDelete = (int)floor(ratio);
                if (lambda > offsetX.x()) {
                    numPixelsToDelete++;
                }
                colP -= numPixelsToDelete;
            }

            // MAP PIXELS FROM INPUT IMAGE TO OUTPUT IMAGE
            if (bytesPerSample == sizeof(char)) {
                memcpy(&((char *)buffer)[col * numColors], &((char *)constScanLine(rowP))[colP * numColors], pixelStep);
            } else if (bytesPerSample == sizeof(short)) {
                memcpy(&((short *)buffer)[col * numColors], &((short *)constScanLine(rowP))[colP * numColors], pixelStep);
            } else if (bytesPerSample == sizeof(float)) {
                memcpy(&((float *)buffer)[col * numColors], &((float *)constScanLine(rowP))[colP * numColors], pixelStep);
            }
        }
    }
    //progressDialog.setValue(rows);

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::rescale(unsigned int rows, unsigned int cols, Qt::AspectRatioMode aspectRatioMode, IppiInterpolationType interpolation)
{
    if (aspectRatioMode != Qt::IgnoreAspectRatio) {
        // CALCULATE NEW IMAGE SIZE IN ROWS AND COLUMNS
        float xLambda = (float)cols / (float)width();
        float yLambda = (float)rows / (float)height();

        if (aspectRatioMode == Qt::KeepAspectRatio) {
            if (xLambda > yLambda) {
                xLambda = yLambda;
            } else {
                yLambda = xLambda;
            }
        } else if (aspectRatioMode == Qt::KeepAspectRatioByExpanding) {
            if (xLambda < yLambda) {
                xLambda = yLambda;
            } else {
                yLambda = xLambda;
            }
        }
        rows = (int)qRound(rows * yLambda);
        cols = (int)qRound(cols * xLambda);
    }

    // MAKE A NEW IMAGE WITH THE USER SPECIFIED SIZE, BUT OTHERWISE
    // IDENTICAL TO THE CURRENT IMAGE
    LAUImage image(rows, cols, depth(), profile(), xRes(), yRes());
    image.setFilename(filename());
    image.setXmlData(xmlData());

    // DECLARE VARIABLES FOR INTEL'S RESIZING FUNCTIONS
    IppiResizeSpec_32f *pSpec = nullptr;
    int specSize = 0, initSize = 0, bufSize = 0;
    Ipp8u *pInitBuf = nullptr, *pBuffer = nullptr;

    // RESIZE THE CONTENTS OF THIS IMAGE INTO THE NEW IMAGE
    IppStatus status;
    IppiPoint offset = {0, 0};
    if (depth() == sizeof(unsigned char)) {
        if (ippiResizeGetSize_8u(size(), image.size(), interpolation, 0, &specSize, &initSize) == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_8u(size(), image.size(), pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_8u(size(), image.size(), pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_8u(size(), image.size(), 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_8u(size(), image.size(), 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_8u(size(), image.size(), pSpec);
                            break;
                        default:
                            status = ippStsNoAntialiasing;
                    }
                    if (status == ippStsNoErr) {
                        if (ippiResizeGetBufferSize_8u(pSpec, image.size(), colors(), &bufSize) == ippStsNoErr) {
                            pBuffer = (Ipp8u *)ippsMalloc_8u(bufSize);
                            if (pBuffer) {
                                if (colors() == 1) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_8u_C1R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_8u_C1R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_8u_C1R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_8u_C1R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_8u_C1R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 3) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_8u_C3R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_8u_C3R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_8u_C3R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_8u_C3R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_8u_C3R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 4) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_8u_C4R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_8u_C4R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_8u_C4R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_8u_C4R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_8u_C4R((const Ipp8u *)constScanLine(0), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                }
                                ippsFree(pBuffer);
                            }
                        }
                    }
                    ippsFree(pSpec);
                }
                ippsFree(pInitBuf);
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        if (ippiResizeGetSize_16u(size(), image.size(), interpolation, 0, &specSize, &initSize) == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_16u(size(), image.size(), pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_16u(size(), image.size(), pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_16u(size(), image.size(), 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_16u(size(), image.size(), 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_16u(size(), image.size(), pSpec);
                            break;
                        default:
                            status = ippStsNoAntialiasing;
                    }
                    if (status == ippStsNoErr) {
                        if (ippiResizeGetBufferSize_8u(pSpec, image.size(), colors(), &bufSize) == ippStsNoErr) {
                            pBuffer = (Ipp8u *)ippsMalloc_8u(bufSize);
                            if (pBuffer) {
                                if (colors() == 1) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_16u_C1R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_16u_C1R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_16u_C1R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_16u_C1R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_16u_C1R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 3) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_16u_C3R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_16u_C3R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_16u_C3R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_16u_C3R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_16u_C3R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 4) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_16u_C4R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_16u_C4R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_16u_C4R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_16u_C4R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_16u_C4R((const Ipp16u *)constScanLine(0), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                }
                                ippsFree(pBuffer);
                            }
                        }
                    }
                    ippsFree(pSpec);
                }
                ippsFree(pInitBuf);
            }
        }
    } else if (depth() == sizeof(float)) {
        if (ippiResizeGetSize_32f(size(), image.size(), interpolation, 0, &specSize, &initSize) == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_32f(size(), image.size(), pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_32f(size(), image.size(), pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_32f(size(), image.size(), 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_32f(size(), image.size(), 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_32f(size(), image.size(), pSpec);
                            break;
                        default:
                            status = ippStsNoAntialiasing;
                    }
                    if (status == ippStsNoErr) {
                        if (ippiResizeGetBufferSize_8u(pSpec, image.size(), colors(), &bufSize) == ippStsNoErr) {
                            pBuffer = (Ipp8u *)ippsMalloc_8u(bufSize);
                            if (pBuffer) {
                                if (colors() == 1) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_32f_C1R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_32f_C1R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_32f_C1R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_32f_C1R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_32f_C1R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 3) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_32f_C3R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_32f_C3R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_32f_C3R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_32f_C3R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_32f_C3R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                } else if (colors() == 4) {
                                    switch (interpolation) {
                                        case ippNearest:
                                            status = ippiResizeNearest_32f_C4R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        case ippLinear:
                                            status = ippiResizeLinear_32f_C4R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippCubic:
                                            status = ippiResizeCubic_32f_C4R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippLanczos:
                                            status = ippiResizeLanczos_32f_C4R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), ippBorderRepl, 0, pSpec, pBuffer);
                                            break;
                                        case ippSuper:
                                            status = ippiResizeSuper_32f_C4R((const Ipp32f *)constScanLine(0), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, image.size(), pSpec, pBuffer);
                                            break;
                                        default:
                                            status = ippStsNoAntialiasing;
                                    }
                                }
                                ippsFree(pBuffer);
                            }
                        }
                    }
                    ippsFree(pSpec);
                }
                ippsFree(pInitBuf);
            }
        }
    }
    // RETURN THE NEWLY CREATED IMAGE
    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool LAUImage::rescaleToDisk(QString fileString, unsigned int rows, unsigned int cols, IppiInterpolationType interpolation, QWidget *parent)
{
    bool flag = true;

    // STORE THE TARGET OUTPUT IMAGE SIZE AS AN IPP STRUCTURE
    IppiSize inputSize = size();
    inputSize.height -= 5;
    IppiSize outputSize = {(int)cols, (int)rows};

    // OPEN TIFF FILE FOR SAVING THE IMAGE
    TIFF *outputTiff = TIFFOpen(fileString.toLatin1(), "w");
    if (!outputTiff) {
        return (false);
    }

    // WRITE FORMAT PARAMETERS TO CURRENT TIFF DIRECTORY
    TIFFSetField(outputTiff, TIFFTAG_IMAGEWIDTH, (unsigned long)outputSize.width);
    TIFFSetField(outputTiff, TIFFTAG_IMAGELENGTH, (unsigned long)outputSize.height);
    TIFFSetField(outputTiff, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
    TIFFSetField(outputTiff, TIFFTAG_XRESOLUTION, xRes());
    TIFFSetField(outputTiff, TIFFTAG_YRESOLUTION, yRes());
    TIFFSetField(outputTiff, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(outputTiff, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
    TIFFSetField(outputTiff, TIFFTAG_SAMPLESPERPIXEL, (unsigned short)colors());
    TIFFSetField(outputTiff, TIFFTAG_BITSPERSAMPLE, (unsigned short)(depth() << 3));
    TIFFSetField(outputTiff, TIFFTAG_PHOTOMETRIC, (unsigned short)photometricInterpretation());
#ifndef _TTY_WIN_
    //    TIFFSetField(outputTiff, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
    //    TIFFSetField(outputTiff, TIFFTAG_PREDICTOR, 2);
    TIFFSetField(outputTiff, TIFFTAG_ROWSPERSTRIP, 1);
#endif

    // SEE IF WE HAVE TO TELL THE TIFF READER THAT WE ARE STORING
    // PIXELS IN 32-BIT FLOATING POINT FORMAT
    if (depth() == sizeof(float)) {
        TIFFSetField(outputTiff, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
    }

    // WRITE THE FILESTRING TO THE DOCUMENT NAME TIFFTAG
    TIFFSetField(outputTiff, TIFFTAG_DOCUMENTNAME, filename().toLatin1().data());
    if (!parentName().isEmpty()) {
        TIFFSetField(outputTiff, TIFFTAG_PAGENAME, parentName().toLatin1().data());
    }

    // WRITE XML DATA IF IT EXISTS
    QByteArray xmlByteArray = xmlData();
    if (xmlByteArray.length() > 0) {
        TIFFSetField(outputTiff, TIFFTAG_XMLPACKET, xmlByteArray.length(), xmlByteArray.data());
    }

    // WRITE ICC PROFILE IF IT EXISTS
    if (pProfile()) {
        TIFFSetField(outputTiff, TIFFTAG_ICCPROFILE, numProfileBytes(), pProfile());
    }

    // CALCULATE NUMBER OF ROWS TO COPY AT EACH ITERATION BY AT MOST 1 MILLION PIXELS AT EACH ITERATION
    unsigned int tiffRowCounter = 0;
    unsigned int numRowsToCopyPerIteration = 1000000 / outputSize.width;
    if (numRowsToCopyPerIteration > (unsigned int)outputSize.height) {
        numRowsToCopyPerIteration = outputSize.height;
    }

    // CREATE AN IMAGE TO RECIEVE THE RESCALED IMAGE ONE TILE AT A TIME
    LAUImage image(numRowsToCopyPerIteration, cols, depth(), profile(), xRes(), yRes());

    // DECLARE VARIABLES FOR INTEL'S RESIZING FUNCTIONS
    IppiResizeSpec_32f *pSpec = nullptr;
    int specSize = 0, initSize = 0, bufSize = 0;
    Ipp8u *pInitBuf = nullptr, *pBuffer = nullptr;

    // COPY THIS IMAGE INTO NEW IMAGE USING ROI PROCESSING
    // RESIZE THE CONTENTS OF THIS IMAGE INTO THE NEW IMAGE
    IppStatus status;
    if (depth() == sizeof(unsigned char)) {
        IppStatus sts;
        sts = ippiResizeGetSize_8u(inputSize, outputSize, interpolation, 0, &specSize, &initSize);
        if (sts == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_8u(inputSize, outputSize, pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_8u(inputSize, outputSize, pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_8u(inputSize, outputSize, 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_8u(inputSize, outputSize, 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_8u(inputSize, outputSize, pSpec);
                            break;
                        case ippHahn:
                            status = ippStsErr;
                            break;
                    }
                    if (status == ippStsNoErr) {
                        IppiBorderSize borderSize = {0, 0, 0, 0};
                        if (ippiResizeGetBorderSize_8u(pSpec, &borderSize) == ippStsNoErr) {
                            IppiSize dstTileSize = {(int)outputSize.width, (int)numRowsToCopyPerIteration};
                            if (ippiResizeGetBufferSize_8u(pSpec, dstTileSize, colors(), &bufSize) == ippStsNoErr) {
                                pBuffer = (Ipp8u *)ippsMalloc_8u(2 * bufSize);
                                if (pBuffer) {
                                    IppiPoint offset = {0, 0};

                                    QProgressDialog progressDialog(QString("Rescaling image %1...").arg(filename()), QString("Abort"), 0, outputSize.height, parent);
                                    progressDialog.setFixedWidth(progressDialog.width());
                                    progressDialog.setFixedHeight(progressDialog.height());
                                    for (unsigned int rowMin = 0; rowMin < (unsigned int)outputSize.height; rowMin += numRowsToCopyPerIteration) {
                                        QCoreApplication::processEvents();
                                        if (progressDialog.wasCanceled()) {
                                            flag = false;
                                            break;
                                        }
                                        progressDialog.setValue(rowMin);

                                        // CALCULATE RANGE OF OUTPUT ROWS TO BE COPIED THIS ITERATION
                                        bool dontFlag = false;
                                        int numRows = outputSize.height - rowMin;
                                        if (numRows > (int)numRowsToCopyPerIteration) {
                                            dontFlag = true;
                                            numRows = numRowsToCopyPerIteration;
                                        }

                                        IppiBorderType borderType = ippBorderRepl;
                                        if (rowMin > borderSize.borderTop) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemTop);
                                        }
                                        if ((rowMin + numRows) < (outputSize.height - borderSize.borderBottom)) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemBottom);
                                        }

                                        // UPDATE DESTINATION TILE SIZE TO REFLECT HOW MANY ROWS WE WILL COPY ON THIS ITERATION
                                        dstTileSize.height = numRows;

                                        IppiPoint srcOffset, dstOffset = {(int)0, (int)rowMin};
                                        IppiSize  srcSizeT = inputSize;
                                        if (ippiResizeGetSrcRoi_8u(pSpec, dstOffset, dstTileSize, &srcOffset, &srcSizeT) == ippStsNoErr) {
                                            if (colors() == 1) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_8u_C1R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_8u_C1R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_8u_C1R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_8u_C1R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_8u_C1R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 3) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_8u_C3R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_8u_C3R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_8u_C3R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_8u_C3R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_8u_C3R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 4) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_8u_C4R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_8u_C4R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_8u_C4R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_8u_C4R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_8u_C4R((const Ipp8u *)constScanLine(srcOffset.y), step(), (Ipp8u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            }
                                            // WRITE EACH ROW OF THE CURRENT IMAGE TO DISK WHEREVER WE LEFT OFF FROM PREVIOUS ITERATION
                                            for (unsigned int row = 0; row < (unsigned int)numRows && tiffRowCounter < (unsigned int)outputSize.height; row++) {
                                                TIFFWriteScanline(outputTiff, image.scanLine(row), tiffRowCounter++, 0);
                                            }
                                        }
                                    }
                                    progressDialog.setValue(outputSize.height);
                                }
                                ippsFree(pBuffer);
                            }
                        }
                        ippsFree(pSpec);
                    }
                    ippsFree(pInitBuf);
                }
            }
        }
    } else if (depth() == sizeof(unsigned short)) {
        if (ippiResizeGetSize_16u(inputSize, outputSize, interpolation, 0, &specSize, &initSize) == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_16u(inputSize, outputSize, pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_16u(inputSize, outputSize, pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_16u(inputSize, outputSize, 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_16u(inputSize, outputSize, 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_16u(inputSize, outputSize, pSpec);
                            break;
                        case ippHahn:
                            status = ippStsErr;
                            break;
                    }
                    if (status == ippStsNoErr) {
                        IppiBorderSize borderSize = {0, 0, 0, 0};
                        if (ippiResizeGetBorderSize_16u(pSpec, &borderSize) == ippStsNoErr) {
                            IppiSize dstTileSize = {(int)outputSize.width, (int)numRowsToCopyPerIteration};
                            if (ippiResizeGetBufferSize_16u(pSpec, dstTileSize, colors(), &bufSize) == ippStsNoErr) {
                                pBuffer = (Ipp8u *)ippsMalloc_8u(bufSize);
                                if (pBuffer) {
                                    IppiPoint offset = {0, 0};

                                    QProgressDialog progressDialog(QString("Rescaling image %1...").arg(filename()), QString("Abort"), 0, outputSize.height, parent);
                                    progressDialog.setModal(true);
                                    progressDialog.setFixedWidth(progressDialog.width());
                                    progressDialog.setFixedHeight(progressDialog.height());
                                    progressDialog.setModal(true);
                                    for (unsigned int rowMin = 0; rowMin < (unsigned int)outputSize.height; rowMin += numRowsToCopyPerIteration) {
                                        if (progressDialog.wasCanceled()) {
                                            flag = false;
                                            break;
                                        }
                                        progressDialog.setValue(rowMin);
                                        qApp->processEvents();

                                        // CALCULATE RANGE OF OUTPUT ROWS TO BE COPIED THIS ITERATION
                                        int numRows = outputSize.height - rowMin;
                                        if (numRows > (int)numRowsToCopyPerIteration) {
                                            numRows = numRowsToCopyPerIteration;
                                        }

                                        IppiBorderType borderType = ippBorderRepl;
                                        if (rowMin > borderSize.borderTop) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemTop);
                                        }
                                        if ((rowMin + numRows) < (outputSize.height - borderSize.borderBottom)) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemBottom);
                                        }

                                        IppiPoint srcOffset, dstOffset = {(int)0, (int)rowMin};
                                        IppiSize  srcSizeT = inputSize;
                                        if (ippiResizeGetSrcRoi_16u(pSpec, dstOffset, dstTileSize, &srcOffset, &srcSizeT) == ippStsNoErr) {
                                            if (colors() == 1) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_16u_C1R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_16u_C1R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_16u_C1R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_16u_C1R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_16u_C1R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 3) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_16u_C3R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_16u_C3R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_16u_C3R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_16u_C3R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_16u_C3R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 4) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_16u_C4R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_16u_C4R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_16u_C4R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_16u_C4R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_16u_C4R((const Ipp16u *)constScanLine(srcOffset.y), step(), (Ipp16u *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            }
                                            // WRITE EACH ROW OF THE CURRENT IMAGE TO DISK WHEREVER WE LEFT OFF FROM PREVIOUS ITERATION
                                            for (int row = 0; row < numRows && (int)tiffRowCounter < (int)outputSize.height; row++) {
                                                TIFFWriteScanline(outputTiff, image.scanLine(row), tiffRowCounter++, 0);
                                            }
                                        }
                                    }
                                    progressDialog.setValue(outputSize.height);
                                }
                                ippsFree(pBuffer);
                            }
                        }
                        ippsFree(pSpec);
                    }
                    ippsFree(pInitBuf);
                }
            }
        }
    } else if (depth() == sizeof(float)) {
        if (ippiResizeGetSize_32f(inputSize, outputSize, interpolation, 0, &specSize, &initSize) == ippStsNoErr) {
            pInitBuf = (Ipp8u *)ippsMalloc_8u(initSize);
            if (pInitBuf) {
                pSpec = (IppiResizeSpec_32f *)ippsMalloc_8u(specSize);
                if (pSpec) {
                    switch (interpolation) {
                        case ippNearest:
                            status = ippiResizeNearestInit_32f(inputSize, outputSize, pSpec);
                            break;
                        case ippLinear:
                            status = ippiResizeLinearInit_32f(inputSize, outputSize, pSpec);
                            break;
                        case ippCubic:
                            status = ippiResizeCubicInit_32f(inputSize, outputSize, 1.0, 0.0, pSpec, pInitBuf);
                            break;
                        case ippLanczos:
                            status = ippiResizeLanczosInit_32f(inputSize, outputSize, 3, pSpec, pInitBuf);
                            break;
                        case ippSuper:
                            status = ippiResizeSuperInit_32f(inputSize, outputSize, pSpec);
                            break;
                        case ippHahn:
                            status = ippStsErr;
                            break;
                    }
                    if (status == ippStsNoErr) {
                        IppiBorderSize borderSize = {0, 0, 0, 0};
                        if (ippiResizeGetBorderSize_32f(pSpec, &borderSize) == ippStsNoErr) {
                            IppiSize dstTileSize = {(int)outputSize.width, (int)numRowsToCopyPerIteration};
                            if (ippiResizeGetBufferSize_32f(pSpec, dstTileSize, colors(), &bufSize) == ippStsNoErr) {
                                pBuffer = (Ipp8u *)ippsMalloc_8u(bufSize);
                                if (pBuffer) {
                                    IppiPoint offset = {0, 0};

                                    QProgressDialog progressDialog(QString("Rescaling image %1...").arg(filename()), QString("Abort"), 0, outputSize.height, parent);
                                    progressDialog.setModal(true);
                                    progressDialog.setFixedWidth(progressDialog.width());
                                    progressDialog.setFixedHeight(progressDialog.height());
                                    progressDialog.setModal(true);
                                    for (unsigned int rowMin = 0; rowMin < (unsigned int)outputSize.height; rowMin += numRowsToCopyPerIteration) {
                                        if (progressDialog.wasCanceled()) {
                                            flag = false;
                                            break;
                                        }
                                        progressDialog.setValue(rowMin);
                                        qApp->processEvents();

                                        // CALCULATE RANGE OF OUTPUT ROWS TO BE COPIED THIS ITERATION
                                        int numRows = outputSize.height - rowMin;
                                        if (numRows > (int)numRowsToCopyPerIteration) {
                                            numRows = numRowsToCopyPerIteration;
                                        }

                                        IppiBorderType borderType = ippBorderRepl;
                                        if (rowMin > borderSize.borderTop) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemTop);
                                        }
                                        if ((rowMin + numRows) < (outputSize.height - borderSize.borderBottom)) {
                                            borderType = (IppiBorderType)((int)borderType | (int)ippBorderInMemBottom);
                                        }

                                        IppiPoint srcOffset, dstOffset = {(int)0, (int)rowMin};
                                        IppiSize  srcSizeT = inputSize;
                                        if (ippiResizeGetSrcRoi_32f(pSpec, dstOffset, dstTileSize, &srcOffset, &srcSizeT) == ippStsNoErr) {
                                            if (colors() == 1) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_32f_C1R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_32f_C1R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_32f_C1R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_32f_C1R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_32f_C1R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 3) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_32f_C3R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_32f_C3R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_32f_C3R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_32f_C3R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_32f_C3R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            } else if (colors() == 4) {
                                                switch (interpolation) {
                                                    case ippNearest:
                                                        status = ippiResizeNearest_32f_C4R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippLinear:
                                                        status = ippiResizeLinear_32f_C4R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippCubic:
                                                        status = ippiResizeCubic_32f_C4R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippLanczos:
                                                        status = ippiResizeLanczos_32f_C4R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, borderType, 0, pSpec, pBuffer);
                                                        break;
                                                    case ippSuper:
                                                        status = ippiResizeSuper_32f_C4R((const Ipp32f *)constScanLine(srcOffset.y), step(), (Ipp32f *)image.scanLine(0), image.step(), offset, dstTileSize, pSpec, pBuffer);
                                                        break;
                                                    case ippHahn:
                                                        status = ippStsErr;
                                                        break;
                                                }
                                            }
                                            // WRITE EACH ROW OF THE CURRENT IMAGE TO DISK WHEREVER WE LEFT OFF FROM PREVIOUS ITERATION
                                            for (int row = 0; row < numRows && (int)tiffRowCounter < (int)outputSize.height; row++) {
                                                TIFFWriteScanline(outputTiff, image.scanLine(row), tiffRowCounter++, 0);
                                            }
                                        }
                                    }
                                    progressDialog.setValue(outputSize.height);
                                }
                                ippsFree(pBuffer);
                            }
                        }
                        ippsFree(pSpec);
                    }
                    ippsFree(pInitBuf);
                }
            }
        }
    }
    // CLOSE TIFF FILE
    TIFFRewriteDirectory(outputTiff);
    TIFFClose(outputTiff);
    return (flag);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QImage LAUImage::preview(QSize size, Qt::AspectRatioMode aspectRatioMode, Qt::TransformationMode transformMode)
{
    Q_UNUSED(transformMode);

    // CALCULATE NEW IMAGE SIZE IN ROWS AND COLUMNS
    float xLambda = (float)size.width() / (float)width();
    float yLambda = (float)size.height() / (float)height();

    if (aspectRatioMode == Qt::KeepAspectRatio) {
        if (xLambda > yLambda) {
            xLambda = yLambda;
        } else {
            yLambda = xLambda;
        }
    } else if (aspectRatioMode == Qt::KeepAspectRatioByExpanding) {
        if (xLambda < yLambda) {
            xLambda = yLambda;
        } else {
            yLambda = xLambda;
        }
    }
    size.setHeight((int)qRound(height()*yLambda));
    size.setWidth((int)qRound(width()*xLambda));

    // RESCALE THIS IMAGE USING NEW SIZE
    LAUImage scanImage = this->rescale(size.height(), size.width());

    // CONVERT IMAGE TO UNSIGNED CHAR, IF IT ISN'T ALREADY
    if (scanImage.depth() != sizeof(unsigned char)) {
        scanImage = scanImage.convertToUChar();
    }

    // CREATE TRANSFORM FROM THIS PROFILE TO RGB, IF IT ISN'T ALREADY IN AN RGB PROFILE
    cmsHPROFILE iccProfile = cmsCreate_sRGBProfile();
    scanImage = scanImage.convertToProfile(iccProfile);
    cmsCloseProfile(iccProfile);

    // TRANSFORM ROW BY ROW FROM THIS IMAGE BUFFER DIRECTLY TO THE SCAN LINES OF THE QIMAGE OBJECT
    QImage image(scanImage.width(), scanImage.height(), QImage::Format_RGB888);

    for (unsigned int row = 0; row < scanImage.height(); row++) {
        unsigned char *toBuffer = image.scanLine(row);
        unsigned char *fromBuffer = scanImage.scanLine(row);
        memcpy(toBuffer, fromBuffer, scanImage.width()*scanImage.colors());
    }

    return (image);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
LAUImage LAUImage::interlaceMask(double angle, double xPitch, double yPitch, int xChannels, int yChannels)
{
    LAUImage qt_bayer(256, 1, 1, sizeof(float));
    for (unsigned short n = 0; n < 256; n++) {
        unsigned char pixel = ((n & 0x01) << 7) | ((n & 0x02) << 5) | ((n & 0x04) << 3) | ((n & 0x08) << 1) | ((n & 0x10) >> 1) | ((n & 0x20) >> 3) | ((n & 0x40) >> 5) | ((n & 0x80) >> 7);
        *((float *)qt_bayer.constScanLine(n)) = (float)pixel / 255.0f;
    }

    //#define USELINEARINTERPOLATION
#ifdef USELINEARINTERPOLATION
    LAUImage buffer(1, width(), 1, sizeof(float), xRes(), yRes());
    memset(buffer.constScanLine(0), 0, buffer.step());

    for (unsigned int row = 0; row < 256; row++) {
        for (unsigned int col = 0; col < width(); col++) {
#ifdef USEVARIABLEPITCH
            double position = double(col) / double(xRes());

            double xPitchLocal = 62.100072755417941;
            xPitchLocal += 3.747420020638982e-06 * pow(position, 6.0);
            xPitchLocal += -1.565685282209731e-04 * pow(position, 5.0);
            xPitchLocal += 0.002351663590537 * pow(position, 4.0);
            xPitchLocal += -0.013997623667610 * pow(position, 3.0);
            xPitchLocal += 0.011190590201846 * pow(position, 2.0);
            xPitchLocal += 0.159276445329705 * pow(position, 1.0);

            double dxdx = xPitchLocal / xRes();
            double colP = (double)col + (double)((float *)qt_bayer.constScanLine(0))[row % 256];
            double x = (double)colP * dxdx;

            int xChn = (int)floor(x * (double) xChannels);
#else
            double dxdx = xPitch / xRes();
            double colP = (double)col + (double)((float *)qt_bayer.constScanLine(0))[row % 256];
            double x = (double)colP * dxdx;

            int xChn = (int)floor(x * (double) xChannels);
#endif
            ((float *)buffer.constScanLine(0))[col] += (float)xChn;
        }
    }

    for (unsigned int col = 0; col < width(); col++) {
        ((float *)buffer.constScanLine(0))[col] /= 256.0f;
    }

    LAUImage image(height(), width(), 1, sizeof(unsigned short), xRes(), yRes());
    for (unsigned int row = 0; row < height(); row++) {
        for (unsigned int col = 0; col < width(); col++) {
            float lambda = ((float *)buffer.constScanLine(0))[col];
            unsigned short index = (unsigned short)(qFloor(lambda) % xChannels) << 6;

            // GET THE FRACTIONAL PART OF LAMBDA
            lambda = lambda - (float)qFloor(lambda);

            // JAM THE FRACTIONAL PART INTO THE LEAST SIGNIFICANT 6 BITS OF THE INDEX
            index = index + (unsigned short)qFloor(64.0f * lambda);

            // SAVE THE INDED INTO OUR INTERLACE MASK
            ((unsigned short *)image.constScanLine(row))[col] = index;
        }
    }
#else
    double dxdx = +(xPitch / xRes()) * cos(angle * 0.017453292519943);
    double dydx = +(xPitch / xRes()) * sin(angle * 0.017453292519943);
    double dxdy = -(yPitch / yRes()) * sin(angle * 0.017453292519943);
    double dydy = +(yPitch / yRes()) * cos(angle * 0.017453292519943);

    // NUMBER OF LENS ACROSS THE PAGE
    int numLensAcrossPage = qRound((double)width() / xRes() * xPitch);

    // CALCULATE THE CENTER PIXEL OF THE CURRENT ROW AS THE
    // REFERENCE PIXEL THIS MUST BE IN THE CENTER OF A LENS
    int row = 0;
    int col = (width() + 1) / 2;
    double rowP = (double)row + (double)((float *)qt_bayer.constScanLine(0))[col % 256];
    double colP = (double)col + (double)((float *)qt_bayer.constScanLine(0))[row % 256];

    QPointF ptC = QPointF((double)colP * dxdx + (double)rowP * dxdy, (double)colP * dydx + (double)rowP * dydy);

    if ((numLensAcrossPage % 2) == 1) {
        ptC.setX(0.5 - ptC.x());
        ptC.setY(0.5 - ptC.y());
    } else {
        ptC.setX(0.0 - ptC.x());
        ptC.setY(0.0 - ptC.y());
    }

    LAUImage image(height(), width(), 1, sizeof(unsigned short), xRes(), yRes());
    for (int row = 0; row < (int)height(); row++) {
#ifdef USEVARIABLEPITCH
        double position = double(col) / double(xRes());

        double xPitchLocal = 62.100072755417941;
        xPitchLocal += 3.747420020638982e-06 * pow(position, 6.0);
        xPitchLocal += -1.565685282209731e-04 * pow(position, 5.0);
        xPitchLocal += 0.002351663590537 * pow(position, 4.0);
        xPitchLocal += -0.013997623667610 * pow(position, 3.0);
        xPitchLocal += 0.011190590201846 * pow(position, 2.0);
        xPitchLocal += 0.159276445329705 * pow(position, 1.0);

        double dxdx = +(xPitchLocal / xRes()) * cos(angle * 0.017453292519943);
        double dydx = +(xPitchLocal / xRes()) * sin(angle * 0.017453292519943);
#endif
        for (int col = 0; col < (int)width(); col++) {
            double rowP = (double)row + (double)((float *)qt_bayer.constScanLine(0))[col % 256];
            double colP = (double)col + (double)((float *)qt_bayer.constScanLine(0))[row % 256];

            // ADD THE OFFSET POINT SO THAT THE CENTER PIXEL IS THE CENTER OF A LENS
            QPointF ptA = QPointF((double)colP * dxdx + (double)rowP * dxdy, (double)colP * dydx + (double)rowP * dydy);
                    ptA = ptC + ptA;

            double xCoord = ptA.x() * (double) xChannels;
            double yCoord = ptA.y() * (double) yChannels;

            int xChn = (int)floor(xCoord + 1000000 * xChannels) % xChannels;
            int yChn = (int)floor(yCoord + 1000000 * yChannels) % yChannels;

            ((unsigned short *)image.constScanLine(row))[col] = xChn * 64;

            //((float *)image.constScanLine(row))[3 * col + 0] = xChn;
            //((float *)image.constScanLine(row))[3 * col + 1] = yChn;
            //((float *)image.constScanLine(row))[3 * col + 2] = xChannels * yChn + xChn;

            //QPointF ptB = ptA + QPointF(dxdx, dydx);
            //QPointF ptC = ptA + QPointF(dxdy, dydy);
            //QPointF ptD = ptA + QPointF(dxdx + dxdy, dydx + dydy);

            //int lbA = (int)floor(ptA.x() * (double) channels);
            //int lbB = (int)floor(ptB.x() * (double) channels);
            //int lbC = (int)floor(ptC.x() * (double) channels);
            //int lbD = (int)floor(ptD.x() * (double) channels);

            //double lensX = (double)col * dxdx + (double)row * dxdy;
            //double lensY = (double)col * dydx + (double)row * dydy;

            //double chn = (double)channels * (lensX - floor(lensX));

            //((float *)image.constScanLine(row))[3 * col + 0] = (int)floor(ptA.x() * (double) channels);
            //((float *)image.constScanLine(row))[3 * col + 1] = (int)floor(ptA.y() * (double) channels);
        }
    }

    for (int row = 0; row < (int)height(); row++) {
#ifdef USEVARIABLEPITCH
        double position = double(col) / double(xRes());

        double xPitchLocal = 62.100072755417941;
        xPitchLocal += 3.747420020638982e-06 * pow(position, 6.0);
        xPitchLocal += -1.565685282209731e-04 * pow(position, 5.0);
        xPitchLocal += 0.002351663590537 * pow(position, 4.0);
        xPitchLocal += -0.013997623667610 * pow(position, 3.0);
        xPitchLocal += 0.011190590201846 * pow(position, 2.0);
        xPitchLocal += 0.159276445329705 * pow(position, 1.0);

        double dxdx = +(xPitchLocal / xRes()) * cos(angle * 0.017453292519943);
        double dydx = +(xPitchLocal / xRes()) * sin(angle * 0.017453292519943);
#endif
        for (int col = 0; col < (int)width(); col++) {
            double rowPa = (double)row + (double)((float *)qt_bayer.constScanLine(0))[col % 256];
            double colPa = (double)col + (double)((float *)qt_bayer.constScanLine(0))[row % 256];

            QPointF ptA = QPointF((double)colPa * dxdx + (double)rowPa * dxdy, (double)colPa * dydx + (double)rowPa * dydy);
                    ptA = ptC + ptA;

            ptA.setX(ptA.x() - qFloor(ptA.x()));
            ptA.setY(ptA.y() - qFloor(ptA.y()));

            double rowPb = (double)(row + 1) + (double)((float *)qt_bayer.constScanLine(0))[col % 256];
            double colPb = (double)(col + 1) + (double)((float *)qt_bayer.constScanLine(0))[row % 256];

            QPointF ptB = QPointF((double)colPb * dxdx + (double)rowPb * dxdy, (double)colPb * dydx + (double)rowPb * dydy);
                    ptB = ptC + ptB;

            ptB.setX(ptB.x() - qFloor(ptB.x()));
            ptB.setY(ptB.y() - qFloor(ptB.y()));

            if (ptA.x() <= 0.5 && ptB.x() > 0.5) {
                ((unsigned short *)image.constScanLine(row))[qMax(0, col - 1)] += 0x8000;
                ((unsigned short *)image.constScanLine(row))[qMax(0, qMin(col + 0, (int)width() - 1))] += 0x8000;
                ((unsigned short *)image.constScanLine(row))[qMin(col + 1, (int)width() - 1)] += 0x8000;
            }
        }
    }
#endif
    return (image);
}
