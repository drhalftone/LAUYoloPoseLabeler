#include "laucmswidget.h"

QString LAUCMSDialog::rgbProfileString = QString();
QString LAUCMSDialog::cmykProfileString = QString();
QString LAUCMSDialog::grayProfileString = QString();
QString LAUCMSDialog::spotProfileString = QString();
QString LAUCMSDialog::rgbPolicyProfileString = QString();
QString LAUCMSDialog::cmykPolicyProfileString = QString();
QString LAUCMSDialog::grayPolicyProfileString = QString();
QString LAUCMSDialog::cmsIntentString = QString();

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
LAUCMSDialog::LAUCMSDialog(QWidget *parent) : QDialog(parent)
{
    // INITIALIZE THE PROFILE STRINGS IF NECESSARY
    if (rgbProfileString.isEmpty()) {
        initializeCMSStrings();
    }

    // SET THE LAYOUT AND THE WINDOW TITLE
    this->setWindowTitle(QString("Color Settings"));
    this->setLayout(new QVBoxLayout());

    // CREATE A GROUP BOX FOR THE PROFILE STRINGS
    QGroupBox *groupBox = new QGroupBox(QString("Working Spaces:"));
    groupBox->setLayout(new QFormLayout());
    ((QFormLayout *)(groupBox->layout()))->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    this->layout()->addWidget(groupBox);

    rgbComboBox = new QComboBox();
    rgbComboBox->addItems(LAUCMSWidget::findICCProfiles(3));
    rgbComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("RGB:"), rgbComboBox);

    cmykComboBox = new QComboBox();
    cmykComboBox->addItems(LAUCMSWidget::findICCProfiles(4));
    cmykComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("CMYK:"), cmykComboBox);

    grayComboBox = new QComboBox();
    grayComboBox->addItems(LAUCMSWidget::findICCProfiles(1));
    grayComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("Gray:"), grayComboBox);

    spotComboBox = new QComboBox();
    spotComboBox->addItems(LAUCMSWidget::findICCProfiles(1));
    spotComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("Spot:"), spotComboBox);

    // CREATE A GROUP BOX FOR THE POLICY STRINGS
    groupBox = new QGroupBox(QString("Color Management Policies:"));
    groupBox->setLayout(new QFormLayout());
    ((QFormLayout *)(groupBox->layout()))->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    this->layout()->addWidget(groupBox);

    rgbPolicyComboBox = new QComboBox();
    rgbPolicyComboBox->addItem(QString("Off"));
    rgbPolicyComboBox->addItem(QString("Preserve Embedded Profiles"));
    rgbPolicyComboBox->addItem(QString("Convert to Working RGB"));
    rgbPolicyComboBox->setCurrentText(QString("Preserve Embedded Profiles"));
    rgbPolicyComboBox->setDisabled(true);
    rgbPolicyComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("RGB:"), rgbPolicyComboBox);

    cmykPolicyComboBox = new QComboBox();
    cmykPolicyComboBox->addItem(QString("Off"));
    cmykPolicyComboBox->addItem(QString("Preserve Embedded Profiles"));
    cmykPolicyComboBox->addItem(QString("Convert to Working CMYK"));
    cmykPolicyComboBox->setCurrentText(QString("Preserve Embedded Profiles"));
    cmykPolicyComboBox->setDisabled(true);
    cmykPolicyComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("CMYK:"), cmykPolicyComboBox);

    grayPolicyComboBox = new QComboBox();
    grayPolicyComboBox->addItem(QString("Off"));
    grayPolicyComboBox->addItem(QString("Preserve Embedded Profiles"));
    grayPolicyComboBox->addItem(QString("Convert to Working Gray"));
    grayPolicyComboBox->setCurrentText(QString("Preserve Embedded Profiles"));
    grayPolicyComboBox->setDisabled(true);
    grayPolicyComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("Gray:"), grayPolicyComboBox);

    cmsIntentComboBox = new QComboBox();
    cmsIntentComboBox->addItem(QString("Perceptual"));
    cmsIntentComboBox->addItem(QString("Saturation"));
    cmsIntentComboBox->addItem(QString("Relative Colorimetric"));
    cmsIntentComboBox->addItem(QString("Absolute Colorimetric"));
    cmsIntentComboBox->setCurrentText(QString("Preserve Embedded Profiles"));
    cmsIntentComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    ((QFormLayout *)(groupBox->layout()))->addRow(QString("Intent:"), cmsIntentComboBox);

    ((QVBoxLayout *)(this->layout()))->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, nullptr);
    connect(buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(reject()));
    this->layout()->addWidget(buttonBox);

    rgbComboBox->setCurrentText(rgbProfileString);
    cmykComboBox->setCurrentText(cmykProfileString);
    grayComboBox->setCurrentText(grayProfileString);
    spotComboBox->setCurrentText(spotProfileString);
    //rgbPolicyComboBox->setCurrentText(rgbPolicyProfileString);
    //cmykPolicyComboBox->setCurrentText(cmykPolicyProfileString);
    //grayPolicyComboBox->setCurrentText(grayPolicyProfileString);
    cmsIntentComboBox->setCurrentText(cmsIntentString);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
