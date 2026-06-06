/**
 * @file algo_7073.cpp
 */
#include "crypto7073/algo_7073.h"
QVector<double> algo_7073::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
