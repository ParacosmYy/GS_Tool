/**
 * @file algo_4272.cpp
 */
#include "compress4272/algo_4272.h"
QVector<double> algo_4272::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
