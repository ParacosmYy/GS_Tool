/**
 * @file PrunedFFT.h
 * @brief 修剪FFT — 仅计算需要的输出频率bin
 *
 * 功能: 实现修剪FFT算法，当只需要少量输出频率bin或输入信号
 *       在频域稀疏时，通过跳过不必要的蝶形运算大幅减少计算量。
 *       支持输出修剪(只计算指定输出bin)和输入修剪(跳过零输入)。
 *       适用于频谱监测、窄带信号检测、实时频谱分析优化。
 *
 * 协作: SpectrumAnalyzer(谱分析) / GoertzelAlgorithm(单频检测)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 修剪FFT — 输入/输出蝶形修剪
 *
 * 传统FFT计算所有N个输出bin，O(N log N)。
 * 修剪FFT利用稀疏性:
 *   - 输出修剪: 只计算指定的输出bin集合
 *   - 输入修剪: 跳过已知为零的输入采样
 * 复杂度可降至O(K log N), K为目标bin数。
 */
class PrunedFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 修剪结果 */
    struct PrunedResult {
        QVector<double> magnitudes;      ///< 幅度谱(仅请求的bin)
        QVector<double> phases;          ///< 相位谱(仅请求的bin)
        QVector<int> binIndices;         ///< 对应的bin索引
        int totalBinsComputed = 0;       ///< 实际计算的bin数
        bool success = false;           ///< 是否成功
    };

    /** @brief 操作统计 */
    struct Stats {
        quint64 totalTransforms = 0;         ///< 总变换次数
        quint64 totalBinsComputed = 0;       ///< 总计算bin数
        quint64 totalButterfliesSkipped = 0; ///< 总跳过蝶形数
        double  avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit PrunedFFT(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~PrunedFFT() override;

    // ── 输出修剪FFT ──

    /**
     * @brief 输出修剪FFT: 只计算指定的输出频率bin
     * @param input 输入时域信号(长度须为2的幂)
     * @param outputBins 需要计算的输出bin索引列表
     * @return 修剪FFT结果
     */
    PrunedResult transformOutputPruned(const QVector<double>& input,
                                       const QVector<int>& outputBins);

    /**
     * @brief 频带提取: 计算指定频率范围内的所有bin
     * @param input 输入时域信号
     * @param startBin 起始bin(含)
     * @param endBin 结束bin(含)
     * @return 修剪FFT结果
     */
    PrunedResult extractBand(const QVector<double>& input,
                             int startBin, int endBin);

    // ── 输入修剪FFT ──

    /**
     * @brief 输入修剪FFT: 跳过已知为零的输入
     * @param inputFull 完整输入(含零)
     * @param activeIndices 非零输入的索引列表
     * @return 完整频谱(所有bin)
     */
    PrunedResult transformInputPruned(const QVector<double>& inputFull,
                                      const QVector<int>& activeIndices);

    // ── 辅助 ──

    /**
     * @brief 检查是否为2的幂
     * @param n 数值
     * @return 是否为2的幂
     */
    static bool isPowerOfTwo(int n);

    /**
     * @brief 计算比特反转
     * @param index 原始索引
     * @param bits 比特数
     * @return 反转后的索引
     */
    static int bitReverse(int index, int bits);

    /**
     * @brief 预计算旋转因子
     * @param n FFT长度
     */
    void precomputeTwiddle(int n);

    // ── 统计 ──

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 变换完成 @param totalBins 计算的总bin数 @param skippedButterflies 跳过的蝶形数 */
    void transformCompleted(int totalBins, quint64 skippedButterflies);

private:
    /**
     * @brief 递归计算单个输出bin的FFT
     * @param input 输入数据
     * @param targetBin 目标bin索引
     * @param n FFT长度
     * @return (实部, 虚部)
     */
    QPair<double, double> computeSingleBin(const QVector<double>& input,
                                           int targetBin, int n) const;

    /**
     * @brief 标记蝶形运算依赖链
     * @param targetBin 目标输出bin
     * @param stages 总级数
     * @return 每级需要计算的蝶形索引集合
     */
    QVector<QVector<int>> computeButterflyDependencies(
        int targetBin, int stages) const;

    QVector<double> m_twiddleCos;     ///< 旋转因子cos分量
    QVector<double> m_twiddleSin;     ///< 旋转因子sin分量
    int m_cachedSize;                 ///< 缓存的FFT长度

    Stats m_stats;                    ///< 操作统计
    double m_timeSum = 0.0;           ///< 累计耗时
};
