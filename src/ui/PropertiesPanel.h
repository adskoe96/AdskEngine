#pragma once

#include <QWidget>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QPushButton>
#include "SceneObject.h"
#include "Cube.h"
#include "Light.h"

class Scene;

class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(Scene* scene, QWidget* parent = nullptr);

public slots:
    void onObjectSelected(SceneObject* object);

private slots:
    void onDeleteClicked();

private:
    void clearPanel();
    void setupForCube(Cube* cube);
    void setupForLight(Light* light);

    Scene* scene;
    QVBoxLayout* mainLayout;
    QFormLayout* formLayout;
    SceneObject* currentObject = nullptr;

    QDoubleSpinBox* posXSpin;
    QDoubleSpinBox* posYSpin;
    QDoubleSpinBox* posZSpin;
    QDoubleSpinBox* rotXSpin;
    QDoubleSpinBox* rotYSpin;
    QDoubleSpinBox* rotZSpin;
    QDoubleSpinBox* sizeXSpin;
    QDoubleSpinBox* sizeYSpin;
    QDoubleSpinBox* sizeZSpin;

    QComboBox* lightTypeCombo;
    QDoubleSpinBox* intensitySpin;
    QPushButton* colorButton;

    QPushButton* deleteButton;
};