/**
 * @file LiftingScheme.h
 * @brief 提升格式小波引擎 — 预测/更新步骤/整数小波
 *
 * 提供基于提升格式(Lifting Scheme)的小波变换实现,
 * 支持自定义预测/更新步骤、整数到整数的小波变换(无损)、
 * 内联计算(O(1)额外空间)。适用于嵌入式调试中的
 * 信号压缩和实时数据流处理。
 */
#ifndef LIFTING_SCHEME_H
#define LIFTING_SCHEME_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class LiftingScheme
 * @brief 提升格式小波变换引擎
 *
 * 标准提升格式: Split → Predict → Update → Normalize
 * 内置多种常见小波(CDF 5/3, CDF 9/7, Haar)。
 */
class LiftingScheme : public QObject {
    Q_OBJECT

public:
    /** @brief 内置小波类型 */
    enum WaveletType {
        Haar = 0,           ///< Haar小波(最简单)
        Cdf53 = 1,          ///< CDF 5/3(无损整数小波)
        Cdf97 = 2,          ///< CDF 9/7(JPEG2000有损)
        Custom = 3          ///< 自定义预测/更新系数
    };
    Q_ENUM(WaveletType)

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalForward = 0;       ///< 正变换次数
        quint64 totalInverse = 0;       ///< 逆变换次数
        quint64 totalSamples = 0;       ///< 处理的样本总数
        double  avgTimeMs = 0.0;        ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit LiftingScheme(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~LiftingScheme() override;

    // ── 配置 ──

    /** @brief 设置小波类型 */
    void setWaveletType(WaveletType type);

    /** @brief 获取当前小波类型 */
    WaveletType waveletType() const;

    /**
     * @brief 设置自定义预测/更新系数
     * @param predict 预测步骤系数
     * @param update 更新步骤系数
     */
    void setCustomCoeffs(const QVector<double>& predict,
                         const QVector<double>& update);

    // ── 浮点变换 ──

    /**
     * @brief 多级正变换
     * @param signal 输入信号
     * @param levels 级数(0=最大)
     * @return 变换系数(交替: s0,d0,s1,d1,...)
     */
    QVector<double> forward(const QVector<double>& signal, int levels = 0);

    /**
     * @brief 多级逆变换
     * @param coefficients 变换系数
     * @param originalLength 原始长度
     * @param levels 级数
     * @return 重构信号
     */
    QVector<double> inverse(const QVector<double>& coefficients,
                            int originalLength, int levels);

    // ── 整数变换(无损) ──

    /**
     * @brief 整数到整数正变换(无损)
     * @param signal 输入整数信号
     * @return 整数变换系数
     */
    QVector<qint64> forwardInt(const QVector<qint64>& signal);

    /**
     * @brief 整数到整数逆变换(无损)
     * @param coefficients 整数变换系数
     * @param originalLength 原始长度
     * @return 重构整数信号
     */
    QVector<qint64> inverseInt(const QVector<qint64>& coefficients,
                               int originalLength);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 正变换完成信号 @param count 系数数量 */
    void forwardCompleted(int count);
    /** @brief 逆变换完成信号 @param length 信号长度 */
    void inverseCompleted(int length);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 加载内置小波系数 */
    void loadBuiltinCoeffs(WaveletType type);

    /** @brief 单级正变换(浮点) */
    void forwardOneLevel(QVector<double>& signal, int length);

    /** @brief 单级逆变换(浮点) */
    void inverseOneLevel(QVector<double>& signal, int length);

    /** @brief 单级正变换(整数, CDF 5/3) */
    void forwardOneLevelInt(QVector<qint64>& signal, int length);

    /** @brief 单级逆变换(整数, CDF 5/3) */
    void inverseOneLevelInt(QVector<qint64>& signal, int length);

    WaveletType m_type = Cdf53;            ///< 当前小波类型
    QVector<double> m_predictCoeffs;        ///< 预测系数
    QVector<double> m_updateCoeffs;         ///< 更新系数
    double m_predictScale = 1.0;            ///< 预测缩放因子
    double m_updateScale = 1.0;             ///< 更新缩放因子

    mutable Stats m_stats;                  ///< 操作统计
};

#endif // LIFTING_SCHEME_H
