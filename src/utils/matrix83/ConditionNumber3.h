#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief ConditionNumber3 - 矩阵条件数估计器
 *
 * 估计矩阵的条件数(kappa)和数值稳定性指标，
 * 支持1-范数、2-范数和无穷范数条件数计算。
 */
class ConditionNumber3 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalEstimates = 0;
        int totalWarnings = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit ConditionNumber3(QObject* parent = nullptr);

    /** @brief 计算1-范数条件数 */
    double conditionNumber1(const QVector<QVector<double>>& matrix) const;

    /** @brief 计算2-范数条件数(奇异值比) */
    double conditionNumber2(const QVector<QVector<double>>& matrix);

    /** @brief 计算无穷范数条件数 */
    double conditionNumberInf(const QVector<QVector<double>>& matrix) const;

    /** @brief 评估数值稳定性等级: good/fair/poor/singular */
    QString stabilityRating(double conditionNumber) const;

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void conditionEstimated(double kappa, QString rating);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
};
