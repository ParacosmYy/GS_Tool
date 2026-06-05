/**
 * @file WaveformMathStats.cpp
 * @brief 波形数学运算引擎 -- 统计 getter 实现
 */

#include "chart/math/WaveformMath.h"

/** @brief 获取总计算次数 */
quint64 WaveformMath::totalEvaluations() const
{
    return m_totalEvaluations;
}

/** @brief 获取总计算点数 */
quint64 WaveformMath::totalPointsComputed() const
{
    return m_totalPointsComputed;
}

/** @brief 获取解析错误总次数 */
quint64 WaveformMath::totalParseErrors() const
{
    return m_totalParseErrors;
}

/** @brief 获取计算错误总次数 */
quint64 WaveformMath::totalEvalErrors() const
{
    return m_totalEvalErrors;
}

/** @brief 获取表达式添加总次数 */
quint64 WaveformMath::totalExpressionsAdded() const
{
    return m_totalExpressionsAdded;
}

/** @brief 获取表达式移除总次数 */
quint64 WaveformMath::totalExpressionsRemoved() const
{
    return m_totalExpressionsRemoved;
}

/** @brief 重置所有统计计数器 */
void WaveformMath::resetStatistics()
{
    m_totalEvaluations = 0;
    m_totalPointsComputed = 0;
    m_totalParseErrors = 0;
    m_totalEvalErrors = 0;
    m_totalExpressionsAdded = 0;
    m_totalExpressionsRemoved = 0;
}
