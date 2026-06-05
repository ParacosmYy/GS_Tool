/**
 * @file algo_2831.cpp
 */
#include "tree2831/algo_2831.h"
QVector<double> algo_2831::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
