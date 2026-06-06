/**
 * @file algo_7306.cpp
 */
#include "signal7306/algo_7306.h"
QVector<double> algo_7306::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
