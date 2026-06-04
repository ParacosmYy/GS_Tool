/**
 * @file PacketReassembler.cpp
 * @brief 数据包重组引擎实现 - 构造/配置/数据输入/模式处理
 *
 * 核心流程:
 *   1. feedData() 追加数据到缓冲区
 *   2. 根据当前 DelimiterMode 分发到对应处理方法
 *   3. 检测到完整包后调用 emitPacket() 发射信号
 *   4. 超时检测由独立 QTimer 驱动
 *
 * 统计相关方法见 PacketReassemblerStats.cpp。
 */

#include "protocol/reassembly/PacketReassembler.h"

#include <QDateTime>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造重组引擎(初始化默认配置和超时定时器) @param parent 父对象 */
PacketReassembler::PacketReassembler(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    m_timeoutTimer->setSingleShot(true);
    connect(m_timeoutTimer, &QTimer::timeout,
            this, [this]() { checkTimeout(); });
}

/** @brief 析构(停止超时定时器) */
PacketReassembler::~PacketReassembler()
{
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}

// ============================================================================
// 配置接口
// ============================================================================

/**
 * @brief 设置重组配置
 * @param config 新的重组配置
 *
 * 验证配置参数合法性后应用,并重置缓冲区状态。
 * 长度字段大小仅允许1/2/4字节,最大包大小限制在硬性上限内。
 */
void PacketReassembler::setConfig(const ReassemblyConfig& config)
{
    /* 验证长度字段大小 */
    if (config.lengthFieldSize != 1 &&
        config.lengthFieldSize != 2 &&
        config.lengthFieldSize != 4) {
        return;
    }

    /* 限制最大包大小 */
    m_config = config;
    if (m_config.maxPacketSize <= 0 ||
        m_config.maxPacketSize > kHardMaxPacketSize) {
        m_config.maxPacketSize = kHardMaxPacketSize;
    }

    /* 固定帧头模式需要帧头数据 */
    if (m_config.mode == DelimiterMode::FixedHeader &&
        m_config.startMarker.isEmpty() &&
        m_config.headerSize <= 0) {
        return; /* 配置无效,至少需要startMarker或headerSize */
    }

    /* 起止标记模式需要起止标记 */
    if (m_config.mode == DelimiterMode::StartEndMarkers &&
        (m_config.startMarker.isEmpty() || m_config.endMarker.isEmpty())) {
        return;
    }

    reset();
}

/** @brief 获取当前重组配置 @return 配置的const引用 */
const PacketReassembler::ReassemblyConfig& PacketReassembler::config() const
{
    return m_config;
}

// ============================================================================
// 数据输入
// ============================================================================

/**
 * @brief 喂入数据到重组引擎
 * @param data 输入的字节流
 *
 * 追加数据到内部缓冲区,更新统计,然后根据当前模式分发处理。
 * 每次调用视为一个分片(更新分片计数)。
 */
void PacketReassembler::feedData(const QByteArray& data)
{
    if (data.isEmpty()) {
        return;
    }

    /* 检查超大包 - 单次输入超过最大包大小则直接丢弃 */
    if (m_buffer.size() + data.size() > m_config.maxPacketSize) {
        m_stats.totalOversizeDrops++;
        emit packetDropped(2); /* 原因: 超大 */
        dropBuffer(2);
        return;
    }

    /* 首个分片: 记录时间并启动计时器 */
    if (m_buffer.isEmpty()) {
        m_firstFragmentTime = QDateTime::currentMSecsSinceEpoch();
        m_assemblyTimer.start();
        m_currentFragmentCount = 0;
    }

    /* 追加到缓冲区 */
    m_buffer.append(data);
    m_currentFragmentCount++;

    /* 更新统计 */
    m_stats.totalFragmentsReceived++;
    m_stats.totalBytesProcessed += static_cast<quint64>(data.size());
    m_stats.currentBufferSize = m_buffer.size();

    emit fragmentReceived(data.size());

    /* 根据模式分发处理 */
    switch (m_config.mode) {
    case DelimiterMode::FixedHeader:
        processFixedHeader();
        break;
    case DelimiterMode::StartEndMarkers:
        processStartEndMarkers();
        break;
    case DelimiterMode::LengthField:
        processLengthField();
        break;
    case DelimiterMode::Timeout:
        processTimeout();
        break;
    case DelimiterMode::Manual:
        /* 手动模式: 不自动处理,等待外部调用者控制 */
        break;
    }
}

