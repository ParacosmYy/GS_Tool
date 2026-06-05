/**
 * @file algo_1072.cpp
 * @brief Algorithm module 1072
 */
#include "compress1072/algo_1072.h"
QVector<double> algo_1072::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
