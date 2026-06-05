/**
 * @file algo_4873.cpp
 */
#include "crypto4873/algo_4873.h"
QVector<double> algo_4873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
