#pragma once
#include <QWidget>
#include "Scene.h"

class Toolbar : public QWidget {
    Q_OBJECT
public:
    explicit Toolbar(Scene* scene, QWidget* parent = nullptr);

signals:
    void environmentSettingsRequested();

private:
    Scene* scene;
};