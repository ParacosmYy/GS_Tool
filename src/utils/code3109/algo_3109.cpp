/**
 * @file algo_3109.cpp
 */
#include "code3109/algo_3109.h"
QVector<double> algo_3109::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
