/**
 * @file algo_4992.cpp
 */
#include "compress4992/algo_4992.h"
QVector<double> algo_4992::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
