#ifndef LAUIMAGE_H
#define LAUIMAGE_H

#include <QDir>
#include <QFile>
#include <QtCore>
#include <QImage>
#include <QDebug>
#include <QLabel>
#include <QScreen>
#include <QDialog>
#include <QObject>
#include <QPainter>
#include <QSettings>
#include <QPushButton>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QApplication>
#include <QProgressDialog>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QByteArray>
#include <QStringList>
#include <QFileDialog>
#include <QInputDialog>
#include <QCheckBox>

namespace libtiff
{
#include "tiffio.h"
}

#include "ipp.h"
#include "lcms2.h"
#include "xmmintrin.h"
#include "smmintrin.h"

#include "laucmswidget.h"

typedef struct {
    QString string;
    int value;
} ProfileString;

class LAUImageInspectorTextLabel;
class LAUImageInspectorLabel;
class LAUImageInspector;

void myTIFFWarningHandler(const char *stringA, const char *stringB, va_list);
void myTIFFErrorHandler(const char *stringA, const char *stringB, va_list);

class LAUImageData : public QSharedData
{
public:
    LAUImageData();
    LAUImageData(const LAUImageData &other);
    ~LAUImageData();

    QString fileString;
    unsigned int numRows, numCols, numChns, numByts, stepBytes;
    unsigned short photometric;
    float xResolution, yResolution;

    unsigned char *pProfile;
    cmsUInt32Number numProfileBytes;

    QByteArray xmlByteArray;
    QByteArray parentName;
    void *buffer;

    static int instanceCounter;

    void allocateBuffer();
    bool setProfile(cmsHPROFILE iccProfile);
};

class LAUImage : public QObject
{
    Q_OBJECT

public:
    explicit LAUImage(unsigned int rows, unsigned int cols, unsigned int dpth, cmsHPROFILE iccProfile, float xRes = 72.0, float yRes = 72.0);
    explicit LAUImage(unsigned int rows = 0, unsigned int cols = 0, unsigned int chns = 0, unsigned int dpth = 0, float xRes = 72.0, float yRes = 72.0);
    explicit LAUImage(const QString filename);
    ~LAUImage();

    LAUImage(libtiff::TIFF *currentTiffDirectory);
    LAUImage(const QImage image, float xRes = 72.0, float yRes = 72.0);
    LAUImage(const LAUImage &other) : data(other.data) { ; }

    LAUImage &operator = (const LAUImage &other)
    {
        if (this != &other) {
            data = other.data;
        }
        return (*this);
    }

    LAUImage interlaceMask(double angle = 0.0, double xPitch = 1.0, double yPitch = 1.0, int xChannels = 1, int yChannels = 1);

    void setXmlData(const QByteArray byteArray);
    void replace(const LAUImage &other);

    bool save(QString fileName = QString());
    bool save(libtiff::TIFF *currentTiffDirectory);
    bool load(libtiff::TIFF *inTiff);

    bool loadInto(QString filename, cmsHPROFILE inProfile);
    bool loadInto(libtiff::TIFF *inTiff, cmsHPROFILE inProfile);

    inline bool isValid() const
    {
        return (!isNull());
    }

    inline bool isNull() const
    {
        return (data->buffer == nullptr);
    }

    inline void setFilename(const QString string)
    {
        data->fileString = string;
    }

    inline QString filename() const
    {
        return (data->fileString);
    }

    inline unsigned int width() const
    {
        return (data->numCols);
    }

    inline unsigned int height() const
    {
        return (data->numRows);
    }

    inline unsigned int depth() const
    {
        return (data->numByts);
    }

    inline unsigned int colors() const
    {
        return (data->numChns);
    }

    inline unsigned int step() const
    {
        return (data->stepBytes);
    }

    inline unsigned int block() const
    {
        return (data->numByts * data->numChns);
    }

