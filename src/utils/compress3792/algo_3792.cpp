/**
 * @file algo_3792.cpp
 */
#include "compress3792/algo_3792.h"
QVector<double> algo_3792::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
