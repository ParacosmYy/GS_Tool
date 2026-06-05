/**
 * @file MinSpanTree2.h
 * @brief Minimum spanning tree (Prim/Kruskal)
 */
#pragma once
#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Minimum spanning tree (Prim/Kruskal)
 */
class MinSpanTree2 : public QObject {
    Q_OBJECT
public:
    struct Stats { quint64 calls = 0; quint64 items = 0; quint64 errors = 0; };
    explicit MinSpanTree2(QObject *p = nullptr) : QObject(p) {}
    ~MinSpanTree2() override = default;
    QVector<double> compute(const QVector<double> &input);
    Stats stats() const { return m_stats; }
    void resetStats() { m_stats = {}; }
signals:
    void computed(const QVector<double> &result);
private:
    Stats m_stats;
};

