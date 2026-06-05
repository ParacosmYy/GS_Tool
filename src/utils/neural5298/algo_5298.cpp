/**
 * @file algo_5298.cpp
 */
#include "neural5298/algo_5298.h"
QVector<double> algo_5298::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
