/**
 * @file PacketAssembler.cpp
 * @brief 数据包组装器实现 -- 构造/配置/数据输入/策略处理/超时检测
 *
 * 核心流程:
 *   1. feed() 接收分片，根据策略分发到对应处理方法
 *   2. 首个分片创建 AssemblyContext，后续分片追加
 *   3. 每次追加后检查是否所有分片已到齐，到齐则组装发射
 *   4. 超时定时器周期扫描，清理超时未完成包
 *
 * 统计相关逻辑内联在本文件中(updateStats)。
 */

#include "protocol/assembly/PacketAssembler.h"

#include <QDateTime>
#include <QSet>
#include <algorithm>

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造数据包组装器
 * @param parent 父对象
 *
 * 初始化默认配置，创建超时扫描定时器并连接槽。
 * 定时器间隔取 fragmentTimeoutMs 的一半以平衡精度与开销。
 */
PacketAssembler::PacketAssembler(QObject* parent)
    : QObject(parent)
    , m_timeoutTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("PacketAssembler"));
    setupTimeoutTimer();
}

/** @brief 析构(停止超时定时器) */
PacketAssembler::~PacketAssembler()
{
    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }
}

// ============================================================================
// 超时定时器初始化
// ============================================================================

/** @brief 初始化超时扫描定时器，间隔为配置超时的一半 */
void PacketAssembler::setupTimeoutTimer()
{
    m_timeoutTimer->setSingleShot(false);
    // 扫描间隔取超时阈值的一半，确保不会漏掉超时包
    int interval = qMax(100, m_config.fragmentTimeoutMs / 2);
    m_timeoutTimer->setInterval(interval);

    connect(m_timeoutTimer, &QTimer::timeout,
            this, &PacketAssembler::checkTimeouts);
}

// ============================================================================
// 配置接口
// ============================================================================

/**
 * @brief 设置组装器配置
 * @param config 新的组装器配置
 *
 * 验证参数合法性后应用。超时定时器间隔同步更新。
 * 最大缓冲区不超过硬性上限。
 */
void PacketAssembler::setConfig(const AssemblerConfig& config)
{
    m_config = config;
    // 限制缓冲区大小在硬性上限内
    m_config.maxBufferSize = qBound(1024, m_config.maxBufferSize,
                                    kHardMaxBufferSize);
    m_config.maxConcurrentPackets = qBound(1, m_config.maxConcurrentPackets, 1024);

    // 更新定时器间隔
    if (m_config.fragmentTimeoutMs > 0) {
        int interval = qMax(100, m_config.fragmentTimeoutMs / 2);
        m_timeoutTimer->setInterval(interval);
        if (!m_timeoutTimer->isActive()) {
            m_timeoutTimer->start();
        }
    } else {
        m_timeoutTimer->stop();
    }
}

/** @brief 获取当前配置 @return 当前配置的常量引用 */
const PacketAssembler::AssemblerConfig& PacketAssembler::config() const
{
    return m_config;
}

// ============================================================================
// 数据输入
// ============================================================================

/**
 * @brief 喂入一个分片
 * @param fragment 分片数据
 * @param sequenceNumber 分片序号(从0开始)
 * @param totalFragments 该包的总分片数
 *
 * 验证参数后更新统计，根据重组策略分发处理。
 * 序列号必须 >= 0，总分片数必须 >= 1。
 */
void PacketAssembler::feed(const QByteArray& fragment, int sequenceNumber, int totalFragments)
{
    // 参数校验
    if (sequenceNumber < 0 || totalFragments < 1) {
        ++m_stats.totalFragmentsDropped;
        return;
    }
    if (totalFragments > kMaxTotalFragments) {
        ++m_stats.totalFragmentsDropped;
        return;
    }
    if (sequenceNumber >= totalFragments) {
        ++m_stats.totalFragmentsDropped;
        return;
    }

    // 缓冲区限制预检
    if (!checkBufferLimits(fragment.size())) {
        ++m_stats.totalFragmentsDropped;
        return;
    }

    ++m_stats.totalFragmentsReceived;
    emit fragmentReceived(sequenceNumber);

    // 如果只有一个分片，直接完成
    if (totalFragments == 1) {
        m_completedPackets.append(fragment);
        ++m_stats.totalPacketsAssembled;
        m_sumAssemblyTimeMs += 0; // 单分片无需计算耗时
        emit packetAssembled(fragment);
        updateStats();
        return;
    }

    // 启动超时定时器(如果未运行且配置了超时)
    if (m_config.fragmentTimeoutMs > 0 && !m_timeoutTimer->isActive()) {
        m_timeoutTimer->start();
    }

    // 根据策略分发
    switch (m_config.strategy) {
    case ReassemblyStrategy::SequenceNumber:
        feedBySequenceNumber(fragment, sequenceNumber, totalFragments);
        break;
    case ReassemblyStrategy::Offset:
        feedByOffset(fragment, sequenceNumber, totalFragments);
        break;
    case ReassemblyStrategy::TimestampWindow:
        feedByTimestampWindow(fragment, sequenceNumber, totalFragments);
        break;
    }

    updateStats();
}

