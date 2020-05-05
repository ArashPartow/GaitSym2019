#include "DialogGeoms.h"
#include "ui_DialogGeoms.h"

#include "Geom.h"
#include "Simulation.h"
#include "Preferences.h"
#include "SphereGeom.h"
#include "CappedCylinderGeom.h"
#include "BoxGeom.h"
#include "PlaneGeom.h"
#include "GSUtil.h"
#include "Marker.h"
#include "DialogProperties.h"

#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>


DialogGeoms::DialogGeoms(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGeoms)
{
    ui->setupUi(this);

    setWindowTitle(tr("Geom Builder"));
#ifdef Q_OS_MACOS
    setWindowFlags(windowFlags() & (~Qt::Dialog) | Qt::Window); // allows the window to be resized on macs
#endif
    restoreGeometry(Preferences::valueQByteArray("DialogGeomsGeometry"));

    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->pushButtonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->pushButtonProperties, SIGNAL(clicked()), this, SLOT(properties()));

    // this logic monitors for changing values
    QList<QWidget *> widgets = this->findChildren<QWidget *>();
    for (auto it = widgets.begin(); it != widgets.end(); it++)
    {
        QComboBox *comboBox = dynamic_cast<QComboBox *>(*it);
        if (comboBox) connect(comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(comboBoxChanged(int)));
        QLineEdit *lineEdit = dynamic_cast<QLineEdit *>(*it);
        if (lineEdit) connect(lineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(lineEditChanged(const QString &)));
        QSpinBox *spinBox = dynamic_cast<QSpinBox *>(*it);
        if (spinBox) connect(spinBox, SIGNAL(valueChanged(const QString &)), this, SLOT(spinBoxChanged(const QString &)));
        QCheckBox *checkBox = dynamic_cast<QCheckBox *>(*it);
        if (checkBox) connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    }

}

DialogGeoms::~DialogGeoms()
{
    delete ui;
}

