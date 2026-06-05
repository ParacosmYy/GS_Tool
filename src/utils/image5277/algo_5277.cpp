/**
 * @file algo_5277.cpp
 */
#include "image5277/algo_5277.h"
QVector<double> algo_5277::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
