/**
 * @file NaiveBayesClassifier.h
 * @brief 朴素贝叶斯分类器 — 高斯/多项式模式
 *
 * 功能: 实现朴素贝叶斯分类，支持高斯(连续特征)
 *       和多项式(离散特征)两种模式，用于文本和通用分类。
 *
 * 协作: CrossValidator(交叉验证) / StackedEnsemble(集成学习)
 */
#ifndef NAIVEBAYESCLASSIFIER_H
#define NAIVEBAYESCLASSIFIER_H

#include <QObject>
#include <QVector>
#include <QMap>

/**
 * @brief 朴素贝叶斯分类器
 */
class NaiveBayesClassifier : public QObject {
    Q_OBJECT

public:
    /** @brief 分类模式 */
    enum class Mode {
        Gaussian,       ///< 高斯模式(连续特征)
        Multinomial     ///< 多项式模式(离散特征)
    };
    Q_ENUM(Mode)

    /** @brief 统计 */
    struct Stats {
        quint64 totalTrainings = 0;         ///< 累计训练次数
        quint64 totalPredictions = 0;       ///< 累计预测次数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit NaiveBayesClassifier(QObject* parent = nullptr);

    /**
     * @brief 训练分类器
     * @param data 特征数据(每行一个样本)
     * @param labels 标签列表
     * @param mode 分类模式
     * @return 是否成功
     */
    bool train(const QVector<QVector<double>>& data,
               const QVector<int>& labels,
               Mode mode = Mode::Gaussian);

    /**
     * @brief 预测样本类别
     * @param sample 输入样本特征
     * @return 预测类别
     */
    int predict(const QVector<double>& sample) const;

    /**
     * @brief 预测各类别概率
     * @param sample 输入样本特征
     * @return 各类别概率
     */
    QVector<QPair<int, double>> predictProba(
        const QVector<double>& sample) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 训练完成 @param sampleCount 训练样本数 */
    void trainingCompleted(int sampleCount);

private:
    /** @brief 高斯模式下的类别参数 */
    struct ClassParams {
        double mean = 0.0;          ///< 特征均值
        double variance = 1.0;      ///< 特征方差
    };

    /** @brief 计算高斯对数概率 */
    double gaussianLogProb(double x, double mean, double var) const;

    /** @brief 计算多项式对数概率 */
    double multinomialLogProb(double x, int label, int featIdx) const;

    Mode m_mode = Mode::Gaussian;                           ///< 当前模式
    QMap<int, double> m_classPriors;                        ///< 类别先验概率
    QMap<int, QVector<ClassParams>> m_gaussianParams;       ///< 高斯参数
    QMap<int, QVector<QMap<double, double>>> m_multiParams; ///< 多项式参数
    int m_totalSamples = 0;                                 ///< 训练样本总数
    Stats m_stats;                                           ///< 统计信息
    mutable double m_timeSum = 0.0;                          ///< 累计耗时
};

#endif // NAIVEBAYESCLASSIFIER_H
