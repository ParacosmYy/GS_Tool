/**
 * @file WaveformMath.cpp
 * @brief 波形数学运算引擎实现 -- 表达式管理 + 计算执行
 *
 * 实现 addExpression/removeExpression/evaluate/evaluateAll/applyUnary/applyBinary 等。
 * 统计接口见 WaveformMathStats.cpp。
 */

#include "chart/math/WaveformMath.h"
#include "chart/math/MathExpressionParser.h"

#include <QtMath>
#include <algorithm>

// ============================================================
// 构造
// ============================================================

/** @brief 构造波形数学引擎 @param parent 父对象 */
WaveformMath::WaveformMath(QObject* parent)
    : QObject(parent)
{
}

// ============================================================
// 表达式管理
// ============================================================

/** @brief 添加数学表达式 @param expr 表达式 @return ID; -1=无效 */
int WaveformMath::addExpression(const MathExpression& expr)
{
    if (!validateExpression(expr)) {
        ++m_totalParseErrors;
        return -1;
    }

    int id = m_nextId++;
    m_expressions[id] = expr;
    ++m_totalExpressionsAdded;
    emit expressionAdded(id);
    return id;
}

/** @brief 移除数学表达式 @param id 表达式ID @return true=成功 */
bool WaveformMath::removeExpression(int id)
{
    if (!m_expressions.contains(id)) {
        return false;
    }
    m_expressions.remove(id);
    ++m_totalExpressionsRemoved;
    emit expressionRemoved(id);
    return true;
}

/** @brief 清空所有表达式 */
void WaveformMath::clearExpressions()
{
    const auto keys = m_expressions.keys();
    for (int id : keys) {
        emit expressionRemoved(id);
    }
    m_expressions.clear();
}

/** @brief 获取所有已注册表达式 @return 表达式列表 */
QVector<MathExpression> WaveformMath::expressions() const
{
    return m_expressions.values().toVector();
}

// ============================================================
// 计算执行
// ============================================================

/** @brief 执行指定表达式的计算 @param expressionId 表达式ID @param xData X轴数据 @param channelData 通道数据 @return 计算结果 */
MathResult WaveformMath::evaluate(int expressionId,
                                   const QVector<double>& xData,
                                   const QMap<int, QVector<double>>& channelData)
{
    MathResult failResult;
    failResult.virtualChannel = -1;

    if (!m_expressions.contains(expressionId)) {
        ++m_totalEvalErrors;
        emit evalError(expressionId, QStringLiteral("表达式ID不存在"));
        return failResult;
    }

    const MathExpression& expr = m_expressions[expressionId];

    // 提取源通道数据
    QVector<QVector<double>> sources;
    for (int ch : expr.sourceChannels) {
        if (!channelData.contains(ch) || channelData[ch].isEmpty()) {
            ++m_totalEvalErrors;
            emit evalError(expressionId,
                           QStringLiteral("通道 %1 数据缺失").arg(ch + 1));
            return failResult;
        }
        sources.append(channelData[ch]);
    }

    // 确定输出长度(取最短)
    int len = xData.size();
    for (const auto& s : sources) {
        len = qMin(len, s.size());
    }
    if (len == 0) {
        ++m_totalEvalErrors;
        emit evalError(expressionId, QStringLiteral("数据长度为零"));
        return failResult;
    }

    QVector<double> yData;

    // 二元运算: 需要2个源通道
    if (expr.operation == MathOp::Add || expr.operation == MathOp::Subtract
        || expr.operation == MathOp::Multiply || expr.operation == MathOp::Divide) {
        if (sources.size() < 2) {
            ++m_totalEvalErrors;
            emit evalError(expressionId, QStringLiteral("二元运算需要2个源通道"));
            return failResult;
        }
        yData = applyBinary(expr.operation,
                            sources[0].mid(0, len), sources[1].mid(0, len));
    } else if (!sources.isEmpty()) {
        // 一元运算
        yData = applyUnary(expr.operation, sources[0].mid(0, len),
                            expr.parameter, xData.mid(0, len));
    } else {
        // 无通道数据 → 常量
        yData.fill(expr.parameter, len);
    }

    if (yData.isEmpty()) {
        ++m_totalEvalErrors;
        return failResult;
    }

    ++m_totalEvaluations;
    m_totalPointsComputed += static_cast<quint64>(yData.size());

    MathResult result;
    result.virtualChannel = 1000 + expressionId;
    result.name = expr.expression;
    result.xData = xData.mid(0, len);
    result.yData = yData;
    result.expression = expr;

    emit evaluationComplete(expressionId, result);
    return result;
}

/** @brief 执行所有表达式的计算 @param xData X轴数据 @param channelData 通道数据 @return 所有结果 */
QVector<MathResult> WaveformMath::evaluateAll(const QVector<double>& xData,
                                               const QMap<int, QVector<double>>& channelData)
{
    QVector<MathResult> results;
    const auto keys = m_expressions.keys();
    for (int id : keys) {
        MathResult r = evaluate(id, xData, channelData);
        if (r.virtualChannel >= 0) {
            results.append(r);
        }
    }
    return results;
}

// ============================================================
// 表达式解析 (委托)
// ============================================================

/** @brief 解析表达式文本 @param text 表达式字符串 @return 解析结果 */
MathExpression WaveformMath::parseExpression(const QString& text)
{
    MathExpressionParser parser;
    MathExpression expr = parser.parse(text);
    if (!parser.errorMessage().isEmpty()) {
        ++m_totalParseErrors;
    }
    return expr;
}

/** @brief 验证表达式是否可执行 @param expr 表达式 @return true=有效 */
bool WaveformMath::validateExpression(const MathExpression& expr) const
{
    if (expr.expression.trimmed().isEmpty()) {
        return false;
    }
    if (expr.sourceChannels.isEmpty()) {
        return false;
    }
    for (int ch : expr.sourceChannels) {
        if (ch < 0 || ch > 7) {
            return false;
        }
    }
    return true;
}

