/**
 * @file algo_4522.cpp
 */
#include "poly4522/algo_4522.h"
QVector<double> algo_4522::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
