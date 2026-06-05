/**
 * @file algo_3797.cpp
 */
#include "image3797/algo_3797.h"
QVector<double> algo_3797::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
