/**
 * @file BchCode.h
 * @brief BCH编解码器 — Berlekamp-Massey算法的BCH纠错码
 *
 * 功能: 实现GF(2^m)上的BCH码编码与解码，支持可配置码长、
 *       纠错能力。解码采用Berlekamp-Massey算法求错误位置多项式，
 *       Chien搜索定位错误。适用于通信前向纠错、存储纠错。
 *
 * 协作: ReedSolomon(纠删码) / AesCbc(加密通信)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QBitArray>

/**
 * @brief BCH编解码器 — Berlekamp-Massey纠错
 */
class BchCode : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalEncodings = 0;                ///< 累计编码次数
        int totalDecodings = 0;                ///< 累计解码次数
        int totalErrorsCorrected = 0;          ///< 累计纠正错误数
        double avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param m GF(2^m)的m值(通常3~8)
     * @param t 纠错能力(可纠正t个错误)
     * @param parent 父对象
     */
    explicit BchCode(int m = 4, int t = 2, QObject* parent = nullptr);

    /**
     * @brief 编码: 信息比特 → 码字
     * @param message 信息比特
     * @return 码字比特(含校验位)
     */
    QBitArray encode(const QBitArray& message);

    /**
     * @brief 解码: 接收码字 → 纠正后的信息比特
     * @param received 接收码字
     * @return 纠正后的信息比特; 空表示不可纠正
     */
    QBitArray decode(const QBitArray& received);

    /**
     * @brief 计算伴随式
     * @param received 接收码字
     * @return 伴随式向量(2t个值)
     */
    QVector<int> computeSyndromes(const QBitArray& received) const;

    /**
     * @brief Berlekamp-Massey算法求错误定位多项式
     * @param syndromes 伴随式
     * @return 错误定位多项式系数
     */
    QVector<int> berlekampMassey(const QVector<int>& syndromes) const;

    /**
     * @brief Chien搜索定位错误位置
     * @param sigma 错误定位多项式
     * @return 错误位置索引列表
     */
    QList<int> chienSearch(const QVector<int>& sigma) const;

    /**
     * @brief GF(2^m)乘法
     * @param a 元素a
     * @param b 元素b
     * @return a*b in GF(2^m)
     */
    int gfMultiply(int a, int b) const;

    /**
     * @brief GF(2^m)求逆
     * @param a 元素a
     * @return a^(-1); 0表示无逆
     */
    int gfInverse(int a) const;

    /**
     * @brief 获取码长n
     */
    int codeLength() const { return m_n; }

    /**
     * @brief 获取信息位长度k
     */
    int messageLength() const { return m_k; }

    /**
     * @brief 获取纠错能力t
     */
    int errorCapability() const { return m_t; }

    /**
     * @brief 获取生成多项式(系数)
     */
    QVector<int> generatorPolynomial() const { return m_genPoly; }

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
     * @brief 编码完成
     * @param codeLength 码字长度
     */
    void encodingCompleted(int codeLength);

    /**
     * @brief 解码完成
     * @param errorsCorrected 纠正的错误数
     * @param success 是否成功
     */
    void decodingCompleted(int errorsCorrected, bool success);

private:
    /**
     * @brief 初始化GF(2^m)对数/反对数表
     */
    void initGaloisField();

    /**
     * @brief 计算生成多项式
     */
    void computeGeneratorPolynomial();

    /**
     * @brief GF(2^m)多项式求值
     * @param poly 多项式系数
     * @param x 求值点(GF元素)
     * @return 多项式值
     */
    int gfPolyEval(const QVector<int>& poly, int x) const;

    /**
     * @brief GF(2)多项式模除(编码用)
     */
    QBitArray gf2PolyDiv(const QBitArray& dividend, const QBitArray& divisor) const;

    int m_m;                                   ///< GF(2^m)的m
    int m_t;                                   ///< 纠错能力
    int m_n;                                   ///< 码长 = 2^m - 1
    int m_k;                                   ///< 信息位长度

    QVector<int> m_alphaTo;                    ///< 反对数表
    QVector<int> m_indexOf;                    ///< 对数表
    QVector<int> m_genPoly;                    ///< 生成多项式

    Stats m_stats;
    double m_timeSum = 0.0;                     ///< 处理时间累加器
};
