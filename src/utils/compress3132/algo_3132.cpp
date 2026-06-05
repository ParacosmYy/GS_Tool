/**
 * @file algo_3132.cpp
 */
#include "compress3132/algo_3132.h"
QVector<double> algo_3132::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
