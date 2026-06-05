/**
 * @file DataSynchronizer.h
 * @brief 数据同步引擎 -- 将多个数据流按时间戳对齐合并
 *
 * 维护多个命名的数据流(每流保存 timestamp→value 映射)，
 * 在 pushData 时检测跨流时间戳是否落在 maxDrift 窗口内，
 * 对齐后构造 SyncPoint 并发射 syncPointReady 信号。
 * 适用场景: 多传感器数据融合、多通道串口数据对齐。
 */
#ifndef DATASYNCHRONIZER_H
#define DATASYNCHRONIZER_H

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

/**
 * @brief 数据同步引擎，按时间戳对齐多个数据流
 *
 * 每个数据流独立维护一个有序 timestamp→value 表，
 * 当所有已注册流都有数据落在同一 maxDrift 窗口内时，
 * 通过线性插值生成对齐后的 SyncPoint。
 */
class DataSynchronizer : public QObject {
    Q_OBJECT

public:
    /** @brief 同步点: 某个对齐时刻各流的值 */
    struct SyncPoint {
        qint64 timestamp = 0;                ///< 对齐后的基准时间戳(ms)
        QMap<QString, double> values;        ///< 各流在此时刻的(插值)值
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalSyncPoints = 0;         ///< 已生成的同步点总数
        quint64 totalAlignments = 0;         ///< 已执行的跨流对齐次数
        double  maxDriftMs      = 0.0;       ///< 历史最大漂移量(ms)
        double  avgDriftMs      = 0.0;       ///< 历史平均漂移量(ms)
        quint64 droppedPoints   = 0;         ///< 因漂移超限被丢弃的数据点数
    };

    /** @brief 构造数据同步引擎 @param parent 父对象 */
    explicit DataSynchronizer(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DataSynchronizer() override;

    // ---- 流管理 ----

    /** @brief 注册一个新的数据流 @param name 流名称(需唯一) */
    void addStream(const QString &name);

    /** @brief 移除指定名称的数据流 @param name 流名称 */
    void removeStream(const QString &name);

    /** @brief 查询所有已注册流名称 @return 流名称列表 */
    QStringList streams() const;

    // ---- 数据输入 ----

    /**
     * @brief 向指定流推入一条带时间戳的数据
     *
     * 推入后自动尝试跨流对齐: 若所有流在 maxDrift 窗口内都有数据，
     * 则生成 SyncPoint 并发射 syncPointReady。
     * @param stream 目标流名称
     * @param timestamp 数据时间戳(ms)
     * @param value 数据值
     */
    void pushData(const QString &stream, qint64 timestamp, double value);

    // ---- 数据查询 ----

    /**
     * @brief 查询指定时间范围内的已对齐同步点
     * @param fromMs 起始时间戳(ms)，含
     * @param toMs   结束时间戳(ms)，含
     * @return 落在范围内的同步点列表(时间升序)
     */
    QVector<SyncPoint> getSyncedData(qint64 fromMs, qint64 toMs) const;

    // ---- 配置 ----

    /** @brief 设置最大允许漂移(默认50ms)，超出的数据点将被丢弃 @param ms 最大漂移毫秒数 */
    void setMaxDrift(double ms);

    /** @brief 获取当前最大漂移设置 @return 最大漂移(ms) */
    double maxDrift() const;

    // ---- 统计 ----

    /** @brief 获取运行时统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器(不影响已注册流和缓冲数据) */
    void resetStatistics();

    /** @brief 清空所有流的缓冲数据和已对齐点(保留流注册) */
    void clearBuffers();

signals:
    /** @brief 新的同步点就绪 @param point 对齐后的同步点 */
    void syncPointReady(const DataSynchronizer::SyncPoint &point);

    /** @brief 某流漂移超限，数据点被丢弃 @param stream 流名称 @param driftMs 实际漂移量(ms) */
    void driftExceeded(const QString &stream, double driftMs);

private:
    /**
     * @brief 尝试跨流对齐并生成 SyncPoint
     *
     * 以最新推入数据的时间戳为基准，检查其余流是否在 maxDrift 内有数据，
     * 有则通过最近值或插值构造同步点。
     * @param referenceTs 基准时间戳
     */
    void tryAlign(qint64 referenceTs);

    /**
     * @brief 在指定流中查找最接近 targetTs 的值(线性插值)
     * @param stream 流名称
     * @param targetTs 目标时间戳
     * @param ok 输出参数: 是否找到有效值
     * @return 插值后的值
     */
    double interpolateNear(const QString &stream, qint64 targetTs, bool &ok) const;

    /** @brief 修剪各流中过旧的缓冲数据(保留最近 maxDrift*2 范围) */
    void pruneBuffers();

    // ── 数据存储 ──
    QMap<QString, QMap<qint64, double>> m_streams;   ///< 各流的 timestamp→value 有序映射
    double m_maxDriftMs = 50.0;                       ///< 最大允许漂移(ms)
    QVector<SyncPoint> m_syncedPoints;                ///< 已生成的同步点历史
    int m_maxSyncedPoints = 10000;                    ///< 同步点历史最大保留数量

    // ── 统计 ──
    Stats m_stats;                                    ///< 运行时统计
    double m_driftSum = 0.0;                          ///< 用于计算平均漂移的累加器
};

#endif // DATASYNCHRONIZER_H
