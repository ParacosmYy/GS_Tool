/**
 * @file LfsrSequence.h
 * @brief LFSR线性反馈移位寄存器 — 伪随机序列生成
 *
 * 功能: 实现线性反馈移位寄存器(LFSR)，支持自定义多项式
 *       反馈抽头，生成伪随机二进制序列，检测最大长度序列(m序列)。
 *
 * 协作: SignalGeneratorWidget(信号生成) / EntropyCalculator(熵分析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QElapsedTimer>

/**
 * @brief LFSR线性反馈移位寄存器
 */
class LfsrSequence : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalGenerated = 0;      ///< 累计生成次数
        quint64 totalBits = 0;           ///< 累计生成比特数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit LfsrSequence(QObject* parent = nullptr);

    /**
     * @brief 设置反馈多项式
     * @param taps 反馈抽头位置列表(从1开始)
     * @param degree 寄存器位宽
     */
    void setPolynomial(const QVector<int>& taps, int degree);

    /**
     * @brief 生成指定比特数的序列(打包为字节)
     * @param bits 所需比特数
     * @return 生成的字节序列(MSB优先)
     */
    QByteArray generate(int bits);

    /**
     * @brief 生成指定数量的比特
     * @param count 比特数量
     * @return 比特值列表(0或1)
     */
    QVector<int> generateBits(int count);

    /**
     * @brief 检查当前多项式是否产生最大长度序列
     * @return true表示m序列(周期=2^degree-1)
     */
    bool maximalLength() const;

    /** @brief 获取当前寄存器状态 */
    quint32 state() const { return m_state; }

    /** @brief 获取多项式阶数 */
    int degree() const { return m_degree; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 序列生成完成 @param bits 生成比特数 */
    void generated(int bits);

private:
    /** @brief 执行单步移位并返回输出比特 */
    int step();

    /** @brief 检测序列周期 */
    int detectPeriod() const;

    static constexpr quint32 MAX_DEGREE = 32; ///< 最大寄存器位宽

    quint32     m_state = 1;        ///< 当前寄存器状态(非零)
    int         m_degree = 8;       ///< 多项式阶数
    QVector<int> m_taps;            ///< 反馈抽头位置
    quint32     m_tapMask = 0;      ///< 抽头掩码(预计算)

    mutable Stats        m_stats;
    mutable double       m_totalTimeMs = 0.0;
    mutable QElapsedTimer m_timer;
};
