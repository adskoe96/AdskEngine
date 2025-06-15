#include "ConsolePanel.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTextCursor>
#include <QTextCharFormat>

QMutex ConsolePanel::mutex;

ConsolePanel& ConsolePanel::instance(QWidget* parent) {
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    static ConsolePanel* instance = nullptr;

    if (!instance) {
        instance = new ConsolePanel(parent);
    }
    return *instance;
}

ConsolePanel::ConsolePanel(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    consoleOutput = new QTextEdit();
    consoleOutput->setReadOnly(true);
    layout->addWidget(consoleOutput);

    clearButton = new QPushButton("Clear");
    connect(clearButton, &QPushButton::clicked, consoleOutput, &QTextEdit::clear);
    layout->addWidget(clearButton);

    log(LogType::Info, "[Console initialized]");
}

void ConsolePanel::log(LogType type, const QString& text) {
    if (!consoleOutput) {
        qWarning() << "Console output not initialized!";
        return;
    }

    QMutexLocker locker(&mutex);
    QString timestamp = QDateTime::currentDateTime().toString("[hh:mm:ss]");
    QString formattedLog = QString("%1 [%2] %3")
        .arg(timestamp)
        .arg(logTypeToString(type))
        .arg(text);

    QMetaObject::invokeMethod(this, [this, type, formattedLog]() {
        appendToConsole(type, formattedLog);
        }, Qt::QueuedConnection);
}

void ConsolePanel::appendToConsole(LogType type, const QString& formattedText) {
    QTextCharFormat format;

    switch (type) {
    case LogType::Error:
        format.setForeground(QColor(255, 100, 100));
        format.setFontWeight(QFont::Bold);
        break;
    case LogType::Warning:
        format.setForeground(QColor(255, 200, 50));
        break;
    case LogType::Info:
        format.setForeground(QColor(100, 200, 255));
        break;
    default:
        format.setForeground(consoleOutput->palette().text().color());
        break;
    }

    QTextCursor cursor(consoleOutput->document());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(formattedText + "\n", format);
    consoleOutput->ensureCursorVisible();
}

QString ConsolePanel::logTypeToString(LogType type) const {
    switch (type) {
    case LogType::Info: return "INFO";
    case LogType::Warning: return "WARNING";
    case LogType::Error: return "ERROR";
    default: return "UNKNOWN";
    }
}

void ConsolePanel::logInfo(const QString& text) { log(LogType::Info, text); }
void ConsolePanel::logWarning(const QString& text) { log(LogType::Warning, text); }
void ConsolePanel::logError(const QString& text) { log(LogType::Error, text); }

void ConsolePanel::sLog(LogType type, const QString& text) { instance().log(type, text); }
void ConsolePanel::sInfo(const QString& text) { instance().logInfo(text); }
void ConsolePanel::sWarning(const QString& text) { instance().logWarning(text); }
void ConsolePanel::sError(const QString& text) { instance().logError(text); }