/**
 * @file algo_7237.cpp
 */
#include "image7237/algo_7237.h"
QVector<double> algo_7237::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
