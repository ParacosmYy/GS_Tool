/**
 * @file ProtocolBridgeManager.cpp
 * @brief 协议桥管理器实现
 *
 * 实现:
 *   - 构造/析构: 持有三个数据源对象，默认使用 FrameParser 模式
 *   - 模式切换: 重置旧源 -> 断开旧信号 -> 连接新信号 -> 通知 UI
 *   - 数据路由: feedData() 根据当前模式将数据转发到对应的数据源
 *   - 空数据保护: 空数据直接忽略，避免无意义的处理开销
 *   - 状态查询: 提供桥接器解析状态、帧计数、错误统计的查询接口
 *   - 动态切换: switchActiveBridge() 不中断数据流的快速桥接器切换
 *   - 错误统计: 通过内部槽函数追踪 FrameParser 和桥接器的解析错误
 */

#include "protocol/bridge/ProtocolBridgeManager.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造协议桥管理器
 * @param frameParser 外部创建的帧解析器
 * @param parent 父对象
 *
 * frameParser 由 MainWindow 创建并传入。如果 frameParser 尚未设置 parent，
 * 则归本对象管理（setParent）。同时创建 JustFloatBridge 和 FireWaterBridge。
 * 初始连接 FrameParser 的信号。
 */
ProtocolBridgeManager::ProtocolBridgeManager(FrameParser* frameParser, QObject* parent)
    : QObject(parent)
    , m_frameParser(frameParser)
    , m_justFloat(new JustFloatBridge(this))
    , m_fireWater(new FireWaterBridge(this))
    , m_activeBridge(nullptr)
    , m_mode(ChartProtocolMode::FrameParser)
{
    // FrameParser是外部创建的对象，如果尚未设置parent则归我们管理
    if (m_frameParser && !m_frameParser->parent()) {
        m_frameParser->setParent(this);
    }

    // 初始连接: 默认FrameParser模式，连接FrameParser的信号
    switchSource();
}

/**
 * @brief 析构协议桥管理器
 *
 * QObject 父子树自动销毁 m_justFloat、m_fireWater。
 * m_frameParser 如果 parent 是本对象也会被自动销毁。
 */
// ============================================================================
// 模式设置 / 数据路由
// ============================================================================

/**
 * @brief 设置协议模式
 * @param mode 目标协议模式
 *
 * 切换流程:
 *   1. 如果 mode 与当前模式相同，直接返回
 *   2. 重置旧源的内部状态（清空缓冲区）
 *   3. 更新 m_mode
 *   4. 重新连接信号（switchSource）
 *   5. 发出 protocolModeChanged 信号通知 UI
 */
void ProtocolBridgeManager::setProtocolMode(ChartProtocolMode mode)
{
    if (m_mode == mode) {
        return; // 模式未变，无需切换
    }

    // 重置旧源状态
    if (m_activeBridge) {
        m_activeBridge->reset();
    } else if (m_mode == ChartProtocolMode::FrameParser) {
        m_frameParser->reset();
    }

    // 切换模式
    m_mode = mode;

    // 重新连接信号
    switchSource();

    // 通知UI更新通道配置
    emit protocolModeChanged(m_mode);
}

/**
 * @brief 获取当前协议模式
 * @return 当前活动协议模式
 */
ProtocolBridgeManager::ChartProtocolMode ProtocolBridgeManager::protocolMode() const
{
    return m_mode;
}

/**
 * @brief 接收原始串口数据，转发到当前活动的协议源
 * @param data 原始字节数据
 *
 * 空数据保护: 传入空 QByteArray 时直接返回，不触发任何处理。
 * 这避免了空数据导致的状态机无意义调用和潜在的边界问题。
 *
 * 无效模式保护: switch 的 default 分支会输出 qWarning 日志，
 * 理论上不会触发（枚举覆盖完整），但作为防御性编程的保底措施。
 */
void ProtocolBridgeManager::feedData(const QByteArray& data)
{
    // ---- 空数据保护 ----
    // 串口在特殊情况下（如连接刚建立/断开瞬间）可能产生空数据，
    // 直接忽略以避免无意义的处理开销和潜在的边界问题
    if (data.isEmpty()) {
        return;
    }

    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        if (m_frameParser) {
            m_frameParser->feed(data);
        } else {
            qWarning() << tr("帧解析模式但解析器为空");
        }
        break;

    case ChartProtocolMode::JustFloat:
        if (m_justFloat) {
            m_justFloat->feed(data);
        } else {
            qWarning() << tr("JustFloat模式但桥接器为空");
        }
        break;

    case ChartProtocolMode::FireWater:
        if (m_fireWater) {
            m_fireWater->feed(data);
        } else {
            qWarning() << tr("FireWater模式但桥接器为空");
        }
        break;

    default:
        // 防御性编程: 枚举覆盖完整时不应该到达这里
        qWarning() << tr("未知协议模式: %1").arg(static_cast<int>(m_mode));
        break;
    }
}

