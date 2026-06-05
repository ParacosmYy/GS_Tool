/**
 * @file algo_5940.cpp
 */
#include "sort5940/algo_5940.h"
QVector<double> algo_5940::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
