#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QByteArray>
#include <functional>
#include <QQueue>

class CommandEngine : public QObject {
    Q_OBJECT
public:
    using CommandFunc = std::function<QByteArray(const QByteArray &input)>;
    explicit CommandEngine(QObject *parent = nullptr);
    ~CommandEngine() override;

    void registerCommand(const QString &name, const QString &description, CommandFunc fn);
    void unregisterCommand(const QString &name);
    QByteArray execute(const QString &name, const QByteArray &input);
    QStringList availableCommands() const;
    QString commandDescription(const QString &name) const;

    void startMacro(const QString &name);
    void stopMacro();
    void replayMacro(const QString &name);
    QStringList macroNames() const;

signals:
    void commandExecuted(const QString &name, const QByteArray &result);
    void macroRecorded(const QString &name, int commandCount);
    void macroReplayFinished(const QString &name);
    void error(const QString &message);

private:
    struct CmdEntry { CommandFunc fn; QString desc; };
    QMap<QString, CmdEntry> m_commands;
    bool m_recording = false;
    QString m_currentMacro;
    struct MacroStep { QString cmd; QByteArray input; };
    QMap<QString, QList<MacroStep>> m_macros;
    QQueue<MacroStep> m_replayQueue;
};