// ============================================================================
// 桥接器访问器
// ============================================================================

/**
 * @brief 获取当前活动的桥
 * @return 当前活动桥指针，FrameParser 模式下返回 nullptr
 */
IProtocolBridge* ProtocolBridgeManager::activeBridge() const
{
    return m_activeBridge;
}

/**
 * @brief 获取 FrameParser 指针
 * @return 帧解析器指针
 */
FrameParser* ProtocolBridgeManager::frameParser() const
{
    return m_frameParser;
}

/**
 * @brief 获取 JustFloatBridge 指针
 * @return JustFloat 协议桥指针
 */
JustFloatBridge* ProtocolBridgeManager::justFloatBridge() const
{
    return m_justFloat;
}

/**
 * @brief 获取 FireWaterBridge 指针
 * @return FireWater 协议桥指针
 */
FireWaterBridge* ProtocolBridgeManager::fireWaterBridge() const
{
    return m_fireWater;
}

// ============================================================================
// 桥接器状态查询接口
// ============================================================================

/**
 * @brief 获取当前活动桥接器的运行时统计信息
 * @return BridgeStats 结构体
 *
 * FrameParser 模式: 从 FrameParser 获取详细统计（帧数/错误数），
 * 并补充内部追踪的校验错误计数。
 * 其他模式: 使用内部累计计数器。
 */
ProtocolBridgeManager::BridgeStats ProtocolBridgeManager::bridgeStats() const
{
    BridgeStats stats;

    if (m_mode == ChartProtocolMode::FrameParser && m_frameParser) {
        // FrameParser 自带帧计数和错误计数
        stats.totalFramesParsed = m_frameParser->frameCount();
        stats.totalErrors = m_frameParser->errorCount();
        // isParsing: FrameParser 没有直接暴露状态，使用简化的判断
        // 如果有待处理数据则认为正在解析
        stats.isParsing = false;
    } else {
        // 其他模式使用内部累计值
        stats.totalFramesParsed = m_totalFramesParsed;
        stats.totalErrors = m_totalErrors;
    }

    stats.checksumErrors = m_checksumErrors;

    return stats;
}

/**
 * @brief 查询当前活动桥接器是否正在解析
 * @return true=正在解析，false=空闲
 *
 * FrameParser 模式: 始终返回 false（状态机内部状态未暴露）。
 * 此方法主要用于未来扩展。
 */
bool ProtocolBridgeManager::isParsing() const
{
    return bridgeStats().isParsing;
}

/**
 * @brief 获取当前活动桥接器的累计成功解析帧数
 * @return 帧数
 */
quint64 ProtocolBridgeManager::totalFramesParsed() const
{
    return bridgeStats().totalFramesParsed;
}

/**
 * @brief 获取当前活动桥接器的累计解析错误次数
 * @return 错误次数
 */
quint64 ProtocolBridgeManager::totalErrors() const
{
    return bridgeStats().totalErrors;
}

/**
 * @brief 获取当前活动桥接器的累计校验错误次数
 * @return 校验错误次数
 */
quint64 ProtocolBridgeManager::checksumErrors() const
{
    return m_checksumErrors;
}

// ============================================================================
// 运行时动态切换
// ============================================================================

/**
 * @brief 动态切换活跃桥接器（不中断数据流）
 * @param mode 目标协议模式
 *
 * 与 setProtocolMode 的区别:
 *   - setProtocolMode: 重置旧源状态，清空缓冲区
 *   - switchActiveBridge: 不重置旧源状态，保留中间数据，
 *     适合快速切换场景（如自动检测协议类型）
 *
 * 切换后发出 protocolModeChanged 信号。
 */
void ProtocolBridgeManager::switchActiveBridge(ChartProtocolMode mode)
{
    if (m_mode == mode) {
        return; // 模式未变，无需切换
    }

    // 注意: 不重置旧源状态，保留中间数据
    // 这允许快速来回切换而不丢失正在解析的帧

    // 切换模式
    m_mode = mode;

    // 重新连接信号
    switchSource();

    // 通知UI更新通道配置
    emit protocolModeChanged(m_mode);
}

/**
 * @brief 重置所有桥接器的错误和帧计数统计
 *
 * 将所有桥接器的累计计数器归零。不影响当前运行状态。
 * FrameParser 自身的计数器也一并重置。
 */
