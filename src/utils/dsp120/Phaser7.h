#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Phaser7 - 移相器效果第7代实现
 *
 * 通过级联全通滤波器产生相位偏移，与干信号混合
 * 形成梳状滤波效果，支持LFO调制及多阶配置。
 */
class Phaser7 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalProcessFrames = 0; double avgProcessingTimeMs = 0.0; };
    explicit Phaser7(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 处理音频帧进行移相效果
     * @param inputFrame 输入音频采样帧
     * @return 移相处理后的音频帧
     */
    QVector<double> processFrame(const QVector<double>& inputFrame);

    /**
     * @brief 设置LFO（低频振荡器）参数
     * @param rateHz LFO速率 (Hz)
     * @param depth 调制深度 [0, 1]
     * @param waveform 波形类型 (sine/triangle/square)
     */
    void setLFO(double rateHz, double depth, const QString& waveform = "sine");

    /**
     * @brief 设置全通滤波器级数
     * @param stages 级数（通常为2/4/6/8）
     */
    void setStages(int stages);

    /**
     * @brief 设置干/湿信号混合比例
     * @param mix 混合比例 [0, 1]，0为全干信号，1为全效果
     */
    void setMix(double mix);

    /**
     * @brief 设置反馈量
     * @param feedback 反馈量 [0, 0.99]
     */
    void setFeedback(double feedback);

signals:
    void phasingCompleted(int frameCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
