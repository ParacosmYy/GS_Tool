/**
 * @file Expander2.h
 * @brief 动态扩展器(上下行压缩+比例依赖攻击释放+自动增益) — Dynamic Expander with Upward/Downward Compression, Ratio-dependent Attack/Release and Auto-gain
 *
 * 功能: 实现动态扩展器，支持上行/下行压缩模式、
 *       比例依赖的攻击/释放时间、自动增益补偿和RMS/Peak检测。
 *
 * 协作: Compressor5(压缩器) / Limiter4(限制器) / GateFilter3(门限滤波)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 动态扩展器处理器
 */
class Expander2 : public QObject {
    Q_OBJECT

public:
    /** @brief 扩展模式 */
    enum Mode { Downward = 0, Upward = 1 };

    /** @brief 检测类型 */
    enum Detection { RMS = 0, Peak = 1 };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalFrames = 0;
        int blockSize = 0;
        double gainReduction = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Expander2(QObject *parent = nullptr);
    ~Expander2() override;

    void setThreshold(double dB);
    void setRatio(double ratio);
    void setAttack(double ms);
    void setRelease(double ms);
    void setKnee(double dB);
    void setMode(Mode mode);
    void setDetection(Detection det);
    void setAutoGain(bool enabled);

    /** @brief 处理音频帧 */
    QVector<double> process(const QVector<double>& input);

    /** @brief 计算扩展增益(dB) */
    double computeGain(double inputLeveldB) const;

    /** @brief 获取增益曲线 */
    QVector<QPair<double, double>> gainCurve(double minDb = -60,
                                              double maxDb = 0,
                                              int points = 256) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void frameProcessed(int samples, double gainReductionDb);

private:
    double m_threshold = -20.0;   ///< dB
    double m_ratio = 2.0;
    double m_attack = 1.0;        ///< ms
    double m_release = 100.0;     ///< ms
    double m_knee = 6.0;          ///< dB
    Mode m_mode = Downward;
    Detection m_detection = RMS;
    bool m_autoGain = false;

    double m_sampleRate = 44100.0;
    double m_envelope = 0.0;      ///< Current envelope level
    double m_currentGain = 1.0;   ///< Linear gain

    Stats m_stats;
    double m_timeSum = 0.0;

    double toLinear(double dB) const { return qPow(10.0, dB / 20.0); }
    double toDb(double linear) const;
};
