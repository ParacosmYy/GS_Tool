/**
 * @file algo_5613.cpp
 */
#include "crypto5613/algo_5613.h"
QVector<double> algo_5613::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
