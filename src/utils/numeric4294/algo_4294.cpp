/**
 * @file algo_4294.cpp
 */
#include "numeric4294/algo_4294.h"
QVector<double> algo_4294::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
