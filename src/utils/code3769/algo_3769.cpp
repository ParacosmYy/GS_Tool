/**
 * @file algo_3769.cpp
 */
#include "code3769/algo_3769.h"
QVector<double> algo_3769::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
