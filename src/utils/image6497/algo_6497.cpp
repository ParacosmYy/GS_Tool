/**
 * @file algo_6497.cpp
 */
#include "image6497/algo_6497.h"
QVector<double> algo_6497::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
