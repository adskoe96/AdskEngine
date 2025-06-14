#include "ConsolePanel.h"
#include <QVBoxLayout>
#include <QTextEdit>

ConsolePanel::ConsolePanel(QWidget* parent)
    : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    consoleOutput = new QTextEdit();
    consoleOutput->setReadOnly(true);
    consoleOutput->setText("Console log...");
    layout->addWidget(consoleOutput);
}

void ConsolePanel::NewLog(QString newText)
{
    consoleOutput->append(newText);
}
