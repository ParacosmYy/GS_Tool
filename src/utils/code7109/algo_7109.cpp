/**
 * @file algo_7109.cpp
 */
#include "code7109/algo_7109.h"
QVector<double> algo_7109::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
