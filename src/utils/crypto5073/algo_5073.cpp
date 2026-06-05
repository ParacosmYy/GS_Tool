/**
 * @file algo_5073.cpp
 */
#include "crypto5073/algo_5073.h"
QVector<double> algo_5073::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
