/**
 * @file algo_3094.cpp
 */
#include "numeric3094/algo_3094.h"
QVector<double> algo_3094::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
