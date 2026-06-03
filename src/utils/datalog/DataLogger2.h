#pragma once
#include <QObject>
#include <QFile>
#include <QString>
#include <QByteArray>
#include <QMap>

class DataLogger : public QObject {
    Q_OBJECT
public:
    enum Level { Debug, Info, Warning, Error };
    Q_ENUM(Level)
    explicit DataLogger(QObject *parent = nullptr);
    ~DataLogger() override;
    bool open(const QString &filePath);
    void close();
    void log(Level level, const QString &message);
    void logData(const QByteArray &data, const QString &tag = "");
    void setLevel(Level minLevel);
    void setAutoFlush(bool enable);
    void setMaxFileSize(qint64 bytes);
    qint64 fileSize() const;
    bool isOpen() const;
    int entryCount() const;
signals:
    void logged(Level level, const QString &message);
    void fileOpened(const QString &path);
    void fileClosed();
    void fileFull(const QString &path);
private:
    void writeEntry(const QString &line);
    QFile m_file;
    Level m_minLevel = Debug;
    bool m_autoFlush = true;
    qint64 m_maxSize = 100 * 1024 * 1024;
    int m_count = 0;
};
