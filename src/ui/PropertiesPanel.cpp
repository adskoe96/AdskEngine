#include "PropertiesPanel.h"
#include <QLabel>

static constexpr int SLIDER_MAX = 1000;

PropertiesPanel::PropertiesPanel(Scene* scene, QWidget* parent)
    : QWidget(parent), scene(scene)
{
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(6, 6, 6, 6);

    auto* title = new QLabel("Properties");
    title->setStyleSheet("font-weight: bold; color: white;");
    mainLayout->addWidget(title);

    formLayout = new QFormLayout();
    mainLayout->addLayout(formLayout);

    // Create controls
    posXSpin = new QDoubleSpinBox(); posXSpin->setRange(-100000, 100000); posXSpin->setSingleStep(1);
    posYSpin = new QDoubleSpinBox(); posYSpin->setRange(-100000, 100000); posYSpin->setSingleStep(1);
    posZSpin = new QDoubleSpinBox(); posZSpin->setRange(-100000, 100000); posZSpin->setSingleStep(1);
    rotXSpin = new QDoubleSpinBox(); rotXSpin->setRange(-3600, 3600); rotXSpin->setSingleStep(1);
    rotYSpin = new QDoubleSpinBox(); rotYSpin->setRange(-3600, 3600); rotYSpin->setSingleStep(1);
    rotZSpin = new QDoubleSpinBox(); rotZSpin->setRange(-3600, 3600); rotZSpin->setSingleStep(1);
    sizeXSpin = new QDoubleSpinBox(); sizeXSpin->setRange(0, 100); sizeXSpin->setSingleStep(0.1);
    sizeYSpin = new QDoubleSpinBox(); sizeYSpin->setRange(0, 100); sizeYSpin->setSingleStep(0.1);
    sizeZSpin = new QDoubleSpinBox(); sizeZSpin->setRange(0, 100); sizeZSpin->setSingleStep(0.1);
    sizeXSlider = new QSlider(Qt::Horizontal); sizeXSlider->setRange(0, SLIDER_MAX);
    sizeYSlider = new QSlider(Qt::Horizontal); sizeYSlider->setRange(0, SLIDER_MAX);
    sizeZSlider = new QSlider(Qt::Horizontal); sizeZSlider->setRange(0, SLIDER_MAX);
    lightTypeCombo = new QComboBox(); lightTypeCombo->addItems({ "Directional", "Point", "Spot" });
    intensitySpin = new QDoubleSpinBox(); intensitySpin->setRange(0, 10); intensitySpin->setSingleStep(0.1);
    colorButton = new QPushButton();
    deleteButton = new QPushButton("Delete Object"); deleteButton->setEnabled(false);

    // Connect signals to object
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
    connect(sizeXSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::syncScaleX);
    connect(sizeYSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::syncScaleY);
    connect(sizeZSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &PropertiesPanel::syncScaleZ);
    connect(sizeXSlider, &QSlider::valueChanged, this, &PropertiesPanel::sliderScaleX);
    connect(sizeYSlider, &QSlider::valueChanged, this, &PropertiesPanel::sliderScaleY);
    connect(sizeZSlider, &QSlider::valueChanged, this, &PropertiesPanel::sliderScaleZ);
    connect(lightTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [this](int i) {
            if (Light* l = dynamic_cast<Light*>(currentObject.data())) {
                l->type = LightType(i);
                emit l->propertiesChanged();
            }
        });
    connect(intensitySpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
        [this](double v) {
            if (Light* l = dynamic_cast<Light*>(currentObject.data())) {
                l->intensity = v;
                emit l->propertiesChanged();
            }
        });
    connect(colorButton, &QPushButton::clicked, [this]() {
        if (Light* l = dynamic_cast<Light*>(currentObject.data())) {
            QColor c = QColorDialog::getColor(QColor::fromRgbF(l->diffuse.r, l->diffuse.g, l->diffuse.b, l->diffuse.a), this);
            if (c.isValid()) {
                l->diffuse = { static_cast<float>(c.redF()), static_cast<float>(c.greenF()), static_cast<float>(c.blueF()), static_cast<float>(c.alphaF()) };
                colorButton->setStyleSheet("background-color:" + c.name());
                emit l->propertiesChanged();
            }
        }
        });
    connect(deleteButton, &QPushButton::clicked, this, &PropertiesPanel::onDeleteClicked);

    clearPanel();
}

void PropertiesPanel::clearPanel() {
    QLayoutItem* item;
    while ((item = formLayout->takeAt(0)) != nullptr) {
        if (auto* widget = item->widget()) {
            formLayout->removeWidget(widget);
            widget->hide();  // если переиспользуешь, иначе:
            // widget->deleteLater(); // если они временные
        }
        delete item;
    }

    auto* label = new QLabel("No object selected");
    label->setStyleSheet("color: gray;");
    formLayout->addRow(label);

    deleteButton->setEnabled(false);
}

