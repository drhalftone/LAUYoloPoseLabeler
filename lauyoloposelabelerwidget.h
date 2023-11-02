#ifndef LAUYOLOPOSELABELERWIDGET_H
#define LAUYOLOPOSELABELERWIDGET_H

#include <QLabel>
#include <QImage>
#include <QWidget>
#include <QKeyEvent>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QSpinBox>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPainter>

#include "lauimage.h"

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
class LAUFiducialLabel : public QLabel
{
    Q_OBJECT

public:
    LAUFiducialLabel(QLabel *parent = nullptr) : QLabel(parent) { ; }

    void setPixmap(QPixmap map) { pixmap = map; update(); }

protected:
    void paintEvent(QPaintEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);

signals:
    void emitPaint(QPainter *painter, QSize sze);
    void emitMousePressEvent(int x, int y);
    void emitMouseRightClickEvent(QMouseEvent *event);

private:
    QPixmap pixmap;
};

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
class LAUFiducialWidget : public QWidget
{
    Q_OBJECT

public:
    LAUFiducialWidget(QWidget *parent = nullptr);

    int index = -1;
    QLineEdit *lineEdit = nullptr;
    QSpinBox *xSpinBox = nullptr;
    QSpinBox *ySpinBox = nullptr;
    QRadioButton *zRadioButton = nullptr;
};

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
class LAUYoloPoseLabelerPalette : public QWidget
{
    Q_OBJECT

public:
    LAUYoloPoseLabelerPalette(QByteArray xmlByteArray, QWidget *parent = nullptr);
    LAUYoloPoseLabelerPalette(QStringList labels = QStringList(), QStringList fiducials = QStringList(), QWidget *parent = nullptr);
    ~LAUYoloPoseLabelerPalette();

    QByteArray xml() const;
    void setXml(QByteArray string);
    void setImageSize(int x, int y)
    {
        imageWidth = x;
        imageHeight = y;
    }

    void setDirty(bool state) { dirtyFlag = state; }
    bool isDirty() const { return(dirtyFlag); }
    QString labelString(QRect *rect) const;

public slots:
    void onPaintEvent(QPainter *painter, QSize sze);
    void onNextButtonClicked(bool state)
    {
        emit emitNextButtonClicked(state);
    }

    void onPreviousButtonClicked(bool state)
    {
        emit emitPreviousButtonClicked(state);
    }

    void onLabelIndexChanged(int)
    {
        dirtyFlag = true;
    }

    void onSpinBoxValueChanged(int x, int y)
    {
        fiducialWidgets.constFirst()->xSpinBox->setValue(x);
        fiducialWidgets.constFirst()->ySpinBox->setValue(y);
        dirtyFlag = true;
        onEnableNextFiducial();
    }

    void onVisibleRadioButtonToggled()
    {
        fiducialWidgets.constFirst()->zRadioButton->toggle();
        dirtyFlag = true;
        onEnableNextFiducial();
    }

    void onEnableNextFiducial();
    void onEnablePreviousFiducial();

private:
    bool dirtyFlag = false;
    int imageWidth, imageHeight;
    void initialize(QStringList labels, QStringList fiducials);
    QList<LAUFiducialWidget*> fiducialWidgets;
    QComboBox *labelsComboBox;

signals:
    void emitNextButtonClicked(bool state);
    void emitPreviousButtonClicked(bool state);
};

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
class LAUYoloPoseLabelerWidget : public QWidget
{
    Q_OBJECT

public:
    LAUYoloPoseLabelerWidget(QStringList strings = QStringList(), QWidget *parent = nullptr);
    ~LAUYoloPoseLabelerWidget();

    bool isValid() const { return(fileStrings.isEmpty() == false); }

public slots:
    void onContextMenuTriggered(QMouseEvent *event);
    void onExportLabelsForYoloTraining();
    void onPreviousButtonClicked(bool state);
    void onNextButtonClicked(bool state);

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
        Q_UNUSED(obj);

        if (event->type() == QEvent::KeyPress) {
            if (((QKeyEvent*)event)->key() == Qt::Key_Tab){
                palette->onEnableNextFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_S){
                palette->onEnableNextFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Down){
                palette->onEnableNextFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Backtab){
                palette->onEnablePreviousFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_W){
                palette->onEnablePreviousFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Up){
                palette->onEnablePreviousFiducial();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Space){
                palette->onVisibleRadioButtonToggled();
                label->update();
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Left){
                this->onPreviousButtonClicked(true);
                return (true);
            } else if (((QKeyEvent*)event)->key() == Qt::Key_Right){
                this->onNextButtonClicked(true);
                return (true);
            }
        }
        return (false);
    }

private:
    void initialize();
    LAUImage image;
    QStringList fileStrings;
    LAUFiducialLabel *label = nullptr;
    LAUYoloPoseLabelerPalette *palette = nullptr;
};
#endif // LAUYOLOPOSELABELERWIDGET_H
