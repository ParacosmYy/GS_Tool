/**
 * @file DataReducer.h
 * @brief 数据降采样引擎 -- 高频数据降采样同时保留关键特征
 *
 * 提供4种降采样方法: Decimation(等距抽样)、Average(均值)、MinMax(极值)、
 * LTTB(Largest Triangle Three Buckets，视觉最优降采样)。
 * 适用于波形显示性能优化和长期数据归档。
 * 支持 RMSE 误差评估，降采样完成后发射 reductionComplete 信号。
 */
#ifndef DATAREDUCER_H
#define DATAREDUCER_H

#include <QList>
#include <QObject>
#include <QPair>

/**
 * @brief 数据降采样引擎
 *
 * 输入为 (timestamp, value) 数据点列表，输出为指定目标点数的降采样结果。
 * LTTB 算法在保留视觉特征方面表现最优，是波形显示降采样的推荐方法。
 */
class DataReducer : public QObject {
    Q_OBJECT

public:
    /** @brief 降采样方法 */
    enum class ReductionMethod {
        Decimation,  ///< 等距抽样: 每隔 N 个点保留一个
        Average,     ///< 均值降采样: 分桶后输出每个桶的平均值
        MinMax,      ///< 极值降采样: 分桶后输出每个桶的最小值和最大值
        LTTB         ///< 最大三角形三桶算法: 视觉最优降采样
    };
    Q_ENUM(ReductionMethod)

    /** @brief 数据点 */
    using DataPoint = QPair<qint64, double>;

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalPointsInput  = 0;   ///< 累计输入数据点总数
        quint64 totalPointsOutput = 0;   ///< 累计输出数据点总数
        double  reductionRatio    = 0.0; ///< 平均降采样比率(output/input)
        double  peakError         = 0.0; ///< 历史最大 RMSE
        double  avgError          = 0.0; ///< 历史平均 RMSE
    };

    /** @brief 构造数据降采样引擎 @param parent 父对象 */
    explicit DataReducer(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~DataReducer() override;

    // ---- 配置 ----

    /** @brief 设置降采样方法(默认LTTB) @param method 降采样方法枚举 */
    void setMethod(ReductionMethod method);

    /** @brief 获取当前降采样方法 @return 方法枚举 */
    ReductionMethod method() const;

    /** @brief 设置目标输出点数(默认500，最小2) @param count 目标点数 */
    void setTargetPoints(int count);

    /** @brief 获取当前目标输出点数 @return 目标点数 */
    int targetPoints() const;

    // ---- 降采样 ----

    /**
     * @brief 对输入数据执行降采样
     *
     * 根据 setMethod/setTargetPoints 的配置执行降采样。
     * 完成后发射 reductionComplete 信号。
     * @param data 输入数据点列表(时间戳升序)
     * @return 降采样后的数据点列表
     */
    QList<DataPoint> reduce(const QList<DataPoint> &data);

    // ---- 误差评估 ----

    /**
     * @brief 计算 RMSE(均方根误差)评估降采样质量
     *
     * 将 reduced 通过线性插值重采样到 original 的时间戳上，
     * 然后计算与 original 的 RMSE。该指标反映降采样保留原始
     * 波形特征的程度。
     * @param original 原始数据点列表
     * @param reduced  降采样后的数据点列表
     * @return RMSE 值，值越小表示保真度越高
     */
    double estimateError(const QList<DataPoint> &original,
                         const QList<DataPoint> &reduced) const;

    // ---- 统计 ----

    /** @brief 获取运行时统计快照 @return Stats 结构体 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /**
     * @brief 降采样完成信号
     * @param inputCount 输入数据点数
     * @param outputCount 输出数据点数
     */
    void reductionComplete(int inputCount, int outputCount);

private:
    /** @brief 等距抽样降采样 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
    static QList<DataPoint> reduceDecimation(const QList<DataPoint> &data, int target);

    /** @brief 均值降采样 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
    static QList<DataPoint> reduceAverage(const QList<DataPoint> &data, int target);

    /** @brief 极值降采样(每桶保留min和max) @param data 输入数据 @param target 目标点数 @return 降采样结果 */
    static QList<DataPoint> reduceMinMax(const QList<DataPoint> &data, int target);

    /** @brief LTTB最大三角形三桶算法 @param data 输入数据 @param target 目标点数 @return 降采样结果 */
    static QList<DataPoint> reduceLTTB(const QList<DataPoint> &data, int target);

    /**
     * @brief 计算三个点构成的三角形面积(用于LTTB)
     * @param p1 第一个点
     * @param p2 第二个点(候选点)
     * @param p3 第三个点
     * @return 三角形面积的绝对值
     */
    static double triangleArea(const DataPoint &p1, const DataPoint &p2, const DataPoint &p3);

    /**
     * @brief 在 reduced 上对 targetTs 进行线性插值
     * @param reduced 降采样数据(时间戳升序)
     * @param targetTs 目标时间戳
     * @param ok 输出参数: 是否能插值(目标在范围外时为false)
     * @return 插值后的值
     */
    static double linearInterpolate(const QList<DataPoint> &reduced,
                                    qint64 targetTs, bool &ok);

    // ── 配置 ──
    ReductionMethod m_method = ReductionMethod::LTTB; ///< 当前降采样方法
    int m_targetPoints = 500;                          ///< 目标输出点数

    // ── 统计 ──
    Stats m_stats;                                     ///< 运行时统计
    double m_errorSum = 0.0;                           ///< RMSE 累加器(用于计算平均误差)
    quint64 m_errorCount = 0;                          ///< RMSE 计算次数
};

#endif // DATAREDUCER_H
