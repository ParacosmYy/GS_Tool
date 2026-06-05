/**
 * @file algo_3352.cpp
 */
#include "compress3352/algo_3352.h"
QVector<double> algo_3352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
