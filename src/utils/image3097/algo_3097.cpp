/**
 * @file algo_3097.cpp
 */
#include "image3097/algo_3097.h"
QVector<double> algo_3097::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
