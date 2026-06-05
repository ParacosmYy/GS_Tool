/**
 * @file algo_3452.cpp
 */
#include "compress3452/algo_3452.h"
QVector<double> algo_3452::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
