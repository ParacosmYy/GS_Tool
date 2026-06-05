#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief NumberTheoretic4 - 数论变换(NTT)
 *
 * 在有限域上的离散傅里叶变换，使用模算术替代
 * 浮点运算，适用于精确卷积和多项式乘法。
 */
class NumberTheoretic4 : public QObject {
    Q_OBJECT
public:
    struct Stats {
        int totalTransforms = 0;
        int totalConvolutions = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit NumberTheoretic4(QObject* parent = nullptr);

    /** @brief 设置模数和原根(需满足NTT条件) */
    bool setModulus(quint64 modulus, quint64 primitiveRoot);

    /** @brief 执行前向NTT */
    QVector<quint64> forward(const QVector<quint64>& input);

    /** @brief 执行逆NTT */
    QVector<quint64> inverse(const QVector<quint64>& input);

    /** @brief 使用NTT计算精确卷积 */
    QVector<quint64> convolution(const QVector<quint64>& a, const QVector<quint64>& b);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int size, quint64 modulus);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    quint64 m_modulus = 998244353;
    quint64 m_primitiveRoot = 3;
};