LAUCMSDialog::~LAUCMSDialog()
{
    ;
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSDialog::initializeCMSStrings()
{
    QSettings settings;
    rgbProfileString = settings.value(QString("LAUCMSDialog::rgbComboBox"), QString()).toString();
    cmykProfileString = settings.value(QString("LAUCMSDialog::cmykComboBox"), QString()).toString();
    grayProfileString = settings.value(QString("LAUCMSDialog::grayComboBox"), QString()).toString();
    spotProfileString = settings.value(QString("LAUCMSDialog::spotComboBox"), QString()).toString();
    rgbPolicyProfileString = settings.value(QString("LAUCMSDialog::rgbPolicyComboBox"), QString()).toString();
    cmykPolicyProfileString = settings.value(QString("LAUCMSDialog::cmykPolicyComboBox"), QString()).toString();
    grayPolicyProfileString = settings.value(QString("LAUCMSDialog::grayPolicyComboBox"), QString()).toString();
    cmsIntentString = settings.value(QString("LAUCMSDialog::cmsIntentComboBox"), QString()).toString();
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSDialog::accept()
{
    QSettings settings;
    settings.setValue(QString("LAUCMSDialog::rgbComboBox"), rgbComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::cmykComboBox"), cmykComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::grayComboBox"), grayComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::spotComboBox"), spotComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::rgbPolicyComboBox"), rgbPolicyComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::cmykPolicyComboBox"), cmykPolicyComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::grayPolicyComboBox"), grayPolicyComboBox->currentText());
    settings.setValue(QString("LAUCMSDialog::cmsIntentComboBox"), cmsIntentComboBox->currentText());

    // COPY THE PROFILE STRINGS FROM THE SETTINGS INSTANCE TO THE STATIC STRINGS
    initializeCMSStrings();

    QDialog::accept();
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
LAUCMSWidget::LAUCMSWidget(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QVBoxLayout);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->setSpacing(6);

    inputComboBox = new QComboBox();

    QWidget *widget = new QWidget();
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setContentsMargins(0, 0, 0, 0);
    widget->layout()->setSpacing(6);
    this->layout()->addWidget(widget);

    QLabel *label = new QLabel(QString("Input Profile:"));
    label->setFixedWidth(120);
    widget->layout()->addWidget(label);
    widget->layout()->addWidget(inputComboBox);

    widget = new QWidget();
    widget->setLayout(new QHBoxLayout);
    widget->layout()->setSpacing(6);
    widget->layout()->setContentsMargins(0, 0, 0, 0);
    this->layout()->addWidget(widget);

    label = new QLabel(QString("Output Profile:"));
    label->setFixedWidth(120);
    widget->layout()->addWidget(label);

    outputComboBox = new QComboBox();
    widget->layout()->addWidget(outputComboBox);

    this->initializeColorProfileList();

    QSettings settings;
    this->setInputICCProfile(settings.value(QString("inputColorProfileIndex"), QString()).toString());
    this->setOutputICCProfile(settings.value(QString("outputColorProfileIndex"), QString()).toString());

    inputComboBox->setMinimumWidth(300);
    outputComboBox->setMinimumWidth(300);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
