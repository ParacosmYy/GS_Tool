/**
 * @file GeneralizedLinearModel.h
 * @brief 广义线性模型 — Logistic/Probit/Poisson链接
 *
 * 功能: 实现GLM框架，支持logistic回归(二分类)、
 *       probit回归和Poisson回归(计数数据)三种链接函数。
 *
 * 协作: CrossValidator(模型验证) / StackedEnsemble(集成基模型)
 */
#ifndef GENERALIZEDLINEARMODEL_H
#define GENERALIZEDLINEARMODEL_H

#include <QObject>
#include <QVector>

/**
 * @brief 广义线性模型
 */
class GeneralizedLinearModel : public QObject {
    Q_OBJECT

public:
    /** @brief 分布族/链接函数 */
    enum class Family {
        Logistic,      ///< Logistic回归(二分类)
        Probit,        ///< Probit回归(二分类)
        Poisson        ///< Poisson回归(计数数据)
    };
    Q_ENUM(Family)

    /** @brief 统计 */
    struct Stats {
        quint64 totalFits = 0;              ///< 累计拟合次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit GeneralizedLinearModel(QObject* parent = nullptr);

    /**
     * @brief 拟合GLM
     * @param data 特征数据(每行一个样本)
     * @param labels 响应变量
     * @param family 分布族
     * @return 偏差度(Deviance, 越小越好)
     */
    double fit(const QVector<QVector<double>>& data,
               const QVector<double>& labels,
               Family family = Family::Logistic);

    /**
     * @brief 预测样本响应值
     * @param sample 输入样本特征
     * @return 预测值
     */
    double predict(const QVector<double>& sample) const;

    /**
     * @brief 预测概率(Logistic/Probit)
     * @param sample 输入样本特征
     * @return 预测概率[0,1]
     */
    double predictProba(const QVector<double>& sample) const;

    /** @brief 获取模型系数 */
    QVector<double> coefficients() const { return m_coeffs; }
    double intercept() const { return m_intercept; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 拟合完成 @param deviance 最终偏差度 */
    void fitCompleted(double deviance);

private:
    /** @brief 链接函数(线性预测→均值) */
    double inverseLink(double eta) const;

    /** @brief 链接函数导数 */
    double inverseLinkDeriv(double eta) const;

    /** @brief 计算偏差度 */
    double deviance(const QVector<QVector<double>>& data,
                    const QVector<double>& labels) const;

    Family m_family = Family::Logistic;     ///< 分布族
    QVector<double> m_coeffs;               ///< 回归系数
    double m_intercept = 0.0;               ///< 截距
    mutable Stats m_stats;                           ///< 统计信息
    mutable double m_timeSum = 0.0;          ///< 累计耗时
};

#endif // GENERALIZEDLINEARMODEL_H
