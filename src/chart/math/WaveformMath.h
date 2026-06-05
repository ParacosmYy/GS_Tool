/**
 * @file WaveformMath.h
 * @brief 波形数学运算引擎 -- 管理数学表达式并对波形通道执行计算
 *
 * 负责: 表达式增删管理、按表达式对通道数据执行数学运算、
 *       虚拟通道结果生成、统计计数。
 * 协作: MathExpressionParser(解析文本) → WaveformMath(执行计算) → ChartModel(展示结果)
 */

#ifndef WAVEFORMMATH_H
#define WAVEFORMMATH_H

#include <QMap>
#include <QObject>
#include <QVector>
#include "chart/math/MathTypes.h"

/**
 * @class WaveformMath
 * @brief 波形数学运算引擎
 *
 * 用法: addExpression(expr) → evaluate(id, xData, channelData) → 获取 MathResult
 * 支持二元运算(Add/Subtract/Multiply/Divide)、一元函数(Abs/Sqrt/Log/Exp/Sin/Cos/Tan)、
 * 微积分(Derivative/Integral)、统计聚合(Average/Max/Min)、变换(Scale/Offset)和滤波(LowPass/HighPass)。
 */
class WaveformMath : public QObject {
    Q_OBJECT

public:
    /** @brief 构造波形数学引擎 @param parent 父对象 */
    explicit WaveformMath(QObject* parent = nullptr);

    // ── 表达式管理 ──

    /**
     * @brief 添加数学表达式
     * @param expr 数学表达式
     * @return 表达式ID (>=0); -1=表达式无效
     */
    int addExpression(const MathExpression& expr);

    /**
     * @brief 移除数学表达式
     * @param id 表达式ID
     * @return true=成功移除
     */
    bool removeExpression(int id);

    /** @brief 清空所有表达式 */
    void clearExpressions();

    /**
     * @brief 获取所有已注册表达式
     * @return 表达式列表
     */
    QVector<MathExpression> expressions() const;

    // ── 计算执行 ──

    /**
     * @brief 执行指定表达式的计算
     * @param expressionId 表达式ID
     * @param xData X轴数据 (时间/采样序号)
     * @param channelData 通道索引到Y数据的映射
     * @return 计算结果; virtualChannel==-1表示失败
     */
    MathResult evaluate(int expressionId,
                        const QVector<double>& xData,
                        const QMap<int, QVector<double>>& channelData);

    /**
     * @brief 执行所有表达式的计算
     * @param xData X轴数据
     * @param channelData 通道索引到Y数据的映射
     * @return 所有表达式的计算结果
     */
    QVector<MathResult> evaluateAll(const QVector<double>& xData,
                                     const QMap<int, QVector<double>>& channelData);

    // ── 表达式解析 (委托给 MathExpressionParser) ──

    /**
     * @brief 解析表达式文本
     * @param text 表达式字符串
     * @return 解析结果
     */
    MathExpression parseExpression(const QString& text);

    /**
     * @brief 验证表达式是否可执行
     * @param expr 数学表达式
     * @return true=有效
     */
    bool validateExpression(const MathExpression& expr) const;

    // ── 统计 ──

    /** @brief 获取总计算次数 */
    quint64 totalEvaluations() const;
    /** @brief 获取总计算点数 (累计) */
    quint64 totalPointsComputed() const;
    /** @brief 获取解析错误总次数 */
    quint64 totalParseErrors() const;
    /** @brief 获取计算错误总次数 */
    quint64 totalEvalErrors() const;
    /** @brief 获取表达式添加总次数 */
    quint64 totalExpressionsAdded() const;
    /** @brief 获取表达式移除总次数 */
    quint64 totalExpressionsRemoved() const;
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 表达式添加信号 @param id 表达式ID */
    void expressionAdded(int id);

    /** @brief 表达式移除信号 @param id 表达式ID */
    void expressionRemoved(int id);

    /** @brief 计算完成信号 @param id 表达式ID @param result 计算结果 */
    void evaluationComplete(int id, const MathResult& result);

    /** @brief 计算错误信号 @param id 表达式ID @param error 错误描述 */
    void evalError(int id, const QString& error);

private:
    /**
     * @brief 执行单通道一元运算
     * @param op 运算类型
     * @param data 输入数据
     * @param param 附加参数
     * @param xData X轴数据 (微积分用)
     * @return 计算结果
     */
    QVector<double> applyUnary(MathOp op, const QVector<double>& data,
                                double param, const QVector<double>& xData);

    /**
     * @brief 执行双通道二元运算
     * @param op 运算类型
     * @param a 第一个通道数据
     * @param b 第二个通道数据
     * @return 计算结果
     */
    QVector<double> applyBinary(MathOp op, const QVector<double>& a,
                                 const QVector<double>& b);

    // ── 数据成员 ──
    QMap<int, MathExpression> m_expressions;  ///< ID → 表达式
    int m_nextId = 0;                         ///< 下一个表达式ID

    // 统计计数器
    quint64 m_totalEvaluations = 0;       ///< 总计算次数
    quint64 m_totalPointsComputed = 0;    ///< 总计算点数
    quint64 m_totalParseErrors = 0;       ///< 解析错误次数
    quint64 m_totalEvalErrors = 0;        ///< 计算错误次数
    quint64 m_totalExpressionsAdded = 0;  ///< 表达式添加次数
    quint64 m_totalExpressionsRemoved = 0;///< 表达式移除次数
};

#endif // WAVEFORMMATH_H
