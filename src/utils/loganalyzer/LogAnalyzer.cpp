#include "utils/loganalyzer/LogAnalyzer.h"

LogAnalyzer::LogAnalyzer(QObject *parent) : QObject(parent) {}
LogAnalyzer::~LogAnalyzer() = default;

void LogAnalyzer::loadContent(const QString &content) {
    m_rawLines = content.split('\n');
    parseEntries();
    m_filtered = m_allEntries;
    emit contentLoaded(m_allEntries.size());
}

void LogAnalyzer::setFilterPattern(const QString &pattern) {
    m_filterPattern.setPattern(pattern);
    m_filterPattern.optimize();
    QList<LogEntry> result;
    for (const auto &e : m_allEntries) {
        if (e.level >= m_minLevel) {
            if (m_filterPattern.pattern().isEmpty()
                || m_filterPattern.match(e.message).hasMatch())
                result.append(e);
        }
    }
    m_filtered = result;
    emit filterChanged(m_filtered.size());
}

void LogAnalyzer::setLogLevelFilter(LogLevel minLevel) {
    m_minLevel = minLevel;
    setFilterPattern(m_filterPattern.pattern());
}

QList<LogAnalyzer::LogEntry> LogAnalyzer::filteredEntries() const { return m_filtered; }

QList<LogAnalyzer::LogEntry> LogAnalyzer::search(const QString &keyword, bool cs) const {
    QList<LogEntry> result;
    Qt::CaseSensitivity sens = cs ? Qt::CaseSensitive : Qt::CaseInsensitive;
    for (const auto &e : m_allEntries) {
        if (e.message.contains(keyword, sens)) {
            result.append(e);
            emit entryFound(e);
        }
    }
    return result;
}

QMap<LogAnalyzer::LogLevel, int> LogAnalyzer::levelCounts() const {
    QMap<LogLevel, int> counts;
    for (const auto &e : m_allEntries) counts[e.level]++;
    return counts;
}

int LogAnalyzer::totalEntries() const { return m_allEntries.size(); }
int LogAnalyzer::filteredCount() const { return m_filtered.size(); }

QStringList LogAnalyzer::sources() const {
    QStringList srcs;
    for (const auto &e : m_allEntries)
        if (!e.source.isEmpty() && !srcs.contains(e.source)) srcs << e.source;
    return srcs;
}

void LogAnalyzer::clear() {
    m_rawLines.clear(); m_allEntries.clear(); m_filtered.clear();
}

void LogAnalyzer::parseEntries() {
    m_allEntries.clear();
    int lineNum = 0;
    for (const auto &line : m_rawLines) {
        lineNum++;
        if (line.trimmed().isEmpty()) continue;
        LogEntry entry;
        entry.lineNumber = lineNum;
        entry.level = detectLevel(line);
        entry.timestamp = extractTimestamp(line);
        entry.message = line.trimmed();
        m_allEntries.append(entry);
    }
}

LogAnalyzer::LogLevel LogAnalyzer::detectLevel(const QString &line) const {
    QString l = line.toLower();
    if (l.contains("fatal") || l.contains("critical")) return Fatal;
    if (l.contains("error") || l.contains("err:")) return Error;
    if (l.contains("warn")) return Warning;
    if (l.contains("info")) return Info;
    return Debug;
}

QString LogAnalyzer::extractTimestamp(const QString &line) const {
    static QRegularExpression re("(\d{4}[-/]\d{2}[-/]\d{2}[\sT]\d{2}:\d{2}:\d{2})");
    auto m = re.match(line);
    return m.hasMatch() ? m.captured(1) : QString();
}
