/**
 * @file algo_2897.cpp
 */
#include "image2897/algo_2897.h"
QVector<double> algo_2897::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
