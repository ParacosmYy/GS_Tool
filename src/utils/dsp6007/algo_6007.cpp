/**
 * @file algo_6007.cpp
 */
#include "dsp6007/algo_6007.h"
QVector<double> algo_6007::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
