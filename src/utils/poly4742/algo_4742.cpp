/**
 * @file algo_4742.cpp
 */
#include "poly4742/algo_4742.h"
QVector<double> algo_4742::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
