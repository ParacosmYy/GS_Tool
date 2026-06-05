/**
 * @file TurboDecoder.h
 * @brief Turbo解码器 — 并行级联/迭代BCJR/Log-MAP算法
 *
 * 提供Turbo码(并行级联卷积码)的完整解码器实现,
 * 支持迭代BCJR(前向后向)算法、Log-MAP和Max-Log-MAP近似、
 * 可配置迭代次数和生成多项式。
 * 适用于嵌入式调试中的通信系统仿真和信道编码验证。
 */
#ifndef TURBO_DECODER_H
#define TURBO_DECODER_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <cmath>

/**
 * @class TurboDecoder
 * @brief Turbo码解码器
 *
 * 并行级联卷积码, 两个RSC(递归系统卷积)编码器通过
 * 交织器连接, 迭代BCJR解码。
 */
class TurboDecoder : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalIterations = 0;    ///< 迭代总次数
        quint64 totalBits = 0;          ///< 处理的比特总数
        double  avgIterations = 0.0;    ///< 平均迭代次数
        double  avgTimeMs = 0.0;        ///< 平均解码耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit TurboDecoder(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~TurboDecoder() override;

    // ── 配置 ──

    /**
     * @brief 设置最大迭代次数
     * @param maxIter 最大迭代(默认8)
     */
    void setMaxIterations(int maxIter);

    /** @brief 获取最大迭代次数 */
    int maxIterations() const;

    /**
     * @brief 设置约束长度
     * @param length 约束长度(默认3)
     */
    void setConstraintLength(int length);

    /**
     * @brief 设置生成多项式(八进制)
     * @param g1 系统位多项式(默认7)
     * @param g2 校验位多项式(默认5)
     */
    void setGeneratorPolynomials(int g1, int g2);

    // ── 解码 ──

    /**
     * @brief Turbo解码(Log-MAP算法)
     * @param sysReceived 系统位LLR(来自信道)
     * @param par1Received 第一校验位LLR
     * @param par2Received 第二校验位LLR(交织后)
     * @param interleaver 交织模式
     * @return 解码后的信息比特(硬判决)
     */
    QVector<quint8> decode(const QVector<double>& sysReceived,
                           const QVector<double>& par1Received,
                           const QVector<double>& par2Received,
                           const QVector<int>& interleaver);

    /**
     * @brief 使用Max-Log-MAP近似解码(低复杂度)
     * @param sysReceived 系统位LLR
     * @param par1Received 第一校验位LLR
     * @param par2Received 第二校验位LLR
     * @param interleaver 交织模式
     * @return 解码后的信息比特
     */
    QVector<quint8> decodeMaxLogMap(const QVector<double>& sysReceived,
                                    const QVector<double>& par1Received,
                                    const QVector<double>& par2Received,
                                    const QVector<int>& interleaver);

    // ── 辅助 ──

    /**
     * @brief 生成交织模式(伪随机)
     * @param length 数据长度
     * @param seed 随机种子
     * @return 交织模式(索引数组)
     */
    static QVector<int> generateInterleaver(int length, quint32 seed = 0);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 解码完成信号 @param bits 解码比特数 @param iters 实际迭代次数 */
    void decoded(int bits, int iters);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief BCJR算法(Log-MAP) */
    QVector<double> bcjrLogMap(const QVector<double>& sys,
                               const QVector<double>& par,
                               const QVector<double>& extrinsic);

    /** @brief BCJR算法(Max-Log-MAP近似) */
    QVector<double> bcjrMaxLogMap(const QVector<double>& sys,
                                  const QVector<double>& par,
                                  const QVector<double>& extrinsic);

    /** @brief 前向递归(Alpha) */
    void computeAlpha(const QVector<double>& branchMetrics,
                      QVector<QVector<double>>& alpha);

    /** @brief 后向递归(Beta) */
    void computeBeta(const QVector<double>& branchMetrics,
                     QVector<QVector<double>>& beta);

    /** @brief 计算分支度量 */
    QVector<double> computeBranchMetrics(const QVector<double>& sys,
                                         const QVector<double>& par,
                                         const QVector<double>& extrinsic);

    /** @brief trellis状态数 */
    int stateCount() const;

    /** @brief 下一状态(给定当前状态和输入) */
    int nextState(int state, int input) const;

    /** @brief 编码器输出(给定状态和输入) */
    int output(int state, int input) const;

    /** @brief log(exp(a)+exp(b)) 数值稳定实现 */
    static double logSum(double a, double b);

    int m_maxIter = 8;              ///< 最大迭代次数
    int m_constraintLen = 3;        ///< 约束长度
    int m_g1 = 7;                   ///< 生成多项式1(八进制)
    int m_g2 = 5;                   ///< 生成多项式2(八进制)
    int m_numStates;                ///< trellis状态数

    mutable Stats m_stats;          ///< 操作统计
};

#endif // TURBO_DECODER_H
