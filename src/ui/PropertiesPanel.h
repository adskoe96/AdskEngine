#pragma once
#include "Scene.h"
#include "SceneObject.h"
#include "RigidBodyComponent.h"
#include "BoxColliderComponent.h"
#include "SphereColliderComponent.h"
#include "Light.h"
#include <QWidget>
#include <QFormLayout>
#include <QPushButton>
#include <QPointer>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QInputDialog>

class PropertiesPanel : public QWidget {
    Q_OBJECT
public:
    explicit PropertiesPanel(Scene* scene, QWidget* parent = nullptr);

public slots:
    void onObjectSelected(QPointer<SceneObject> obj);
    void updateUI();

private slots:
    void onAddComponent();

private:
    void clearPanel();

    Scene* scene;
    QVBoxLayout* vbox;
    QFormLayout* formLayout;
    QPointer<SceneObject> currentObject;
    QLayoutItem* layItem;
    QString selected;

    QPushButton* addBtn;
    QPushButton* delBtn;
};