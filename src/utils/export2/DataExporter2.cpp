#include "utils/export2/DataExporter2.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>

DataExporter::DataExporter(QObject *parent) : QObject(parent) {}
DataExporter::~DataExporter() = default;

bool DataExporter::exportToFile(const QString &path, const QByteArray &data, Format fmt) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) { emit exportError(path, f.errorString()); return false; }
    QByteArray out = convert(data, fmt);
    f.write(out);
    f.close();
    emit exportComplete(path, out.size());
    return true;
}

bool DataExporter::exportToFile(const QString &path, const QList<QList<QByteArray>> &rows, Format fmt) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) { emit exportError(path, f.errorString()); return false; }
    QTextStream out(&f);
    if (fmt == Csv) { for (const auto &row : rows) { QStringList cells; for (const auto &c : row) cells << c; out << cells.join(",") << "\n"; } }
    else if (fmt == Tsv) { for (const auto &row : rows) { QStringList cells; for (const auto &c : row) cells << c; out << cells.join("\t") << "\n"; } }
    else if (fmt == Json) { QJsonArray arr; for (const auto &row : rows) { QJsonArray r; for (const auto &c : row) r.append(QString::fromUtf8(c)); arr.append(r); } out << QJsonDocument(arr).toJson(); }
    f.close();
    emit exportComplete(path, 0);
    return true;
}

QByteArray DataExporter::convert(const QByteArray &data, Format fmt) const {
    switch (fmt) {
    case Hex: return data.toHex(' ');
    case Base64: return data.toBase64();
    case Binary: return data;
    case Json: { QJsonObject obj; obj["data"] = QString::fromLatin1(data.toHex()); obj["size"] = data.size(); return QJsonDocument(obj).toJson(); }
    default: return data;
    }
}

void DataExporter::setIncludeHeader(bool i) { m_includeHeader = i; }
void DataExporter::setTimestampColumn(bool t) { m_timestampCol = t; }
void DataExporter::setEncoding(const QString &c) { m_encoding = c; }
