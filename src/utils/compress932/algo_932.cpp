/**
 * @file algo_932.cpp
 * @brief Algorithm module 932
 */
#include "compress932/algo_932.h"
QVector<double> algo_932::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
