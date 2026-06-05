/**
 * @file DataNormalizer.h
 * @brief 数据归一化引擎 — MinMax/Z-Score/Decimal/Log/Vector归一化
 *
 * 功能: 5种归一化方法，将数值数据映射到标准范围，
 *       支持实时流式归一化和批量归一化。
 *
 * 协作: DataScalerWidget(缩放显示) / FftEngine(频谱归一化)
 */
#ifndef DATANORMALIZER_H
#define DATANORMALIZER_H

#include <QObject>
#include <QVector>

/**
 * @brief 数据归一化引擎 — 5种归一化方法，支持实时流式与批量处理
 */
class DataNormalizer : public QObject {
    Q_OBJECT

public:
    /** @brief 归一化方法枚举 */
    enum class NormalizeMethod {
        MinMax,         ///< Min-Max归一化到[0,1]
        ZScore,         ///< Z-Score标准化(均值0标准差1)
        DecimalScaling, ///< 小数定标归一化
        LogTransform,   ///< 对数变换
        VectorNorm      ///< 向量范数归一化(L2)
    };
    Q_ENUM(NormalizeMethod)

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalValuesNormalized = 0;  ///< 累计归一化数值数
        quint64 totalBatchesProcessed = 0;  ///< 累计批处理次数
        double  peakInputValue  = 0.0;      ///< 输入峰值
        double  peakOutputValue = 0.0;      ///< 输出峰值
        quint64 totalErrors = 0;            ///< 累计错误次数(如log负数)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit DataNormalizer(QObject* parent = nullptr);

    /** @brief 设置归一化方法 @param method 归一化方法 */
    void setMethod(NormalizeMethod method);

    /** @brief 设置目标范围(仅MinMax有效) @param min 最小值 @param max 最大值 */
    void setTargetRange(double min, double max);

    /** @brief 归一化单个值 @param value 输入值 @return 归一化后的值 */
    double normalize(double value);

    /** @brief 批量归一化 @param data 输入数据 @return 归一化后的数据 */
    QVector<double> normalizeBatch(const QVector<double>& data);

    /** @brief 反归一化(仅MinMax方法可逆) @param value 归一化后的值 @return 原始值 */
    double denormalize(double value) const;

    /** @brief 重置归一化参数(不重置统计) */
    void reset();

    /** @brief 获取统计信息 @return 统计信息常量引用 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 单值归一化完成 @param input 原始值 @param output 归一化值 */
    void normalized(double input, double output);

    /** @brief 批量归一化完成 @param count 数据条数 */
    void batchNormalized(int count);

private:
    /** @brief MinMax归一化实现 @param value 输入值 @return 归一化值 */
    double normalizeMinMax(double value) const;

    /** @brief Z-Score标准化实现 @param value 输入值 @return 标准化值 */
    double normalizeZScore(double value) const;

    /** @brief 小数定标归一化实现 @param value 输入值 @return 归一化值 */
    double normalizeDecimal(double value) const;

    /** @brief 对数变换实现 @param value 输入值 @return 变换值 */
    double normalizeLog(double value) const;

    /** @brief 向量范数归一化实现(L2) @param value 输入值 @return 归一化值 */
    double normalizeVector(double value) const;

    /** @brief 从批量数据计算归一化参数 @param data 输入数据 */
    void updateParams(const QVector<double>& data);

    NormalizeMethod m_method;   ///< 当前归一化方法
    double m_targetMin;         ///< 目标范围最小值
    double m_targetMax;         ///< 目标范围最大值
    double m_dataMin;           ///< 数据集最小值
    double m_dataMax;           ///< 数据集最大值
    double m_mean;              ///< 数据集均值
    double m_stddev;            ///< 数据集标准差
    double m_scaleFactor;       ///< 小数定标因子(10^j)
    double m_vectorNorm;        ///< 向量L2范数
    bool   m_paramsValid;       ///< 参数是否已计算

    Stats m_stats;              ///< 运行时统计
};
#endif // DATANORMALIZER_H
