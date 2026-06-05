/**
 * @file Scrambler.h
 * @brief 数据扰码器 — 伪随机序列扰码/解扰
 *
 * 功能: 基于LFSR(线性反馈移位寄存器)的加扰/解扰，
 *       支持自定义多项式/初始状态，统计处理字节数/耗时。
 */
#ifndef SCRAMBLER_H
#define SCRAMBLER_H

#include <QObject>
#include <QByteArray>

class Scrambler : public QObject {
    Q_OBJECT
public:
    /** 扰码统计 */
    struct Stats {
        quint64 totalBytesProcessed = 0;
        quint64 totalOperations = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    /** @brief 构造 @param polynomial LFSR多项式(如0x8003) @param initialState 初始状态 @param parent 父对象 */
    explicit Scrambler(quint32 polynomial = 0x8003,
                       quint32 initialState = 0xFFFF,
                       QObject* parent = nullptr);

    /** @brief 扰码 @param data 输入数据 @return 扰码后数据 */
    QByteArray scramble(const QByteArray& data);

    /** @brief 解扰 @param data 扰码数据 @return 解扰后数据 */
    QByteArray descramble(const QByteArray& data);

    /** @brief 设置多项式 */
    void setPolynomial(quint32 poly);

    /** @brief 设置初始状态 */
    void setInitialState(quint32 state);

    /** @brief 重置LFSR */
    void reset();

    /** @brief 生成伪随机序列 @param length 长度 @return 字节序列 */
    QByteArray generateSequence(int length);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(int bytesProcessed, bool isScramble);

private:
    /** 执行LFSR一步 */
    quint8 nextByte();

    quint32 m_polynomial;
    quint32 m_initialState;
    quint32 m_lfsrState;
    Stats m_stats;
    double m_timeSum;
};

#endif // SCRAMBLER_H
