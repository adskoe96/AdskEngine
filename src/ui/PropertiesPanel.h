#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QColorDialog>
#include <QMessageBox>
#include <QPointer>
#include "Scene.h"
#include "SceneObject.h"
#include "Cube.h"
#include "Light.h"

class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(Scene* scene, QWidget* parent = nullptr);

public slots:
    void onObjectSelected(QPointer<SceneObject> object);
    void updateUI();

private slots:
    void onDeleteClicked();
    void syncScaleX(double v);
    void syncScaleY(double v);
    void syncScaleZ(double v);
    void sliderScaleX(int v);
    void sliderScaleY(int v);
    void sliderScaleZ(int v);

private:
    void clearPanel();
    void setupForCube(Cube* cube);
    void setupForLight(Light* light);

    Scene* scene;
    QVBoxLayout* mainLayout;
    QFormLayout* formLayout;
    QPointer<SceneObject> currentObject = nullptr;

    QDoubleSpinBox* posXSpin;
    QDoubleSpinBox* posYSpin;
    QDoubleSpinBox* posZSpin;
    QDoubleSpinBox* rotXSpin;
    QDoubleSpinBox* rotYSpin;
    QDoubleSpinBox* rotZSpin;
    QDoubleSpinBox* sizeXSpin;
    QDoubleSpinBox* sizeYSpin;
    QDoubleSpinBox* sizeZSpin;
    QSlider* sizeXSlider;
    QSlider* sizeYSlider;
    QSlider* sizeZSlider;

    QComboBox* lightTypeCombo;
    QDoubleSpinBox* intensitySpin;
    QPushButton* colorButton;
    QPushButton* deleteButton;
};