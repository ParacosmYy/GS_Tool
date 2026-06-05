/**
 * @file ChirpZTransform2.h
 * @brief Chirp Z变换 — 任意频率轴的频谱分析
 *
 * 功能:
 *   - 任意频率范围 [fStart, fEnd] 的精细频谱分析
 *   - 可变分辨率: 指定输出点数M无需等于输入点数N
 *   - 螺旋轮廓参数(A, W)控制频率轴
 *   - 基于FFT的快速卷积实现, 复杂度O((N+M)log(N+M))
 *   - 支持复数输入/输出
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @class ChirpZTransform2
 * @brief Chirp Z变换(CZT)引擎 — 任意Z平面轮廓频谱分析
 *
 * CZT是DFT的推广: X_k = sum_n(x_n * z_n^{-k})，其中 z_n = A * W^{-n}。
 * 通过选择A(起始点)和W(步进因子)可实现任意频率范围的分析。
 */
class ChirpZTransform2 : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTransforms = 0;        /**< 总变换次数 */
        int totalInputSamples = 0;      /**< 总输入样本数 */
        int totalOutputBins = 0;        /**< 总输出频点数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 复数类型 */
    using Complex = QPair<double, double>;

    /** @brief 变换结果 */
    struct CZTResult {
        QVector<Complex> spectrum;      /**< 频谱(复数) */
        QVector<double> frequencies;    /**< 对应频率点 */
        QVector<double> magnitudes;     /**< 幅度谱 */
        QVector<double> phases;         /**< 相位谱 */
        int inputSize = 0;              /**< 输入长度 */
        int outputSize = 0;             /**< 输出长度 */
    };

    /** @brief 构造函数 */
    explicit ChirpZTransform2(QObject* parent = nullptr);

    /**
     * @brief 在指定频率范围内执行CZT
     * @param input 输入采样数据
     * @param sampleRate 采样率(Hz)
     * @param freqStart 起始频率(Hz)
     * @param freqEnd 结束频率(Hz)
     * @param numOutput 输出频点数(M)
     * @return CZT结果(频谱+幅度+相位)
     */
    CZTResult transform(const QVector<double>& input,
                          double sampleRate,
                          double freqStart,
                          double freqEnd,
                          int numOutput) const;

    /**
     * @brief 使用自定义A/W参数执行CZT
     * @param input 复数输入
     * @param A 起始点(复数)
     * @param W 步进因子(复数)
     * @param M 输出点数
     * @return 复数频谱
     */
    QVector<Complex> transformGeneral(
        const QVector<Complex>& input,
        const Complex& A,
        const Complex& W,
        int M) const;

    /**
     * @brief 快速傅里叶变换(Cooley-Tukey, 就地)
     * @param data 复数数组(长度必须为2的幂)
     * @param inverse 是否为逆变换
     */
    void fft(QVector<Complex>& data, bool inverse = false) const;

    /**
     * @brief 计算幅度谱
     * @param spectrum 复数频谱
     * @return 幅度数组
     */
    QVector<double> computeMagnitudes(const QVector<Complex>& spectrum) const;

    /**
     * @brief 计算相位谱
     * @param spectrum 复数频谱
     * @return 相位数组(弧度)
     */
    QVector<double> computePhases(const QVector<Complex>& spectrum) const;

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 变换完成信号 */
    void transformCompleted(int inputSize, int outputSize, double timeMs);

private:
    /** @brief 复数乘法 */
    Complex cmul(const Complex& a, const Complex& b) const;

    /** @brief 复数加法 */
    Complex cadd(const Complex& a, const Complex& b) const;

    /** @brief 复数共轭 */
    Complex cconj(const Complex& a) const;

    /** @brief 下一个2的幂 */
    int nextPow2(int n) const;

    /** @brief 极坐标转复数 */
    Complex polar(double r, double theta) const;

    mutable Stats m_stats;           /**< 统计信息 */
    mutable double m_timeSum = 0.0;  /**< 累计时间 */
};
