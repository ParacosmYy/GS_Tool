/**
 * @file algo_6923.cpp
 */
#include "string6923/algo_6923.h"
QVector<double> algo_6923::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
