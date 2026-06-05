/**
 * @file StereoWidener.h
 * @brief 立体声展宽器 — M/S处理/交叉馈送/宽度控制
 *
 * 基于中间/侧边(Mid/Side)解码的立体声展宽处理器:
 *   - M/S编码分离居中信号和立体声差信号
 *   - 侧边通道增益控制调节立体声宽度
 *   - 交叉馈送模拟扬声器串音以防止过度展宽的不自然感
 *   - 低频保持居中避免相位问题
 * 统计处理帧数和平均处理延迟。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief 立体声展宽处理器
 */
class StereoWidener : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalFramesProcessed = 0;  ///< 总处理帧数
        quint64 totalSamplesProcessed = 0; ///< 总处理采样数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        double peakLevelL = 0.0;           ///< 左声道峰值
        double peakLevelR = 0.0;           ///< 右声道峰值
    };

    /**
     * @brief 构造函数
     * @param sampleRate 采样率
     * @param parent 父对象
     */
    explicit StereoWidener(double sampleRate = 44100.0, QObject* parent = nullptr);

    /**
     * @brief 处理一帧立体声数据(交错L/R)
     * @param input 交错输入 [L0,R0,L1,R1,...]
     * @return 交错输出 [L0',R0',L1',R1',...]
     */
    QVector<float> process(const QVector<float>& input);

    /**
     * @brief 处理独立左右声道缓冲区
     * @param left 左声道
     * @param right 右声道
     */
    void processBuffers(QVector<float>& left, QVector<float>& right);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 设置宽度系数 0.0(单声道)~1.0(正常)~2.0(展宽) */
    void setWidth(double width);
    /** @brief 获取宽度系数 */
    double width() const { return m_width; }
    /** @brief 设置交叉馈送量 0.0~1.0 */
    void setCrossfeed(double amount);
    /** @brief 设置低频居中截止频率(Hz) */
    void setCenterFreq(double freq);
    /** @brief 重置滤波器状态 */
    void reset();

signals:
    /** 处理完成 */
    void frameProcessed(int sampleCount, double processingTimeMs);

private:
    /** 应用M/S编码和宽度控制 */
    void applyMidSide(float& left, float& right);
    /** 应用交叉馈送 */
    void applyCrossfeed(float& left, float& right);
    /** 低通滤波器保持低频居中 */
    float processLowpass(float sample, float& state);

    double m_sampleRate;
    double m_width;
    double m_crossfeedAmount;
    double m_centerFreq;
    float m_lpStateL;
    float m_lpStateR;
    float m_cfStateL1;
    float m_cfStateR1;
    float m_cfStateL2;
    float m_cfStateR2;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
