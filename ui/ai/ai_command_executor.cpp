#include "ai_command_executor.h"
#include "../mainwindow/main_window.h"
#include <QRegularExpression>

namespace FreeEffect {

AICommandExecutor::AICommandExecutor(MainWindow* mainWindow, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mainWindow) {
    registerCommands();
}

void AICommandExecutor::registerCommands() {
    m_commands["new_composition"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onNewComposition();
            return true;
        }
        return false;
    };

    m_commands["import"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onImportFile();
            return true;
        }
        return false;
    };

    m_commands["save"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onSaveProject();
            return true;
        }
        return false;
    };

    m_commands["undo"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onUndo();
            return true;
        }
        return false;
    };

    m_commands["redo"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onRedo();
            return true;
        }
        return false;
    };

    m_commands["add_solid"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Solid layer creation: Use Layer > New > Solid (Ctrl+Y)");
        return true;
    };

    m_commands["add_text"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Text layer creation: Use Layer > New > Text");
        return true;
    };

    m_commands["add_null"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Null object creation: Use Layer > New > Null Object");
        return true;
    };

    m_commands["add_adjustment"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Adjustment layer: Use Layer > New > Adjustment Layer");
        return true;
    };

    m_commands["delete_layer"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Delete selected layer: Press Delete key");
        return true;
    };

    m_commands["zoom_in"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Zoom in: Use View > Zoom In");
        return true;
    };

    m_commands["zoom_out"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Zoom out: Use View > Zoom Out");
        return true;
    };

    m_commands["fit_to_window"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Fit to window: Use View > Fit to Window");
        return true;
    };

    m_commands["toggle_grid"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Toggle grid: Use View > Show Grid");
        return true;
    };

    m_commands["toggle_rulers"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Toggle rulers: Use View > Show Rulers");
        return true;
    };

    m_commands["play"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow && m_mainWindow->getTimelinePanel()) {
            emit message("Playback started");
            return true;
        }
        return false;
    };

    m_commands["pause"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        emit message("Playback paused");
        return true;
    };

    m_commands["render"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onAddToRenderQueue();
            return true;
        }
        return false;
    };

    m_commands["show_preferences"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onShowPreferences();
            return true;
        }
        return false;
    };

    m_commands["show_about"] = [this](const AICommand& cmd) -> bool {
        Q_UNUSED(cmd);
        if (m_mainWindow) {
            m_mainWindow->onShowAbout();
            return true;
        }
        return false;
    };
}

AICommand AICommandExecutor::parseCommand(const QString& text) {
    AICommand cmd;
    QString lower = text.toLower().trimmed();

    // Pattern: /command or /command params
    QRegularExpression cmdRegex("^/(\\w+)(?:\\s+(.*))?$");
    QRegularExpressionMatch match = cmdRegex.match(lower);

    if (match.hasMatch()) {
        cmd.action = match.captured(1);
        QString paramStr = match.captured(2).trimmed();
        if (!paramStr.isEmpty()) {
            cmd.params["arg"] = paramStr;
        }
        return cmd;
    }

    // Natural language patterns
    struct Pattern {
        QRegularExpression regex;
        QString action;
    };

    static const std::vector<Pattern> patterns = {
        {QRegularExpression("(new|create)\\s+(comp|composition|project)"), "new_composition"},
        {QRegularExpression("(import|open)\\s+(file|media|footage|video|image|audio)"), "import"},
        {QRegularExpression("^save$|^save\\s+project"), "save"},
        {QRegularExpression("^undo$"), "undo"},
        {QRegularExpression("^redo$"), "redo"},
        {QRegularExpression("(add|create|new)\\s+solid"), "add_solid"},
        {QRegularExpression("(add|create|new)\\s+text"), "add_text"},
        {QRegularExpression("(add|create|new)\\s+null"), "add_null"},
        {QRegularExpression("(add|create|new)\\s+adjustment"), "add_adjustment"},
        {QRegularExpression("(delete|remove)\\s+layer"), "delete_layer"},
        {QRegularExpression("zoom\\s+in"), "zoom_in"},
        {QRegularExpression("zoom\\s+out"), "zoom_out"},
        {QRegularExpression("fit\\s+(to\\s+)?window"), "fit_to_window"},
        {QRegularExpression("(toggle|show|hide)\\s+grid"), "toggle_grid"},
        {QRegularExpression("(toggle|show|hide)\\s+rulers"), "toggle_rulers"},
        {QRegularExpression("^play$|^start\\s+playback"), "play"},
        {QRegularExpression("^pause$|^stop$|^stop\\s+playback"), "pause"},
        {QRegularExpression("(render|export|add\\s+to\\s+render)"), "render"},
        {QRegularExpression("(show|open)\\s+preferences"), "show_preferences"},
        {QRegularExpression("(show|open)\\s+about"), "show_about"},
    };

    for (const auto& p : patterns) {
        if (p.regex.match(lower).hasMatch()) {
            cmd.action = p.action;
            return cmd;
        }
    }

    cmd.action = "unknown";
    return cmd;
}

bool AICommandExecutor::executeCommand(const AICommand& cmd) {
    auto it = m_commands.find(cmd.action.toStdString());
    if (it != m_commands.end()) {
        bool success = it->second(cmd);
        emit commandExecuted(cmd.action, success);
        return success;
    }
    emit commandExecuted(cmd.action, false);
    return false;
}

bool AICommandExecutor::parseAndExecute(const QString& text) {
    if (text.trimmed().isEmpty()) return false;

    // Only process if text starts with / or matches known command patterns
    QString trimmed = text.trimmed();
    bool isCommand = trimmed.startsWith('/') || trimmed.toLower().startsWith("command:");

    if (!isCommand) {
        // Check if it looks like a command (not conversational)
        QString lower = trimmed.toLower();
        QStringList commandKeywords = {
            "new composition", "create composition", "import", "save", "undo", "redo",
            "add solid", "add text", "add null", "delete layer", "zoom in", "zoom out",
            "fit to window", "toggle grid", "toggle rulers", "play", "pause", "render",
            "show preferences", "show about", "open preferences", "new solid", "new text"
        };

        bool looksLikeCommand = false;
        for (const auto& kw : commandKeywords) {
            if (lower.contains(kw)) {
                looksLikeCommand = true;
                break;
            }
        }

        if (!looksLikeCommand) return false;
    }

    // Strip "command:" prefix if present
    if (trimmed.toLower().startsWith("command:")) {
        trimmed = trimmed.mid(8).trimmed();
    }

    AICommand cmd = parseCommand(trimmed);
    return executeCommand(cmd);
}

QStringList AICommandExecutor::getAvailableCommands() {
    return QStringList()
        << "new_composition" << "import" << "save" << "undo" << "redo"
        << "add_solid" << "add_text" << "add_null" << "add_adjustment"
        << "delete_layer" << "zoom_in" << "zoom_out" << "fit_to_window"
        << "toggle_grid" << "toggle_rulers" << "play" << "pause"
        << "render" << "show_preferences" << "show_about";
}

} // namespace FreeEffect
