/**
 * @file algo_7752.cpp
 */
#include "compress7752/algo_7752.h"
QVector<double> algo_7752::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
