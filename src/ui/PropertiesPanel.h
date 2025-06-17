#pragma once
#include "Scene.h"
#include "SceneObject.h"

#include <QWidget>
#include <QFormLayout>
#include <QPushButton>
#include <QPointer>

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
    QFormLayout* formLayout;
    QPointer<SceneObject> currentObject;
};