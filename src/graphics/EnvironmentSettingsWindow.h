#pragma once

#include <QDialog>
#include <QTabWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QColorDialog>
#include <QCheckBox>
#include "Scene.h"

class EnvironmentSettingsWindow : public QDialog {
    Q_OBJECT
public:
    explicit EnvironmentSettingsWindow(Scene* scene, QWidget* parent = nullptr);

public slots:
    void applySettings();

private slots:
    void onSkyboxBrowse();
    void onAmbientColorChange();
    void onLightTypeChanged(int index);

private:
    Scene* scene;

    // Skybox
    QLineEdit* skyboxPathEdit;
    QPushButton* skyboxBrowseButton;

    // Lighting
    QComboBox* lightTypeCombo;
    QPushButton* ambientColorButton;
    QColor ambientColor;
    QLineEdit* lightIntensityEdit;
    QCheckBox* shadowsCheckbox;
    QCheckBox* lightingCheckbox;

    void setupSkyboxTab(QTabWidget* tabs);
    void setupLightingTab(QTabWidget* tabs);
    void loadCurrentSettings();
};