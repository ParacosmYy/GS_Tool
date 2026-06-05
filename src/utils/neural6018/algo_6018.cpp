/**
 * @file algo_6018.cpp
 */
#include "neural6018/algo_6018.h"
QVector<double> algo_6018::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
