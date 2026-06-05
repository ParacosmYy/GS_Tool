/**
 * @file ConstantQTransform.h
 * @brief 常数Q变换(CQT) — 音乐信号分析的对数频率变换
 *
 * 功能: 实现常数Q变换，频率轴按对数分布，适合音乐音高检测、
 *       音阶分析。支持可配置的最小/最大频率、bins/octave、
 *       FFT核计算和稀疏化。适用于音频信号频谱分析。
 *
 * 协作: AdaptiveFFT(频谱分析) / Welch(功率谱估计)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QVector2D>

/**
 * @brief 常数Q变换(CQT)引擎
 */
class ConstantQTransform : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalTransformsComputed = 0;       ///< 累计CQT计算次数
        int totalKernelsBuilt = 0;             ///< 累计核构建次数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /** @brief CQT参数配置 */
    struct Parameters {
        double minFreq = 32.70;    ///< 最低频率(C1)
        double maxFreq = 4186.0;   ///< 最高频率(C8)
        int binsPerOctave = 12;    ///< 每倍频程频率bin数
        double sampleRate = 44100.0;///< 采样率
        double threshold = 0.0054; ///< 稀疏化阈值
    };

    /**
     * @brief 构造函数
     * @param params CQT参数
     * @param parent 父对象
     */
    explicit ConstantQTransform(const Parameters& params = Parameters(),
                                QObject* parent = nullptr);

    /**
     * @brief 设置CQT参数并重建核
     * @param params 新参数
     */
    void setParameters(const Parameters& params);

    /**
     * @brief 对输入信号执行CQT
     * @param signal 时域输入信号
     * @return CQT复数输出(实部/虚部交织)
     */
    QVector<double> compute(const QVector<double>& signal);

    /**
     * @brief 取CQT幅度谱
     * @param signal 时域输入信号
     * @return 各频率bin的幅度值
     */
    QVector<double> computeMagnitude(const QVector<double>& signal);

    /**
     * @brief 取CQT功率谱
     * @param signal 时域输入信号
     * @return 各频率bin的功率值
     */
    QVector<double> computePower(const QVector<double>& signal);

    /**
     * @brief 获取各bin的中心频率
     * @return 频率数组(Hz)
     */
    QVector<double> centerFrequencies() const;

    /**
     * @brief 获取总频率bin数
     */
    int totalBins() const { return m_totalBins; }

    /**
     * @brief 获取最大FFT长度
     */
    int maxFftLength() const { return m_maxFftLen; }

    /**
     * @brief 获取当前参数
     */
    Parameters parameters() const { return m_params; }

    /**
     * @brief 获取统计信息
     */
    const Stats& stats() const { return m_stats; }

    /**
     * @brief 重置统计信息
     */
    void resetStatistics();

signals:
    /**
     * @brief CQT变换完成
     * @param totalBins 频率bin总数
     * @param signalLength 输入信号长度
     */
    void transformComputed(int totalBins, int signalLength);

private:
    /**
     * @brief 构建CQT稀疏核矩阵
     */
    void buildKernel();

    /**
     * @brief 计算基频Q值
     */
    double computeQ() const;

    Parameters m_params;                        ///< CQT参数
    int m_totalBins = 0;                        ///< 总bin数
    int m_maxFftLen = 0;                        ///< 最大FFT长度

    /** @brief 稀疏核存储(每个bin: FFT长度, 实部索引+值, 虚部索引+值) */
    struct SparseKernel {
        int fftLen = 0;
        QVector<int> indices;
        QVector<double> realValues;
        QVector<double> imagValues;
    };
    QVector<SparseKernel> m_kernels;           ///< 各bin的稀疏核

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
