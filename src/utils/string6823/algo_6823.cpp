/**
 * @file algo_6823.cpp
 */
#include "string6823/algo_6823.h"
QVector<double> algo_6823::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
