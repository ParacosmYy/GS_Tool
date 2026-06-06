/**
 * @file algo_7018.cpp
 */
#include "neural7018/algo_7018.h"
QVector<double> algo_7018::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
