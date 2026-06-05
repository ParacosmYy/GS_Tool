/**
 * @file algo_4693.cpp
 */
#include "crypto4693/algo_4693.h"
QVector<double> algo_4693::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
