/**
 * @file algo_3917.cpp
 */
#include "image3917/algo_3917.h"
QVector<double> algo_3917::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
