#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QList>
#include <QMap>

class ProtocolAnalyzer : public QObject {
    Q_OBJECT
public:
    struct Field {
        QString name;
        int offset;
        int bitLength;
        QByteArray value;
        QString displayValue;
        QString description;
    };
    struct ProtocolMessage {
        QByteArray rawData;
        QString protocolName;
        QList<Field> fields;
        bool valid = false;
        QString errorMessage;
    };
    explicit ProtocolAnalyzer(QObject *parent = nullptr);
    ~ProtocolAnalyzer() override;
    void setProtocolTemplate(const QString &name, const QList<Field> &tpl);
    void removeTemplate(const QString &name);
    ProtocolMessage parse(const QByteArray &data, const QString &tplName);
    QList<ProtocolMessage> parseStream(const QByteArray &data, const QString &tplName, int frameSize);
    QStringList templates() const;
    void clearTemplates();
signals:
    void messageParsed(const ProtocolMessage &msg);
    void parseError(const QString &tplName, const QString &error);
private:
    QMap<QString, QList<Field>> m_templates;
};
