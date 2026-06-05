/**
 * @file BSplineInterp.h
 * @brief B样条插值器 — 节点插入与求值
 *
 * 功能: 实现B样条(Basis Spline)曲线插值, 支持任意阶数(常用2次/3次),
 *       提供节点向量构造、Boehm节点插入算法和de Boor求值算法。
 *       B样条具有局部支撑性, 修改一个控制点仅影响局部曲线。
 *
 * 协作: CurveEditor(曲线编辑) / PathPlanner(路径规划) / SurfaceFitter(曲面拟合)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief B样条插值器
 *
 * 支持均匀、准均匀和开放(钳位)节点向量, 可执行:
 * - de Boor算法: O(p) 计算任意参数位置的曲线值
 * - Boehm节点插入: 不改变曲线形状地插入新节点
 * - 导数计算: 一阶和二阶导数
 * - 曲线细分: 提高精度用于渲染
 */
class BSplineInterp : public QObject
{
    Q_OBJECT

public:
    /** @brief 节点向量类型 */
    enum class KnotType {
        Uniform,            ///< 均匀节点
        Clamped,            ///< 钳位(开放)节点 — 两端重复p+1次
        NonUniform         ///< 非均匀(用户自定义)
    };
    Q_ENUM(KnotType)

    /** @brief 2D点 */
    using Point2D = QPair<double, double>;

    /** @brief 曲线采样结果 */
    struct CurveSample {
        double parameter = 0.0;            ///< 参数值
        double x = 0.0;                    ///< X坐标
        double y = 0.0;                    ///< Y坐标
        double dx = 0.0;                   ///< 一阶导数 dx/dt
        double dy = 0.0;                   ///< 一阶导数 dy/dt
        double curvature = 0.0;            ///< 曲率
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalEvaluations = 0;          ///< 累计求值次数
        int totalKnotInsertions = 0;       ///< 累计节点插入次数
        int totalCurvesBuilt = 0;          ///< 累计构建曲线次数
        int totalSamplesGenerated = 0;     ///< 累计采样点数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    explicit BSplineInterp(QObject* parent = nullptr);

    /**
     * @brief 从控制点构建B样条曲线
     * @param controlPoints 控制点数组
     * @param degree 样条阶数(2=二次, 3=三次)
     * @param knotType 节点向量类型
     * @return true=构建成功
     */
    bool buildCurve(const QVector<Point2D>& controlPoints, int degree = 3,
                     KnotType knotType = KnotType::Clamped);

    /**
     * @brief 在参数 t 处求曲线值(de Boor算法)
     * @param t 参数值 [0, 1]
     * @return 曲线上的点
     */
    Point2D evaluate(double t) const;

    /**
     * @brief 在参数 t 处求曲线值和导数
     * @param t 参数值 [0, 1]
     * @return 采样结果(含导数和曲率)
     */
    CurveSample evaluateWithDerivatives(double t) const;

    /**
     * @brief 批量采样曲线
     * @param numSamples 采样点数
     * @return 采样结果数组
     */
    QVector<CurveSample> sampleCurve(int numSamples = 100) const;

    /**
     * @brief Boehm节点插入: 在指定位置插入一个节点
     * @param t 新节点位置
     * @return true=插入成功
     */
    bool insertKnot(double t);

    /**
     * @brief 通过节点插入将曲线细分为更密集的控制点
     * @param subdivisions 每段的细分数
     * @return 细分后的控制点
     */
    QVector<Point2D> refineCurve(int subdivisions = 5) const;

    /**
     * @brief 计算曲线总弧长
     * @param numSegments 积分段数
     * @return 弧长
     */
    double arcLength(int numSegments = 200) const;

    /**
     * @brief 根据弧长参数化获取点
     * @param s 弧长比例 [0, 1]
     * @return 曲线上的点
     */
    Point2D evaluateByArcLength(double s) const;

    /** @brief 获取当前控制点 */
    QVector<Point2D> controlPoints() const;

    /** @brief 获取当前节点向量 */
    QVector<double> knotVector() const;

    /** @brief 获取样条阶数 */
    int degree() const;

    Stats stats() const;
    void resetStatistics();

private:
    /**
     * @brief de Boor递归求值算法
     * @param degree 当前递归阶数
     * @param knotSpan 节点区间索引
     * @param t 参数值
     * @param component 0=x, 1=y
     * @return 求值结果
     */
    double deBoor(int degree, int knotSpan, double t, int component) const;

    /**
     * @brief 找到参数 t 所在的节点区间
     * @param t 参数值
     * @return 节点区间索引
     */
    int findKnotSpan(double t) const;

    /**
     * @brief 生成钳位节点向量
     * @param nControlPoints 控制点数
     * @param degree 阶数
     * @return 节点向量
     */
    QVector<double> generateClampedKnots(int nControlPoints, int degree) const;

    /**
     * @brief 生成均匀节点向量
     * @param nControlPoints 控制点数
     * @param degree 阶数
     * @return 节点向量
     */
    QVector<double> generateUniformKnots(int nControlPoints, int degree) const;

    /**
     * @brief 计算B样条基函数值(Cox-de Boor递推)
     * @param i 基函数索引
     * @param p 阶数
     * @param t 参数值
     * @return 基函数值
     */
    double basisFunction(int i, int p, double t) const;

    QVector<Point2D> m_controlPoints;       ///< 控制点
    QVector<double> m_knots;                ///< 节点向量
    int m_degree = 3;                       ///< 样条阶数

    Stats m_stats;
    double m_timeSum = 0.0;
};
