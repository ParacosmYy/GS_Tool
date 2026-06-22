/**
 * @file IProtocolParser.h
 * @brief 协议解析接口
 */
#ifndef IPROTOCOL_PARSER_H
#define IPROTOCOL_PARSER_H
#include <QByteArray>
#include <QJsonObject>
#include <QString>
class IProtocolParser {
public:
    virtual ~IProtocolParser() = default;
    virtual QString protocolName() const = 0;
    virtual bool parse(const QByteArray& raw, QJsonObject& result) = 0;
    virtual QByteArray serialize(const QJsonObject& data) = 0;
    virtual bool validate(const QByteArray& raw) = 0;
};
#endif
