/**
 * @file algo_5402.cpp
 */
#include "poly5402/algo_5402.h"
QVector<double> algo_5402::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
