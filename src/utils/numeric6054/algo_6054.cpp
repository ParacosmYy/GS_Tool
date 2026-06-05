/**
 * @file algo_6054.cpp
 */
#include "numeric6054/algo_6054.h"
QVector<double> algo_6054::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
