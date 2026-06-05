/**
 * @file algo_3477.cpp
 */
#include "image3477/algo_3477.h"
QVector<double> algo_3477::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
