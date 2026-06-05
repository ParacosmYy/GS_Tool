#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 音调性分析实现 (版本5)
 *
 * 计算信号的音调性指标，用于区分音调成分与噪声成分，支持多频音调检测。
 */
class Tonality5 : public QObject {
    Q_OBJECT
public:
    /// 音调检测结果
    struct ToneInfo {
        double frequencyHz = 0.0;  ///< 音调频率(Hz)
        double amplitudeDb = 0.0;  ///< 幅度(dB)
        double prominence = 0.0;   ///< 突出度
    };

    /// 统计信息结构
    struct Stats {
        int totalAnalyses = 0;          ///< 总分析次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        double lastTonalityIndex = 0.0; ///< 最近一次音调性指数
    };

    explicit Tonality5(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 计算音调性指数
     * @param powerSpectrum 功率谱密度
     * @param sampleRate 采样率(Hz)
     * @return 音调性指数 [0.0, 1.0]
     */
    double computeTonalityIndex(const QVector<double>& powerSpectrum, double sampleRate);

    /**
     * @brief 检测主要音调成分
     * @param powerSpectrum 功率谱密度
     * @param sampleRate 采样率(Hz)
     * @param maxTones 最大检测音调数
     * @return 检测到的音调列表
     */
    QVector<ToneInfo> detectTones(const QVector<double>& powerSpectrum, double sampleRate, int maxTones = 5);

    /**
     * @brief 从时域信号直接分析音调性
     * @param samples 时域采样数据
     * @param sampleRate 采样率(Hz)
     * @return 音调性指数
     */
    double analyzeFromTimeDomain(const QVector<double>& samples, double sampleRate);

signals:
    /// 分析完成信号
    void analysisCompleted(double tonalityIndex);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_lastTonality = 0.0;
};
