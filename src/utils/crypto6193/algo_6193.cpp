/**
 * @file algo_6193.cpp
 */
#include "crypto6193/algo_6193.h"
QVector<double> algo_6193::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
