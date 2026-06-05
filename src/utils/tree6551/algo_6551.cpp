/**
 * @file algo_6551.cpp
 */
#include "tree6551/algo_6551.h"
QVector<double> algo_6551::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