LAUCMSWidget::~LAUCMSWidget()
{
    ;
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::saveSettings()
{
    QSettings settings;

    inputProfile = inputComboBox->currentText();
    settings.setValue(QString("inputColorProfileIndex"), inputProfile);
    outputProfile = outputComboBox->currentText();
    settings.setValue(QString("outputColorProfileIndex"), outputProfile);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setInputProfileComboBox(int channels, bool flag)
{
    inputComboBox->clear();
    if (flag) {
        inputComboBox->insertItem(0, QString("Embedded profile"));
    } else {
        if (channels == 1) {
            inputComboBox->insertItems(0, oneColorProfileList);
        } else if (channels == 2) {
            inputComboBox->insertItems(0, twoColorProfileList);
        } else if (channels == 3) {
            inputComboBox->insertItems(0, thrColorProfileList);
        } else if (channels == 4) {
            inputComboBox->insertItems(0, forColorProfileList);
        } else if (channels == 5) {
            inputComboBox->insertItems(0, fveColorProfileList);
        } else if (channels == 6) {
            inputComboBox->insertItems(0, sixColorProfileList);
        }
    }
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setOutputProfileComboBox(int channels, bool flag)
{
    outputComboBox->clear();
    if (channels == 1) {
        outputComboBox->insertItems(0, oneColorProfileList);
    } else if (channels == 2) {
        outputComboBox->insertItems(0, twoColorProfileList);
    } else if (channels == 3) {
        outputComboBox->insertItems(0, thrColorProfileList);
    } else if (channels == 4) {
        outputComboBox->insertItems(0, forColorProfileList);
    } else if (channels == 5) {
        outputComboBox->insertItems(0, fveColorProfileList);
    } else if (channels == 6) {
        outputComboBox->insertItems(0, sixColorProfileList);
    }
    if (flag) {
        outputComboBox->insertItem(0, QString("Match input profile..."));
    }
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::initializeColorProfileList()
{
    oneColorProfileList = findICCProfiles(1);
    twoColorProfileList = findICCProfiles(2);
    thrColorProfileList = findICCProfiles(3);
    forColorProfileList = findICCProfiles(4);
    fveColorProfileList = findICCProfiles(5);
    sixColorProfileList = findICCProfiles(6);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
QStringList LAUCMSWidget::findICCProfiles(int channels)
{
    static QList<ProfileString> profileList;
    if (profileList.count() == 0) {
        QStringList colorProfileList;
        QStringList directoryList;
        QDir currentDirectory;

#if defined(Q_OS_WIN)
        if (currentDirectory.exists(QString("C:/Windows/System32/spool/drivers/color"))) {
            directoryList.append(QString("C:/Windows/System32/spool/drivers/color"));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icm")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }
#elif defined(Q_OS_MAC)
        if (currentDirectory.exists(QString("/System/Library/ColorSync/Profiles"))) {
            directoryList.append(QString("/System/Library/ColorSync/Profiles"));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }

        if (currentDirectory.exists(QString("/Library/ColorSync/Profiles"))) {
            directoryList.append(QString("/Library/ColorSync/Profiles"));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }

        if (currentDirectory.exists(QDir::homePath().append(QString("/Library/ColorSync/Profiles")))) {
            directoryList.append(QDir::homePath().append(QString("/Library/ColorSync/Profiles")));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }

        if (currentDirectory.exists(QDir::homePath().append(QString("/Profiles")))) {
            directoryList.append(QDir::homePath().append(QString("/Profiles")));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }
#elif defined(Q_OS_UNIX)
        if (currentDirectory.exists(QString("$HOME/.local/share/color/icc"))) {
            directoryList.append(QString("$HOME/.local/share/color/icc"));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }
        if (currentDirectory.exists(QString("/usr/share/color/icc"))) {
            directoryList.append(QString("/usr/share/color/icc"));
            while (directoryList.count() > 0) {
                currentDirectory.setPath(directoryList.takeFirst());
                QStringList list = currentDirectory.entryList();
                while (list.count() > 0) {
                    QString item = list.takeFirst();
                    if (!item.startsWith(".")) {
                        QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                        if (dir.exists()) {
                            directoryList.append(dir.absolutePath());
                        } else if (item.endsWith(".icc")) {
                            colorProfileList.append(currentDirectory.absolutePath().append(QString("/").append(item))); //file->fileName());
                        }
                    }
                }
            }
        }
#endif
        colorProfileList.sort();

        // GO THROUGH THE PROFILE LIST AND LOAD EACH PROFILE INTO MEMORY ONE AT A TIME
        for (int n = 0; n < colorProfileList.count(); n++) {
            ProfileString item = { colorProfileList.at(n), 0 };

            // READ THE PROFILES IN AND GET THE NUMBER OF CHANNELS IN THE PROFILE
            cmsHPROFILE iccProfile = cmsOpenProfileFromFile(item.string.toLatin1(), "r");
            item.value = howManyColorsDoesThisProfileHave(iccProfile);
            cmsCloseProfile(iccProfile);

            // ADD THE PROFILE AND ITS NUMBER OF COLORS INTO OUR STATIC LIST
            profileList.append(item);
        }
    }

    QStringList colorProfileList;
    for (int n = 0; n < profileList.count(); n++) {
        if (profileList.at(n).value == channels) {
            colorProfileList.append(profileList.at(n).string);
            qDebug() << profileList.at(n).string << profileList.at(n).value;

        }
    }

    return (colorProfileList);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
QString LAUCMSWidget::inputICCProfile()
{
    inputProfile = inputComboBox->currentText();
    if (inputProfile == QString("Load from disk...")) {
#if defined(Q_OS_MAC)
        QString dirString("/Library/ColorSync/Profiles");
#elif defined(Q_OS_WIN)
        QString dirString("C:/Windows/System32/spool/drivers/color");
#elif defined(Q_OS_UNIX)
        QString dirString("$HOME/.local/share/color/icc");
#endif
        QSettings settings;
        dirString = settings.value(QString("cmsDirectoryString"), dirString).toString();
        inputProfile = QFileDialog::getOpenFileName(0, QString("Select input ICC profile..."), dirString, QString("*.icc;*.icm"));
    }
    return (inputProfile);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
QString LAUCMSWidget::outputICCProfile()
{
    outputProfile = outputComboBox->currentText();
    if (outputProfile == QString("Load from disk...")) {
#if defined(Q_OS_MAC)
        QString dirString("/Library/ColorSync/Profiles");
#elif defined(Q_OS_WIN)
        QString dirString("C:/Windows/System32/spool/drivers/color");
#elif defined(Q_OS_UNIX)
        QString dirString("$HOME/.local/share/color/icc");
#endif
        QSettings settings;
        dirString = settings.value(QString("cmsDirectoryString"), dirString).toString();
        outputProfile = QFileDialog::getOpenFileName(0, QString("Select output ICC profile..."), dirString, QString("*.icc;*.icm"));
    }
    return (outputProfile);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setInputProfileDisabled(bool state)
{
    inputComboBox->setDisabled(state);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setOutputProfileDisabled(bool state)
{
    outputComboBox->setDisabled(state);
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setInputICCProfile(QString fileString)
{
    int index = inputComboBox->findText(fileString);
    if (index > -1) {
        inputComboBox->setCurrentIndex(index);
    }
}

/***************************************************************************************************/
/***************************************************************************************************/
/***************************************************************************************************/
void LAUCMSWidget::setOutputICCProfile(QString fileString)
{
    int index = outputComboBox->findText(fileString);
    if (index > -1) {
        outputComboBox->setCurrentIndex(index);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHTRANSFORM LAUCMSWidget::transform(int inByts, int otByts)
{
    return (transform(inputICCProfile(), outputICCProfile(), inByts, otByts));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHTRANSFORM LAUCMSWidget::transform(QString inProfileString, QString otProfileString, int inByts, int otByts)
{
    cmsHPROFILE inProfile = profile(inProfileString);
    cmsHPROFILE otProfile = profile(otProfileString);
    return (transform(inProfile, otProfile, inByts, otByts));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHTRANSFORM LAUCMSWidget::transform(cmsHPROFILE inProfile, cmsHPROFILE otProfile, int inByts, int otByts)
{
    // CREATE TRANSFORM
    if (inProfile && otProfile) {
        unsigned int inColors = howManyColorsDoesThisProfileHave(inProfile);
        unsigned int otColors = howManyColorsDoesThisProfileHave(otProfile);

        // CREATE A PARAMETERS FOR DEFINING TRANSFORM TO CONVERT BETWEEN PROFILES
        cmsUInt32Number inFormat = COLORSPACE_SH(PT_ANY) | CHANNELS_SH(inColors % 16) | BYTES_SH(inByts % 8);
        cmsUInt32Number otFormat = COLORSPACE_SH(PT_ANY) | CHANNELS_SH(otColors % 16) | BYTES_SH(otByts % 8);

        return (cmsCreateTransform(inProfile, inFormat, otProfile, otFormat, INTENT_PERCEPTUAL, 0));
    } else {
        return (nullptr);
    }
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHPROFILE LAUCMSWidget::profile(QString profileString)
{
    return (cmsOpenProfileFromFile(profileString.toLatin1(), "r"));
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
unsigned int LAUCMSWidget::howManyColorsDoesThisProfileHave(QString profileString)
{
    cmsHPROFILE inProfile = profile(profileString);
    unsigned int numColors = howManyColorsDoesThisProfileHave(inProfile);
    cmsCloseProfile(inProfile);
    return (numColors);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
cmsHPROFILE LAUCMSWidget::loadProfileFromDisk(int channels, QString label)
{
    cmsHPROFILE iccProfile = nullptr;

    // OTHERWISE, ASK THE USER TO SPECIFY A PARTICULAR ICC PROFILE
    static int index = 0;
    bool wasNotCancelledFlag = false;
    QStringList profileList = findICCProfiles(channels);
    if (index >= profileList.count()) {
        index = 0;
    }
    QString profileString = QInputDialog::getItem(nullptr, label, QString("Select an ICC profile for current scan image..."), profileList, index, false, &wasNotCancelledFlag);
    if (wasNotCancelledFlag) {
        // PRESERVE THE INDEX OF SELECTED PROFILE FOR THE NEXT IMAGE
        for (index = profileList.count() - 1; index > 0; index--) {
            if (profileString == profileList.at(index)) {
                break;
            }
        }
        if (profileString == QString("Load from disk...")) {
            QSettings settings;
            QString directory = settings.value("LAUCMSWidget::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
            profileString = QFileDialog::getOpenFileName(0, QString("Select input profile..."), directory, QString("*.icc;*.icm"));
            if (profileString.isEmpty() == false) {
                settings.setValue("LAUCMSWidget::lastUsedDirectory", QFileInfo(profileString).absolutePath());
            }
        }
        if (!profileString.isNull()) {
            iccProfile = cmsOpenProfileFromFile(profileString.toLatin1(), "r");
        }
    }

    return (iccProfile);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
unsigned short LAUCMSWidget::whatPhotometricTagMatchesThisProfile(cmsHPROFILE iccProfile)
{
    switch (cmsGetColorSpace(iccProfile)) {
        case cmsSigLabData:
            return (PHOTOMETRIC_ICCLAB); //                           = 0x4C616220,  // 'Lab '
        case cmsSigYCbCrData:
            return (PHOTOMETRIC_YCBCR); //                         = 0x59436272,  // 'YCbr'
        case cmsSigRgbData:
            return (PHOTOMETRIC_RGB); //                           = 0x52474220,  // 'RGB '
        case cmsSigGrayData:
            return (PHOTOMETRIC_MINISBLACK); //                          = 0x47524159,  // 'GRAY'
        case cmsSigCmykData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x434D594B,  // 'CMYK'
        case cmsSigCmyData:
            return (PHOTOMETRIC_SEPARATED); //                           = 0x434D5920,  // 'CMY '
        case cmsSigMCH1Data:
            return (PHOTOMETRIC_MINISBLACK); //                          = 0x4D434831,  // 'MCH1'
        case cmsSigMCH3Data:
            return (PHOTOMETRIC_RGB); //                          = 0x4D434833,  // 'MCH3'
        case cmsSigMCH4Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434834,  // 'MCH4'
        case cmsSigMCH5Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434835,  // 'MCH5'
        case cmsSigMCH6Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434836,  // 'MCH6'
        case cmsSigMCH7Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434837,  // 'MCH7'
        case cmsSigMCH8Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434838,  // 'MCH8'
        case cmsSigMCH9Data:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434839,  // 'MCH9'
        case cmsSigMCHAData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434841,  // 'MCHA'
        case cmsSigMCHBData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434842,  // 'MCHB'
        case cmsSigMCHCData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434843,  // 'MCHC'
        case cmsSigMCHDData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434844,  // 'MCHD'
        case cmsSigMCHEData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434845,  // 'MCHE'
        case cmsSigMCHFData:
            return (PHOTOMETRIC_SEPARATED); //                          = 0x4D434846,  // 'MCHF'
        case cmsSigNamedData:
            return (PHOTOMETRIC_MINISBLACK); //                         = 0x6e6d636c,  // 'nmcl'
        case cmsSig1colorData:
            return (PHOTOMETRIC_MINISBLACK); //                        = 0x31434C52,  // '1CLR'
        case cmsSig3colorData:
            return (PHOTOMETRIC_RGB); //                        = 0x33434C52,  // '3CLR'
        case cmsSig4colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x34434C52,  // '4CLR'
        case cmsSig5colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x35434C52,  // '5CLR'
        case cmsSig6colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x36434C52,  // '6CLR'
        case cmsSig7colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x37434C52,  // '7CLR'
        case cmsSig8colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x38434C52,  // '8CLR'
        case cmsSig9colorData:
            return (PHOTOMETRIC_SEPARATED); //                        = 0x39434C52,  // '9CLR'
        case cmsSig10colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x41434C52,  // 'ACLR'
        case cmsSig11colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x42434C52,  // 'BCLR'
        case cmsSig12colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x43434C52,  // 'CCLR'
        case cmsSig13colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x44434C52,  // 'DCLR'
        case cmsSig14colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x45434C52,  // 'ECLR'
        case cmsSig15colorData:
            return (PHOTOMETRIC_SEPARATED); //                       = 0x46434C52,  // 'FCLR'
        default:
            return (0);
    }
    return (0);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
unsigned int LAUCMSWidget::howManyColorsDoesThisProfileHave(cmsHPROFILE iccProfile)
{
    if (iccProfile == nullptr) {
        return (0);
    }
    switch (cmsGetColorSpace(iccProfile)) {
        case cmsSigXYZData:
            return (3); //                          = 0x58595A20,  // 'XYZ '
        case cmsSigLabData:
            return (3); //                           = 0x4C616220,  // 'Lab '
        case cmsSigLuvData:
            return (3); //                           = 0x4C757620,  // 'Luv '
        case cmsSigYCbCrData:
            return (3); //                         = 0x59436272,  // 'YCbr'
        case cmsSigYxyData:
            return (3); //                           = 0x59787920,  // 'Yxy '
        case cmsSigRgbData:
            return (3); //                           = 0x52474220,  // 'RGB '
        case cmsSigGrayData:
            return (1); //                          = 0x47524159,  // 'GRAY'
        case cmsSigHsvData:
            return (3); //                           = 0x48535620,  // 'HSV '
        case cmsSigHlsData:
            return (3); //                           = 0x484C5320,  // 'HLS '
        case cmsSigCmykData:
            return (4); //                          = 0x434D594B,  // 'CMYK'
        case cmsSigCmyData:
            return (3); //                           = 0x434D5920,  // 'CMY '
        case cmsSigMCH1Data:
            return (1); //                          = 0x4D434831,  // 'MCH1'
        case cmsSigMCH2Data:
            return (2); //                          = 0x4D434832,  // 'MCH2'
        case cmsSigMCH3Data:
            return (3); //                          = 0x4D434833,  // 'MCH3'
        case cmsSigMCH4Data:
            return (4); //                          = 0x4D434834,  // 'MCH4'
        case cmsSigMCH5Data:
            return (5); //                          = 0x4D434835,  // 'MCH5'
        case cmsSigMCH6Data:
            return (6); //                          = 0x4D434836,  // 'MCH6'
        case cmsSigMCH7Data:
            return (7); //                          = 0x4D434837,  // 'MCH7'
        case cmsSigMCH8Data:
            return (8); //                          = 0x4D434838,  // 'MCH8'
        case cmsSigMCH9Data:
            return (9); //                          = 0x4D434839,  // 'MCH9'
        case cmsSigMCHAData:
            return (10); //                          = 0x4D434841,  // 'MCHA'
        case cmsSigMCHBData:
            return (11); //                          = 0x4D434842,  // 'MCHB'
        case cmsSigMCHCData:
            return (12); //                          = 0x4D434843,  // 'MCHC'
        case cmsSigMCHDData:
            return (13); //                          = 0x4D434844,  // 'MCHD'
        case cmsSigMCHEData:
            return (14); //                          = 0x4D434845,  // 'MCHE'
        case cmsSigMCHFData:
            return (15); //                          = 0x4D434846,  // 'MCHF'
        case cmsSigNamedData:
            return (1); //                         = 0x6e6d636c,  // 'nmcl'
        case cmsSig1colorData:
            return (1); //                        = 0x31434C52,  // '1CLR'
        case cmsSig2colorData:
            return (2); //                        = 0x32434C52,  // '2CLR'
        case cmsSig3colorData:
            return (3); //                        = 0x33434C52,  // '3CLR'
        case cmsSig4colorData:
            return (4); //                        = 0x34434C52,  // '4CLR'
        case cmsSig5colorData:
            return (5); //                        = 0x35434C52,  // '5CLR'
        case cmsSig6colorData:
            return (6); //                        = 0x36434C52,  // '6CLR'
        case cmsSig7colorData:
            return (7); //                        = 0x37434C52,  // '7CLR'
        case cmsSig8colorData:
            return (8); //                        = 0x38434C52,  // '8CLR'
        case cmsSig9colorData:
            return (9); //                        = 0x39434C52,  // '9CLR'
        case cmsSig10colorData:
            return (10); //                       = 0x41434C52,  // 'ACLR'
        case cmsSig11colorData:
            return (11); //                       = 0x42434C52,  // 'BCLR'
        case cmsSig12colorData:
            return (12); //                       = 0x43434C52,  // 'CCLR'
        case cmsSig13colorData:
            return (13); //                       = 0x44434C52,  // 'DCLR'
        case cmsSig14colorData:
            return (14); //                       = 0x45434C52,  // 'ECLR'
        case cmsSig15colorData:
            return (15); //                       = 0x46434C52,  // 'FCLR'
        case cmsSigLuvKData:
            return (4); //                          = 0x4C75764B   // 'LuvK'
    }
    return (0);
}
