/**
 * @file algo_5833.cpp
 */
#include "crypto5833/algo_5833.h"
QVector<double> algo_5833::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