// ============================================================================
// 序列号策略
// ============================================================================

/**
 * @brief 按序列号重组处理
 * @param fragment 分片数据
 * @param seq 序列号
 * @param total 总分片数
 *
 * 序列号策略下，packetId 取第一个分片的序列号(固定为0)。
 * 所有序列号相同的包归入同一 AssemblyContext。
 */
void PacketAssembler::feedBySequenceNumber(const QByteArray& fragment, int seq, int total)
{
    // 在序列号策略下，packetId = 0 (第一个包从seq=0开始)
    // 若需要多包并发，使用 seq 所在的包范围
    // 简化实现: 每个包的第一个分片(seq=0)创建新上下文
    int packetId = 0;

    if (seq == 0) {
        // 首分片: 创建新的组装上下文或追加到已存在的
        if (!m_activePackets.contains(packetId)) {
            // 如果当前已有一个活跃包，先处理
            if (m_activePackets.size() >= m_config.maxConcurrentPackets) {
                evictOldestPacket();
            }
            AssemblyContext ctx;
            ctx.packetId = packetId;
            ctx.totalFragments = total;
            ctx.firstFragmentTime = QDateTime::currentMSecsSinceEpoch();
            ctx.lastFragmentTime = ctx.firstFragmentTime;
            ctx.assemblyTimer.start();
            ctx.fragments[seq] = fragment;
            ctx.receivedSeqs.insert(seq);
            ctx.bufferSize = fragment.size();
            m_activePackets[packetId] = ctx;
        } else {
            // 已有上下文(seq=0重复到达)，检查重复
            auto& ctx = m_activePackets[packetId];
            if (ctx.receivedSeqs.contains(seq)) {
                ++m_stats.totalFragmentsDropped; // 重复分片
                return;
            }
            ctx.fragments[seq] = fragment;
            ctx.receivedSeqs.insert(seq);
            ctx.bufferSize += fragment.size();
            ctx.lastFragmentTime = QDateTime::currentMSecsSinceEpoch();
        }
    } else {
        // 非首分片: 查找对应的上下文
        if (!m_activePackets.contains(packetId)) {
            // 未找到上下文(首分片未到达或已超时)，丢弃
            ++m_stats.totalFragmentsDropped;
            return;
        }
        auto& ctx = m_activePackets[packetId];
        if (ctx.receivedSeqs.contains(seq)) {
            ++m_stats.totalFragmentsDropped; // 重复分片
            return;
        }
        ctx.fragments[seq] = fragment;
        ctx.receivedSeqs.insert(seq);
        ctx.bufferSize += fragment.size();
        ctx.lastFragmentTime = QDateTime::currentMSecsSinceEpoch();
        ctx.totalFragments = qMax(ctx.totalFragments, total);
    }

    tryCompletePacket(packetId);
}

// ============================================================================
// 偏移量策略
// ============================================================================

/**
 * @brief 按偏移量重组处理
 * @param fragment 分片数据
 * @param seq 序列号(作为偏移量索引)
 * @param total 总分片数
 *
 * 偏移量策略下，seq 被视为字节偏移量的索引。
 * 分片按照 seq 值顺序排列，适用于分片携带偏移信息的协议。
 */
void PacketAssembler::feedByOffset(const QByteArray& fragment, int seq, int total)
{
    // 偏移量策略使用与序列号类似的packetId机制
    // 每个包的首分片(seq=0)创建上下文
    int packetId = 0;

    if (!m_activePackets.contains(packetId)) {
        if (seq == 0) {
            if (m_activePackets.size() >= m_config.maxConcurrentPackets) {
                evictOldestPacket();
            }
            AssemblyContext ctx;
            ctx.packetId = packetId;
            ctx.totalFragments = total;
            ctx.firstFragmentTime = QDateTime::currentMSecsSinceEpoch();
            ctx.lastFragmentTime = ctx.firstFragmentTime;
            ctx.assemblyTimer.start();
            ctx.fragments[seq] = fragment;
            ctx.receivedSeqs.insert(seq);
            ctx.bufferSize = fragment.size();
            m_activePackets[packetId] = ctx;
        } else {
            // 首分片未到达，丢弃
            ++m_stats.totalFragmentsDropped;
            return;
        }
    } else {
        auto& ctx = m_activePackets[packetId];
        if (ctx.receivedSeqs.contains(seq)) {
            ++m_stats.totalFragmentsDropped; // 重复分片
            return;
        }
        ctx.fragments[seq] = fragment;
        ctx.receivedSeqs.insert(seq);
        ctx.bufferSize += fragment.size();
        ctx.lastFragmentTime = QDateTime::currentMSecsSinceEpoch();
        ctx.totalFragments = qMax(ctx.totalFragments, total);
    }

    tryCompletePacket(packetId);
}

