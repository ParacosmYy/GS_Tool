#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ReedSolomon12 - Reed-Solomon纠错码第12代实现
 *
 * 提供RS编解码功能，支持GF(2^m)上的多项式运算、
 * Berlekamp-Massey解码算法及纠错能力配置。
 */
class ReedSolomon12 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit ReedSolomon12(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief RS编码
     * @param message 原始消息符号序列
     * @param nsym 校验符号数量
     * @return 编码后的码字（消息+校验）
     */
    QVector<int> encode(const QVector<int>& message, int nsym);

    /**
     * @brief RS解码与纠错
     * @param received 接收到的含误码字
     * @param nsym 校验符号数量
     * @return 纠错后的消息符号序列
     */
    QVector<int> decode(const QVector<int>& received, int nsym);

    /**
     * @brief 计算伴随式（Syndrome）
     * @param received 接收码字
     * @param nsym 校验符号数量
     * @return 伴随式多项式系数
     */
    QVector<int> computeSyndromes(const QVector<int>& received, int nsym);

    /**
     * @brief Berlekamp-Massey算法求错误定位多项式
     * @param syndromes 伴随式
     * @return 错误定位多项式系数
     */
    QVector<int> berlekampMassey(const QVector<int>& syndromes);

signals:
    void decodeCompleted(int correctedErrors);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
