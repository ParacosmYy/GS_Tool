/**
 * @file algo_1987.cpp
 * @brief Algorithm module 1987
 */
#include "dsp1987/algo_1987.h"
QVector<double> algo_1987::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
