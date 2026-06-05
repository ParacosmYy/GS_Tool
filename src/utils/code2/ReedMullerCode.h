/**
 * @file ReedMullerCode.h
 * @brief Reed-Muller纠错码 — 编码/解码/生成矩阵
 *
 * 功能: 实现RM(r,m)纠错编码，支持任意阶数r和长度参数m。
 *       提供编码、多数逻辑解码、生成矩阵构造、最小距离计算。
 *       适用于通信信道编码、数据容错存储。
 *
 * * 协作: CRC(校验) / DataValidator(数据验证)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief Reed-Muller纠错码 — RM(r,m)编码解码
 */
class ReedMullerCode : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;           ///< 累计编码次数
        quint64 totalDecoded = 0;           ///< 累计解码次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param r RM阶数(order)
     * @param m RM长度参数(码长 n = 2^m)
     * @param parent 父对象
     */
    explicit ReedMullerCode(int r, int m, QObject* parent = nullptr);

    /**
     * @brief 编码消息
     * @param message 消息位向量(长度 = sum_{i=0}^{r} C(m,i))
     * @return 编码后的码字(长度 = 2^m)
     */
    QVector<int> encode(const QVector<int>& message);

    /**
     * @brief 解码接收码字(多数逻辑解码)
     * @param received 接收的码字(长度 = 2^m)
     * @return 解码后的消息位向量
     */
    QVector<int> decode(const QVector<int>& received);

    /**
     * @brief 获取生成矩阵
     * @return 生成矩阵(每行为一个基向量)
     */
    QVector<QVector<int>> generatorMatrix() const;

    /**
     * @brief 计算最小汉明距离
     * @return 最小距离 = 2^(m-r)
     */
    int minimumDistance() const;

    /**
     * @brief 获取消息长度(k)
     * @return 消息位数
     */
    int messageLength() const;

    /**
     * @brief 获取码字长度(n)
     * @return 码字长度
     */
    int codewordLength() const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param msgLen 消息长度 @param codeLen 码字长度 */
    void encoded(int msgLen, int codeLen);

    /** @brief 解码完成信号 @param errors 纠正的错误数 */
    void decoded(int errors);

private:
    /**
     * @brief 构造RM生成矩阵
     * @param r 阶数
     * @param m 长度参数
     * @return 生成矩阵
     */
    static QVector<QVector<int>> buildGeneratorMatrix(int r, int m);

    /**
     * @brief 计算二项式系数 C(n,k)
     * @param n 总数
     * @param k 选取数
     * @return 组合数
     */
    static int binomial(int n, int k);

    /**
     * @brief GF(2)矩阵向量乘法
     * @param matrix 矩阵
     * @param vec 向量
     * @return 结果向量(模2)
     */
    static QVector<int> gf2Multiply(const QVector<QVector<int>>& matrix,
                                     const QVector<int>& vec);

    /**
     * @brief 构造所有m变量的r阶以内子集
     * @param m 变量数
     * @param r 最大阶数
     * @return 子集列表(每个子集为变量索引集合)
     */
    static QVector<QVector<int>> buildMonomials(int m, int r);

    /**
     * @brief 评估单项式在给定赋值下的值
     * @param monomial 变量索引集合
     * @param assignment 变量赋值(0/1)
     * @return 评估结果(0或1)
     */
    static int evaluateMonomial(const QVector<int>& monomial,
                                const QVector<int>& assignment);

    int m_order;                                ///< RM阶数(r)
    mutable int m_lengthParam;                  ///< RM长度参数(m)
    int m_n;                                    ///< 码字长度(2^m)
    int m_k;                                    ///< 消息长度
    QVector<QVector<int>> m_generator;          ///< 生成矩阵
    QVector<QVector<int>> m_monomials;          ///< 单项式列表(对应生成矩阵行)

    QElapsedTimer m_timer;    ///< 计时器
    double m_timeSum;          ///< 累计耗时
    Stats m_stats;
};