// ============================================================================
// 模式处理: 固定帧头
// ============================================================================

/**
 * @brief 固定帧头模式处理
 *
 * 查找缓冲区中的帧头位置,根据帧头大小和配置的固定长度确定包边界。
 * 如果未配置固定长度(fixedPacketLength=0),则尝试从帧头后读取长度字段。
 */
void PacketReassembler::processFixedHeader()
{
    while (!m_buffer.isEmpty()) {
        /* 查找帧头位置 */
        int headerPos = m_buffer.indexOf(m_config.startMarker);
        if (headerPos < 0) {
            /* 未找到帧头: 保留尾部可能的帧头前缀 */
            int keepBytes = m_config.startMarker.size() - 1;
            if (m_buffer.size() > keepBytes && keepBytes > 0) {
                m_buffer = m_buffer.right(keepBytes);
                m_stats.currentBufferSize = m_buffer.size();
            }
            return;
        }

        /* 丢弃帧头之前的垃圾数据 */
        if (headerPos > 0) {
            m_buffer.remove(0, headerPos);
            m_stats.currentBufferSize = m_buffer.size();
        }

        /* 确定期望长度 */
        if (m_expectedLength <= 0) {
            if (m_config.fixedPacketLength > 0) {
                m_expectedLength = m_config.fixedPacketLength;
            } else {
                /* 从帧头后读取长度 */
                int headerLen = m_config.headerSize > 0 ?
                                    m_config.headerSize :
                                    m_config.startMarker.size();
                if (m_buffer.size() < headerLen + m_config.lengthFieldSize) {
                    return; /* 数据不足,等待更多数据 */
                }
                m_expectedLength = readLengthField(
                    m_buffer, headerLen + m_config.lengthFieldOffset,
                    m_config.lengthFieldSize);

                if (m_config.lengthIncludesHeader) {
                    /* 长度已包含帧头 */
                } else {
                    m_expectedLength += headerLen;
                }
            }
        }

        /* 检查数据是否完整 */
        if (m_expectedLength > 0 &&
            m_buffer.size() >= m_expectedLength) {
            QByteArray packetData = m_buffer.left(m_expectedLength);
            m_buffer.remove(0, m_expectedLength);
            m_expectedLength = -1;
            m_stats.currentBufferSize = m_buffer.size();
            emitPacket(packetData);
        } else {
            /* 数据不足,等待更多数据 */
            /* 启动/重启超时定时器 */
            if (m_config.timeoutMs > 0) {
                m_timeoutTimer->start(m_config.timeoutMs);
            }
            return;
        }
    }
}

// ============================================================================
// 模式处理: 起止标记
// ============================================================================

/**
 * @brief 起止标记模式处理
 *
 * 在缓冲区中查找起始标记和结束标记,提取两者之间的完整数据。
 * 支持在同一个缓冲区中检测多个连续的包。
 */
void PacketReassembler::processStartEndMarkers()
{
    while (!m_buffer.isEmpty()) {
        /* 查找起始标记 */
        int startPos = m_buffer.indexOf(m_config.startMarker);
        if (startPos < 0) {
            /* 未找到起始标记: 保留尾部可能的起始标记前缀 */
            int keepBytes = m_config.startMarker.size() - 1;
            if (m_buffer.size() > keepBytes && keepBytes > 0) {
                m_buffer = m_buffer.right(keepBytes);
                m_stats.currentBufferSize = m_buffer.size();
            }
            return;
        }

        /* 丢弃起始标记之前的垃圾数据 */
        if (startPos > 0) {
            m_buffer.remove(0, startPos);
            m_stats.currentBufferSize = m_buffer.size();
        }

        /* 跳过起始标记,查找结束标记 */
        int searchStart = m_config.startMarker.size();
        int endPos = m_buffer.indexOf(m_config.endMarker, searchStart);
        if (endPos < 0) {
            /* 未找到结束标记: 等待更多数据 */
            if (m_config.timeoutMs > 0) {
                m_timeoutTimer->start(m_config.timeoutMs);
            }
            return;
        }

        /* 提取完整数据包(包含起止标记) */
        int packetEnd = endPos + m_config.endMarker.size();
        QByteArray packetData = m_buffer.left(packetEnd);
        m_buffer.remove(0, packetEnd);
        m_stats.currentBufferSize = m_buffer.size();

        emitPacket(packetData);
    }
}

