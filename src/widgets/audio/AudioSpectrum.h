/**
 * @file AudioSpectrum.h
 * @brief 音频频谱可视化组件 - 将PCM数据实时转换为频谱柱状图
 *
 * 职责:
 *   1. 接收PCM原始音频数据并进行FFT变换
 *   2. 将频谱数据渲染为带平滑动画的柱状图
 *   3. 支持自定义采样率、FFT大小、分贝范围和柱数
 */

#pragma once
#include <QWidget>
#include <QByteArray>
#include <QVector>

/**
 * @brief 音频频谱可视化组件
 *
 * 持续接收PCM数据，执行FFT运算后以柱状图形式展示频谱分布。
 * 支持平滑因子调节以获得流畅的动画效果。
 */
class AudioSpectrum : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造频谱组件
     * @param parent 父widget
     */
    explicit AudioSpectrum(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~AudioSpectrum() override;

    /**
     * @brief 设置音频采样率
     * @param rate 采样率（Hz），默认44100
     */
    void setSampleRate(int rate);

    /**
     * @brief 设置FFT窗口大小
     * @param size FFT大小（必须为2的幂），默认1024
     */
    void setFftSize(int size);

    /**
     * @brief 输入PCM音频数据
     * @param pcmData 原始PCM字节数据
     */
    void feedData(const QByteArray &pcmData);

    /**
     * @brief 设置分贝显示范围
     * @param minDb 最小分贝值，默认-80.0
     * @param maxDb 最大分贝值，默认0.0
     */
    void setDbRange(double minDb, double maxDb);

    /**
     * @brief 设置频谱柱数量
     * @param bars 柱数，默认32
     */
    void setBarCount(int bars);

    /**
     * @brief 设置频谱平滑因子
     * @param factor 平滑系数（0.0~1.0），值越大越平滑，默认0.7
     */
    void setSmoothFactor(double factor);

    /**
     * @brief 获取当前采样率
     * @return 采样率（Hz）
     */
    int sampleRate() const;

    /**
     * @brief 获取当前FFT大小
     * @return FFT窗口大小
     */
    int fftSize() const;

signals:
    /** @brief 频谱数据更新完成 @param magnitudes 各柱的幅度值数组 */
    void spectrumUpdated(const QVector<double> &magnitudes);

    /** @brief 峰值频率发生变化 @param freq 新的峰值频率（Hz） */
    void peakFrequencyChanged(double freq);

protected:
    /** @brief 绘制频谱柱状图 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 窗口大小变更时重新计算柱宽 */
    void resizeEvent(QResizeEvent *event) override;

private:
    /** @brief 执行FFT运算并更新频谱数据 */
    void processFft();

    int m_sampleRate = 44100;      ///< 音频采样率
    int m_fftSize = 1024;          ///< FFT窗口大小
    int m_barCount = 32;           ///< 频谱柱数量
    double m_minDb = -80.0;        ///< 最小分贝值
    double m_maxDb = 0.0;          ///< 最大分贝值
    double m_smoothFactor = 0.7;   ///< 频谱平滑因子
    QVector<double> m_magnitudes;  ///< FFT原始幅度值
    QVector<double> m_smoothed;    ///< 平滑后的幅度值
    QByteArray m_buffer;           ///< PCM数据缓冲区

    // ---- 统计计数器 ----
    quint64 m_totalFftRuns = 0;           ///< 总FFT运算次数
    quint64 m_totalPcmBytes = 0;          ///< 总输入PCM字节数
    quint64 m_totalRepaints = 0;          ///< 总重绘次数
    quint64 m_totalPeakChanges = 0;       ///< 总峰值频率变化次数
    quint64 m_totalConfigChanges = 0;     ///< 总配置变更次数

public:
    /** @brief 获取总FFT运算次数 @return 累计FFT次数 */
    quint64 totalFftRuns() const { return m_totalFftRuns; }
    /** @brief 获取总输入PCM字节数 @return 累计PCM字节 */
    quint64 totalPcmBytes() const { return m_totalPcmBytes; }
    /** @brief 获取总重绘次数 @return 累计重绘 */
    quint64 totalRepaints() const { return m_totalRepaints; }
    /** @brief 获取总峰值频率变化次数 @return 累计峰值变化次数 */
    quint64 totalPeakChanges() const { return m_totalPeakChanges; }
    /** @brief 获取总配置变更次数 @return 累计配置变更次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }
    /** @brief 重置频谱统计计数器 */
    void resetSpectrumStatistics() { m_totalFftRuns = 0; m_totalPcmBytes = 0; m_totalRepaints = 0; m_totalPeakChanges = 0; m_totalConfigChanges = 0; }
};
