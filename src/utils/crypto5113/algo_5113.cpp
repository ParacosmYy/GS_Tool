/**
 * @file algo_5113.cpp
 */
#include "crypto5113/algo_5113.h"
QVector<double> algo_5113::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
