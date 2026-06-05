#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief CascadeCode8 - 级联码第8代实现
 *
 * 提供内外码级联的纠错编码方案，支持RS+卷积码、
 * BCH+LDPC等常见级联组合及迭代解码。
 */
class CascadeCode8 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalDecodeOps = 0; double avgProcessingTimeMs = 0.0; };
    explicit CascadeCode8(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 级联编码（先外码后内码）
     * @param dataBits 原始信息比特
     * @param outerCode 外码类型 (RS/BCH)
     * @param innerCode 内码类型 (Conv/LDPC)
     * @return 级联编码后的码字
     */
    QVector<int> encode(const QVector<int>& dataBits,
                        const QString& outerCode, const QString& innerCode);

    /**
     * @brief 级联迭代解码
     * @param received 接收序列
     * @param maxOuterIterations 外层迭代次数
     * @return 解码后的信息比特
     */
    QVector<int> decode(const QVector<double>& received, int maxOuterIterations = 3);

    /**
     * @brief 设置外码参数
     * @param codeLength 码长
     * @param errorCorrection 纠错能力
     */
    void setOuterCodeParams(int codeLength, int errorCorrection);

    /**
     * @brief 设置内码参数
     * @param constraintLength 约束长度
     * @param generatorPolynomials 生成多项式（八进制）
     */
    void setInnerCodeParams(int constraintLength, const QVector<int>& generatorPolynomials);

signals:
    void decodeCompleted(int correctedErrors);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
