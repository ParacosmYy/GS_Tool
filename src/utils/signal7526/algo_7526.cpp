/**
 * @file algo_7526.cpp
 */
#include "signal7526/algo_7526.h"
QVector<double> algo_7526::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
