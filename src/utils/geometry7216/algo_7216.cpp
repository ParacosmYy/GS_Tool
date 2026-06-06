/**
 * @file algo_7216.cpp
 */
#include "geometry7216/algo_7216.h"
QVector<double> algo_7216::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
