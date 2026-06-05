#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Multitaper4 - 多锥谱估计
 *
 * 使用多个正交离散扁长椭球序列(DPSS/Slepian)作为锥窗，
 * 减少频谱估计方差，提供更稳定的功率谱估计。
 */
class Multitaper4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEstimates = 0;
        int totalTapers = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Multitaper4(QObject* parent = nullptr);

    /** @brief 设置时间带宽积NW和锥数 */
    bool setParameters(double NW, int numTapers);

    /** @brief 估计功率谱密度 */
    QVector<double> estimate(const QVector<double>& signal);

    /** @brief 获取DPSS锥函数 */
    QVector<QVector<double>> tapers() const;

    /** @brief 获取各锥的功率谱(未平均) */
    QVector<QVector<double>> individualSpectra(const QVector<double>& signal);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void estimationCompleted(int taperCount, int fftSize);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    double m_NW = 4.0;
    int m_numTapers = 7;
    QVector<QVector<double>> m_tapers;
};
