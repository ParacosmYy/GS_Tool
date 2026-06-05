/**
 * @file algo_4352.cpp
 */
#include "compress4352/algo_4352.h"
QVector<double> algo_4352::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
