#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include "editor/WelcomeWindow.h"
#include "wwp.h"

void setDarkTheme(QApplication& app) {
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(40, 40, 40));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(30, 30, 30));
    darkPalette.setColor(QPalette::AlternateBase, QColor(40, 40, 40));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(50, 50, 50));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);

    darkPalette.setColor(QPalette::Highlight, QColor(100, 100, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    app.setPalette(darkPalette);
}

int main(int argc, char* argv[]) {
    wwp::set_packs_directory("content");
    QApplication app(argc, argv);

    setDarkTheme(app);

    WelcomeWindow window;
    window.show();

    return app.exec();
}
