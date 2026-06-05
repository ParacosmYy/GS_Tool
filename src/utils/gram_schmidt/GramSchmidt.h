/**
 * @file GramSchmidt.h
 * @brief Gram-Schmidt正交化 — 经典与修正算法
 *
 * 功能: 将一组线性无关向量正交化为标准正交基。
 *       支持经典Gram-Schmidt (CGS)和修正Gram-Schmidt (MGS)。
 *       MGS数值稳定性优于CGS，推荐默认使用。
 *
 * 协作: QrDecomposition(QR分解) / EigenSolver(特征值求解)
 */
#ifndef GRAMSCHMIDT_H
#define GRAMSCHMIDT_H

#include <QObject>
#include <QVector>

/**
 * @brief Gram-Schmidt正交化引擎
 */
class GramSchmidt : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        quint64 totalOrthogonalizations = 0;  ///< 累计正交化次数
        double avgProcessingTimeMs = 0.0;     ///< 平均处理时间(ms)
    };

    explicit GramSchmidt(QObject* parent = nullptr);

    /** @brief 经典Gram-Schmidt正交化
     *  @param vectors 输入向量组(每行为一个向量)
     *  @return 正交化后的标准正交基 */
    QVector<QVector<double>> orthogonalize(
        const QVector<QVector<double>>& vectors);

    /** @brief 修正Gram-Schmidt正交化(数值更稳定)
     *  @param vectors 输入向量组(每行为一个向量)
     *  @return 正交化后的标准正交基 */
    QVector<QVector<double>> modifiedOrthogonalize(
        const QVector<QVector<double>>& vectors);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 正交化完成 @param vectorCount 向量数 */
    void orthogonalizationCompleted(int vectorCount);

private:
    /** @brief 向量点积 */
    static double dotProduct(const QVector<double>& a,
                             const QVector<double>& b);

    /** @brief 向量范数 */
    static double norm(const QVector<double>& v);

    /** @brief 向量数乘 */
    static QVector<double> scale(const QVector<double>& v, double s);

    /** @brief 向量减法 */
    static QVector<double> subtract(const QVector<double>& a,
                                    const QVector<double>& b);

    double m_timeSum;   ///< 处理时间累加器
    Stats  m_stats;     ///< 统计信息
};

#endif // GRAMSCHMIDT_H
