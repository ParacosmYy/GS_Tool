/**
 * @file RidgeDetector.h
 * @brief 脊线检测引擎 — 连续波峰/波谷追踪与模式识别
 *
 * 功能: 在时频数据或时序数据中检测连续脊线(能量轨迹)，
 *       支持幅度阈值/连续性/频率斜率三种检测模式。
 *
 * 协作: PeakDetector(单点峰值) / FftEngine(频谱脊线)
 */
#ifndef RIDGEDETECTOR_H
#define RIDGEDETECTOR_H

#include <QObject>
#include <QVector>
#include <QList>

class RidgeDetector : public QObject {
    Q_OBJECT

public:
    /** @brief 脊线段 */
    struct Ridge {
        int startRow = 0;               ///< 起始行(时间)
        int endRow = 0;                 ///< 结束行
        int startCol = 0;               ///< 起始列(频率/通道)
        int endCol = 0;                 ///< 结束列
        double peakAmplitude = 0.0;     ///< 峰值幅度
        double averageAmplitude = 0.0;  ///< 平均幅度
        int length = 0;                 ///< 脊线长度(行数)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalScans = 0;             ///< 累计扫描次数
        quint64 totalRidgesFound = 0;       ///< 累计检测脊线数
        double  averageRidgeLength = 0.0;   ///< 平均脊线长度
        int     longestRidge = 0;           ///< 最长脊线
        double  peakAmplitude = 0.0;        ///< 峰值幅度
    };

    explicit RidgeDetector(QObject* parent = nullptr);

    /** @brief 设置幅度阈值 @param threshold 阈值 */
    void setAmplitudeThreshold(double threshold);

    /** @brief 设置最小连续长度 @param length 最小行数 */
    void setMinRidgeLength(int length);

    /** @brief 设置最大列跳变 @param jump 最大跳变列数 */
    void setMaxColumnJump(int jump);

    /** @brief 检测2D数据中的脊线 @param data 2D数据(行优先) @param cols 列数 @return 脊线列表 */
    QList<Ridge> detect(const QVector<double>& data, int cols);

    /** @brief 在单列序列中检测脊线 @param sequence 1D数据 @return 脊线列表 */
    QList<Ridge> detectSequence(const QVector<double>& sequence);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 脊线检测完成 @param count 检测到的脊线数 */
    void ridgesDetected(int count);

private:
    double m_amplitudeThreshold;   ///< 幅度阈值
    int m_minRidgeLength;          ///< 最小脊线长度
    int m_maxColumnJump;           ///< 最大列跳变

    double m_lengthSum;            ///< 长度累加器
    Stats m_stats;
};

#endif // RIDGEDETECTOR_H
