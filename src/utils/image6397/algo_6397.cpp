/**
 * @file algo_6397.cpp
 */
#include "image6397/algo_6397.h"
QVector<double> algo_6397::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
