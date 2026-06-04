/**
 * @file IProtocolParser.h
 * @brief 协议解析器接口 — 自定义协议解析的统一契约
 *
 * 所有协议解析器通过此接口向上层提供统一的数据解析服务。
 * ProtocolEngine/ProtocolBridgeManager通过此接口调度不同解析器。
 * 层级: L0 纯虚接口层
 */
#ifndef IPROTOCOLPARSER_H
#define IPROTOCOLPARSER_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QString>

/** @brief 解析结果枚举 — 描述单次feed()调用的状态 */
enum class ParseStatus {
    Complete,       ///< 完整帧解析成功
    Partial,        ///< 数据不完整，需要更多
    Error,          ///< 解析错误
    Discarded       ///< 数据被丢弃
};

/**
 * @brief 协议解析器接口
 * 协作: ProtocolEngine(调度) / ProtocolBridgeManager(管理) / FrameParser(内置实现)
 */
class IProtocolParser : public QObject {
    Q_OBJECT
public:
    explicit IProtocolParser(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IProtocolParser() = default;
    virtual ParseStatus feed(const QByteArray& data) = 0; ///< 输入原始字节流
    virtual void reset() = 0;                              ///< 重置状态
    virtual QString protocolName() const = 0;              ///< 协议名称
    virtual QString protocolVersion() const = 0;           ///< 协议版本
    virtual void configure(const QVariantMap& c) = 0;      ///< 配置解析器
    virtual QVariantMap configuration() const = 0;         ///< 获取配置
    virtual bool isReady() const = 0;                      ///< 是否就绪
    virtual int bufferedBytes() const = 0;                 ///< 缓冲区大小
    virtual quint64 totalFramesParsed() const = 0;         ///< 成功解析帧数
    virtual quint64 totalErrors() const = 0;               ///< 解析错误数
    virtual quint64 totalBytesFed() const = 0;             ///< 输入字节总数
signals:
    void frameParsed(const QVariantMap& f, const QByteArray& raw); ///< 帧解析成功
    void parseError(const QString& reason, const QByteArray& d);   ///< 帧解析错误
    void parserStatusChanged(ParseStatus s);                       ///< 状态变化
};

Q_DECLARE_METATYPE(ParseStatus)
#endif // IPROTOCOLPARSER_H
