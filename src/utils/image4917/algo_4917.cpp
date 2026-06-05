/**
 * @file algo_4917.cpp
 */
#include "image4917/algo_4917.h"
QVector<double> algo_4917::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
