/**
 * @file BCHCode4.h
 * @brief BCH循环码编解码器(Berlekamp-Massey) — BCH Cyclic Code Encoder/Decoder with BM Algorithm
 *
 * 功能: 实现BCH循环码的编码和解码。编码基于GF(2)上生成多项式。
 *       解码使用Berlekamp-Massey算法求解错误定位多项式，
 *       Chien搜索定位错误位置，支持多比特纠错。
 *
 * 协作: ReedSolomon3(RS码) / ConvolutionalCode(卷积码) / CrcCalculator(CRC校验)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief BCH循环码编解码器
 */
class BCHCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;           ///< 累计编码次数
        quint64 totalDecodes = 0;           ///< 累计解码次数
        quint64 totalErrorsCorrected = 0;   ///< 累计纠正错误数
        quint64 decodeFailures = 0;         ///< 解码失败次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造BCH码
     * @param m GF(2^m)的m值(码长n=2^m-1)
     * @param t 纠错能力(可纠正t个错误)
     * @param parent 父QObject
     */
    explicit BCHCode4(int m = 4, int t = 1, QObject* parent = nullptr);
    ~BCHCode4() override;

    /**
     * @brief 编码：信息比特 -> 码字
     * @param message 信息比特(k位)
     * @return 码字(n位)
     */
    QVector<int> encode(const QVector<int>& message) const;

    /**
     * @brief 解码：接收码字 -> 纠错后信息比特
     * @param received 接收码字(n位)
     * @return 纠错后的信息比特(k位)，失败返回空
     */
    QVector<int> decode(const QVector<int>& received);

    /** @brief 码长n = 2^m - 1 */
    int n() const { return m_n; }

    /** @brief 信息位数k */
    int k() const { return m_k; }

    /** @brief 纠错能力t */
    int t() const { return m_t; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 解码完成 @param errors 纠正的错误数 @param success 是否成功 */
    void decodeCompleted(int errors, bool success);

private:
    /** @brief GF(2^m)域元素乘法(使用指数/对数表) */
    int gfMul(int a, int b) const;

    /** @brief GF(2^m)域元素加法(异或) */
    static int gfAdd(int a, int b) { return a ^ b; }

    /** @brief GF(2^m)域元素求逆 */
    int gfInverse(int a) const;

    /** @brief 计算伴随式 */
    QVector<int> computeSyndromes(const QVector<int>& received) const;

    /** @brief Berlekamp-Massey算法求错误定位多项式 */
    QVector<int> berlekampMassey(const QVector<int>& syndromes) const;

    /** @brief Chien搜索：求错误位置 */
    QVector<int> chienSearch(const QVector<int>& errorLocator) const;

    /** @brief 生成GF(2)上的生成多项式 */
    void buildGeneratorPolynomial();

    /** @brief 初始化GF(2^m)域运算表 */
    void buildGField();

    int m_m;        ///< GF(2^m)的m
    int m_t;        ///< 纠错能力
    int m_n;        ///< 码长
    int m_k;        ///< 信息位数

    QVector<int> m_gfExp;       ///< 指数表
    QVector<int> m_gfLog;       ///< 对数表
    QVector<int> m_generator;   ///< 生成多项式系数(降幂)

    Stats m_stats;
    double m_timeSum = 0.0;
};
