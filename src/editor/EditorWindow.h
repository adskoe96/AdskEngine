#pragma once
#include <QMainWindow>
#include "Scene.h"

class Scene;

class EditorWindow : public QMainWindow {
    Q_OBJECT
public:
    EditorWindow(const QString& projectPath, QWidget* parent = nullptr);

private slots:
    void openEnvironmentSettings();

private:
    QString m_projectPath;
    Scene* scene;
};