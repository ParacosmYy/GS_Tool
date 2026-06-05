/**
 * @file algo_6191.cpp
 */
#include "tree6191/algo_6191.h"
QVector<double> algo_6191::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