void DialogGeoms::accept() // this catches OK and return/enter
{
    qDebug() << "DialogGeoms::accept()";

    std::map<std::string, std::unique_ptr<Marker>> *markerList = m_simulation->GetMarkerList();
    QString strapTab = ui->tabWidget->tabText(ui->tabWidget->currentIndex());
    if (strapTab == "Sphere")
    {
        m_outputGeom = std::make_unique<SphereGeom>(m_simulation->GetSpaceID(), ui->lineEditSphereRadius->value());
    }
    else if (strapTab == "Capsule")
    {
        m_outputGeom = std::make_unique<CappedCylinderGeom>(m_simulation->GetSpaceID(),
                                                            ui->lineEditCapsuleRadius->value(), ui->lineEditCapsuleLength->value());
    }
    else if (strapTab == "Box")
    {
        m_outputGeom = std::make_unique<BoxGeom>(m_simulation->GetSpaceID(), ui->lineEditBoxLengthX->value(),
                                                 ui->lineEditBoxLengthY->value(), ui->lineEditBoxLengthZ->value());
    }
    else if (strapTab == "Plane")
    {
        Marker *geomMarker = markerList->at(ui->comboBoxGeomMarker->currentText().toStdString()).get();
        pgd::Vector normal = geomMarker->GetAxis(Marker::Axis::Z);
        pgd::Vector point = geomMarker->GetPosition();
        double a = normal.x;
        double b = normal.y;
        double c = normal.z;
        double d = normal.Dot(point);
        m_outputGeom = std::make_unique<PlaneGeom>(m_simulation->GetSpaceID(), a, b, c, d);
    }

    m_outputGeom->setName(ui->lineEditGeomID->text().toStdString());
    m_outputGeom->setSimulation(m_simulation);
    m_outputGeom->setGeomMarker(markerList->at(ui->comboBoxGeomMarker->currentText().toStdString()).get());
    m_outputGeom->SetSpringDamp(ui->lineEditSpring->value(), ui->lineEditDamp->value(), m_simulation->GetTimeIncrement());
    m_outputGeom->SetContactMu(ui->lineEditMu->value());
    m_outputGeom->SetContactBounce(ui->lineEditBounce->value());
    m_outputGeom->SetAbort(ui->checkBoxAbort->isChecked());

    if (m_inputGeom)
    {
        m_outputGeom->setColour1(m_inputGeom->colour1());
        m_outputGeom->setColour2(m_inputGeom->colour2());
        m_outputGeom->setSize1(m_inputGeom->size1());
        m_outputGeom->setSize2(m_inputGeom->size2());
    }
    else
    {
        m_outputGeom->setColour1(Preferences::valueQColor("GeomColour1").name(QColor::HexArgb).toStdString());
        m_outputGeom->setColour2(Preferences::valueQColor("GeomColour2").name(QColor::HexArgb).toStdString());
        m_outputGeom->setSize1(Preferences::valueDouble("GeomSize1"));
        m_outputGeom->setSize2(Preferences::valueDouble("GeomSize2"));
    }

    if (m_properties.size() > 0)
    {
        if (m_properties.count("GeomColour1"))
            m_outputGeom->setColour1(qvariant_cast<QColor>(m_properties["GeomColour1"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("GeomColour2"))
            m_outputGeom->setColour1(qvariant_cast<QColor>(m_properties["GeomColour2"].value).name(QColor::HexArgb).toStdString());
        if (m_properties.count("GeomSize1"))
            m_outputGeom->setSize1(m_properties["GeomSize1"].value.toDouble());
        if (m_properties.count("GeomSize2"))
            m_outputGeom->setSize2(m_properties["GeomSize2"].value.toDouble());
    }

    Preferences::insert("DialogGeomsGeometry", saveGeometry());
    QDialog::accept();
}

void DialogGeoms::reject() // this catches cancel, close and escape key
{
    qDebug() << "DialogGeoms::reject()";
    Preferences::insert("DialogGeomsGeometry", saveGeometry());
    QDialog::reject();
}

void DialogGeoms::closeEvent(QCloseEvent *event)
{
    Preferences::insert("DialogGeomsGeometry", saveGeometry());

    QDialog::closeEvent(event);
}

void DialogGeoms::lateInitialise()
{
    Q_ASSERT_X(m_simulation, "DialogGeoms::lateInitialise", "m_simulation undefined");

    // set the marker lists
    QStringList markerIDs;
    for (auto &&it : *m_simulation->GetMarkerList()) markerIDs.append(QString::fromStdString(it.first));
    ui->comboBoxGeomMarker->addItems(markerIDs);

    // now set some sensible defaults
    ui->lineEditSpring->setBottom(std::numeric_limits<double>::min());
    ui->lineEditSpring->setValue(Preferences::valueDouble("GlobalDefaultSpringConstant"));
    ui->lineEditDamp->setBottom(std::numeric_limits<double>::min());
    ui->lineEditDamp->setValue(Preferences::valueDouble("GlobalDefaultDampingConstant"));
    ui->lineEditBounce->setBottom(0);
    ui->lineEditBounce->setValue(0);
    ui->lineEditMu->setBottom(0);
    ui->lineEditMu->setValue(1);
    ui->checkBoxAbort->setChecked(false);
    ui->checkBoxAdhesion->setChecked(false);

    ui->lineEditSphereRadius->setBottom(std::numeric_limits<double>::min());
    ui->lineEditSphereRadius->setValue(1);
    ui->lineEditCapsuleRadius->setBottom(std::numeric_limits<double>::min());
    ui->lineEditCapsuleRadius->setValue(1);
    ui->lineEditCapsuleLength->setBottom(std::numeric_limits<double>::min());
    ui->lineEditCapsuleLength->setValue(1);
    ui->lineEditBoxLengthX->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthX->setValue(1);
    ui->lineEditBoxLengthY->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthY->setValue(1);
    ui->lineEditBoxLengthZ->setBottom(std::numeric_limits<double>::min());
    ui->lineEditBoxLengthZ->setValue(1);

    if (!m_inputGeom)
    {
        // set default new name
        auto nameSet = simulation()->GetNameSet();
        ui->lineEditGeomID->addStrings(nameSet);
        int initialNameCount = 0;
        QString initialName = QString("Geom%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
        while (nameSet.count(initialName.toStdString()))
        {
            initialNameCount++;
            initialName = QString("Geom%1").arg(initialNameCount, 3, 10, QLatin1Char('0'));
            if (initialNameCount >= 999) break; // only do this for the first 999 markers
        }
        ui->lineEditGeomID->setText(initialName);
        return;
    }

    QStringList tabNames;
    for (int i = 0; i < ui->tabWidget->count(); i++) tabNames.push_back(ui->tabWidget->tabText(i));

    std::string s;
    m_inputGeom->saveToAttributes();
    ui->lineEditGeomID->setText(QString::fromStdString(m_inputGeom->findAttribute("ID"s)));
    ui->lineEditGeomID->setEnabled(false);
    ui->comboBoxGeomMarker->setCurrentText(QString::fromStdString(m_inputGeom->findAttribute("MarkerID"s)));
    if ((s = m_inputGeom->findAttribute("SpringConstant"s)).size()) ui->lineEditSpring->setValue(GSUtil::Double(s));
    if ((s = m_inputGeom->findAttribute("DampingConstant"s)).size()) ui->lineEditDamp->setValue(GSUtil::Double(s));
    if ((s = m_inputGeom->findAttribute("Bounce"s)).size()) ui->lineEditBounce->setValue(GSUtil::Double(s));
    if ((s = m_inputGeom->findAttribute("Mu"s)).size()) ui->lineEditMu->setValue(GSUtil::Double(s));
    if ((s = m_inputGeom->findAttribute("Abort"s)).size()) ui->checkBoxAbort->setChecked(GSUtil::Bool(s));
    if ((s = m_inputGeom->findAttribute("Adhesion"s)).size()) ui->checkBoxAdhesion->setChecked(GSUtil::Bool(s));

    SphereGeom *sphereGeom = dynamic_cast<SphereGeom *>(m_inputGeom);
    if (sphereGeom)
    {
        if ((s = sphereGeom->findAttribute("Radius"s)).size()) ui->lineEditSphereRadius->setValue(GSUtil::Double(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Sphere"));
    }

    CappedCylinderGeom *cappedCylinderGeom = dynamic_cast<CappedCylinderGeom *>(m_inputGeom);
    if (cappedCylinderGeom)
    {
        if ((s = cappedCylinderGeom->findAttribute("Radius"s)).size()) ui->lineEditCapsuleRadius->setValue(GSUtil::Double(s));
        if ((s = cappedCylinderGeom->findAttribute("Length"s)).size()) ui->lineEditCapsuleLength->setValue(GSUtil::Double(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Capsule"));
    }

    BoxGeom *boxGeom = dynamic_cast<BoxGeom *>(m_inputGeom);
    if (boxGeom)
    {
        if ((s = boxGeom->findAttribute("LengthX"s)).size()) ui->lineEditBoxLengthX->setValue(GSUtil::Double(s));
        if ((s = boxGeom->findAttribute("LengthY"s)).size()) ui->lineEditBoxLengthY->setValue(GSUtil::Double(s));
        if ((s = boxGeom->findAttribute("LengthZ"s)).size()) ui->lineEditBoxLengthZ->setValue(GSUtil::Double(s));
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Box"));
    }

    PlaneGeom *planeGeom = dynamic_cast<PlaneGeom *>(m_inputGeom);
    if (planeGeom)
    {
        ui->tabWidget->setCurrentIndex(tabNames.indexOf("Plane"));
    }
}

void DialogGeoms::comboBoxChanged(int /*index*/)
{
    updateActivation();
}

void DialogGeoms::lineEditChanged(const QString &/*text*/)
{
    updateActivation();
}

void DialogGeoms::spinBoxChanged(const QString &/*text*/)
{
    updateActivation();
}

void DialogGeoms::checkBoxChanged(int /*index*/)
{
    updateActivation();
}

void DialogGeoms::updateActivation()
{
    bool okEnable = true;
    QString textCopy = ui->lineEditGeomID->text();
    int pos = ui->lineEditGeomID->cursorPosition();
    if (ui->lineEditGeomID->validator()->validate(textCopy, pos) != QValidator::Acceptable) okEnable = false;

    ui->pushButtonOK->setEnabled(okEnable);
}

void DialogGeoms::properties()
{
    DialogProperties dialogProperties(this);

    SettingsItem geomColour1 = Preferences::settingsItem("GeomColour1");
    SettingsItem geomColour2 = Preferences::settingsItem("GeomColour2");
    SettingsItem geomSize1 = Preferences::settingsItem("GeomSize1");
    SettingsItem geomSize2 = Preferences::settingsItem("GeomSize2");

    if (m_inputGeom)
    {
        geomColour1.value = QColor(QString::fromStdString(m_inputGeom->colour1().GetHexArgb()));
        geomColour2.value = QColor(QString::fromStdString(m_inputGeom->colour2().GetHexArgb()));
        geomSize1.value = m_inputGeom->size1();
        geomSize2.value = m_inputGeom->size2();
    }
    m_properties.clear();
    m_properties = { { geomColour1.key, geomColour1 },
                     { geomColour2.key, geomColour2 },
                     { geomSize1.key, geomSize1 },
                     { geomSize2.key, geomSize2 } };

    dialogProperties.setInputSettingsItems(m_properties);
    dialogProperties.initialise();

    int status = dialogProperties.exec();
    if (status == QDialog::Accepted)
    {
        dialogProperties.update();
        m_properties = dialogProperties.getOutputSettingsItems();
    }
}

Simulation *DialogGeoms::simulation() const
{
    return m_simulation;
}

void DialogGeoms::setSimulation(Simulation *simulation)
{
    m_simulation = simulation;
}

std::unique_ptr<Geom> DialogGeoms::outputGeom()
{
    return std::move(m_outputGeom);
}

void DialogGeoms::setInputGeom(Geom *inputGeom)
{
    m_inputGeom = inputGeom;
}

