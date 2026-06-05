/**
 * @file algo_4681.cpp
 */
#include "interp4681/algo_4681.h"
QVector<double> algo_4681::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
