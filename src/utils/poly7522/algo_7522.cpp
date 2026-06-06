/**
 * @file algo_7522.cpp
 */
#include "poly7522/algo_7522.h"
QVector<double> algo_7522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
