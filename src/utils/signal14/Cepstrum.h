/**
 * @file Cepstrum.h
 * @brief 倒谱分析 — 基频检测与同态滤波
 *
 * 功能: 计算实倒谱和复倒谱，支持基音周期检测、共振峰提取、
 *       同态滤波 (声门激励与声道响应分离)、倒谱平滑等。
 *       内置 FFT 引擎，不依赖外部 FFT 模块。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / PeakDetector(峰值检测)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief 倒谱分析引擎
 *
 * 倒谱是 "频谱的对数的频谱"，用于将卷积信号分解为
 * 源激励分量 (高倒频率) 和系统响应分量 (低倒频率)。
 * 典型应用: 基频检测、共振峰估计、回声检测。
 */
class Cepstrum : public QObject {
    Q_OBJECT

public:
    /** @brief 倒谱类型 */
    enum class CepstrumType {
        Real,       ///< 实倒谱: log|X(f)| 的 IDFT
        Complex     ///< 复倒谱: 完整复频谱的 IDFT
    };
    Q_ENUM(CepstrumType)

    /** @brief 基频检测结果 */
    struct PitchResult {
        double frequency = 0.0;     ///< 基频 (Hz)
        double period = 0.0;        ///< 基音周期 (采样点数)
        double confidence = 0.0;    ///< 置信度 [0, 1]
        bool valid = false;         ///< 检测是否有效
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalComputations = 0;          ///< 累计计算次数
        int totalSamplesProcessed = 0;      ///< 累计处理采样数
        int totalPitchDetections = 0;       ///< 累计基频检测次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理时间(ms)
    };

    explicit Cepstrum(QObject* parent = nullptr);

    /**
     * @brief 设置采样率
     * @param rate 采样率 (Hz)
     */
    void setSampleRate(double rate);

    /**
     * @brief 计算倒谱
     * @param data 时域输入数据
     * @param type 倒谱类型
     * @return 倒谱序列
     */
    QVector<double> compute(const QVector<double>& data,
                            CepstrumType type = CepstrumType::Real);

    /**
     * @brief 从倒谱检测基频
     * @param cepstrum 倒谱序列
     * @param minFreqHz 搜索下限频率 (Hz)
     * @param maxFreqHz 搜索上限频率 (Hz)
     * @return 基频检测结果
     */
    PitchResult detectPitch(const QVector<double>& cepstrum,
                            double minFreqHz = 50.0,
                            double maxFreqHz = 500.0) const;

    /**
     * @brief 低频倒谱 liftering (提取声道响应)
     * @param cepstrum 倒谱序列
     * @param cutoffQuefrency 截断倒频率 (采样点索引)
     * @return liftering 后的倒谱
     */
    QVector<double> lowpassLifter(const QVector<double>& cepstrum,
                                  int cutoffQuefrency) const;

    /**
     * @brief 倒谱平滑: 倒谱截断 -> IFFT -> 重建信号
     * @param data 时域输入数据
     * @param keepCoefficients 保留的倒谱系数个数
     * @return 平滑后的时域数据
     */
    QVector<double> smooth(const QVector<double>& data,
                           int keepCoefficients);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 倒谱计算完成 @param size 数据长度 @param type 倒谱类型 */
    void cepstrumReady(int size, CepstrumType type);

    /** @brief 基频检测完成 @param freq 基频 @param confidence 置信度 */
    void pitchDetected(double freq, double confidence);

private:
    void fft(QVector<double>& real, QVector<double>& imag);
    void ifft(QVector<double>& real, QVector<double>& imag);
    int nextPowerOf2(int n) const;

    double m_sampleRate;          ///< 采样率 (Hz)
    Stats m_stats;                ///< 统计信息
    double m_timeSum = 0.0;       ///< 处理时间累加器
};
