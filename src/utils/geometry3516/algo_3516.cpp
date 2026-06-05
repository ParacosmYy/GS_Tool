/**
 * @file algo_3516.cpp
 */
#include "geometry3516/algo_3516.h"
QVector<double> algo_3516::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
