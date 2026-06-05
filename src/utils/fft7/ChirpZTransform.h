/**
 * @file ChirpZTransform.h
 * @brief Chirp Z-Transform 变换引擎 — 频域缩放FFT
 *
 * 功能: 实现Chirp Z-Transform(CZT)，支持在任意频率区间上
 *       进行高分辨率频谱分析(zoom-FFT)，无需计算全频带FFT。
 *       内部使用Bluestein算法将CZT转化为循环卷积并用FFT加速。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / FftEngine(FFT加速)
 * @author Serial Tool Team
 * @date 2026-06-05
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QtGlobal>

/**
 * @class ChirpZTransform
 * @brief Chirp Z-Transform 变换引擎
 *
 * 在Z平面任意弧段上计算z变换，实现zoom-FFT功能。
 * 使用Bluestein算法将非2幂长度DFT转化为循环卷积，
 * 再利用基2 FFT加速计算，复杂度 O(N log N)。
 */
class ChirpZTransform : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息结构体 */
    struct Stats {
        quint64 totalTransforms = 0;        ///< 累计变换次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit ChirpZTransform(QObject *parent = nullptr);

    /**
     * @brief 在指定频率范围内执行Chirp Z变换
     * @param signal 输入时域信号
     * @param startFreq 起始频率(归一化, 0~0.5)
     * @param endFreq 结束频率(归一化, 0~0.5)
     * @param numPoints 输出频率点数
     * @return 变换结果的幅度谱
     */
    QVector<double> transform(const QVector<double> &signal,
                              double startFreq, double endFreq,
                              int numPoints);

    /**
     * @brief 以中心频率和带宽执行zoom-FFT
     * @param signal 输入时域信号
     * @param centerFreq 中心频率(归一化, 0~0.5)
     * @param bandwidth 分析带宽(归一化, >0)
     * @param numPoints 输出频率点数
     * @return 变换结果的幅度谱
     */
    QVector<double> zoom(const QVector<double> &signal,
                         double centerFreq, double bandwidth,
                         int numPoints);

    /** @brief 获取统计信息 @return 常量引用 */
    const Stats &stats() const { return m_stats; }

    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 变换完成信号 @param numPoints 输出点数 @param timeMs 耗时 */
    void transformCompleted(int numPoints, double timeMs);

private:
    /**
     * @brief 基2 FFT内部实现(就地)
     * @param real 实部数组
     * @param imag 虚部数组
     * @param inverse 是否为逆变换
     */
    void fftImpl(QVector<double> &real, QVector<double> &imag,
                 bool inverse) const;

    /**
     * @brief 求下一个2的幂
     * @param n 输入值
     * @return >= n 的最小2的幂
     */
    static int nextPowerOf2(int n);

    Stats m_stats;          ///< 统计信息
    double m_timeSum = 0.0; ///< 处理时间累加器
};