void PropertiesPanel::onObjectSelected(QPointer<SceneObject> object) {
    QSignalBlocker blockerPosX(posXSpin);
    QSignalBlocker blockerPosY(posYSpin);

    if (currentObject) {
        disconnect(currentObject, nullptr, this, nullptr);
    }

    currentObject = object;
    clearPanel();
    if (!currentObject) return;

    deleteButton->setEnabled(true);
    connect(currentObject, &SceneObject::propertiesChanged, this, &PropertiesPanel::updateUI, Qt::UniqueConnection);

    formLayout->removeRow(0);
    formLayout->addRow("Position X:", posXSpin); posXSpin->show();
    formLayout->addRow("Position Y:", posYSpin); posYSpin->show();
    formLayout->addRow("Position Z:", posZSpin); posZSpin->show();
    formLayout->addRow("Rotation X:", rotXSpin); rotXSpin->show();
    formLayout->addRow("Rotation Y:", rotYSpin); rotYSpin->show();
    formLayout->addRow("Rotation Z:", rotZSpin); rotZSpin->show();
    formLayout->addRow("Scale X:", sizeXSpin); sizeXSpin->show();
    formLayout->addRow(sizeXSlider); sizeXSlider->show();
    formLayout->addRow("Scale Y:", sizeYSpin); sizeYSpin->show();
    formLayout->addRow(sizeYSlider); sizeYSlider->show();
    formLayout->addRow("Scale Z:", sizeZSpin); sizeZSpin->show();
    formLayout->addRow(sizeZSlider); sizeZSlider->show();

    if (Light* l = dynamic_cast<Light*>(currentObject.data())) {
        setupForLight(l);
    }
    else if (Cube* c = dynamic_cast<Cube*>(currentObject.data())) {
        setupForCube(c);
    }

    updateUI();
}

void PropertiesPanel::updateUI() {
    if (!currentObject) return;
    posXSpin->setValue(currentObject->getPositionX());
    posYSpin->setValue(currentObject->getPositionY());
    posZSpin->setValue(currentObject->getPositionZ());
    rotXSpin->setValue(currentObject->getRotationX());
    rotYSpin->setValue(currentObject->getRotationY());
    rotZSpin->setValue(currentObject->getRotationZ());
    sizeXSpin->setValue(currentObject->getScaleX());
    sizeYSpin->setValue(currentObject->getScaleY());
    sizeZSpin->setValue(currentObject->getScaleZ());
}

void PropertiesPanel::syncScaleX(double v) {
    if (!currentObject) return; QSignalBlocker blockerSlider(sizeXSlider); currentObject->setScaleX(float(v)); sizeXSlider->setValue(int(v / 100.0 * SLIDER_MAX));
}
void PropertiesPanel::syncScaleY(double v) {
    if (!currentObject) return; QSignalBlocker blockerSlider(sizeYSlider); currentObject->setScaleY(float(v)); sizeYSlider->setValue(int(v / 100.0 * SLIDER_MAX));
}
void PropertiesPanel::syncScaleZ(double v) {
    if (!currentObject) return; QSignalBlocker blockerSlider(sizeZSlider); currentObject->setScaleZ(float(v)); sizeZSlider->setValue(int(v / 100.0 * SLIDER_MAX));
}
void PropertiesPanel::sliderScaleX(int v) { sizeXSpin->setValue(double(v) / SLIDER_MAX * 100.0); }
void PropertiesPanel::sliderScaleY(int v) { sizeYSpin->setValue(double(v) / SLIDER_MAX * 100.0); }
void PropertiesPanel::sliderScaleZ(int v) { sizeZSpin->setValue(double(v) / SLIDER_MAX * 100.0); }

void PropertiesPanel::setupForCube(Cube*) { /* no extra */ }

void PropertiesPanel::setupForLight(Light* light) {
    formLayout->addRow("Type:", lightTypeCombo); lightTypeCombo->show();
    formLayout->addRow("Intensity:", intensitySpin); intensitySpin->show();
    formLayout->addRow("Color:", colorButton); colorButton->show();
    lightTypeCombo->setCurrentIndex(int(light->type));
    intensitySpin->setValue(light->intensity);
    QColor c = QColor::fromRgbF(light->diffuse.r, light->diffuse.g, light->diffuse.b);
    colorButton->setStyleSheet("background-color:" + c.name());
}

void PropertiesPanel::onDeleteClicked() {
    if (!currentObject) return;

    if (QMessageBox::question(this, "Delete", "Delete this object?", QMessageBox::Yes | QMessageBox::No)
        == QMessageBox::Yes) {
        // ќтключаем сигналы перед удалением
        disconnect(currentObject, &SceneObject::propertiesChanged, this, &PropertiesPanel::updateUI);
        scene->removeObject(currentObject);
        currentObject = nullptr; // явно обнул€ем (хот€ QPointer и так это сделает)
        clearPanel();
    }
}