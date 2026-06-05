/**
 * @file algo_2866.cpp
 */
#include "signal2866/algo_2866.h"
QVector<double> algo_2866::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
