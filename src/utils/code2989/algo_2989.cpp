/**
 * @file algo_2989.cpp
 */
#include "code2989/algo_2989.h"
QVector<double> algo_2989::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
