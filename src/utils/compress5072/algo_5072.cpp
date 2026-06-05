/**
 * @file algo_5072.cpp
 */
#include "compress5072/algo_5072.h"
QVector<double> algo_5072::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
