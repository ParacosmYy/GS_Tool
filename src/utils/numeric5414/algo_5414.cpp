/**
 * @file algo_5414.cpp
 */
#include "numeric5414/algo_5414.h"
QVector<double> algo_5414::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
