/**
 * @file algo_7133.cpp
 */
#include "crypto7133/algo_7133.h"
QVector<double> algo_7133::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
