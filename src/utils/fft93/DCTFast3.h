#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 快速DCT(离散余弦变换)工具类
 *
 * 提供快速离散余弦变换功能，支持DCT-I/II/III/IV类型选择，
 * 广泛用于信号压缩和特征提取。
 */
class DCTFast3 : public QObject {
    Q_OBJECT
public:
    /// 计算统计信息
    struct Stats {
        int totalComputations = 0;  ///< 总计算次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    explicit DCTFast3(QObject* parent = nullptr);

    /** @brief 设置变换长度 */
    void setSize(int size);

    /** @brief 设置DCT类型(1-4) */
    void setType(int type);

    /** @brief 对输入信号执行快速DCT计算 */
    QVector<double> compute(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 计算完成信号，返回输出系数点数 */
    void computationCompleted(int coeffSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_size = 256;
    int m_type = 2;
};
