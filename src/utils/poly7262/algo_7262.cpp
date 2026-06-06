/**
 * @file algo_7262.cpp
 */
#include "poly7262/algo_7262.h"
QVector<double> algo_7262::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
