/**
 * @file algo_5512.cpp
 */
#include "compress5512/algo_5512.h"
QVector<double> algo_5512::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
