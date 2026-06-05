/**
 * @file algo_5449.cpp
 */
#include "code5449/algo_5449.h"
QVector<double> algo_5449::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
