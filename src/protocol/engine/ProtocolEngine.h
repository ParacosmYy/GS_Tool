/**
 * @file ProtocolEngine.h
 * @brief 自定义协议解析引擎
 *
 * 接收原始串口字节流，根据 ProtocolSchema 定义的帧格式
 * 自动完成帧同步、长度解析、校验及字段提取。
 */

#ifndef PROTOCOL_ENGINE_H
#define PROTOCOL_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>

class ProtocolSchema;

/**
 * @class ProtocolEngine
 * @brief 协议帧解析引擎
 *
 * 持续接收串口数据，按当前 ProtocolSchema 完成帧检测，
 * 解析成功后发射 frameParsed 信号，失败时发射 parseError。
 */
class ProtocolEngine : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit ProtocolEngine(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolEngine() override;

    /**
     * @brief 设置协议帧结构定义
     * @param schema 指向已加载的 ProtocolSchema 对象
     */
    void setSchema(ProtocolSchema *schema);

    /**
     * @brief 向引擎喂入新的串口数据
     * @param data 新接收到的原始字节流
     */
    void feedData(const QByteArray &data);

    /**
     * @brief 重置解析状态，清空内部缓冲区
     */
    void reset();

    /**
     * @brief 获取当前使用的协议定义
     * @return 协议定义指针，未设置时为 nullptr
     */
    ProtocolSchema *currentSchema() const;

signals:
    /**
     * @brief 帧解析完成信号
     * @param fields 字段名→字段值的映射
     * @param rawData 完整的原始帧字节
     */
    void frameParsed(const QVariantMap &fields, const QByteArray &rawData);

    /**
     * @brief 解析错误信号
     * @param error 错误描述
     */
    void parseError(const QString &error);

private:
    ProtocolSchema *m_schema = nullptr; ///< 当前协议定义
    QByteArray m_buffer;                ///< 内部接收缓冲区
};

#endif // PROTOCOL_ENGINE_H
