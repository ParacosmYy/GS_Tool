/**
 * @file algo_4081.cpp
 */
#include "interp4081/algo_4081.h"
QVector<double> algo_4081::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
