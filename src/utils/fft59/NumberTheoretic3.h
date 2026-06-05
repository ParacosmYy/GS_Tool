#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

class NumberTheoretic3 : public QObject {
    Q_OBJECT
public:
    struct Stats { int totalTransforms = 0; int totalPoints = 0; double avgProcessingTimeMs = 0.0; };
    explicit NumberTheoretic3(QObject* parent = nullptr);
    void setSize(int n);
    void setModulus(qint64 mod);
    void setPrimitiveRoot(qint64 root);
    QVector<qint64> forward(const QVector<qint64>& data);
    QVector<qint64> inverse(const QVector<qint64>& data);
    int size() const { return m_n; }
    Stats stats() const { return m_stats; }
    void resetStatistics();
signals:
    void transformCompleted(int n);
private:
    int m_n = 256; qint64 m_mod = 998244353; qint64 m_root = 3;
    qint64 modPow(qint64 base, qint64 exp, qint64 mod) const;
    Stats m_stats; double m_timeSum = 0.0;
};
