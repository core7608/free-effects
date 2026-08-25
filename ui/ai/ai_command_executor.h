#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <functional>
#include <unordered_map>

namespace FreeEffect {

class MainWindow;

struct AICommand {
    QString action;
    std::unordered_map<QString, QString> params;
};

class AICommandExecutor : public QObject {
    Q_OBJECT
public:
    explicit AICommandExecutor(MainWindow* mainWindow, QObject* parent = nullptr);
    ~AICommandExecutor() override = default;

    bool parseAndExecute(const QString& text);

    static QStringList getAvailableCommands();

signals:
    void commandExecuted(const QString& action, bool success);
    void message(const QString& text);

private:
    AICommand parseCommand(const QString& text);
    bool executeCommand(const AICommand& cmd);
    void registerCommands();

    MainWindow* m_mainWindow;
    std::unordered_map<QString, std::function<bool(const AICommand&)>> m_commands;
};

} // namespace FreeEffect