// ============================================================================
// 模式处理: 长度字段
// ============================================================================

/**
 * @brief 长度字段模式处理
 *
 * 从缓冲区指定偏移处读取长度字段,等待足够字节后提取完整包。
 * 长度字段支持1/2/4字节,大端序。
 */
void PacketReassembler::processLengthField()
{
    while (!m_buffer.isEmpty()) {
        /* 检查是否有足够数据读取长度字段 */
        int minHeaderSize = m_config.lengthFieldOffset + m_config.lengthFieldSize;
        if (m_buffer.size() < minHeaderSize) {
            /* 数据不足,等待更多数据 */
            if (m_config.timeoutMs > 0) {
                m_timeoutTimer->start(m_config.timeoutMs);
            }
            return;
        }

        /* 读取期望长度 */
        if (m_expectedLength <= 0) {
            m_expectedLength = readLengthField(
                m_buffer, m_config.lengthFieldOffset,
                m_config.lengthFieldSize);

            if (m_expectedLength <= 0) {
                /* 长度字段值为0或无效,丢弃一个字节后重试 */
                m_buffer.remove(0, 1);
                m_stats.currentBufferSize = m_buffer.size();
                continue;
            }

            /* 检查超大包 */
            if (m_expectedLength > m_config.maxPacketSize) {
                m_stats.totalOversizeDrops++;
                emit packetDropped(2);
                dropBuffer(2);
                return;
            }
        }

        /* 检查数据是否完整 */
        if (m_buffer.size() >= m_expectedLength) {
            QByteArray packetData = m_buffer.left(m_expectedLength);
            m_buffer.remove(0, m_expectedLength);
            m_expectedLength = -1;
            m_stats.currentBufferSize = m_buffer.size();
            emitPacket(packetData);
        } else {
            /* 数据不足,等待更多数据 */
            if (m_config.timeoutMs > 0) {
                m_timeoutTimer->start(m_config.timeoutMs);
            }
            return;
        }
    }
}

// ============================================================================
// 模式处理: 超时
// ============================================================================

/**
 * @brief 超时模式处理
 *
 * 启动/重启超时定时器,当定时器到期时将缓冲区中的所有数据作为一个包发出。
 * 每次feedData()都会重启定时器,实现"收到数据后等待一段静默期"的语义。
 */
void PacketReassembler::processTimeout()
{
    if (m_config.timeoutMs > 0) {
        m_timeoutTimer->start(m_config.timeoutMs);
    }
}

// ============================================================================
// 超时检测
// ============================================================================

/**
 * @brief 检查是否超时
 * @return true=发生超时并丢弃了缓冲数据
 *
 * 仅在缓冲区非空且正在重组时触发超时处理。
 * 超时后将缓冲区内容作为一个包发出(Timeout模式)或直接丢弃(其他模式)。
 */
bool PacketReassembler::checkTimeout()
{
    if (m_buffer.isEmpty()) {
        return false;
    }

    /* 超时模式: 超时后将缓冲区内容作为完整包发出 */
    if (m_config.mode == DelimiterMode::Timeout) {
        if (!m_buffer.isEmpty()) {
            QByteArray packetData;
            packetData.swap(m_buffer);
            m_stats.currentBufferSize = 0;
            emitPacket(packetData);
        }
        return true;
    }

    /* 其他模式: 超时视为丢包 */
    int lostBytes = m_buffer.size();
    m_stats.totalTimeoutDrops++;
    emit reassemblyTimeout(lostBytes);
    emit packetDropped(1); /* 原因: 超时 */
    dropBuffer(1);
    return true;
}

// ============================================================================
// 辅助方法
// ============================================================================

/**
 * @brief 发送重组完成的数据包
 * @param packetData 完整的包数据
 *
 * 创建 ReassembledPacket,更新统计(平均值/峰值),发射 packetReassembled 信号。
 */