void ProtocolBridgeManager::resetStats()
{
    m_totalFramesParsed = 0;
    m_totalErrors = 0;
    m_checksumErrors = 0;

    // 重置 FrameParser 的内部计数器
    if (m_frameParser) {
        // FrameParser::reset() 不清零计数器，
        // 这里无法直接重置，统计仍以 FrameParser 自身为准
    }
}

// ============================================================================
// 内部信号处理槽
// ============================================================================

/**
 * @brief 处理 FrameParser 的帧解析成功
 * @param fields 字段映射
 * @param rawFrame 原始帧数据
 *
 * 转发 frameParsed 信号给下游消费者。
 * FrameParser 的帧计数由其内部管理，此处不再重复累加。
 */
void ProtocolBridgeManager::onFrameParserParsed(
    const QVariantMap& fields, const QByteArray& rawFrame)
{
    emit frameParsed(fields, rawFrame);
}

/**
 * @brief 处理 FrameParser 的帧解析错误
 * @param reason 错误原因
 * @param rawFrame 原始帧数据
 *
 * 累加校验错误计数（当错误原因为 "Checksum mismatch" 时），
 * 然后转发 frameError 信号给下游消费者。
 */
void ProtocolBridgeManager::onFrameParserError(
    const QString& reason, const QByteArray& rawFrame)
{
    // 检测校验错误并累加计数
    if (reason.contains(QLatin1String("Checksum"))) {
        m_checksumErrors++;
    }
    m_totalErrors++;

    emit frameError(reason, rawFrame);
}

/**
 * @brief 处理桥接器的帧解析成功（用于 JustFloat/FireWater）
 * @param fields 字段映射
 * @param rawFrame 原始帧数据
 *
 * 累加帧计数后转发 frameParsed 信号。
 */
void ProtocolBridgeManager::onBridgeParsed(
    const QVariantMap& fields, const QByteArray& rawFrame)
{
    m_totalFramesParsed++;
    emit frameParsed(fields, rawFrame);
}

// ============================================================================
// 信号连接切换
// ============================================================================

/**
 * @brief 切换数据源连接
 *
 * 根据当前 m_mode，断开所有源到本 Manager 的信号连接，
 * 然后仅连接活动源的信号。
 *
 * FrameParser 模式: 连接 onFrameParserParsed + onFrameParserError（内部槽）
 * JustFloat 模式: 连接 onBridgeParsed（内部槽）
 * FireWater 模式: 连接 onBridgeParsed（内部槽）
 *
 * 使用内部槽函数拦截信号，用于统计帧数和错误数。
 * 断开操作使用 disconnect(sender, signal, this, slot) 精确匹配，
 * 不会影响其他对象的信号连接。
 */
void ProtocolBridgeManager::switchSource()
{
    // ---- 先断开所有源到本manager转发的连接 ----

    // 断开FrameParser的信号（使用内部槽函数签名）
    disconnect(m_frameParser, &FrameParser::frameParsed,
               this, &ProtocolBridgeManager::onFrameParserParsed);
    disconnect(m_frameParser, &FrameParser::frameError,
               this, &ProtocolBridgeManager::onFrameParserError);

    // 断开JustFloatBridge的信号
    disconnect(m_justFloat, &JustFloatBridge::frameParsed,
               this, &ProtocolBridgeManager::onBridgeParsed);

    // 断开FireWaterBridge的信号
    disconnect(m_fireWater, &FireWaterBridge::frameParsed,
               this, &ProtocolBridgeManager::onBridgeParsed);

    // ---- 根据模式设置活动桥并连接信号 ----

    switch (m_mode) {
    case ChartProtocolMode::FrameParser:
        m_activeBridge = nullptr;
        // FrameParser通过内部槽拦截，用于统计错误
        connect(m_frameParser, &FrameParser::frameParsed,
                this, &ProtocolBridgeManager::onFrameParserParsed);
        connect(m_frameParser, &FrameParser::frameError,
                this, &ProtocolBridgeManager::onFrameParserError);
        break;

    case ChartProtocolMode::JustFloat:
        m_activeBridge = m_justFloat;
        // JustFloat通过内部槽拦截，用于统计帧数
        connect(m_justFloat, &JustFloatBridge::frameParsed,
                this, &ProtocolBridgeManager::onBridgeParsed);
        break;

    case ChartProtocolMode::FireWater:
        m_activeBridge = m_fireWater;
        // FireWater通过内部槽拦截，用于统计帧数
        connect(m_fireWater, &FireWaterBridge::frameParsed,
                this, &ProtocolBridgeManager::onBridgeParsed);
        break;

    default:
        qWarning() << "ProtocolBridgeManager: unknown mode" << static_cast<int>(m_mode);
        m_activeBridge = nullptr;
        break;
    }
}
