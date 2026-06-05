/**
 * @file EyeDiagramEngine.h
 * @brief 眼图引擎 — 叠加显示数字信号的眼图分析
 *
 * 功能: 将数字信号按UI(Unit Interval)分段叠加，
 *       生成眼图数据，测量眼高/眼宽/抖动。
 *
 * 协作: ScopeWidget(显示) / Demodulator(解调后眼图)
 */
#ifndef EYEDIAGRAMENGINE_H
#define EYEDIAGRAMENGINE_H

#include <QObject>
#include <QVector>
#include <QList>

class EyeDiagramEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 眼图参数 */
    struct Config {
        double sampleRate = 10000.0;     ///< 采样率
        double symbolRate = 1000.0;      ///< 符号率
        int    tracesPerEye = 2;         ///< 每眼轨迹数(1=半眼, 2=全眼)
    };

    /** @brief 眼图测量 */
    struct Measurement {
        double eyeHeight = 0.0;          ///< 眼高
        double eyeWidth = 0.0;           ///< 眼宽(UI)
        double jitterPp = 0.0;           ///< 峰峰抖动
        double jitterRms = 0.0;          ///< RMS抖动
        double eyeOpening = 0.0;         ///< 眼开度(%)
        double crossingLevel = 0.0;      ///< 交叉电平
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalTracesOverlaid = 0;
        quint64 totalEyesAnalyzed = 0;
        double  peakEyeHeight = 0.0;
    };

    explicit EyeDiagramEngine(QObject* parent = nullptr);

    void setConfig(const Config& config);

    /** @brief 从信号生成眼图 @param data 信号 @return 叠加数据[ui][trace] */
    QList<QVector<double>> generate(const QVector<double>& data);

    /** @brief 测量眼图参数 @param eyeData 眼图数据 @return 测量结果 */
    Measurement measure(const QList<QVector<double>>& eyeData);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void eyeGenerated(int traceCount);

private:
    Config m_config;
    Stats m_stats;
};

#endif // EYEDIAGRAMENGINE_H
