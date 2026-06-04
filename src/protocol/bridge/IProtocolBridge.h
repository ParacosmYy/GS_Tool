/**
 * @file IProtocolBridge.h
 * @brief 协议桥抽象接口 — 定义原始字节到结构化数据的桥接规范
 *
 * 协议桥将原始串口字节流转换为frameParsed兼容的信号。
 * 实现此接口的桥可以无缝接入现有的ChartModel/ProtocolView管道。
 * 与FrameParser的frameParsed信号完全兼容，下游组件无需知道数据来自哪个协议。
 *
 * 设计模式: 策略模式(Strategy) — 不同的协议桥是可互换的策略。
 * 数据层: 不依赖任何表现层类，纯字节解析 + 信号发射。
 */
#ifndef IPROTOCOLBRIDGE_H
#define IPROTOCOLBRIDGE_H

#include <QObject>
#include <QByteArray>
#include <QVariantMap>
#include <QString>

/**
 * @brief 协议桥抽象接口 — 将原始字节流转换为结构化通道数据
 *
 * 实现类负责: 字节累积、帧边界检测、通道数据解析。
 * 通过frameParsed信号向下游(ChartModel/ProtocolView)推送解析结果。
 *
 * 协作关系:
 *   - ProtocolBridgeManager: 管理桥的创建和切换
 *   - ChartModel: 接收frameParsed信号更新波形数据
 *   - ProtocolView: 接收frameParsed信号更新协议表格
 */
class IProtocolBridge : public QObject {
    Q_OBJECT

public:
    /** @brief 构造函数 @param parent 父对象 */
    explicit IProtocolBridge(QObject* parent = nullptr)
        : QObject(parent) {}

    /** @brief 析构函数 */
    ~IProtocolBridge() override = default;

    ///< 禁止拷贝（QObject派生类）
    IProtocolBridge(const IProtocolBridge&) = delete;
    IProtocolBridge& operator=(const IProtocolBridge&) = delete;

    /**
     * @brief 喂入原始字节流数据
     *
     * 桥在内部累积字节、检测帧边界、解析通道数据。
     * @param data 来自串口/TCP等数据源的原始字节
     */
    virtual void feed(const QByteArray& data) = 0;

    /**
     * @brief 重置桥的内部状态
     *
     * 清空缓冲区、通道配置等，恢复到初始状态。
     */
    virtual void reset() = 0;

    /**
     * @brief 返回协议桥的名称
     * @return 协议名称(用于UI显示和日志记录)
     */
    virtual QString name() const = 0;

signals:
    /**
     * @brief 帧解析完成信号 — 与FrameParser::frameParsed完全相同的签名
     *
     * @param fields 通道名→值的映射(如 "CH1" → 3.14, "CH2" → 2.72)
     * @param rawFrame 本帧的原始字节数据(用于ProtocolView的HEX显示)
     */
    void frameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

public:
    // ---- 协议桥全局统计(static inline，跨所有实例共享) ----

    /** @brief 获取累计桥接器实例创建总数 @return 实例计数 */
    static quint64 totalBridgeInstances() { return s_totalBridgeInstances; }

    /** @brief 获取累计桥接器解析调用总数(feed调用) @return 解析调用计数 */
    static quint64 totalBridgeParses() { return s_totalBridgeParses; }

    /** @brief 获取累计桥接器解析错误总数 @return 错误计数 */
    static quint64 totalBridgeErrors() { return s_totalBridgeErrors; }

    /** @brief 重置所有全局统计计数器(实例数/解析数/错误数) */
    static void resetGlobalStats();

protected:
    /** @brief 子类构造时递增全局实例计数 */
    void incBridgeInstanceCount() { ++s_totalBridgeInstances; }
    /** @brief 子类feed()中递增解析计数 */
    void incBridgeParseCount() { ++s_totalBridgeParses; }
    /** @brief 子类解析错误时递增错误计数 */
    void incBridgeErrorCount() { ++s_totalBridgeErrors; }

private:
    static inline quint64 s_totalBridgeInstances = 0; ///< 累计桥接器实例创建总数
    static inline quint64 s_totalBridgeParses = 0;    ///< 累计桥接器解析调用总数
    static inline quint64 s_totalBridgeErrors = 0;    ///< 累计桥接器解析错误总数
};

#endif // IPROTOCOLBRIDGE_H
