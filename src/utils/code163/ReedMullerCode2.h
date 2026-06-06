/**
 * @file ReedMullerCode2.h
 * @brief Reed-Muller码RM(r,m)编码器 — Reed-Muller RM(r,m) Encoder with Majority-Logic Decoding
 *
 * 功能: 实现Reed-Muller码RM(r,m)的编码与多数逻辑译码。
 *       支持任意阶数r和长度m参数，生成矩阵构造，
 *       基于Hadamard变换的快速多数逻辑译码。
 *
 * 协作: GolayCode24(纠错码) / HammingCode(汉明码) / CrcEngine(CRC校验)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Reed-Muller码编解码器
 */
class ReedMullerCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief 码参数 */
    struct CodeParams {
        int r;              ///< 阶数
        int m;              ///< 参数m(码长=2^m)
        int n;              ///< 码长
        int k;              ///< 信息位数
        int d;              ///< 最小距离
    };

    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;        ///< 累计编码次数
        quint64 totalDecodes = 0;        ///< 累计译码次数
        quint64 totalBitErrors = 0;      ///< 累计纠正比特错误数
        double avgProcessingTimeMs = 0.0;///< 平均处理耗时(ms)
    };

    explicit ReedMullerCode2(QObject* parent = nullptr);
    ~ReedMullerCode2() override;

    /** @brief 设置RM码参数(r,m) */
    bool setParameters(int r, int m);

    /** @brief 获取码参数 */
    CodeParams parameters() const { return m_params; }

    /**
     * @brief 编码信息比特
     * @param message 信息比特(长度=k)
     * @return 码字(长度=n)
     */
    QVector<int> encode(const QVector<int>& message);

    /**
     * @brief 多数逻辑译码
     * @param received 接收码字(长度=n)
     * @return 译码后信息比特
     */
    QVector<int> decode(const QVector<int>& received);

    /**
     * @brief 添加随机噪声(翻转指定数目的比特)
     * @param codeword 码字
     * @param numErrors 错误比特数
     * @return 含噪码字
     */
    QVector<int> addNoise(const QVector<int>& codeword, int numErrors) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param n 码长 */
    void encodeCompleted(int n);
    /** @brief 译码完成 @param corrected 纠错数 */
    void decodeCompleted(int corrected);

private:
    /** @brief 构造生成矩阵G */
    void buildGeneratorMatrix();

    /** @brief 计算组合数C(n,k) */
    static int binomial(int n, int k);

    /** @brief 列举指定权重的所有行索引 */
    QVector<QVector<int>> enumerateRows(int weight) const;

    /** @brief Hadamard变换(快速) */
    void hadamardTransform(QVector<double>& data) const;

    /** @brief 模2加法 */
    static QVector<int> xorVectors(const QVector<int>& a, const QVector<int>& b);

    CodeParams m_params{0, 0, 0, 0, 0};
    QVector<QVector<int>> m_generator;  ///< 生成矩阵(k x n)

    Stats m_stats;
    double m_timeSum = 0.0;
};
