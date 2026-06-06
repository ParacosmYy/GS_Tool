/**
 * @file algo_6933.cpp
 */
#include "crypto6933/algo_6933.h"
QVector<double> algo_6933::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
