/**
 * @file PhaseVocoder.h
 * @brief 相位声码器(时间拉伸/变调) — Phase Vocoder with Phase Locking for Time-Stretch and Pitch-Shift
 *
 * 功能: 基于STFT的相位声码器，实现时间拉伸和变调。
 *       使用相位锁定(phase locking)减少相位伪影，
 *       支持任意拉伸比和音高偏移倍率。
 *
 * 协作: FftEngine(FFT) / ConstantQTransform(常Q变换) / GoertzelFilter(Goertzel滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 相位声码器
 */
class PhaseVocoder : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTimeStretches = 0;     ///< 累计时间拉伸次数
        quint64 totalPitchShifts = 0;       ///< 累计变调次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int lastFrameCount = 0;             ///< 最近处理的帧数
    };

    explicit PhaseVocoder(QObject* parent = nullptr);
    ~PhaseVocoder() override;

    /** @brief 设置FFT窗口大小(必须为2的幂) */
    void setWindowSize(int size);
    /** @brief 设置Hop大小(分析步长) */
    void setHopSize(int hop);
    /** @brief 设置窗函数类型: 0=Hann, 1=Hamming */
    void setWindowType(int type);

    /**
     * @brief 时间拉伸
     * @param signal 输入信号
     * @param stretchRatio 拉伸比(>1变长,<1变短)
     * @return 拉伸后信号
     */
    QVector<double> timeStretch(const QVector<double>& signal, double stretchRatio);

    /**
     * @brief 变调(不改变时长)
     * @param signal 输入信号
     * @param semitones 半音偏移(正值升调,负值降调)
     * @return 变调后信号
     */
    QVector<double> pitchShift(const QVector<double>& signal, double semitones);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 时间拉伸完成 @param ratio 拉伸比 */
    void timeStretchCompleted(double ratio);
    /** @brief 变调完成 @param semitones 半音数 */
    void pitchShiftCompleted(double semitones);

private:
    /** @brief STFT分析单帧 */
    void analyzeFrame(const QVector<double>& frame, QVector<double>& magnitude,
                      QVector<double>& phase) const;

    /** @brief 合成单帧(叠加到输出) */
    void synthesizeFrame(const QVector<double>& magnitude,
                         const QVector<double>& phase, int synthesisHop,
                         QVector<double>& output, int outPos) const;

    /** @brief 相位累积与锁定 */
    void phaseLock(QVector<double>& phase, const QVector<double>& prevPhase,
                   const QVector<double>& magnitude, double omega, double ratio) const;

    /** @brief 生成窗函数 */
    QVector<double> createWindow(int size) const;

    /** @brief 重叠相加 */
    void overlapAdd(QVector<double>& output, const QVector<double>& frame,
                    int pos, const QVector<double>& winSum) const;

    /** @brief 计算瞬时频率 */
    void computeInstantFreq(const QVector<double>& phase,
                            const QVector<double>& prevPhase,
                            double hop, QVector<double>& instFreq) const;

    int m_windowSize = 2048;
    int m_hopSize = 512;
    int m_windowType = 0;

    Stats m_stats;
    double m_timeSum = 0.0;
};
