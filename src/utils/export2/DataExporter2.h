#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QList>

class DataExporter : public QObject {
    Q_OBJECT
public:
    enum Format { Csv, Tsv, Json, Xml, Binary, Hex, Base64 };
    Q_ENUM(Format)
    explicit DataExporter(QObject *parent = nullptr);
    ~DataExporter() override;
    bool exportToFile(const QString &path, const QByteArray &data, Format fmt);
    bool exportToFile(const QString &path, const QList<QList<QByteArray>> &rows, Format fmt);
    QByteArray convert(const QByteArray &data, Format fmt) const;
    void setIncludeHeader(bool include);
    void setTimestampColumn(bool include);
    void setEncoding(const QString &codec);
signals:
    void exportComplete(const QString &path, int bytes);
    void exportError(const QString &path, const QString &error);
    void progress(int percent);
private:
    bool m_includeHeader = true;
    bool m_timestampCol = false;
    QString m_encoding = "UTF-8";
};
