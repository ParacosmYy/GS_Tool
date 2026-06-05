/**
 * @file algo_4932.cpp
 */
#include "compress4932/algo_4932.h"
QVector<double> algo_4932::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
