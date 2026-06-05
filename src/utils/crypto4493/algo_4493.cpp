/**
 * @file algo_4493.cpp
 */
#include "crypto4493/algo_4493.h"
QVector<double> algo_4493::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
