/**
 * @file MathTypes.h
 * @brief 波形数学引擎基础类型 -- 运算枚举、表达式结构、计算结果
 *
 * 提供 WaveformMath 和 MathExpressionParser 共用的数据结构。
 * 所有通道引用使用整数索引 (ch1=0, ch2=1, ...)。
 */

#ifndef MATHTYPES_H
#define MATHTYPES_H

#include <QString>
#include <QVector>

/**
 * @brief 数学运算类型枚举
 *
 * 支持二元运算、一元函数、微积分、统计聚合和滤波操作。
 */
enum class MathOp {
    Add,        ///< 通道加法: chA + chB
    Subtract,   ///< 通道减法: chA - chB
    Multiply,   ///< 通道乘法: chA * chB
    Divide,     ///< 通道除法: chA / chB
    Abs,        ///< 绝对值: |ch|
    Sqrt,       ///< 平方根
    Log,        ///< 自然对数 ln(ch)
    Exp,        ///< 指数 e^ch
    Sin,        ///< 正弦
    Cos,        ///< 余弦
    Tan,        ///< 正切
    Derivative, ///< 一阶数值导数 (差分)
    Integral,   ///< 数值积分 (梯形法)
    Average,    ///< 滑动平均
    Max,        ///< 滑动最大值
    Min,        ///< 滑动最小值
    Scale,      ///< 缩放: ch * parameter
    Offset,     ///< 偏移: ch + parameter
    LowPass,    ///< 一阶低通 IIR
    HighPass    ///< 一阶高通 IIR
};

/**
 * @brief 数学表达式描述
 *
 * 封装一条完整的运算规则，包含原始表达式文本、源通道列表、
 * 运算类型和附加参数（如滤波截止频率、缩放系数等）。
 */
struct MathExpression {
    QString expression;         ///< 原始表达式文本 (如 "ch1 + ch2")
    QVector<int> sourceChannels;///< 源通道索引列表 (0-based)
    MathOp operation = MathOp::Add; ///< 运算类型
    double parameter = 0.0;     ///< 附加参数 (缩放系数/滤波截止频率等)
};

/**
 * @brief 数学运算结果
 *
 * 包含虚拟通道编号、名称、X/Y 数据和对应的表达式。
 */
struct MathResult {
    int virtualChannel = -1;    ///< 虚拟通道编号 (>=1000)
    QString name;               ///< 结果通道名称
    QVector<double> xData;      ///< X 轴数据 (时间/采样序号)
    QVector<double> yData;      ///< Y 轴数据 (计算结果)
    MathExpression expression;  ///< 对应的表达式
};

#endif // MATHTYPES_H
