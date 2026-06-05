/**
 * @file algo_4792.cpp
 */
#include "compress4792/algo_4792.h"
QVector<double> algo_4792::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
