/**
 * @file DataScalerWidget.h
 * @brief 数据缩放/标定组件 — 将原始ADC值转换为工程单位
 *
 * 功能: 支持线性/多项式/查表/指数四种标定模式，
 *       实时统计转换次数/峰值/范围，支持批量转换。
 *
 * 协作: DataAggregator(数据流标定) / DashboardModel(仪表显示)
 */
#ifndef DATASCALERWIDGET_H
#define DATASCALERWIDGET_H

#include <QObject>
#include <QList>
#include <QVector>

/**
 * @brief 数据缩放/标定组件 — ADC原始值转工程单位
 */
class DataScalerWidget : public QObject {
    Q_OBJECT

public:
    /** @brief 标定模式 */
    enum class ScaleMode {
        Linear,         ///< 线性标定: y = slope * x + offset
        Polynomial,     ///< 多项式标定: y = sum(coeffs[i] * x^i)
        LookupTable,    ///< 查表标定: 线性插值
        Exponential     ///< 指数标定: y = a * exp(b * x) + c
    };
    Q_ENUM(ScaleMode)

    /** @brief 标定点(用于查表模式) */
    struct CalibrationPoint {
        double raw    = 0.0;  ///< 原始值
        double scaled = 0.0;  ///< 标定后的工程值
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalConversions  = 0;  ///< 累计转换次数
        quint64 totalCalibrations = 0;  ///< 累计标定次数
        double  peakRawValue      = 0.0;///< 原始值峰值
        double  minValue          = 0.0;///< 工程值最小值
        double  maxValue          = 0.0;///< 工程值最大值
    };

    explicit DataScalerWidget(QObject* parent = nullptr);

    /** @brief 设置线性标定参数 @param slope 斜率 @param offset 偏移 */
    void setLinearCalibration(double slope, double offset);

    /** @brief 设置自定义标定点(查表模式) @param points 标定点列表 */
    void setCustomCalibration(const QList<CalibrationPoint>& points);

    /** @brief 设置多项式系数 @param coeffs 系数数组, coeffs[i]对应x^i */
    void setPolynomialCoeffs(const QVector<double>& coeffs);

    /** @brief 设置指数标定参数 @param a 系数a @param b 系数b @param c 偏移c */
    void setExponentialParams(double a, double b, double c);

    /** @brief 设置当前标定模式 @param mode 标定模式 */
    void setScaleMode(ScaleMode mode);

    /** @brief 转换单个原始值 @param rawValue 原始ADC值 @return 工程值 */
    double convert(double rawValue);

    /** @brief 批量转换 @param rawValues 原始值列表 @return 工程值列表 */
    QList<double> batchConvert(const QList<double>& rawValues);

    /** @brief 获取最近一次原始值 @return 原始值 */
    double lastRaw() const { return m_lastRaw; }

    /** @brief 获取最近一次工程值 @return 工程值 */
    double lastScaled() const { return m_lastScaled; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 值已转换 @param raw 原始值 @param scaled 工程值 */
    void valueConverted(double raw, double scaled);

private:
    double convertLinear(double raw) const;
    double convertPolynomial(double raw) const;
    double convertLookup(double raw) const;
    double convertExponential(double raw) const;

    ScaleMode m_mode;               ///< 当前标定模式
    double m_slope;                 ///< 线性斜率
    double m_offset;                ///< 线性偏移
    QVector<double> m_polyCoeffs;   ///< 多项式系数
    QList<CalibrationPoint> m_lookupTable; ///< 查表标定点(按raw排序)
    double m_expA;                  ///< 指数系数a
    double m_expB;                  ///< 指数系数b
    double m_expC;                  ///< 指数偏移c
    double m_lastRaw;               ///< 最近一次原始值
    double m_lastScaled;            ///< 最近一次工程值

    Stats m_stats;
};

#endif // DATASCALERWIDGET_H
