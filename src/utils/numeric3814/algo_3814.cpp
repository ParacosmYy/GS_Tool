/**
 * @file algo_3814.cpp
 */
#include "numeric3814/algo_3814.h"
QVector<double> algo_3814::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
