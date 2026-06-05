/**
 * @file TurboDecoder.h
 * @brief Turbo码译码器 — BCJR (MAP) 算法
 *
 * 功能: 实现Turbo码的最大后验概率(MAP)译码, 使用BCJR算法计算前后向度量,
 *       支持SOVA(软输出维特比)和Log-MAP两种模式, 可配置迭代次数和码率。
 *       Turbo码广泛用于3G/4G通信系统的前向纠错。
 *
 * 协作: ChannelSimulator(信道仿真) / Interleaver(交织器) / ViterbiDecoder(硬判决对比)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Turbo码译码器
 *
 * 使用BCJR算法实现Turbo码的迭代译码。包含两个分量译码器(基于RSC卷积码),
 * 通过交织器/解交织器交换外信息, 多次迭代后收敛到可靠判决。
 * 支持 Log-MAP (精确) 和 Max-Log-MAP (简化) 两种度量计算模式。
 */
class TurboDecoder : public QObject
{
    Q_OBJECT

public:
    /** @brief 译码算法模式 */
    enum class DecodingMode {
        LogMAP,         ///< Log-MAP: 精确对数域BCJR, 使用 max* 运算符
        MaxLogMAP       ///< Max-Log-MAP: 简化近似, 仅取 max
    };
    Q_ENUM(DecodingMode)

    /** @brief RSC(递归系统卷积码)编码器参数 */
    struct RSCParameters {
        int constraintLength = 3;           ///< 约束长度
        QVector<int> generatorPoly;         ///< 生成多项式(八进制)
        QVector<int> feedbackPoly;          ///< 反馈多项式(八进制)
    };

    /** @brief 译码结果 */
    struct DecodingResult {
        QVector<int> decodedBits;           ///< 译码比特(硬判决)
        QVector<double> softOutput;         ///< 软输出(LLR)
        int iterationsUsed = 0;             ///< 实际使用迭代次数
        bool converged = false;             ///< 是否提前收敛
        double finalLLRMetric = 0.0;        ///< 最终LLR度量
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalFramesDecoded = 0;         ///< 累计译码帧数
        int totalBitsDecoded = 0;           ///< 累计译码比特数
        int totalIterations = 0;            ///< 累计迭代次数
        int totalEarlyConvergences = 0;     ///< 累计提前收敛次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit TurboDecoder(QObject* parent = nullptr);

    /**
     * @brief 配置Turbo译码器参数
     * @param rscParams RSC编码器参数
     * @param interleaverSize 交织器大小(帧长)
     * @param maxIterations 最大迭代次数
     * @param mode 译码模式
     */
    void configure(const RSCParameters& rscParams, int interleaverSize,
                   int maxIterations = 8, DecodingMode mode = DecodingMode::LogMAP);

    /**
     * @brief 译码一帧数据
     * @param systematic 系统位接收序列(信道LLR)
     * @param parity1 分量编码器1校验位(信道LLR)
     * @param parity2 分量编码器2校验位(信道LLR)
     * @return 译码结果
     */
    DecodingResult decode(const QVector<double>& systematic,
                           const QVector<double>& parity1,
                           const QVector<double>& parity2);

    /**
     * @brief 设置交织模式
     * @param pattern 交织索引数组(0-based), 长度必须等于交织器大小
     */
    void setInterleaverPattern(const QVector<int>& pattern);

    /**
     * @brief 生成长度为 N 的伪随机交织模式
     * @param N 交织长度
     * @return 交织索引数组
     */
    QVector<int> generateRandomInterleaver(int N) const;

    /**
     * @brief 计算估计的BER(误比特率)
     * @param transmitted 发送比特
     * @param decoded 译码比特
     * @return 误比特率
     */
    double computeBER(const QVector<int>& transmitted,
                       const QVector<int>& decoded) const;

    /**
     * @brief 获取当前配置
     */
    RSCParameters rscParameters() const;
    int interleaverSize() const;
    int maxIterations() const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief BCJR分量译码器
     * @param systematicLLR 系统位LLR
     * @param parityLLR 校验位LLR
     * @param priorLLR 先验信息LLR
     * @return 外信息LLR
     */
    QVector<double> bcjrDecode(const QVector<double>& systematicLLR,
                                const QVector<double>& parityLLR,
                                const QVector<double>& priorLLR);

    /**
     * @brief 计算前向度量(alpha)
     * @param sysLLR 系统位LLR
     * @param parLLR 校验位LLR
     * @param priorLLR 先验LLR
     * @return 前向度量矩阵 [time][state]
     */
    QVector<QVector<double>> computeAlpha(const QVector<double>& sysLLR,
                                           const QVector<double>& parLLR,
                                           const QVector<double>& priorLLR);

    /**
     * @brief 计算后向度量(beta)
     * @param sysLLR 系统位LLR
     * @param parLLR 校验位LLR
     * @param priorLLR 先验LLR
     * @return 后向度量矩阵 [time][state]
     */
    QVector<QVector<double>> computeBeta(const QVector<double>& sysLLR,
                                          const QVector<double>& parLLR,
                                          const QVector<double>& priorLLR);

    /**
     * @brief max* 运算符: max*(a,b) = max(a,b) + log(1 + exp(-|a-b|))
     * @param a 值1
     * @param b 值2
     * @return max* 结果
     */
    double maxStar(double a, double b) const;

    /**
     * @brief 计算分支度量(gamma)
     * @param sysBit 系统位
     * @param parBit 校验位
     * @param priorBit 先验位
     * @return 分支度量
     */
    double branchMetric(double sysBit, double parBit, double priorBit) const;

    /**
     * @brief 交织操作
     * @param data 输入数据
     * @return 交织后数据
     */
    QVector<double> interleave(const QVector<double>& data) const;

    /**
     * @brief 解交织操作
     * @param data 输入数据
     * @return 解交织后数据
     */
    QVector<double> deinterleave(const QVector<double>& data) const;

    RSCParameters m_rscParams;              ///< RSC编码器参数
    int m_interleaverSize = 0;              ///< 交织器大小
    int m_maxIterations = 8;                ///< 最大迭代次数
    DecodingMode m_mode = DecodingMode::LogMAP; ///< 译码模式
    QVector<int> m_interleaver;             ///< 交织索引
    int m_numStates = 4;                    ///< 状态数 = 2^(constraintLength-1)

    Stats m_stats;
    double m_timeSum = 0.0;
};
