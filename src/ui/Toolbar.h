#pragma once

#include "Scene.h"
#include "Light.h"
#include "MeshRenderer.h"
#include "ConsolePanel.h"
#include <QHBoxLayout>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QWidget>
#include <QPushButton>

class Toolbar : public QWidget {
    Q_OBJECT
public:
    explicit Toolbar(Scene* scene, QWidget* parent = nullptr);

signals:
    void environmentSettingsRequested();

private:
    Scene* scene;

    QMenu* createMenu;

    QToolButton* createButton;
    QAction* createLightAction;
    QAction* createMeshAction;
    QAction* createEmptyAction;

    QPushButton* envSettingsButton;
    QPushButton* playButton;
    QPushButton* saveButton;
    QPushButton* loadButton;
    QPushButton* buildButton;
};