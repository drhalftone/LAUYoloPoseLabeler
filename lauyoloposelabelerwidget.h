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
    void wheelEvent(QWheelEvent *event) { ; }
    void mousePressEvent(QMouseEvent *event) { ; }
    void mouseReleaseEvent(QMouseEvent *event) { ; }
    void mouseMoveEvent(QMouseEvent *event) { ; }
    void keyReleaseEvent(QKeyEvent *event) { ; }
    void keyPressEvent(QKeyEvent *event) { ; }

signals:
    void emitPaint(QPainter *painter);

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

    QLineEdit *lineEdit = nullptr;
    QPushButton *setButton = nullptr;
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

    QByteArray xml();
    void setXml(QByteArray string);

public slots:
    void onPaintEvent(QPainter *painter);
    void onNextButtonClicked(bool state)
    {
        emit emitNextButtonClicked(state);
    }

    void onPreviousButtonClicked(bool state)
    {
        emit emitPreviousButtonClicked(state);
    }

    void onSetButtonClicked() { ; }
    void onSpinBoxValueChanged() { ; }
    void onVisibleRadioButtonToggled() { ; }

    void onEnableNextFiducial();
    void onEnablePreviousFiducial();

private:
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

public slots:
    void onPreviousButtonClicked(bool state);
    void onNextButtonClicked(bool state);

protected:
    bool eventFilter(QObject *obj, QEvent *event)
    {
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
            }
        }
        return (false);
    }

private:
    void initialize();
    LAUImage image;
    bool dirtyFlag = false;
    QStringList fileStrings;
    LAUFiducialLabel *label = nullptr;
    LAUYoloPoseLabelerPalette *palette = nullptr;
};
#endif // LAUYOLOPOSELABELERWIDGET_H
