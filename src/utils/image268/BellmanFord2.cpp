/**
 * @file BellmanFord2.cpp
 * @brief BellmanFord2 implementation
 */
#include "image268/BellmanFord2.h"
#include <QElapsedTimer>
QVector<double> BellmanFord2::compute(const QVector<double> &input) {
    QElapsedTimer t; t.start();
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

