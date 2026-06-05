/**
 * @file algo_3701.cpp
 */
#include "interp3701/algo_3701.h"
QVector<double> algo_3701::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
