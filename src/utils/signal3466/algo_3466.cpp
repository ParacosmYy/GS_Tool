/**
 * @file algo_3466.cpp
 */
#include "signal3466/algo_3466.h"
QVector<double> algo_3466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
