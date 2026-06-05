#pragma once
#include <QObject>
#include <QVector>

/**
 * @brief 快速离散余弦变换
 *
 * 基于FFT实现快速DCT计算，支持DCT-I/II/III/IV四种类型，
 * 广泛用于图像压缩(JPEG)和音频编码(MP3/AAC)。
 */
class DCTFast4 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats {
        int totalComputations = 0;   ///< 已完成计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit DCTFast4(QObject* parent = nullptr);

    /** @brief 设置变换点数(2的幂) */
    void setSize(int n);
    /** @brief 设置DCT类型(1-4) */
    void setType(int type);
    /** @brief 执行快速DCT计算 */
    QVector<double> compute(const QVector<double>& samples);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成，返回输出系数个数 */
    void computationCompleted(int coeffCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 256;
    int m_type = 2;
};
