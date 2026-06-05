/**
 * @file algo_4831.cpp
 */
#include "tree4831/algo_4831.h"
QVector<double> algo_4831::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
