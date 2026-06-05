/**
 * @file FastWalshHadamard.h
 * @brief 快速Walsh-Hadamard变换(FWHT)
 *
 * 功能: 实现快速Walsh-Hadamard变换及其逆变换。
 *       支持任意2^k长度的输入向量，O(n log n)时间复杂度。
 *       统计变换次数/平均耗时。
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @class FastWalshHadamard
 * @brief 快速Walsh-Hadamard变换 — 蝶形运算实现
 *
 * Walsh-Hadamard变换是一种广义傅里叶变换，使用+1/-1基函数。
 * 常用于信号处理、编码理论和压缩感知。
 * 正变换: X[k] = sum(x[i] * walsh(i,k)) / sqrt(n)
 * 逆变换: x[i] = sum(X[k] * walsh(i,k)) / sqrt(n)
 */
class FastWalshHadamard : public QObject
{
    Q_OBJECT

public:
    /** @brief 运行统计信息 */
    struct Stats {
        quint64 totalTransforms = 0;  ///< 总变换次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent QObject父对象
     */
    explicit FastWalshHadamard(QObject* parent = nullptr);

    /**
     * @brief 正向Walsh-Hadamard变换
     * @param input 输入向量(长度必须为2^k)
     * @return 变换结果(与输入等长)
     */
    QVector<double> forward(const QVector<double>& input);

    /**
     * @brief 逆Walsh-Hadamard变换
     * @param input 输入向量(长度必须为2^k)
     * @return 逆变换结果(与输入等长)
     */
    QVector<double> inverse(const QVector<double>& input);

    /** @brief 获取统计信息 */
    Stats stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 变换完成信号 @param size 变换向量长度 */
    void transformCompleted(int size);

private:
    /**
     * @brief 执行FWHT蝶形运算
     * @param data 输入/输出数据(原地计算)
     * @param normalize 是否除以sqrt(n)归一化
     */
    void transformInPlace(QVector<double>& data, bool normalize) const;

    /** @brief 检查是否为2的幂次 */
    bool isPowerOfTwo(int n) const;

    /** @brief 计算下一个2的幂次 */
    int nextPowerOfTwo(int n) const;

    mutable Stats m_stats;         ///< 统计信息
    double m_timeSum = 0.0;        ///< 累计耗时
};
