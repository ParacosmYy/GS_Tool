/**
 * @file algo_6842.cpp
 */
#include "poly6842/algo_6842.h"
QVector<double> algo_6842::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
