#include "WelcomeWindow.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDir>
#include <QMessageBox>
#include <fstream>
#include "EditorWindow.h"

WelcomeWindow::WelcomeWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("AdskEngine - Project Manager");

    central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    createButton = new QPushButton("Create New Project");
    openButton = new QPushButton("Open Existing Project");

    layout->addWidget(createButton);
    layout->addWidget(openButton);
    layout->setAlignment(Qt::AlignCenter);

    connect(createButton, &QPushButton::clicked, this, &WelcomeWindow::onCreateProject);
    connect(openButton, &QPushButton::clicked, this, &WelcomeWindow::onOpenProject);

    central->setLayout(layout);
    setCentralWidget(central);
    resize(400, 200);
}

void WelcomeWindow::onCreateProject() {
    QString projectName = QInputDialog::getText(this, "New Project", "Enter project name:");
    if (projectName.isEmpty()) return;

    QString description = QInputDialog::getText(this, "Project Description", "Enter project description:");

    QString dirPath = QFileDialog::getExistingDirectory(this, "Select Project Directory");
    if (dirPath.isEmpty()) return;

    QDir dir(dirPath);
    if (!dir.mkdir(projectName)) {
        QMessageBox::warning(this, "Error", "Could not create project folder.");
        return;
    }

    QString projectPath = dir.absoluteFilePath(projectName);
    QJsonObject projectJson;
    projectJson["name"] = projectName;
    projectJson["description"] = description;

    QFile file(projectPath + "/" + projectName + ".json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(projectJson).toJson());
        file.close();
    }

    QMessageBox::information(this, "Project Created", "Project successfully created.");
    
    EditorWindow* editor = new EditorWindow(projectPath);
    editor->show();
    this->close();
}

void WelcomeWindow::onOpenProject() {
    QString filePath = QFileDialog::getOpenFileName(this, "Open Project", "", "Project Files (*.json)");
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        QString projectName = obj["name"].toString();
        QString description = obj["description"].toString();

        QMessageBox::information(this, "Project Loaded", "Project: " + projectName + "\n" + description);

        QFileInfo fileInfo(filePath);
        QString projectPath = fileInfo.absolutePath();
        
        EditorWindow* editor = new EditorWindow(projectPath);
        editor->show();
        this->close();
    }
}
