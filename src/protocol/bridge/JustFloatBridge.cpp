/**
 * @file JustFloatBridge.cpp
 * @brief JustFloat协议桥实现 — VOFA+小端浮点字节流解析器
 *
 * 将小端浮点字节流（4字节float + 4字节尾标记）解析为通道数据，
 * 兼容VOFA+的JustFloat协议规范。
 */
#include "protocol/bridge/JustFloatBridge.h"

#include <cstring>
#include <QtMath>

// ============================================================
// JustFloat尾部标记常量定义
// ============================================================
constexpr unsigned char JustFloatBridge::kTailMarker[4];

// ============================================================
// 构造 / 析构
// ============================================================

/** @brief 构造JustFloat协议桥 @param parent 父对象 */
JustFloatBridge::JustFloatBridge(QObject* parent)
    : IProtocolBridge(parent)
    , m_channelCount(0)
    , m_channelsDetected(false)
{
}

// ============================================================
// IProtocolBridge接口实现
// ============================================================

/** @brief 喂入原始字节流(追加缓冲区→溢出保护→循环解析完整帧) @param data 原始字节流 */
void JustFloatBridge::feed(const QByteArray& data)
{
    if (data.isEmpty()) return;

    // 追加到缓冲区
    m_buffer.append(data);

    // 缓冲区溢出保护: 超过最大限制时丢弃最旧的数据
    if (m_buffer.size() > kMaxBufferSize) {
        int excess = m_buffer.size() - kMaxBufferSize;
        m_buffer.remove(0, excess);
    }

    // 循环尝试解析所有完整帧
    int consumed = 0;
    while ((consumed = tryParseFrame()) > 0) {
        m_buffer.remove(0, consumed);
    }
}

/** @brief 重置解析器状态(清空缓冲区+通道计数+检测标志) */
void JustFloatBridge::reset()
{
    m_buffer.clear();
    m_channelCount = 0;
    m_channelsDetected = false;
}

/** @brief 获取协议名称 @return "JustFloat" */
QString JustFloatBridge::name() const
{
    return QStringLiteral("JustFloat");
}

// ============================================================
// 通道配置
// ============================================================

/** @brief 手动设置固定通道数(覆盖自动检测) @param count 通道数(0=自动检测) */
void JustFloatBridge::setFixedChannelCount(int count)
{
    if (count < 0) count = 0;
    if (count > kMaxChannels) count = kMaxChannels;
    m_channelCount = count;
    m_channelsDetected = (count > 0);
}

/** @brief 获取当前通道数 @return 通道数 */
int JustFloatBridge::channelCount() const
{
    return m_channelCount;
}

// ============================================================
// 帧解析核心逻辑
// ============================================================

/** @brief 尝试从缓冲区解析一帧(搜索尾部标记00 00 80 7F) @return 消耗的字节数，0=无完整帧 */
int JustFloatBridge::tryParseFrame()
{
    // 尾部标记至少需要4字节
    if (m_buffer.size() < kTailSize) {
        return 0;
    }

    // 在缓冲区中搜索尾部标记 00 00 80 7F
    // 使用简单的逐字节扫描，避免复杂的模式匹配
    const char* data = m_buffer.constData();
    int dataSize = m_buffer.size();

    for (int i = 0; i <= dataSize - kTailSize; ++i) {
        // 检查尾部标记的4个字节
        bool isTail = (static_cast<unsigned char>(data[i])     == kTailMarker[0] &&
                       static_cast<unsigned char>(data[i + 1]) == kTailMarker[1] &&
                       static_cast<unsigned char>(data[i + 2]) == kTailMarker[2] &&
                       static_cast<unsigned char>(data[i + 3]) == kTailMarker[3]);

        if (!isTail) continue;

        // 找到尾部标记: 从缓冲区起始到尾部标记结束为一帧
        int frameSize = i + kTailSize;
        int floatPayloadSize = i;  // 尾部标记前的float数据字节数

        // float数据必须是4字节的整数倍
        if (floatPayloadSize % kFloatSize != 0 || floatPayloadSize == 0) {
            // 帧数据不合法（不是4字节对齐或无数据），跳过这个假尾部
            continue;
        }

        int detectedChannels = floatPayloadSize / kFloatSize;

        // 通道数上限保护
        if (detectedChannels > kMaxChannels) {
            continue;
        }

        // 如果通道数尚未检测，进行自动检测
        if (!m_channelsDetected) {
            autoDetectChannels(floatPayloadSize);
        }

        // 通道数不匹配（后续帧的通道数必须与第一帧一致）
        if (detectedChannels != m_channelCount) {
            // 通道数变化，可能是残缺帧，跳过
            continue;
        }

        // 解析并发射帧数据
        parseAndEmit(frameSize);
        return frameSize;
    }

    return 0;
}

/** @brief 从缓冲区解析float数据并发射frameParsed信号 @param frameSize 帧总字节数(含尾部标记) */
void JustFloatBridge::parseAndEmit(int frameSize)
{
    const char* data = m_buffer.constData();

    // 解析每个通道的float值（小端序）
    QVariantMap fields;
    for (int ch = 0; ch < m_channelCount; ++ch) {
        const char* ptr = data + ch * kFloatSize;

        // 小端IEEE 754 float: 使用memcpy避免对齐问题
        float value;
        std::memcpy(&value, ptr, sizeof(float));

        // 通道命名: CH1, CH2, ..., CHn
        QString channelName = QStringLiteral("CH%1").arg(ch + 1);
        fields[channelName] = static_cast<double>(value);
    }

    // 提取原始帧数据（用于ProtocolView的HEX显示）
    QByteArray rawFrame = m_buffer.left(frameSize);

    // 发射与FrameParser::frameParsed完全兼容的信号
    emit frameParsed(fields, rawFrame);
}

/** @brief 自动检测通道数(由首帧的float字节数决定) @param floatPayloadSize 首帧float数据字节数 */
void JustFloatBridge::autoDetectChannels(int floatPayloadSize)
{
    m_channelCount = floatPayloadSize / kFloatSize;
    m_channelsDetected = true;
}
