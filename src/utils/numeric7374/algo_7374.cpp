/**
 * @file algo_7374.cpp
 */
#include "numeric7374/algo_7374.h"
QVector<double> algo_7374::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
