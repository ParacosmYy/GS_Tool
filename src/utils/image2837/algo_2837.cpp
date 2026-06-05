/**
 * @file algo_2837.cpp
 */
#include "image2837/algo_2837.h"
QVector<double> algo_2837::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
