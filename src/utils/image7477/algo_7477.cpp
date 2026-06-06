/**
 * @file algo_7477.cpp
 */
#include "image7477/algo_7477.h"
QVector<double> algo_7477::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
