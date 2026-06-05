/**
 * @file PolygonClipper.h
 * @brief Sutherland-Hodgman多边形裁剪
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Sutherland-Hodgman多边形裁剪算法
 *
 * 将多边形对矩形窗口进行裁剪,
 * 支持任意凸多边形裁剪窗口。
 */
class PolygonClipper : public QObject
{
    Q_OBJECT

public:
    /** @brief 二维点 */
    struct Point { double x = 0.0, y = 0.0; };

    /** @brief 统计信息 */
    struct Stats {
        int totalClips = 0;             ///< 总裁剪次数
        int totalInputVertices = 0;     ///< 总输入顶点数
        int totalOutputVertices = 0;    ///< 总输出顶点数
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolygonClipper(QObject* parent = nullptr);

    /**
     * @brief 对矩形窗口裁剪
     * @param polygon 输入多边形顶点
     * @param left,right,bottom,top 裁剪矩形
     * @return 裁剪后多边形
     */
    QVector<Point> clipToRect(const QVector<Point>& polygon,
                               double left, double right,
                               double bottom, double top);

    /**
     * @brief 对任意凸多边形裁剪
     * @param subject 被裁剪多边形
     * @param clipper 裁剪多边形(凸)
     * @return 裁剪结果
     */
    QVector<Point> clipToPolygon(const QVector<Point>& subject,
                                  const QVector<Point>& clipper);

    /**
     * @brief 计算多边形面积
     */
    double polygonArea(const QVector<Point>& poly) const;

    /**
     * @brief 检查点是否在凸多边形内
     */
    bool pointInPolygon(const Point& p, const QVector<Point>& poly) const;

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 裁剪完成信号 */
    void clipped(int inputVertices, int outputVertices);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    QVector<Point> clipByEdge(const QVector<Point>& poly,
                               const Point& e1, const Point& e2);
    Point intersect(const Point& p1, const Point& p2,
                     const Point& e1, const Point& e2) const;
    bool isInside(const Point& p, const Point& e1, const Point& e2) const;
};
