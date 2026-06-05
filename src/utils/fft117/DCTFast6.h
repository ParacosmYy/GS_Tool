#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief DCTFast6 - 快速离散余弦变换第6代实现
 *
 * 提供DCT-I/II/III/IV的快速计算，支持基于FFT的加速、
 * 正交归一化及逆变换。
 */
class DCTFast6 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };
    explicit DCTFast6(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 执行DCT-II（最常用形式，JPEG使用）
     * @param inputSignal 输入时域信号
     * @return DCT系数序列
     */
    QVector<double> forward(const QVector<double>& inputSignal);

    /**
     * @brief 执行逆DCT-II变换
     * @param dctCoefficients DCT系数序列
     * @return 重建的时域信号
     */
    QVector<double> inverse(const QVector<double>& dctCoefficients);

    /**
     * @brief 设置DCT类型
     * @param type DCT类型 (1/2/3/4)
     */
    void setDCTType(int type);

    /**
     * @brief 启用正交归一化
     * @param enabled 是否启用归一化
     */
    void setNormalization(bool enabled);

signals:
    void transformCompleted(int coefficientCount);

private:
    Stats m_stats; double m_timeSum = 0.0;
};
