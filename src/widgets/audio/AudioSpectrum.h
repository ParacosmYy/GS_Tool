/**
 * @file AudioSpectrum.h
 * @brief 音频频谱可视化Widget — PCM数据实时FFT频谱柱状图
 *
 * 数据流:
 *   - feedData() 接收PCM原始字节 → 累积到内部缓冲区
 *   - 缓冲区达到FFT窗口大小 → processFft()执行频谱分析
 *   - 各频段能量 → dB归一化 → 平滑处理 → paintEvent()柱状图渲染
 *
 * 安全约束:
 *   - setDbRange(min, max): max <= min时自动修正为 min+1.0
 *   - setBarCount(0): 自动修正为1
 *   - setFftSize(0): 自动修正为1
 *   - processFft()中dB范围归一化使用防御性除零检查
 *
 * 统计: FFT运算次数/PCM字节数/重绘次数/峰值变化次数/配置变更次数
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
 *
 * 设计模式: 独立Widget — 数据通过feedData输入，自动触发FFT和重绘。
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
     * @param size FFT大小（必须为2的幂，最小1），默认1024
     */
    void setFftSize(int size);

    /**
     * @brief 输入PCM音频数据
     * @param pcmData 原始PCM字节数据(16bit有符号小端)
     *
     * 数据累积到内部缓冲区，达到FFT窗口大小时自动执行频谱分析。
     */
    void feedData(const QByteArray &pcmData);

    /**
     * @brief 设置分贝显示范围
     * @param minDb 最小分贝值，默认-80.0
     * @param maxDb 最大分贝值，默认0.0
     *
     * 若 maxDb <= minDb，自动修正 maxDb = minDb + 1.0 防止除零。
     */
    void setDbRange(double minDb, double maxDb);

    /**
     * @brief 设置频谱柱数量
     * @param bars 柱数（最小1），默认32
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
    /** @brief 绘制频谱柱状图(HSV渐变色) */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 窗口大小变更(预留重算柱宽) */
    void resizeEvent(QResizeEvent *event) override;

private:
    /** @brief 执行FFT运算并更新频谱数据: 分段能量→dB→归一化→平滑 */
    void processFft();

    int m_sampleRate = 44100;      ///< 音频采样率(Hz)
    int m_fftSize = 1024;          ///< FFT窗口大小(最小1)
    int m_barCount = 32;           ///< 频谱柱数量(最小1)
    double m_minDb = -80.0;        ///< 最小分贝值
    double m_maxDb = 0.0;          ///< 最大分贝值(保证 > m_minDb)
    double m_smoothFactor = 0.7;   ///< 频谱平滑因子(0.0~1.0)
    QVector<double> m_magnitudes;  ///< FFT原始幅度值
    QVector<double> m_smoothed;    ///< 平滑后的幅度值
    QByteArray m_buffer;           ///< PCM数据累积缓冲区

    // ---- 统计计数器 ----
    quint64 m_totalFftRuns = 0;           ///< 累计FFT运算次数
    quint64 m_totalPcmBytes = 0;          ///< 累计输入PCM字节数
    quint64 m_totalRepaints = 0;          ///< 累计重绘次数
    quint64 m_totalPeakChanges = 0;       ///< 累计峰值频率变化次数
    quint64 m_totalConfigChanges = 0;     ///< 累计配置变更次数

public:
    /** @brief 获取累计FFT运算次数 @return FFT次数 */
    quint64 totalFftRuns() const { return m_totalFftRuns; }
    /** @brief 获取累计输入PCM字节数 @return PCM字节 */
    quint64 totalPcmBytes() const { return m_totalPcmBytes; }
    /** @brief 获取累计重绘次数 @return 重绘次数 */
    quint64 totalRepaints() const { return m_totalRepaints; }
    /** @brief 获取累计峰值频率变化次数 @return 峰值变化次数 */
    quint64 totalPeakChanges() const { return m_totalPeakChanges; }
    /** @brief 获取累计配置变更次数 @return 配置变更次数 */
    quint64 totalConfigChanges() const { return m_totalConfigChanges; }
    /** @brief 重置所有频谱统计计数器为零 */
    void resetSpectrumStatistics() { m_totalFftRuns = 0; m_totalPcmBytes = 0; m_totalRepaints = 0; m_totalPeakChanges = 0; m_totalConfigChanges = 0; }
};
