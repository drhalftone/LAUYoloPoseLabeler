#ifndef LAUCMSWIDGET_H
#define LAUCMSWIDGET_H

#include <QDir>
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QStandardPaths>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QSettings>
#include "lcms2.h"
#include "tiffio.h"

class LAUCMSWidget : public QWidget
{
    Q_OBJECT

    typedef struct {
        QString string;
        int value;
    } ProfileString;

public:
    explicit LAUCMSWidget(QWidget *parent = 0);
    ~LAUCMSWidget();

    void saveSettings();

    QString inputICCProfile();
    QString outputICCProfile();
    void setInputICCProfile(QString fileString);
    void setOutputICCProfile(QString fileString);
    void setInputProfileComboBox(int channels, bool flag = false);
    void setOutputProfileComboBox(int channels, bool flag = false);

    void setInputProfileDisabled(bool state);
    void setOutputProfileDisabled(bool state);

    cmsHTRANSFORM transform(int inByts = 1, int otByts = 1);

    static QStringList findICCProfiles(int channels);
    static unsigned int howManyColorsDoesThisProfileHave(QString profileString);
    static unsigned int howManyColorsDoesThisProfileHave(cmsHPROFILE iccProfile);
    static unsigned short whatPhotometricTagMatchesThisProfile(cmsHPROFILE iccProfile);
    static cmsHTRANSFORM transform(QString inProfileString, QString otProfileString, int inByts = 1, int otByts = 1);
    static cmsHTRANSFORM transform(cmsHPROFILE inProfile, cmsHPROFILE outProfile, int inByts = 1, int otByts = 1);
    static cmsHPROFILE loadProfileFromDisk(int channels, QString label = QString("Color Profile"));
    static cmsHPROFILE profile(QString profileString);

private:
    QStringList oneColorProfileList;
    QStringList twoColorProfileList;
    QStringList thrColorProfileList;
    QStringList forColorProfileList;
    QStringList fveColorProfileList;
    QStringList sixColorProfileList;

    QString inputProfile;
    QString outputProfile;

    QComboBox *inputComboBox;
    QComboBox *outputComboBox;

    void initializeColorProfileList();
};

class LAUCMSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LAUCMSDialog(QWidget *parent = 0);
    ~LAUCMSDialog();

    static QString rgbProfileString;
    static QString cmykProfileString;
    static QString grayProfileString;
    static QString spotProfileString;
    static QString rgbPolicyProfileString;
    static QString cmykPolicyProfileString;
    static QString grayPolicyProfileString;
    static QString cmsIntentString;

    static void initializeCMSStrings();

protected:
    void accept();

private:
    QComboBox *rgbComboBox;
    QComboBox *cmykComboBox;
    QComboBox *grayComboBox;
    QComboBox *spotComboBox;
    QComboBox *rgbPolicyComboBox;
    QComboBox *cmykPolicyComboBox;
    QComboBox *grayPolicyComboBox;
    QComboBox *cmsIntentComboBox;
    QCheckBox *askWhenOpeningCheckBox;
};

#endif // LAUCMSWIDGET_H
