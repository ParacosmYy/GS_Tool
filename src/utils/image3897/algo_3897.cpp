/**
 * @file algo_3897.cpp
 */
#include "image3897/algo_3897.h"
QVector<double> algo_3897::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
