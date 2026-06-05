/**
 * @file Scrambler.h
 * @brief 伪随机扰码器 — LFSR/Fibonacci/Galois多项式
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief 伪随机扰码/解扰器
 * 支持LFSR Fibonacci/Galois结构, 多项式配置
 */
class Scrambler : public QObject
{
    Q_OBJECT

public:
    /** @brief LFSR结构类型 */
    enum LfsrType {
        Fibonacci,   ///< Fibonacci LFSR(外部反馈)
        Galois       ///< Galois LFSR(内部反馈)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalScrambled = 0;           ///< 累计扰码次数
        int totalDescrambled = 0;         ///< 累计解扰次数
        int totalBitsProcessed = 0;       ///< 累计处理比特数
        double avgProcessingTimeMs = 0.0;
    };

    explicit Scrambler(QObject* parent = nullptr);

    /** @brief 配置多项式 @param taps 反馈抽头位置 @param degree 寄存器阶数 @param type LFSR类型 */
    void configure(const QVector<int>& taps, int degree, LfsrType type = Galois);

    /** @brief 使用预定义多项式(CCITT/ITU/IEEE) @param standard 标准名 */
    void setStandard(const QString& standard);

    /** @brief 扰码 @param data 输入数据 @return 扰码后数据 */
    QByteArray scramble(const QByteArray& data);

    /** @brief 解扰(与扰码相同操作) @param data 输入数据 @return 解扰后数据 */
    QByteArray descramble(const QByteArray& data);

    /** @brief 生成PRBS序列 @param length 比特长度 @return PRBS字节序列 */
    QByteArray generatePRBS(int length);

    /** @brief 获取当前LFSR状态 */
    quint32 state() const { return m_state; }

    /** @brief 设置LFSR初始状态 */
    void setState(quint32 state) { m_state = state; m_initialState = state; }

    /** @brief 重置到初始状态 */
    void reset() { m_state = m_initialState; }

    /** @brief 获取多项式阶数 */
    int degree() const { return m_degree; }

    /** @brief 获取当前多项式信息 */
    QString polynomialInfo() const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 扰码完成 @param bytes 字节数 */
    void scrambleCompleted(int bytes);

private:
    /** @brief 单步Fibonacci LFSR */
    quint8 stepFibonacci();

    /** @brief 单步Galois LFSR */
    quint8 stepGalois();

    QVector<int> m_taps;                 ///< 反馈抽头位置
    int m_degree = 16;                   ///< 寄存器阶数
    LfsrType m_type = Galois;            ///< LFSR类型
    quint32 m_state = 0x1;               ///< 当前状态
    quint32 m_initialState = 0x1;        ///< 初始状态
    bool m_configured = false;           ///< 是否已配置

    Stats m_stats;
    double m_timeSum = 0.0;
};
