#include "lauyoloposelabelerwidget.h"

#include <QMenu>
#include <QFile>
#include <QLabel>
#include <QDebug>
#include <QAction>
#include <QBuffer>
#include <QPainter>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUYoloPoseLabelerWidget::LAUYoloPoseLabelerWidget(QStringList strings, QWidget *parent) : QWidget(parent), fileStrings(strings)
{
    if (fileStrings.isEmpty()){
        QSettings settings;
        QString directory = settings.value("LAUYoloPoseLabelerWidget::lastUsedDirectory", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
        fileStrings = QFileDialog::getOpenFileNames(this, QString("Load images from disk (*.tif)"), directory, QString("*.tif;*.tiff"));
        if (fileStrings.isEmpty() == false) {
            settings.setValue("LAUYoloPoseLabelerWidget::lastUsedDirectory", QFileInfo(fileStrings.first()).absolutePath());
        }
    }

    if (fileStrings.count() > 0){
        image = LAUImage(fileStrings.first());
        this->setWindowTitle(QFileInfo(fileStrings.first()).fileName());
    }
    initialize();
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerWidget::initialize()
{
    qApp->installEventFilter(this);

    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(6,6,6,6);
    this->layout()->setSpacing(6);

    QGroupBox *box = new QGroupBox("Preview:");
    box->setLayout(new QVBoxLayout());
    box->layout()->setContentsMargins(6,6,6,6);
    box->layout()->setSpacing(0);

    label = new LAUFiducialLabel();
    label->setMinimumWidth(960);
    box->layout()->addWidget(label);
    this->layout()->addWidget(box);

    box = new QGroupBox("Palette:");
    box->setLayout(new QVBoxLayout());
    box->layout()->setContentsMargins(6,0,6,6);
    box->layout()->setSpacing(0);

    QStringList labels;
    labels << "Male";
    labels << "Female";

    QStringList fiducials;
    fiducials << "Leg, Right Front";
    fiducials << "Leg, Right Middle";
    fiducials << "Leg, Right Back";
    fiducials << "Leg, Left Front";
    fiducials << "Leg, Left Middle";
    fiducials << "Leg, Left Back";
    fiducials << "Head";
    fiducials << "Antenna, Left";
    fiducials << "Antenna, Right";
    fiducials << "Labial Palp, Left";
    fiducials << "Proboscis";
    fiducials << "Labial Palp, Right";
    fiducials << "Genitalia";

    palette = new LAUYoloPoseLabelerPalette(labels, fiducials);
    connect(label, SIGNAL(emitMousePressEvent(int,int)), palette, SLOT(onSpinBoxValueChanged(int,int)));
    connect(label, SIGNAL(emitPaint(QPainter*,QSize)), palette, SLOT(onPaintEvent(QPainter*,QSize)));
    connect(palette, SIGNAL(emitPreviousButtonClicked(bool)), this, SLOT(onPreviousButtonClicked(bool)));
    connect(palette, SIGNAL(emitNextButtonClicked(bool)), this, SLOT(onNextButtonClicked(bool)));
    connect(label, SIGNAL(emitMouseRightClickEvent(QMouseEvent*)), this, SLOT(onContextMenuTriggered(QMouseEvent*)));
    box->layout()->addWidget(palette);
    box->setFixedWidth(380);
    this->layout()->addWidget(box);

    palette->setXml(image.xmlData());
    palette->setImageSize(image.width(), image.height());
    label->setPixmap(QPixmap::fromImage(image.preview(QSize(image.width(), image.height()))));
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUYoloPoseLabelerWidget::~LAUYoloPoseLabelerWidget()
{
    if (palette->isDirty()){
        image.setXmlData(palette->xml());
        image.save(fileStrings.first());
        palette->setDirty(false);
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerWidget::onPreviousButtonClicked(bool state)
{
    Q_UNUSED(state);

    if (palette->isDirty()){
        image.setXmlData(palette->xml());
        image.save(fileStrings.first());
        palette->setDirty(false);
    }

    if (fileStrings.count() > 1){
        QString string = fileStrings.takeLast();
        fileStrings.prepend(string);
        image = LAUImage(fileStrings.first());
        palette->setXml(image.xmlData());
        palette->setImageSize(image.width(), image.height());
        label->setPixmap(QPixmap::fromImage(image.preview(QSize(image.width(), image.height()))));
        this->setWindowTitle(QFileInfo(fileStrings.first()).fileName());
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerWidget::onNextButtonClicked(bool state)
{
    Q_UNUSED(state);

    if (palette->isDirty()){
        image.setXmlData(palette->xml());
        image.save(fileStrings.first());
        palette->setDirty(false);
    }

    if (fileStrings.count() > 1){
        QString string = fileStrings.takeFirst();
        fileStrings.append(string);
        image = LAUImage(fileStrings.first());
        palette->setXml(image.xmlData());
        palette->setImageSize(image.width(), image.height());
        label->setPixmap(QPixmap::fromImage(image.preview(QSize(image.width(), image.height()))));
        this->setWindowTitle(QFileInfo(fileStrings.first()).fileName());
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerWidget::onExportLabelsForYoloTraining()
{
    QSettings settings;
    QString directory = settings.value("LAUYoloPoseLabelerWidget::inputDirectoryString", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString inputDirectoryString = QFileDialog::getExistingDirectory(this, QString("Set directory to find labeled data..."), directory);
    if (inputDirectoryString.isEmpty() == false) {
        settings.setValue("LAUYoloPoseLabelerWidget::inputDirectoryString", inputDirectoryString);
    } else {
        return;
    }

    // FIND ALL IMAGES INSIDE NESTED FOLDERS OF THE INPUT DIRECTORY
    QStringList inputImageStrings;
    QStringList directoryList;
    QDir currentDirectory;

    directoryList.append(inputDirectoryString);
    while (directoryList.count() > 0) {
        currentDirectory.setPath(directoryList.takeFirst());
        QStringList list = currentDirectory.entryList();
        while (list.count() > 0) {
            QString item = list.takeFirst();
            if (!item.startsWith(".")) {
                QDir dir(currentDirectory.absolutePath().append(QString("/").append(item)));
                if (dir.exists()) {
                    directoryList.append(dir.absolutePath());
                } else if (item.endsWith(".tif")) {
                    inputImageStrings.append(currentDirectory.absolutePath().append(QString("/").append(item)));
                } else if (item.endsWith(".tiff")) {
                    inputImageStrings.append(currentDirectory.absolutePath().append(QString("/").append(item)));
                }
            }
        }
    }

    directory = settings.value("LAUYoloPoseLabelerWidget::outputDirectoryString", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
    QString outputDirectoryString = QFileDialog::getExistingDirectory(this, QString("Set directory for outputing training data..."), directory);
    if (outputDirectoryString.isEmpty() == false) {
        settings.setValue("LAUYoloPoseLabelerWidget::outputDirectoryString", outputDirectoryString);
    } else {
        return;
    }

    QDir imageDir(QString("%1/%2").arg(outputDirectoryString).arg("/images"));
    if (imageDir.exists() == false){
        imageDir.mkdir(imageDir.absolutePath());
    }

    QDir imageTrainDir(QString("%1/%2").arg(imageDir.absolutePath()).arg("/train"));
    if (imageTrainDir.exists() == false){
        imageTrainDir.mkdir(imageTrainDir.absolutePath());
    }

    QDir imageValidDir(QString("%1/%2").arg(imageDir.absolutePath()).arg("/val"));
    if (imageValidDir.exists() == false){
        imageValidDir.mkdir(imageValidDir.absolutePath());
    }

    QDir labelDir(QString("%1/%2").arg(outputDirectoryString).arg("/labels"));
    if (labelDir.exists() == false){
        labelDir.mkdir(labelDir.absolutePath());
    }

    QDir labelTrainDir(QString("%1/%2").arg(labelDir.absolutePath()).arg("/train"));
    if (labelTrainDir.exists() == false){
        labelTrainDir.mkdir(labelTrainDir.absolutePath());
    }

    QDir labelValidDir(QString("%1/%2").arg(labelDir.absolutePath()).arg("/val"));
    if (labelValidDir.exists() == false){
        labelValidDir.mkdir(labelValidDir.absolutePath());
    }

    int validImageCounter = 0;
    for (int n = 0; n < inputImageStrings.count(); n++){
        QString string = inputImageStrings.at(n);
        LAUImage image(string);
        if (image.xmlData().isEmpty() == false){
            validImageCounter++;
            palette->setXml(image.xmlData());
            palette->setImageSize(image.width(), image.height());
            label->setPixmap(QPixmap::fromImage(image.preview(QSize(image.width(), image.height()))));
            this->setWindowTitle(image.filename());
            qApp->processEvents();

            // GET STRING THAT WE CAN WRITE TO LABELS FILE
            int left = (image.width() - 1000)/2;
            int top = (image.height() - 1000)/2;

            QRect rect(left, top, 1000, 1000);
            QString labelString = palette->labelString(rect);

            image = image.crop(left, top, 1000, 1000).rescale(640,640);

            QString newFileString = QString("%1").arg(validImageCounter);
            while (newFileString.length() < 9){
                newFileString.prepend(QString("0"));
            }

            if (validImageCounter % 2){
                image.save(QString("%1/%2.tif").arg(imageTrainDir.absolutePath()).arg(newFileString));
                QFile file(QString("%1/%2.txt").arg(labelTrainDir.absolutePath()).arg(newFileString));
                if (file.open(QIODevice::WriteOnly)){
                    file.write(labelString.toLatin1());
                    file.close();
                }
            } else {
                image.save(QString("%1/%2.tif").arg(imageValidDir.absolutePath()).arg(newFileString));
                QFile file(QString("%1/%2.txt").arg(labelValidDir.absolutePath()).arg(newFileString));
                if (file.open(QIODevice::WriteOnly)){
                    file.write(labelString.toLatin1());
                    file.close();
                }
            }
        }
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerWidget::onContextMenuTriggered(QMouseEvent *event)
{
    QMenu contextMenu(tr("Tools"), this);
    QAction action1("Export Labels for Yolo Training", this);
    connect(&action1, SIGNAL(triggered()), this, SLOT(onExportLabelsForYoloTraining()));
    contextMenu.addAction(&action1);
    contextMenu.exec(event->globalPos());
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUFiducialWidget::LAUFiducialWidget(QWidget *parent) : QWidget(parent)
{
    this->setLayout(new QHBoxLayout());
    this->layout()->setContentsMargins(6,0,0,0);
    this->layout()->setSpacing(6);

    lineEdit = new QLineEdit();
    lineEdit->setText(QString("NO NAME"));
    lineEdit->setMinimumWidth(120);
    this->layout()->addWidget(lineEdit);

    zRadioButton = new QRadioButton();
    zRadioButton->setChecked(true);
    zRadioButton->setFixedWidth(20);
    zRadioButton->setCheckable(true);
    this->layout()->addWidget(zRadioButton);

    xSpinBox = new QSpinBox();
    xSpinBox->setFixedWidth(60);
    xSpinBox->setReadOnly(true);
    xSpinBox->setMaximum(10000);
    xSpinBox->setAlignment(Qt::AlignRight);
    this->layout()->addWidget(xSpinBox);

    ySpinBox = new QSpinBox();
    ySpinBox->setFixedWidth(60);
    ySpinBox->setReadOnly(true);
    ySpinBox->setMaximum(10000);
    ySpinBox->setAlignment(Qt::AlignRight);
    this->layout()->addWidget(ySpinBox);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUYoloPoseLabelerPalette::LAUYoloPoseLabelerPalette(QByteArray xmlByteArray, QWidget *parent) : QWidget(parent)
{
    QStringList labels;
    QStringList fiducials;

    QXmlStreamReader reader(xmlByteArray);
    while (!reader.atEnd()) {
        if (reader.readNext()) {
            QString name = reader.name().toString();
            if (name == "label") {
                labels = reader.readElementText().split(",");
                if (labels.count() > 0){
                    int index = labels.contains(labels.last());
                    if (index > 0){
                        labels.removeLast();
                    }
                }
            } else if (name == "fiducial") {
                QString string = reader.readElementText();
                int indexA = string.indexOf("\"") + 1;
                int indexB = string.lastIndexOf("\"");

                if (indexA > -1 && indexB < string.length()){
                    QString label = string;
                    label = label.left(indexB);
                    label = label.right(indexB - indexA);
                    fiducials << label;
                }
            }
        }
    }
    reader.clear();
    initialize(labels, fiducials);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUYoloPoseLabelerPalette::LAUYoloPoseLabelerPalette(QStringList labels, QStringList fiducials, QWidget *parent) : QWidget(parent)
{
    initialize(labels, fiducials);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerPalette::initialize(QStringList labels, QStringList fiducials)
{
    this->setLayout(new QVBoxLayout());
    this->layout()->setContentsMargins(0,0,0,0);
    this->layout()->setSpacing(6);
    this->setFocusPolicy(Qt::StrongFocus);

    QGroupBox *box = new QGroupBox("Label:");
    box->setLayout(new QVBoxLayout());
    box->layout()->setContentsMargins(6,6,6,6);
    box->layout()->setSpacing(0);
    this->layout()->addWidget(box);

    labelsComboBox = new QComboBox();
    labelsComboBox->addItems(labels);
    connect(labelsComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onLabelIndexChanged(int)));
    box->layout()->addWidget(labelsComboBox);

    box = new QGroupBox("Fiducials:");
    box->setLayout(new QFormLayout());
    box->layout()->setContentsMargins(6,6,6,6);
    box->layout()->setSpacing(0);
    ((QFormLayout*)(box->layout()))->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    for (int n = 0; n < fiducials.count(); n++){
        LAUFiducialWidget *widget = new LAUFiducialWidget();
        widget->lineEdit->setText(fiducials.at(n));
        widget->lineEdit->setReadOnly(true);
        widget->index = n;

        QLabel *label = new QLabel(QString("%1:").arg(n+1));
        label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        ((QFormLayout*)(box->layout()))->addRow(label, widget);
        fiducialWidgets << widget;
    }

    // SET TAB ORDER
    QWidget::setTabOrder(fiducialWidgets.last()->lineEdit, fiducialWidgets.first()->lineEdit);
    for (int n = 0; n < fiducials.count()-1; n++){
        QWidget::setTabOrder(fiducialWidgets.at(n)->lineEdit, fiducialWidgets.at(n+1)->lineEdit);
    }

    // DISABLE ALL EXCEPT THE FIRST FIDUCIAL
    for (int n = 1; n < fiducials.count(); n++){
        fiducialWidgets.at(n)->setDisabled(true);
    }

    this->layout()->addWidget(box);
    ((QVBoxLayout*)(this->layout()))->addStretch();

    QWidget *buttonsWidget = new QWidget();
    buttonsWidget->setLayout(new QHBoxLayout());
    buttonsWidget->layout()->setContentsMargins(-20,0,-20,0);
    buttonsWidget->layout()->setSpacing(6);

    QPushButton *previousButton = new QPushButton("Previous");
    connect(previousButton, SIGNAL(clicked(bool)), this, SLOT(onPreviousButtonClicked(bool)));
    buttonsWidget->layout()->addWidget(previousButton);

    QPushButton *nextButton = new QPushButton("Next");
    connect(nextButton, SIGNAL(clicked(bool)), this, SLOT(onNextButtonClicked(bool)));
    buttonsWidget->layout()->addWidget(nextButton);

    this->layout()->addWidget(buttonsWidget);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
LAUYoloPoseLabelerPalette::~LAUYoloPoseLabelerPalette()
{
    ;
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
QByteArray LAUYoloPoseLabelerPalette::xml() const
{
    // CREATE THE XML DATA PACKET USING QT'S XML STREAM OBJECTS
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    QXmlStreamWriter writer(&buffer);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("LAUYoloPoseFiducials");

    // WRITE THE LABEL TO XML
    QString labelString;
    for (int n = 0; n < labelsComboBox->count(); n++){
        labelString.append(QString("%1,").arg(labelsComboBox->itemText(n)));
    }
    labelString.append(QString("%1").arg(labelsComboBox->currentText()));
    writer.writeTextElement("label", labelString);

    // WRITE THE FIDUCIALS TO XML
    for (int n = 0; n < fiducialWidgets.count(); n++){
        writer.writeTextElement(QString("fiducial"), QString("\"%1\",%2,%3,%4").arg(fiducialWidgets.at(n)->lineEdit->text()).arg(fiducialWidgets.at(n)->zRadioButton->isChecked()).arg(fiducialWidgets.at(n)->xSpinBox->value()).arg(fiducialWidgets.at(n)->ySpinBox->value()));
    }

    // CLOSE OUT THE XML BUFFER
    writer.writeEndElement();
    writer.writeEndDocument();
    buffer.close();

    // EXPORT THE XML BUFFER TO THE XMLPACKET FIELD OF THE TIFF IMAGE
    return(buffer.buffer());
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
QString LAUYoloPoseLabelerPalette::labelString(QRect rect) const
{
    QList<float> fiducials;

    // EXTRACT THE LABEL INDEX
    fiducials << (double)labelsComboBox->currentIndex();

    // EXTRACT THE BOUNDING BOX
    int xMin = 10000;
    int xMax = -1000;
    int yMin = 10000;
    int yMax = -1000;

    for (int n = 0; n < fiducialWidgets.count(); n++){
        xMin = qMin(xMin, fiducialWidgets.at(n)->xSpinBox->value() - rect.left());
        xMax = qMax(xMax, fiducialWidgets.at(n)->xSpinBox->value() - rect.left());
        yMin = qMin(yMin, fiducialWidgets.at(n)->ySpinBox->value() - rect.top());
        yMax = qMax(yMax, fiducialWidgets.at(n)->ySpinBox->value() - rect.top());
    }

    double xLeft = (double)(xMin - 20) / (double)rect.width();
    double xWide = (double)(xMax - xMin + 40) / (double)rect.width();
    double yTop = (double)(yMin - 20) / (double)rect.height();
    double yTall = (double)(yMax - yMin + 40) / (double)rect.height();

    fiducials << (xLeft + xWide/2.0);
    fiducials << (yTop + yTall/2.0);
    fiducials << xWide;
    fiducials << yTall;

    // EXTRACT THE FIDUCIAL COORDINATES
    for (int n = 0; n < fiducialWidgets.count(); n++){
        fiducials << (double)fiducialWidgets.at(n)->xSpinBox->value() / (double)rect.width();
        fiducials << (double)fiducialWidgets.at(n)->ySpinBox->value() / (double)rect.height();
        if (fiducialWidgets.at(n)->zRadioButton->isChecked()){
            fiducials << 1.0;
        } else {
            fiducials << 0.0;
        }
    }

    QString string;
    string.append(QString("%1 ").arg((int)qRound(fiducials.takeFirst())));
    while (fiducials.count() > 0){
        string.append(QString("%1 ").arg(fiducials.takeFirst(), 0, 'f', 6));
    }
    string.chop(1);

    return(string);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerPalette::setXml(QByteArray string)
{
    if (string.isEmpty()){
        for (int n = 0; n < fiducialWidgets.count(); n++){
            LAUFiducialWidget *widget = fiducialWidgets.at(n);
            widget->xSpinBox->setValue(0);
            widget->ySpinBox->setValue(0);
            widget->zRadioButton->setChecked(true);
            labelsComboBox->setCurrentIndex(0);
        }
    } else {
        QXmlStreamReader reader(string);
        while (!reader.atEnd()) {
            if (reader.readNext()) {
                QString name = reader.name().toString();
                if (name == "label") {
                    QStringList labelStrings = reader.readElementText().split(",");
                    if (labelStrings.count() > 0){
                        labelsComboBox->setCurrentText(labelStrings.last().simplified());
                    }
                } else if (name == "fiducial") {
                    QString string = reader.readElementText();
                    int indexA = string.indexOf("\"") + 1;
                    int indexB = string.lastIndexOf("\"");

                    if (indexA > -1 && indexB < string.length()){
                        QString label = string;
                        label = label.left(indexB);
                        label = label.right(indexB - indexA);

                        for (int n = 0; n < fiducialWidgets.count(); n++){
                            if (fiducialWidgets.at(n)->lineEdit->text() == label){
                                string = string.right(string.length() - indexB - 2);
                                QStringList strings = string.split(",");
                                if (strings.count() == 3){
                                    fiducialWidgets.at(n)->zRadioButton->setChecked(strings.at(0).toInt());
                                    fiducialWidgets.at(n)->xSpinBox->setValue(strings.at(1).toInt());
                                    fiducialWidgets.at(n)->ySpinBox->setValue(strings.at(2).toInt());
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
        reader.clear();
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerPalette::onPaintEvent(QPainter *painter, QSize sze)
{
    int xMin = 10000;
    int xMax = -1000;
    int yMin = 10000;
    int yMax = -1000;

    for (int n = 0; n < fiducialWidgets.count(); n++){
        xMin = qMin(xMin, fiducialWidgets.at(n)->xSpinBox->value());
        xMax = qMax(xMax, fiducialWidgets.at(n)->xSpinBox->value());
        yMin = qMin(yMin, fiducialWidgets.at(n)->ySpinBox->value());
        yMax = qMax(yMax, fiducialWidgets.at(n)->ySpinBox->value());
    }

    // DON'T DRAW ANYTHING IF ALL FIDUCIALS ARE EQUAL TO ZERO
    if (xMin == 0 && xMax == 0 && yMin == 0 && yMax == 0){
        return;
    }

    int xLeft = (double)(xMin - 20) / (double)imageWidth * (double)sze.width();
    int xWide = (double)(xMax - xMin + 40) / (double)imageWidth * (double)sze.width();
    int yTop = (double)(yMin - 20) / (double)imageHeight * (double)sze.height();
    int yTall = (double)(yMax - yMin + 40) / (double)imageHeight * (double)sze.height();

    painter->setPen(QPen(Qt::red, 3.0));
    painter->setBrush(QBrush(QColor(Qt::red), Qt::NoBrush));
    painter->drawRect(xLeft, yTop, xWide, yTall);

    for (int n = 0; n < fiducialWidgets.count(); n++){
        QPoint point;
        point.setX((double)fiducialWidgets.at(n)->xSpinBox->value() / (double)imageWidth * (double)sze.width());
        point.setY((double)fiducialWidgets.at(n)->ySpinBox->value() / (double)imageHeight * (double)sze.height());

        if (point.x() == 0 && point.y() == 0){
            continue;
        }

        if (fiducialWidgets.at(n)->zRadioButton->isChecked()){
            painter->setPen(QPen(Qt::red, 6.0));
            painter->setBrush(QBrush(QColor(Qt::red), Qt::SolidPattern));
        } else {
            painter->setPen(QPen(Qt::blue, 6.0));
            painter->setBrush(QBrush(QColor(Qt::blue), Qt::SolidPattern));
        }
        painter->drawEllipse(point.x()-2, point.y()-2, 5, 5);
        painter->drawText(point + QPoint(10,10), QString("%1").arg(fiducialWidgets.at(n)->index + 1));
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerPalette::onEnableNextFiducial()
{
    fiducialWidgets.first()->setDisabled(true);
    fiducialWidgets.append(fiducialWidgets.takeFirst());
    fiducialWidgets.first()->setEnabled(true);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUYoloPoseLabelerPalette::onEnablePreviousFiducial()
{
    fiducialWidgets.first()->setDisabled(true);
    fiducialWidgets.last()->setEnabled(true);
    fiducialWidgets.prepend(fiducialWidgets.takeLast());
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUFiducialLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(QRect(0, 0, this->width(), this->height()), pixmap);

    emit emitPaint(&painter, this->size());

    painter.end();
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUFiducialLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int x = qRound(event->pos().x() / (double)this->width() * pixmap.width());
        int y = qRound(event->pos().y() / (double)this->height() * pixmap.height());

        emit emitMousePressEvent(x, y);
        update();
    }
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void LAUFiducialLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        emit emitMouseRightClickEvent(event);
    }
}
