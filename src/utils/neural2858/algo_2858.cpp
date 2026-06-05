/**
 * @file algo_2858.cpp
 */
#include "neural2858/algo_2858.h"
QVector<double> algo_2858::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
