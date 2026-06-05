/**
 * @file algo_1367.cpp
 * @brief Algorithm module 1367
 */
#include "dsp1367/algo_1367.h"
QVector<double> algo_1367::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
