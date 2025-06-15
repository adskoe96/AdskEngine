#pragma once

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

class WelcomeWindow : public QMainWindow {
    Q_OBJECT

public:
    WelcomeWindow(QWidget* parent = nullptr);

private slots:
    void onCreateProject();
    void onOpenProject();

private:
    QImage* welcomeImage;
    QWidget* central;
    QPushButton* createButton;
    QPushButton* openButton;
};
