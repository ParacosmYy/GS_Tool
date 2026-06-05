#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BchCode11 - BCH纠错码第11代实现
 *
 * 提供BCH编解码功能，支持GF(2)上的二进制BCH码、
 * Peterson-Gorenstein-Zierler解码算法及可配置纠错能力。
 */
class BchCode11 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit BchCode11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 生成BCH码的生成多项式
     * @param codeLength 码长 n
     * @param errorCorrectionT 纠错能力 t
     * @return 生成多项式系数（二进制）
     */
    QVector<int> generatePolynomial(int codeLength, int errorCorrectionT);

    /**
     * @brief BCH编码
     * @param message 原始消息比特序列
     * @param generatorPoly 生成多项式
     * @return 编码后的码字
     */
    QVector<int> encode(const QVector<int>& message, const QVector<int>& generatorPoly);

    /**
     * @brief BCH解码与纠错
     * @param received 接收到的含误码字
     * @param codeLength 码长
     * @param errorCorrectionT 纠错能力
     * @return 纠错后的码字
     */
    QVector<int> decode(const QVector<int>& received, int codeLength, int errorCorrectionT);

    /**
     * @brief 计算伴随式用于错误检测
     * @param received 接收码字
     * @param codeLength 码长
     * @param errorCorrectionT 纠错能力
     * @return 伴随式值列表
     */
    QVector<int> computeSyndromes(const QVector<int>& received, int codeLength,
                                  int errorCorrectionT);

signals:
    void decodeCompleted(int correctedBits);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
