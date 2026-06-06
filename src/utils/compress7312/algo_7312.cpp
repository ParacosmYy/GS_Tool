/**
 * @file algo_7312.cpp
 */
#include "compress7312/algo_7312.h"
QVector<double> algo_7312::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
