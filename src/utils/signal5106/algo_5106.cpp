/**
 * @file algo_5106.cpp
 */
#include "signal5106/algo_5106.h"
QVector<double> algo_5106::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
