/**
 * @file algo_4033.cpp
 */
#include "crypto4033/algo_4033.h"
QVector<double> algo_4033::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
