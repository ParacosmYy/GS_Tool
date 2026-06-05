/**
 * @file algo_4551.cpp
 */
#include "tree4551/algo_4551.h"
QVector<double> algo_4551::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
