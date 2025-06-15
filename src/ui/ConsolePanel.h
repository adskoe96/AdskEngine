#pragma once
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QMutex>
#include <memory>
#include <adskLogsEnums.h>

class ConsolePanel : public QWidget {
    Q_OBJECT

public:
    ConsolePanel(const ConsolePanel&) = delete;
    ConsolePanel& operator=(const ConsolePanel&) = delete;

    static ConsolePanel& instance(QWidget* parent = nullptr);

    void log(LogType type, const QString& text);
    void logInfo(const QString& text);
    void logWarning(const QString& text);
    void logError(const QString& text);

    static void sLog(LogType type, const QString& text);
    static void sInfo(const QString& text);
    static void sWarning(const QString& text);
    static void sError(const QString& text);

private:
    explicit ConsolePanel(QWidget* parent = nullptr);
    ~ConsolePanel() = default;

    QString logTypeToString(LogType type) const;
    void appendToConsole(LogType type, const QString& formattedText);

    QTextEdit* consoleOutput;
    QPushButton* clearButton;
    static QMutex mutex;
};