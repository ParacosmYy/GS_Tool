/**
 * @file algo_987.cpp
 * @brief Algorithm module 987
 */
#include "dsp987/algo_987.h"
QVector<double> algo_987::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
