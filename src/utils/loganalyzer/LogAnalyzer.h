#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QRegularExpression>

class LogAnalyzer : public QObject {
    Q_OBJECT
public:
    enum LogLevel { Debug, Info, Warning, Error, Fatal };
    Q_ENUM(LogLevel)

    struct LogEntry {
        int lineNumber = 0;
        LogLevel level = Info;
        QString timestamp;
        QString message;
        QString source;
    };

    explicit LogAnalyzer(QObject *parent = nullptr);
    ~LogAnalyzer() override;

    void loadContent(const QString &content);
    void setFilterPattern(const QString &pattern);
    void setLogLevelFilter(LogLevel minLevel);
    QList<LogEntry> filteredEntries() const;
    QList<LogEntry> search(const QString &keyword, bool caseSensitive = false) const;
    QMap<LogLevel, int> levelCounts() const;
    int totalEntries() const;
    int filteredCount() const;
    QStringList sources() const;
    void clear();

signals:
    void contentLoaded(int entryCount);
    void filterChanged(int matchCount);
    void entryFound(const LogEntry &entry);

private:
    void parseEntries();
    LogLevel detectLevel(const QString &line) const;
    QString extractTimestamp(const QString &line) const;

    QStringList m_rawLines;
    QList<LogEntry> m_allEntries;
    QList<LogEntry> m_filtered;
    QRegularExpression m_filterPattern;
    LogLevel m_minLevel = Debug;
};
