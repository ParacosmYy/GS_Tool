/**
 * @file ProtocolEngine.cpp
 * @brief 自定义协议解析引擎实现
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
ProtocolEngine::ProtocolEngine(QObject *parent)
    : QObject(parent)
    , m_schema(nullptr)
{
}

/**
 * @brief 析构函数
 */
ProtocolEngine::~ProtocolEngine() = default;

/**
 * @brief 设置协议帧结构定义
 *
 * 将引擎绑定到指定的 ProtocolSchema，后续 feedData() 调用
 * 将根据该定义进行帧解析。传入 nullptr 可清除当前定义。
 *
 * @param schema 协议定义对象指针
 */
void ProtocolEngine::setSchema(ProtocolSchema *schema)
{
    m_schema = schema;
}

/**
 * @brief 向引擎喂入新的串口数据
 *
 * 将数据追加到内部缓冲区，然后尝试按当前 schema
 * 定义的帧格式进行帧同步与解析。解析成功后发射
 * frameParsed() 信号，失败时发射 parseError()。
 *
 * @param data 新接收到的原始字节流
 */
void ProtocolEngine::feedData(const QByteArray &data)
{
    Q_UNUSED(data)
    // TODO: 追加到 m_buffer，尝试帧同步/解析
}

/**
 * @brief 重置解析状态
 *
 * 清空内部接收缓冲区，将解析器状态恢复到初始。
 * 不会清除当前 schema 设置。
 */
void ProtocolEngine::reset()
{
    m_buffer.clear();
}

/**
 * @brief 获取当前使用的协议定义
 * @return 协议定义指针，未设置时为 nullptr
 */
ProtocolSchema *ProtocolEngine::currentSchema() const
{
    return m_schema;
}
