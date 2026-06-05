/**
 * @file algo_3837.cpp
 */
#include "image3837/algo_3837.h"
QVector<double> algo_3837::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
