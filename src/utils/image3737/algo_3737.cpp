/**
 * @file algo_3737.cpp
 */
#include "image3737/algo_3737.h"
QVector<double> algo_3737::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
