/**
 * @file algo_6613.cpp
 */
#include "crypto6613/algo_6613.h"
QVector<double> algo_6613::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
