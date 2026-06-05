/**
 * @file algo_1588.cpp
 * @brief Algorithm module 1588
 */
#include "fft1588/algo_1588.h"
QVector<double> algo_1588::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
