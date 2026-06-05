/**
 * @file MathExpressionParserStats.cpp
 * @brief 数学表达式解析器 -- 统计 getter 实现
 */

#include "chart/math/MathExpressionParser.h"

/** @brief 获取总解析次数 */
quint64 MathExpressionParser::totalParses() const
{
    return m_totalParses;
}

/** @brief 获取解析失败次数 */
quint64 MathExpressionParser::totalParseErrors() const
{
    return m_totalParseErrors;
}

/** @brief 获取使用函数调用的总次数 */
quint64 MathExpressionParser::totalFunctionsUsed() const
{
    return m_totalFunctionsUsed;
}

/** @brief 重置所有统计计数器 */
void MathExpressionParser::resetStatistics()
{
    m_totalParses = 0;
    m_totalParseErrors = 0;
    m_totalFunctionsUsed = 0;
}
