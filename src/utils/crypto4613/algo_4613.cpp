/**
 * @file algo_4613.cpp
 */
#include "crypto4613/algo_4613.h"
QVector<double> algo_4613::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
