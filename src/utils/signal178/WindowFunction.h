/**
 * @file WindowFunction.h
 * @brief 窗函数生成器(Hann/Hamming/Blackman/Kaiser/Bartlett/Flat-top+SCT分析) — Window Function Generator with SCT Analysis
 *
 * 功能: 实现多种窗函数生成，支持Hann/Hamming/Blackman/Kaiser/
 *       Bartlett/Flat-top窗，以及频谱特性(SCT)分析。
 *
 * 协作: FftEngine(FFT引擎) / Compressor2(压缩器) / ChirpZ4(Chirp-z)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 窗函数生成器(含频谱特性分析)
 */
class WindowFunction : public QObject {
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum WindowType {
        Hann = 0, Hamming = 1, Blackman = 2,
        Kaiser = 3, Bartlett = 4, FlatTop = 5,
        Rectangular = 6, BlackmanHarris = 7
    };

    /** @brief 频谱特性指标(SCT) */
    struct SCTAnalysis {
        double sidelobeLevel = 0.0;    ///< 最大旁瓣电平(dB)
        double mainlobeWidth = 0.0;    ///< 主瓣宽度(bin数)
        double scallopLoss = 0.0;      ///< 扇形损耗(dB)
        double enbw = 0.0;             ///< 等效噪声带宽(bin)
        double coherentGain = 0.0;     ///< 相干增益
        double processingGain = 0.0;   ///< 处理增益(dB)
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalGenerated = 0;         ///< 累计生成次数
        int lastSize = 0;                   ///< 最近窗长
        double avgProcessingTimeMs = 0.0;   ///< 平均耗时(ms)
    };

    explicit WindowFunction(QObject *parent = nullptr);
    ~WindowFunction() override;

    void setWindowType(WindowType type);
    void setKaiserBeta(double beta);

    /**
     * @brief 生成窗函数
     * @param N 窗长度
     * @return 窗系数
     */
    QVector<double> generate(int N) const;

    /** @brief 应用窗函数到信号 */
    QVector<double> apply(const QVector<double>& signal) const;

    /** @brief 频谱特性分析(SCT) */
    SCTAnalysis analyze(int N) const;

    /** @brief 生成窗函数的频率响应 */
    QVector<double> frequencyResponse(int N, int fftSize = 0) const;

    /** @brief 批量生成所有窗类型的比较数据 */
    QVector<QPair<WindowType, QVector<double>>> generateAll(int N) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void windowGenerated(int size, WindowType type);

private:
    /** @brief 各窗函数实现 */
    double windowValue(WindowType type, int n, int N) const;

    /** @brief 修正Bessel函数I0(Kaiser窗用) */
    static double besselI0(double x);

    WindowType m_type = Hann;
    double m_kaiserBeta = 8.6;  ///< Kaiser窗β参数

    Stats m_stats;
    double m_timeSum = 0.0;
};
