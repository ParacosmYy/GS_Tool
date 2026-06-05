/**
 * @file ConvolutionalCoder.h
 * @brief 卷积码编解码器 — Viterbi解码纠错
 *
 * 功能: 卷积码编码器生成编码比特流，
 *       Viterbi算法软/硬判决解码，支持可配置约束长度与生成多项式，
 *       统计编解码次数与平均处理耗时。
 */
#ifndef CONVOLUTIONALCODER_H
#define CONVOLUTIONALCODER_H

#include <QObject>
#include <QVector>
#include <QByteArray>
#include <QElapsedTimer>

/**
 * @brief 卷积码编解码器(含Viterbi解码)
 */
class ConvolutionalCoder : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalEncodes = 0;          ///< 累计编码次数
        quint64 totalDecodes = 0;          ///< 累计解码次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(毫秒)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ConvolutionalCoder(QObject* parent = nullptr);

    /**
     * @brief 编码输入字节流
     * @param data 原始数据
     * @return 编码后数据(比特已打包为字节)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief Viterbi解码
     * @param data 接收的编码数据
     * @return 解码后原始数据
     */
    QByteArray decode(const QByteArray& data);

    /**
     * @brief 设置约束长度K
     * @param length 约束长度(3~9)
     */
    void setConstraintLength(int length);

    /**
     * @brief 设置生成多项式(八进制表示)
     * @param polynomials 生成多项式列表
     */
    void setGeneratorPolynomials(const QVector<int>& polynomials);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 编码完成 */
    void encoded();
    /** @brief 解码完成 */
    void decoded();
    /** @brief 错误发生 */
    void error(const QString& message);

private:
    /**
     * @brief 编码单个比特
     * @param inputBit 输入比特
     * @return 编码输出比特(每个生成多项式一个)
     */
    QVector<int> encodeBit(int inputBit);

    /**
     * @brief Viterbi核心算法
     * @param receivedBits 接收的比特序列
     * @return 解码后的比特序列
     */
    QVector<int> viterbiDecode(const QVector<int>& receivedBits);

    /**
     * @brief 计算分支度量
     * @param expected 期望输出
     * @param received 实际接收
     * @return 汉明距离
     */
    int branchMetric(const QVector<int>& expected,
                     const QVector<int>& received) const;

    /**
     * @brief 获取某状态某输入的编码输出
     * @param state 编码器状态
     * @param input 输入比特
     * @return 编码输出
     */
    QVector<int> getOutput(int state, int input) const;

    /**
     * @brief 回溯路径
     * @param paths 路径历史
     * @param finalState 最终状态
     * @param length 回溯长度
     * @return 解码比特
     */
    QVector<int> traceback(const QVector<QVector<int>>& paths,
                           int finalState, int length) const;

    int m_constraintLength;           ///< 约束长度K
    QVector<int> m_generators;        ///< 生成多项式列表
    int m_numStates;                  ///< 状态数 2^(K-1)
    int m_shiftRegister;              ///< 编码器移位寄存器
    mutable Stats m_stats;            ///< 统计信息(mutable支持const方法)
    double m_timeSum;                 ///< 累计处理时间
};

#endif // CONVOLUTIONALCODER_H
