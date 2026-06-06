/**
 * @file algo_7036.cpp
 */
#include "geometry7036/algo_7036.h"
QVector<double> algo_7036::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
