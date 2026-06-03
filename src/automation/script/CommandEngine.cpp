#include "automation/script/CommandEngine.h"

CommandEngine::CommandEngine(QObject *parent) : QObject(parent) {}
CommandEngine::~CommandEngine() = default;

void CommandEngine::registerCommand(const QString &name, const QString &desc, CommandFunc fn) {
    m_commands[name] = {fn, desc};
}

void CommandEngine::unregisterCommand(const QString &name) {
    m_commands.remove(name);
}

QByteArray CommandEngine::execute(const QString &name, const QByteArray &input) {
    auto it = m_commands.constFind(name);
    if (it == m_commands.constEnd()) {
        emit error(tr("Command not found: %1").arg(name));
        return {};
    }
    QByteArray result = it->fn(input);
    if (m_recording) {
        m_macros[m_currentMacro].append({name, input});
    }
    emit commandExecuted(name, result);
    return result;
}

QStringList CommandEngine::availableCommands() const { return m_commands.keys(); }

QString CommandEngine::commandDescription(const QString &name) const {
    auto it = m_commands.constFind(name);
    return it != m_commands.constEnd() ? it->desc : QString();
}

void CommandEngine::startMacro(const QString &name) {
    m_recording = true;
    m_currentMacro = name;
    m_macros[name].clear();
}

void CommandEngine::stopMacro() {
    if (m_recording) {
        emit macroRecorded(m_currentMacro, m_macros[m_currentMacro].size());
        m_recording = false;
    }
}

void CommandEngine::replayMacro(const QString &name) {
    auto it = m_macros.constFind(name);
    if (it == m_macros.constEnd()) {
        emit error(tr("Macro not found: %1").arg(name));
        return;
    }
    m_replayQueue.clear();
    for (const auto &step : it.value()) m_replayQueue.enqueue(step);
    while (!m_replayQueue.isEmpty()) {
        auto step = m_replayQueue.dequeue();
        execute(step.cmd, step.input);
    }
    emit macroReplayFinished(name);
}

QStringList CommandEngine::macroNames() const { return m_macros.keys(); }
