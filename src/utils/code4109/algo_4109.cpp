/**
 * @file algo_4109.cpp
 */
#include "code4109/algo_4109.h"
QVector<double> algo_4109::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
