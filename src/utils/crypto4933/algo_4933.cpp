/**
 * @file algo_4933.cpp
 */
#include "crypto4933/algo_4933.h"
QVector<double> algo_4933::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