    inline void setResolution(float xRes, float yRes)
    {
        data->xResolution = xRes;
        data->yResolution = yRes;
    }

    inline void setPhotometricInterpretation(const unsigned short metric)
    {
        data->photometric = metric;
    }

    inline unsigned short photometricInterpretation() const
    {
        return (data->photometric);
    }

    inline IppiSize size() const
    {
        IppiSize sze = {(int)width(), (int)height()};
        return (sze);
    }

    inline QByteArray xmlData() const
    {
        return (data->xmlByteArray);
    }

    inline float xRes() const
    {
        return (data->xResolution);
    }

    inline float yRes() const
    {
        return (data->yResolution);
    }

    inline float pageWidth() const
    {
        return ((float)data->numCols / data->xResolution);
    }

    inline float pageHeight() const
    {
        return ((float)data->numRows / data->yResolution);
    }

    cmsHPROFILE profile();
    inline bool setProfile(cmsHPROFILE iccProfile)
    {
        return (data->setProfile(iccProfile));
    }

    inline unsigned char *scanLine(unsigned int row)
    {
        return (&(((unsigned char *)(data->buffer))[row * step()]));
    }

    inline unsigned char *constScanLine(unsigned int row) const
    {
        return (&(((unsigned char *)(data->buffer))[row * step()]));
    }

    inline QString parentName() const
    {
        return (QString(data->parentName));
    }

    inline void setParentName(QString string)
    {
        data->parentName = string.toLatin1();
    }

    LAUImage flip(QWidget *parent = nullptr, bool *wasCanceled = nullptr);
    LAUImage rotate90(QWidget *parent = nullptr, bool *wasCanceled = nullptr);
    LAUImage unrotate90(QWidget *parent = nullptr, bool *wasCanceled = nullptr);
    LAUImage flipAndRotate(QWidget *parent = nullptr, bool *wasCanceled = nullptr);
    LAUImage addTitle(QString string, float hght);

    LAUImage crop(unsigned int x, unsigned int y, unsigned int w, unsigned int h);
    LAUImage convertResolution(float xRes, float yRes, Qt::TransformationMode transformMode = Qt::FastTransformation);
    LAUImage convertToProfile(cmsHPROFILE newProfile);
    LAUImage convertToFloatByTable(float *lookUpTable);
    LAUImage convertToUChar();
    LAUImage convertToUShort();
    LAUImage convertToFloat();
    LAUImage convertToBinary(QString method = QString("white noise"));
    LAUImage invert();
    LAUImage extractChannel(unsigned int channel);
    LAUImage rescale(unsigned int rows, unsigned int cols, Qt::AspectRatioMode aspectRatioMode = Qt::IgnoreAspectRatio, IppiInterpolationType interpolation = ippNearest);
    LAUImage stretch(unsigned int rows, double cols);
    bool rescaleToDisk(QString fileString, unsigned int rows, unsigned int cols, IppiInterpolationType interpolation = ippNearest, QWidget *parent = nullptr);

    QImage preview(QSize size = QSize(300, 400), Qt::AspectRatioMode aspectRatioMode = Qt::KeepAspectRatio, Qt::TransformationMode transformMode = Qt::FastTransformation);

    static LAUImage concat(LAUImage imageA, LAUImage imageB, Qt::Orientation orient, unsigned int gap);
    static LAUImage superimpose(LAUImage imageFG, LAUImage imageBG);

private:
    QImage previewImage;
    QSharedDataPointer<LAUImageData> data;

    inline unsigned char *pProfile() const
    {
        return (data->pProfile);
    }
    inline cmsUInt32Number numProfileBytes() const
    {
        return (data->numProfileBytes);
    }
    inline static bool pointYCoordinateLessThan(const QPointF &A, const QPointF &B)
    {
        return (A.y() < B.y());
    }
    inline static double floor(double x)
    {
        return (qRound(x - 0.5));
    }
};

#endif // LAUIMAGE_H