// ============================================================
// 一元运算
// ============================================================

/** @brief 执行单通道一元运算 @param op 运算类型 @param data 输入数据 @param param 附加参数 @param xData X轴数据 @return 计算结果 */
QVector<double> WaveformMath::applyUnary(MathOp op, const QVector<double>& data,
                                          double param, const QVector<double>& xData)
{
    const int n = data.size();
    QVector<double> result(n);

    switch (op) {
    case MathOp::Abs:
        for (int i = 0; i < n; ++i) result[i] = qAbs(data[i]);
        break;

    case MathOp::Sqrt:
        for (int i = 0; i < n; ++i) {
            result[i] = (data[i] >= 0.0) ? qSqrt(data[i]) : qQNaN();
        }
        break;

    case MathOp::Log:
        for (int i = 0; i < n; ++i) {
            result[i] = (data[i] > 0.0) ? qLn(data[i]) : qQNaN();
        }
        break;

    case MathOp::Exp:
        for (int i = 0; i < n; ++i) {
            result[i] = (data[i] > -700.0 && data[i] < 700.0) ? qExp(data[i]) : qQNaN();
        }
        break;

    case MathOp::Sin:
        for (int i = 0; i < n; ++i) result[i] = qSin(data[i]);
        break;

    case MathOp::Cos:
        for (int i = 0; i < n; ++i) result[i] = qCos(data[i]);
        break;

    case MathOp::Tan:
        for (int i = 0; i < n; ++i) result[i] = qTan(data[i]);
        break;

    case MathOp::Derivative:
        if (n < 2) {
            return QVector<double>();
        }
        result.resize(n - 1);
        for (int i = 0; i < n - 1; ++i) {
            double dx = (i + 1 < xData.size()) ? (xData[i + 1] - xData[i]) : 1.0;
            if (qFuzzyIsNull(dx)) dx = 1.0;
            result[i] = (data[i + 1] - data[i]) / dx;
        }
        break;

    case MathOp::Integral:
        if (n < 2) {
            return QVector<double>();
        }
        result[0] = 0.0;
        for (int i = 1; i < n; ++i) {
            double dx = (i < xData.size()) ? (xData[i] - xData[i - 1]) : 1.0;
            if (qFuzzyIsNull(dx)) dx = 1.0;
            result[i] = result[i - 1] + (data[i] + data[i - 1]) * 0.5 * dx;
        }
        break;

    case MathOp::Average: {
        int win = qMax(1, static_cast<int>(param));
        if (win < 1) win = 5;
        double sum = 0.0;
        for (int i = 0; i < n; ++i) {
            sum += data[i];
            if (i >= win) sum -= data[i - win];
            int count = qMin(i + 1, win);
            result[i] = sum / count;
        }
        break;
    }

    case MathOp::Max: {
        int win = qMax(1, static_cast<int>(param));
        if (win < 1) win = 5;
        for (int i = 0; i < n; ++i) {
            double mx = data[i];
            int start = qMax(0, i - win + 1);
            for (int j = start; j <= i; ++j) {
                mx = qMax(mx, data[j]);
            }
            result[i] = mx;
        }
        break;
    }

    case MathOp::Min: {
        int win = qMax(1, static_cast<int>(param));
        if (win < 1) win = 5;
        for (int i = 0; i < n; ++i) {
            double mn = data[i];
            int start = qMax(0, i - win + 1);
            for (int j = start; j <= i; ++j) {
                mn = qMin(mn, data[j]);
            }
            result[i] = mn;
        }
        break;
    }

    case MathOp::Scale:
        for (int i = 0; i < n; ++i) result[i] = data[i] * param;
        break;

    case MathOp::Offset:
        for (int i = 0; i < n; ++i) result[i] = data[i] + param;
        break;

    case MathOp::LowPass: {
        double alpha = qBound(0.0, param, 1.0);
        if (qFuzzyIsNull(alpha)) alpha = 0.1;
        result[0] = data[0];
        for (int i = 1; i < n; ++i) {
            result[i] = alpha * data[i] + (1.0 - alpha) * result[i - 1];
        }
        break;
    }

    case MathOp::HighPass: {
        double alpha = qBound(0.0, param, 1.0);
        if (qFuzzyIsNull(alpha)) alpha = 0.1;
        result[0] = data[0];
        for (int i = 1; i < n; ++i) {
            double lp = alpha * data[i] + (1.0 - alpha) * result[i - 1];
            result[i] = data[i] - lp;
        }
        break;
    }

    default:
        result = data;
        break;
    }

    return result;
}

// ============================================================
// 二元运算
// ============================================================

/** @brief 执行双通道二元运算 @param op 运算类型 @param a 通道A @param b 通道B @return 计算结果 */
QVector<double> WaveformMath::applyBinary(MathOp op, const QVector<double>& a,
                                           const QVector<double>& b)
{
    const int n = qMin(a.size(), b.size());
    QVector<double> result(n);

    switch (op) {
    case MathOp::Add:
        for (int i = 0; i < n; ++i) result[i] = a[i] + b[i];
        break;

    case MathOp::Subtract:
        for (int i = 0; i < n; ++i) result[i] = a[i] - b[i];
        break;

    case MathOp::Multiply:
        for (int i = 0; i < n; ++i) result[i] = a[i] * b[i];
        break;

    case MathOp::Divide:
        for (int i = 0; i < n; ++i) {
            result[i] = qFuzzyIsNull(b[i]) ? qQNaN() : a[i] / b[i];
        }
        break;

    default:
        result = a;
        break;
    }

    return result;
}
