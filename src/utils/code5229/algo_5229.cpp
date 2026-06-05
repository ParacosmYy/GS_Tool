/**
 * @file algo_5229.cpp
 */
#include "code5229/algo_5229.h"
QVector<double> algo_5229::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
