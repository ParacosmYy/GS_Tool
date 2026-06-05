/**
 * @file algo_3038.cpp
 */
#include "neural3038/algo_3038.h"
QVector<double> algo_3038::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
