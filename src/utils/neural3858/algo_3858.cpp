/**
 * @file algo_3858.cpp
 */
#include "neural3858/algo_3858.h"
QVector<double> algo_3858::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
