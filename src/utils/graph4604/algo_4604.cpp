/**
 * @file algo_4604.cpp
 */
#include "graph4604/algo_4604.h"
QVector<double> algo_4604::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
