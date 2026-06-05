/**
 * @file algo_3907.cpp
 */
#include "dsp3907/algo_3907.h"
QVector<double> algo_3907::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
