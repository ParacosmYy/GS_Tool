/**
 * @file ProtocolEngine.cpp
 * @brief 自定义协议解析引擎核心实现
 *
 * 接收原始串口字节流，根据 ProtocolSchema 定义的帧格式
 * 自动完成帧同步、长度解析、字段提取。
 *
 * 解析流程：
 * 1. 数据追加到内部缓冲区
 * 2. 在缓冲区中搜索帧头
 * 3. 读取长度字段，判断帧是否完整
 * 4. 提取完整帧，按字段定义解析各字段值
 * 5. 发射 frameParsed 信号
 *
 * 统计接口见 ProtocolEngineStats.cpp
 * 校验和计算见 ProtocolEngineChecksum.cpp
 */

#include "protocol/engine/ProtocolEngine.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QDataStream>
#include <QDateTime>
#include <QtMath>

/** @brief 缓冲区最大容量 64KB，防止内存膨胀(ProtocolEngineCore.cpp使用) */
static constexpr int MAX_BUFFER_SIZE = 65536;

/** @brief 构造函数 @param parent 父对象指针 */
ProtocolEngine::ProtocolEngine(QObject *parent)
    : QObject(parent)
    , m_schema(nullptr)
    , m_parseErrors(0)
    , m_checksumAlgorithm(ChecksumAlgorithm::Auto)
    , m_framesParsed(0)
    , m_framesRejected(0)
    , m_totalBytesProcessed(0)
    , m_totalValidations(0)
    , m_totalParseErrors(0)
    , m_lastParseTimestamp(0)
    , m_totalCrcErrors(0)
    , m_totalBytesParsed(0)
    , m_crcPassCount(0)
    , m_crcFailCount(0)
    , m_totalValidationPasses(0)
    , m_totalValidationFailures(0)
    , m_totalCrcChecks(0)
{
}

/** @brief 析构函数 */
ProtocolEngine::~ProtocolEngine() = default;

/** @brief 设置协议帧结构定义(绑定schema→自动reset) @param schema 协议定义对象指针 */
void ProtocolEngine::setSchema(ProtocolSchema *schema)
{
    m_schema = schema;
    reset();
}

/** @brief 重置解析状态(清空缓冲区+重置计数器，不清除schema和算法配置) */
void ProtocolEngine::reset()
{
    m_buffer.clear();
    m_framesParsed = 0;
    m_parseErrors = 0;
    m_framesRejected = 0;
    m_totalBytesProcessed = 0;
    m_totalValidations = 0;
    m_totalParseErrors = 0;
    m_lastParseTimestamp = 0;
    m_totalCrcErrors = 0;
    m_totalBytesParsed = 0;
    m_crcPassCount = 0;
    m_crcFailCount = 0;
    m_totalValidationPasses = 0;
    m_totalValidationFailures = 0;
    m_totalCrcChecks = 0;
    m_totalParses = 0;
    m_totalChecksums = 0;
    m_totalMatches = 0;
    m_totalBuilds = 0;
}

/** @brief 获取当前使用的协议定义 @return 协议定义指针，未设置时为 nullptr */
ProtocolSchema *ProtocolEngine::currentSchema() const
{
    return m_schema;
}

// ---- 帧解析核心循环(feedData/tryParseOneFrame)见 ProtocolEngineCore.cpp ----

// findHeader/trimBufferBeforePartialHeader/readLengthField/extractField 实现已拆分至 ProtocolEngineParsing.cpp
