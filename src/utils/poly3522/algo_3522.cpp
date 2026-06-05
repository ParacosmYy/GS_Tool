/**
 * @file algo_3522.cpp
 */
#include "poly3522/algo_3522.h"
QVector<double> algo_3522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
