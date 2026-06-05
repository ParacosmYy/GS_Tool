/**
 * @file StftAnalyzer.h
 * @brief 短时傅里叶变换分析器 — 时频表示
 *
 * 功能: 对信号进行STFT分析，支持多种窗函数和步进长度，
 *       输出时频谱矩阵，统计分析次数/帧数/耗时。
 */
#ifndef STFTANALYZER_H
#define STFTANALYZER_H

#include <QObject>
#include <QVector>

class StftAnalyzer : public QObject {
    Q_OBJECT
public:
    /** 窗函数类型 */
    enum class WindowType { Rectangular, Hamming, Hanning, Blackman };

    /** 时频帧 */
    struct Frame {
        int timeIndex = 0;              ///< 时间帧索引
        QVector<double> magnitude;      ///< 幅度谱
        QVector<double> phase;          ///< 相位谱
    };

    /** 分析统计 */
    struct Stats {
        quint64 totalAnalyses = 0;
        quint64 totalFramesComputed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit StftAnalyzer(QObject* parent = nullptr);

    /** @brief 设置参数 @param fftSize FFT大小 @param hopSize 步进 @param window 窗类型 */
    void setParameters(int fftSize, int hopSize, WindowType window);

    /** @brief 执行STFT @param data 信号 @return 时频帧列表 */
    QList<Frame> analyze(const QVector<double>& data);

    /** @brief 逆STFT(重叠相加) @param frames 帧列表 @param outputLength 输出长度 @return 重建信号 */
    QVector<double> synthesize(const QList<Frame>& frames, int outputLength);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(int frameCount, int fftSize);

private:
    /** 生成窗函数 */
    QVector<double> generateWindow(int length) const;
    /** 单帧DFT */
    void computeDft(const QVector<double>& frame,
                    QVector<double>& magnitude, QVector<double>& phase) const;
    /** 单帧IDFT */
    QVector<double> computeIdft(const QVector<double>& magnitude,
                                const QVector<double>& phase) const;

    int m_fftSize;
    int m_hopSize;
    WindowType m_windowType;
    Stats m_stats;
    double m_timeSum;
};

#endif // STFTANALYZER_H
