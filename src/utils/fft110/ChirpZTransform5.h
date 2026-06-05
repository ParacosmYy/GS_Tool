#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Chirp-Z变换(CZT)实现
 *
 * 在Z平面螺旋轮廓上计算信号的Z变换，支持任意起点和终点的频率分析，
 * 比标准FFT更灵活地聚焦于特定频段，适合高分辨率局部频谱分析。
 */
class ChirpZTransform5 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalTransforms = 0; double avgProcessingTimeMs = 0.0; };

    explicit ChirpZTransform5(QObject* parent = nullptr);

    /** @brief 设置螺旋轮廓参数A(起始点)和W(比率)，控制频谱扫描路径 */
    void setSpiralParams(const QPair<double, double>& A, const QPair<double, double>& W);

    /** @brief 设置输出频率点数M，决定频谱分辨率 */
    void setOutputSize(int m);

    /** @brief 对输入信号执行Chirp-Z变换，返回M个复数频率点 */
    QVector<QPair<double, double>> transform(const QVector<double>& input);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 变换完成信号，返回输出点数 */
    void transformCompleted(int outputSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_outputSize = 256;
    QPair<double, double> m_A;
    QPair<double, double> m_W;
};
