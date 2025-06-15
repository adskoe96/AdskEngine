#include "EnvironmentSettingsWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QFileDialog>
#include <QColorDialog>
#include <QDoubleValidator>

EnvironmentSettingsWindow::EnvironmentSettingsWindow(Scene* scene, QWidget* parent)
    : QDialog(parent), scene(scene)
{
    setWindowTitle("Environment Settings");
    setMinimumSize(500, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Making tabs
    QTabWidget* tabs = new QTabWidget(this);
    mainLayout->addWidget(tabs);

    // Skybox tab
    setupSkyboxTab(tabs);

    // Lighting tab
    setupLightingTab(tabs);

    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* applyButton = new QPushButton("Apply");
    QPushButton* cancelButton = new QPushButton("Cancel");
    buttonLayout->addStretch();
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    mainLayout->addLayout(buttonLayout);

    connect(applyButton, &QPushButton::clicked, this, &EnvironmentSettingsWindow::applySettings);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    loadCurrentSettings();
}

void EnvironmentSettingsWindow::setupSkyboxTab(QTabWidget* tabs) {
    QWidget* skyboxTab = new QWidget();
    QVBoxLayout* skyboxLayout = new QVBoxLayout(skyboxTab);

    // Skybox texture path field
    QHBoxLayout* pathLayout = new QHBoxLayout();
    skyboxLayout->addLayout(pathLayout);

    pathLayout->addWidget(new QLabel("Skybox Texture:"));
    skyboxPathEdit = new QLineEdit();
    pathLayout->addWidget(skyboxPathEdit);

    skyboxBrowseButton = new QPushButton("Browse...");
    pathLayout->addWidget(skyboxBrowseButton);
    connect(skyboxBrowseButton, &QPushButton::clicked, this, &EnvironmentSettingsWindow::onSkyboxBrowse);

    tabs->addTab(skyboxTab, "Skybox");
}

void EnvironmentSettingsWindow::setupLightingTab(QTabWidget* tabs) {
    QWidget* lightingTab = new QWidget();
    QFormLayout* lightingLayout = new QFormLayout(lightingTab);

    // Type of lighting
    lightTypeCombo = new QComboBox();
    lightTypeCombo->addItem("Directional", QVariant(0));
    lightTypeCombo->addItem("Point", QVariant(1));
    lightTypeCombo->addItem("Spot", QVariant(2));
    lightingLayout->addRow("Light Type:", lightTypeCombo);
    connect(lightTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &EnvironmentSettingsWindow::onLightTypeChanged);

    // Color of ambient light
    ambientColorButton = new QPushButton();
    ambientColorButton->setFixedSize(30, 30);
    lightingLayout->addRow("Ambient Color:", ambientColorButton);
    connect(ambientColorButton, &QPushButton::clicked, this, &EnvironmentSettingsWindow::onAmbientColorChange);

    // Light intensity
    lightIntensityEdit = new QLineEdit();
    lightIntensityEdit->setValidator(new QDoubleValidator(0.0, 10.0, 2, this));
    lightingLayout->addRow("Light Intensity:", lightIntensityEdit);

    // Shadows
    shadowsCheckbox = new QCheckBox("Enable Shadows");
    lightingLayout->addRow(shadowsCheckbox);

    lightingCheckbox = new QCheckBox("Enable Lighting");
    lightingLayout->addRow(lightingCheckbox);

    tabs->addTab(lightingTab, "Lighting");
}

void EnvironmentSettingsWindow::loadCurrentSettings() {
    if (!scene) return;

    // Loading skybox settings
    skyboxPathEdit->setText(QString::fromStdString(scene->getSkyboxPath()));

    // Loading lighting settings
    const D3DCOLORVALUE& ambient = scene->getAmbientColor();
    ambientColor = QColor::fromRgbF(ambient.r, ambient.g, ambient.b);
    ambientColorButton->setStyleSheet(QString("background-color: %1").arg(ambientColor.name()));

    lightIntensityEdit->setText(QString::number(scene->getLightIntensity()));
    shadowsCheckbox->setChecked(scene->getShadowsEnabled());
}

void EnvironmentSettingsWindow::onSkyboxBrowse() {
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Skybox Texture",
        "",
        "Image Files (*.png *.jpg *.jpeg *.bmp *.dds)"
    );

    if (!filePath.isEmpty()) {
        skyboxPathEdit->setText(filePath);
    }
}

void EnvironmentSettingsWindow::onAmbientColorChange() {
    QColor newColor = QColorDialog::getColor(ambientColor, this, "Select Ambient Color");
    if (newColor.isValid()) {
        ambientColor = newColor;
        ambientColorButton->setStyleSheet(QString("background-color: %1").arg(ambientColor.name()));
    }
}

void EnvironmentSettingsWindow::onLightTypeChanged(int index) {
    // Реализуйте логику изменения типа освещения при необходимости
    Q_UNUSED(index);
}

void EnvironmentSettingsWindow::applySettings() {
    if (!scene) return;

    std::string newPath = skyboxPathEdit->text().toStdString();
    if (scene->getSkyboxPath() != newPath) {
        scene->setSkyboxPath(newPath);
    }

    D3DCOLORVALUE ambient;
    ambient.r = ambientColor.redF();
    ambient.g = ambientColor.greenF();
    ambient.b = ambientColor.blueF();
    ambient.a = 1.0f;
    scene->setAmbientColor(ambient);

    scene->setLightIntensity(lightIntensityEdit->text().toFloat());
    scene->setShadowsEnabled(shadowsCheckbox->isChecked());
    scene->setLightingEnabled(lightingCheckbox->isChecked());

    emit settingsChanged();

    accept();
}