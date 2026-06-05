/**
 * @file algo_6039.cpp
 */
#include "quantum6039/algo_6039.h"
QVector<double> algo_6039::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
