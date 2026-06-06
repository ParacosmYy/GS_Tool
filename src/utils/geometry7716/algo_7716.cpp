/**
 * @file algo_7716.cpp
 */
#include "geometry7716/algo_7716.h"
QVector<double> algo_7716::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
