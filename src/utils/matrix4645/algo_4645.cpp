/**
 * @file algo_4645.cpp
 */
#include "matrix4645/algo_4645.h"
QVector<double> algo_4645::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
