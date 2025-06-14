#include "PropertiesPanel.h"
#include "Scene.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QMessageBox>

PropertiesPanel::PropertiesPanel(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);

    auto* title = new QLabel("Properties");
    title->setStyleSheet("font-weight: bold; color: white;");
    mainLayout->addWidget(title);

    formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    // pre-create controls
    posXSpin = new QDoubleSpinBox(); posXSpin->setRange(-100, 100); posXSpin->setSingleStep(0.1);
    posYSpin = new QDoubleSpinBox(); posYSpin->setRange(-100, 100); posYSpin->setSingleStep(0.1);
    posZSpin = new QDoubleSpinBox(); posZSpin->setRange(-100, 100); posZSpin->setSingleStep(0.1);

    rotXSpin = new QDoubleSpinBox(); rotXSpin->setRange(-3.14, 3.14); rotXSpin->setSingleStep(0.1);
    rotYSpin = new QDoubleSpinBox(); rotYSpin->setRange(-3.14, 3.14); rotYSpin->setSingleStep(0.1);
    rotZSpin = new QDoubleSpinBox(); rotZSpin->setRange(-3.14, 3.14); rotZSpin->setSingleStep(0.1);

    sizeXSpin = new QDoubleSpinBox(); sizeXSpin->setRange(0, 100); sizeXSpin->setSingleStep(0.1);
    sizeYSpin = new QDoubleSpinBox(); sizeYSpin->setRange(0, 100); sizeYSpin->setSingleStep(0.1);
    sizeZSpin = new QDoubleSpinBox(); sizeZSpin->setRange(0, 100); sizeZSpin->setSingleStep(0.1);

    // connect common
    connect(posXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setPositionX(v); });
    connect(posYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setPositionY(v); });
    connect(posZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setPositionZ(v); });

    connect(rotXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setRotationX(v); });
    connect(rotYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setRotationY(v); });
    connect(rotZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setRotationZ(v); });

    connect(sizeXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setScaleX(v); });
    connect(sizeYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setScaleY(v); });
    connect(sizeZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (currentObject) currentObject->setScaleZ(v); });

    // light-specific
    lightTypeCombo = new QComboBox();
    lightTypeCombo->addItems({ "Directional","Point","Spot" });
    connect(lightTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int i) { if (auto* l = dynamic_cast<Light*>(currentObject)) { l->type = LightType(i); emit l->propertiesChanged(); }});

    intensitySpin = new QDoubleSpinBox(); intensitySpin->setRange(0, 10); intensitySpin->setSingleStep(0.1);
    connect(intensitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) { if (auto* l = dynamic_cast<Light*>(currentObject)) { l->intensity = v; emit l->propertiesChanged(); }});

    colorButton = new QPushButton();
    connect(colorButton, &QPushButton::clicked, [this] {
        if (auto* l = dynamic_cast<Light*>(currentObject)) {
            QColor c = QColorDialog::getColor(
                QColor::fromRgbF(l->diffuse.r, l->diffuse.g, l->diffuse.b), this);
            if (c.isValid()) {
                l->diffuse.r = c.redF();
                l->diffuse.g = c.greenF();
                l->diffuse.b = c.blueF();
                colorButton->setStyleSheet("background-color:" + c.name());
                emit l->propertiesChanged();
            }
        }
    });

    // delete button
    deleteButton = new QPushButton("Delete Object");
    connect(deleteButton, &QPushButton::clicked, this, &PropertiesPanel::onDeleteClicked);
    mainLayout->addWidget(deleteButton);
}

void PropertiesPanel::clearPanel() {
    // hide light-specific
    lightTypeCombo->hide(); intensitySpin->hide(); colorButton->hide();

    // clear dynamic
    QLayoutItem* it = nullptr;
    while ((it = formLayout->takeAt(0))) {
        if (auto* w = it->widget()) {
            if (w != posXSpin && w != posYSpin && w != posZSpin &&
                w != rotXSpin && w != rotYSpin && w != rotZSpin &&
                w != sizeXSpin && w != sizeYSpin && w != sizeZSpin &&
                w != lightTypeCombo && w != intensitySpin && w != colorButton) {
                w->deleteLater();
            }
        }
        delete it;
    }
}

void PropertiesPanel::onObjectSelected(SceneObject* object) {
    clearPanel(); currentObject = object;
    deleteButton->setEnabled(object != nullptr);
    if (!object) {
        formLayout->addRow(new QLabel("No object selected"));
        return;
    }

    if (object == nullptr) return;

    // common
    formLayout->addRow("Position X:", posXSpin);
    formLayout->addRow("Position Y:", posYSpin);
    formLayout->addRow("Position Z:", posZSpin);
    posXSpin->show(); posYSpin->show(); posZSpin->show();
    posXSpin->setValue(object->getPositionX()); posYSpin->setValue(object->getPositionY()); posZSpin->setValue(object->getPositionZ());
    formLayout->addRow("Rotation X:", rotXSpin);
    formLayout->addRow("Rotation Y:", rotYSpin);
    formLayout->addRow("Rotation Z:", rotZSpin);
    rotXSpin->show(); rotYSpin->show(); rotZSpin->show();
    rotXSpin->setValue(object->getRotationX()); rotYSpin->setValue(object->getRotationY()); rotZSpin->setValue(object->getRotationZ());
    formLayout->addRow("Scale X:", sizeXSpin);
    formLayout->addRow("Scale Y:", sizeYSpin);
    formLayout->addRow("Scale Z:", sizeZSpin);
    sizeXSpin->show(); sizeYSpin->show(); sizeZSpin->show();
    sizeXSpin->setValue(object->getScaleX()); sizeYSpin->setValue(object->getScaleY()); sizeZSpin->setValue(object->getScaleZ());
    if (auto* l = dynamic_cast<Light*>(object)) setupForLight(l);
    else if (auto* c = dynamic_cast<Cube*>(object)) setupForCube(c);
}

void PropertiesPanel::setupForCube(Cube*) {
    formLayout->addRow(new QLabel("--- Cube Settings ---"));
}

void PropertiesPanel::setupForLight(Light* light) {
    formLayout->addRow(new QLabel("--- Light Settings ---"));
    formLayout->addRow("Type:", lightTypeCombo);
    lightTypeCombo->setCurrentIndex(int(light->type)); lightTypeCombo->show();
    formLayout->addRow("Intensity:", intensitySpin);
    intensitySpin->setValue(light->intensity); intensitySpin->show();
    formLayout->addRow("Color:", colorButton);
    colorButton->show();
    QColor c = QColor::fromRgbF(light->diffuse.r, light->diffuse.g, light->diffuse.b);
    colorButton->setStyleSheet("background-color:" + c.name());
}

void PropertiesPanel::onDeleteClicked() {
    if (!currentObject) return;
    // confirm
    auto ret = QMessageBox::question(this, "Delete", "Delete this object?", QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;
    scene->removeObject(currentObject);
    currentObject = nullptr;
    clearPanel();
}