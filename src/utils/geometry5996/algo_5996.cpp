/**
 * @file algo_5996.cpp
 */
#include "geometry5996/algo_5996.h"
QVector<double> algo_5996::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