void PacketReassembler::emitPacket(const QByteArray& packetData)
{
    /* 停止超时定时器 */
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    qint64 assemblyMs = m_assemblyTimer.elapsed();

    ReassembledPacket packet;
    packet.data = packetData;
    packet.fragmentCount = m_currentFragmentCount;
    packet.firstFragmentTime = m_firstFragmentTime;
    packet.lastFragmentTime = now;
    packet.assemblyTimeMs = assemblyMs;

    /* 更新累计统计 */
    m_stats.totalPacketsReassembled++;
    m_sumFragmentsPerPacket += static_cast<quint64>(m_currentFragmentCount);
    m_sumAssemblyTimeMs += static_cast<quint64>(assemblyMs);

    /* 计算平均值 */
    if (m_stats.totalPacketsReassembled > 0) {
        m_stats.avgFragmentsPerPacket =
            static_cast<double>(m_sumFragmentsPerPacket) /
            static_cast<double>(m_stats.totalPacketsReassembled);
        m_stats.avgAssemblyTimeMs =
            static_cast<double>(m_sumAssemblyTimeMs) /
            static_cast<double>(m_stats.totalPacketsReassembled);
    }

    /* 更新峰值 */
    if (m_currentFragmentCount > m_stats.peakFragmentCount) {
        m_stats.peakFragmentCount = m_currentFragmentCount;
    }

    /* 重置分片状态 */
    m_currentFragmentCount = 0;
    m_firstFragmentTime = 0;
    m_expectedLength = -1;
    m_assemblyTimer.invalidate();

    emit packetReassembled(packet);
}

/**
 * @brief 丢弃缓冲区数据
 * @param reason 丢弃原因(1=超时 2=超大 3=标记错误)
 *
 * 清空缓冲区并重置所有重组中间状态,但不清零累计统计。
 */
void PacketReassembler::dropBuffer(int reason)
{
    m_buffer.clear();
    m_expectedLength = -1;
    m_firstFragmentTime = 0;
    m_currentFragmentCount = 0;
    m_assemblyTimer.invalidate();
    m_stats.currentBufferSize = 0;

    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    Q_UNUSED(reason)
}

/**
 * @brief 从数据中读取长度字段(大端序)
 * @param data 数据缓冲区
 * @param offset 长度字段偏移
 * @param fieldSize 字段大小(1/2/4字节)
 * @return 解析得到的长度值, 失败返回-1
 */
int PacketReassembler::readLengthField(const QByteArray& data,
                                        int offset,
                                        int fieldSize) const
{
    if (offset < 0 || offset + fieldSize > data.size()) {
        return -1;
    }

    const unsigned char* ptr =
        reinterpret_cast<const unsigned char*>(data.constData() + offset);

    switch (fieldSize) {
    case 1:
        return static_cast<int>(ptr[0]);
    case 2:
        return static_cast<int>((static_cast<quint16>(ptr[0]) << 8) |
                                static_cast<quint16>(ptr[1]));
    case 4:
        return static_cast<int>((static_cast<quint32>(ptr[0]) << 24) |
                                (static_cast<quint32>(ptr[1]) << 16) |
                                (static_cast<quint32>(ptr[2]) << 8) |
                                static_cast<quint32>(ptr[3]));
    default:
        return -1;
    }
}

// ============================================================================
// 状态查询
// ============================================================================

/** @brief 是否正在重组(缓冲区非空) @return true=正在重组 */
bool PacketReassembler::isReassembling() const
{
    return !m_buffer.isEmpty();
}

/** @brief 缓冲区中的字节数 @return 缓冲区大小 */
int PacketReassembler::bufferedBytes() const
{
    return m_buffer.size();
}

/**
 * @brief 重置重组状态(清空缓冲区, 不清零统计)
 *
 * 停止超时定时器,清空缓冲区,重置分片计数和期望长度。
 * 统计计数器不受影响,需单独调用 resetStatistics()。
 */
void PacketReassembler::reset()
{
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
    m_buffer.clear();
    m_expectedLength = -1;
    m_firstFragmentTime = 0;
    m_currentFragmentCount = 0;
    m_assemblyTimer.invalidate();
    m_stats.currentBufferSize = 0;
}

/** @brief 获取运行统计 @return Stats的const引用 */
const PacketReassembler::Stats& PacketReassembler::stats() const
{
    return m_stats;
}