// ============================================================================
// 时间窗口策略
// ============================================================================

/**
 * @brief 按时间窗口重组处理
 * @param fragment 分片数据
 * @param seq 序列号
 * @param total 总分片数
 *
 * 时间窗口策略下，在同一时间窗口内到达的分片归入同一包。
 * 超过窗口后自动创建新的包。适用于无显式包标识的协议。
 */
void PacketAssembler::feedByTimestampWindow(const QByteArray& fragment, int seq, int total)
{
    qint64 now = QDateTime::currentMSecsSinceEpoch();

    // 检查是否需要开新的时间窗口
    if (m_activePackets.isEmpty() || m_timestampWindowStart == 0) {
        m_timestampWindowStart = now;
        m_nextPacketId = 0;
    }

    // 如果超出时间窗口，将当前包标记为完成(如果分片足够)并开新窗口
    if ((now - m_timestampWindowStart) > m_config.timestampWindowMs) {
        // 尝试完成旧窗口中的包
        for (auto it = m_activePackets.begin(); it != m_activePackets.end(); ++it) {
            tryCompletePacket(it.key());
        }
        m_timestampWindowStart = now;
        ++m_nextPacketId;
    }

    int packetId = m_nextPacketId;

    if (!m_activePackets.contains(packetId)) {
        if (m_activePackets.size() >= m_config.maxConcurrentPackets) {
            evictOldestPacket();
        }
        AssemblyContext ctx;
        ctx.packetId = packetId;
        ctx.totalFragments = total;
        ctx.firstFragmentTime = now;
        ctx.lastFragmentTime = now;
        ctx.assemblyTimer.start();
        ctx.fragments[seq] = fragment;
        ctx.receivedSeqs.insert(seq);
        ctx.bufferSize = fragment.size();
        m_activePackets[packetId] = ctx;
    } else {
        auto& ctx = m_activePackets[packetId];
        if (ctx.receivedSeqs.contains(seq)) {
            ++m_stats.totalFragmentsDropped; // 重复分片
            return;
        }
        ctx.fragments[seq] = fragment;
        ctx.receivedSeqs.insert(seq);
        ctx.bufferSize += fragment.size();
        ctx.lastFragmentTime = now;
        // 取最大的 totalFragments 声明
        ctx.totalFragments = qMax(ctx.totalFragments, total);
    }

    tryCompletePacket(packetId);
}

// ============================================================================
// 包完成与发射
// ============================================================================

/**
 * @brief 检查指定包是否所有分片已到齐，若到齐则组装发射
 * @param packetId 包标识
 *
 * 遍历 AssemblyContext 中已收到的分片，若数量等于 totalFragments
 * 则按序列号顺序拼接数据并发射完成信号。
 */
void PacketAssembler::tryCompletePacket(int packetId)
{
    auto it = m_activePackets.find(packetId);
    if (it == m_activePackets.end()) {
        return;
    }

    auto& ctx = it.value();

    // 检查是否所有分片已到达
    if (ctx.receivedSeqs.size() < ctx.totalFragments) {
        // 检查空缺
        checkForGaps(packetId);
        return;
    }

    emitCompletedPacket(ctx);
}

/**
 * @brief 检测指定包的序列空缺
 * @param packetId 包标识
 *
 * 遍历 0~totalFragments-1 范围，记录缺失的序列号。
 * 若配置了 dropOnGap 则丢弃该包。
 */
void PacketAssembler::checkForGaps(int packetId)
{
    auto it = m_activePackets.find(packetId);
    if (it == m_activePackets.end()) {
        return;
    }

    const auto& ctx = it.value();
    for (int i = 0; i < ctx.totalFragments; ++i) {
        if (!ctx.receivedSeqs.contains(i)) {
            ++m_stats.totalGaps;
            if (m_config.dropOnGap) {
                dropPacket(packetId, false);
                return;
            }
        }
    }
}

// (包完成/超时/缓冲区/查询/统计方法移至 PacketAssemblyOps.cpp)
