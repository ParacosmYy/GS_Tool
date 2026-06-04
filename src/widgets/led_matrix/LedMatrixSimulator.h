/**
 * @file LedMatrixSimulator.h
 * @brief LED矩阵模拟器 — NxM网格彩色LED显示仿真控件
 *
 * 模拟嵌入式LED矩阵显示屏(8x8、16x16等)，每个LED可独立设置颜色。
 * 支持圆形/方形/圆角方形LED形状、亮度调节、鼠标点击切换、
 * QImage图像导入映射、PNG快照导出。适用于嵌入式GUI开发中
 * 预览LED点阵图案、字体字形、动画帧序列。
 * 所有颜色通过 ThemeManager 语义色获取，禁止硬编码。
 */

#ifndef LEDMATRIXSIMULATOR_H
#define LEDMATRIXSIMULATOR_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QImage>
#include <QtGlobal>

class QPaintEvent;
class QMouseEvent;
class QWheelEvent;

/**
 * @class LedMatrixSimulator
 * @brief NxM LED矩阵显示模拟器控件
 * @details 维护 rows x cols 的颜色网格，paintEvent 逐个绘制LED单元。
 *          支持圆形/方形/圆角方形三种LED形状、像素间距调节、全局亮度因子、
 *          鼠标左键点击切换LED开关、右键取色、滚轮缩放LED尺寸。
 *          fillImage() 将 QImage 逐像素映射到矩阵，toImage() 反向导出。
 */
class LedMatrixSimulator : public QWidget {
    Q_OBJECT

public:
    /** @brief LED单元形状枚举 */
    enum class Shape {
        Circle,         ///< 圆形LED(默认，模拟真实LED灯珠)
        Square,         ///< 方形LED(像素风格)
        RoundedSquare   ///< 圆角方形LED(现代风格)
    };
    Q_ENUM(Shape)

    /** @brief 运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalLedChanges = 0;        ///< 累计LED颜色变更次数
        quint64 totalPaints = 0;            ///< 累计重绘次数(paintEvent触发)
        quint64 totalClears = 0;            ///< 累计清空操作次数
        quint64 totalImageImports = 0;      ///< 累计图像导入次数
        quint64 totalExports = 0;           ///< 累计PNG导出次数
        quint64 totalMouseClicks = 0;       ///< 累计鼠标点击次数
        int     maxGridSize = 0;            ///< 历史最大网格尺寸(rows*cols)
    };

    /** @brief 构造LED矩阵模拟器 @param parent 父控件指针 */
    explicit LedMatrixSimulator(QWidget* parent = nullptr);

    /** @brief 设置网格尺寸 @param rows 行数(>=1) @param cols 列数(>=1) */
    void setGridSize(int rows, int cols);

    /** @brief 设置指定位置的LED颜色 @param row 行索引 @param col 列索引 @param color LED颜色 */
    void setLed(int row, int col, const QColor& color);

    /** @brief 清空全部LED为关闭状态(黑色) */
    void clear();

    /** @brief 设置LED形状 @param shape 形状枚举 */
    void setShape(Shape shape);

    /** @brief 设置LED间距(像素) @param pixels 间距值(>=0) */
    void setInterval(int pixels);

    /** @brief 设置全局亮度因子 @param brightness 亮度(0.0~1.0) */
    void setBrightness(double brightness);

    /** @brief 从QImage导入图像到矩阵(自动缩放/裁剪) @param image 源图像 */
    void fillImage(const QImage& image);

    /** @brief 将当前矩阵导出为QImage @return 导出的图像 */
    QImage toImage() const;

    /** @brief 导出当前矩阵为PNG文件 @param filePath 目标文件路径 @return true导出成功 */
    bool exportToPng(const QString& filePath);

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

    /** @brief 获取当前行数 @return 行数 */
    int rows() const;

    /** @brief 获取当前列数 @return 列数 */
    int cols() const;

    /** @brief 获取指定位置LED颜色 @param row 行 @param col 列 @return LED颜色 */
    QColor ledColor(int row, int col) const;

    /** @brief 获取当前LED形状 @return 形状枚举 */
    Shape shape() const;

    /** @brief 获取当前LED间距 @return 间距(像素) */
    int interval() const;

    /** @brief 获取当前亮度因子 @return 亮度(0.0~1.0) */
    double brightness() const;

signals:
    /** @brief LED颜色被改变(程序或鼠标) @param row 行 @param col 列 @param color 新颜色 */
    void ledChanged(int row, int col, const QColor& color);

    /** @brief 网格尺寸被改变 @param rows 新行数 @param cols 新列数 */
    void gridSizeChanged(int rows, int cols);

protected:
    /** @brief 绘制事件 — 背景→LED网格 @param event 绘制事件 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 鼠标按下事件 — 左键切换LED @param event 鼠标事件 */
    void mousePressEvent(QMouseEvent* event) override;

    /** @brief 滚轮事件 — 调整LED单元尺寸 @param event 滚轮事件 */
    void wheelEvent(QWheelEvent* event) override;

    /** @brief 建议最小尺寸 @return 基于 grid*minLedSize */
    QSize minimumSizeHint() const override;

private:
    /** @brief 根据控件尺寸计算每个LED的像素大小 @return LED边长 */
    int computeLedSize() const;

    /** @brief 计算网格绘制区域的总偏移量(居中对齐) @param ledSize LED大小 @return 左上角偏移 */
    QPointF computeGridOffset(int ledSize) const;

    /** @brief 根据鼠标位置计算对应的行列索引 @param pos 鼠标位置 @param ledSize LED大小 @return 行列对(-1表示越界) */
    QPair<int, int> posToRowCol(const QPointF& pos, int ledSize) const;

    /** @brief 绘制单个LED单元(根据当前Shape绘制对应形状) @param painter 画笔 @param cx 中心X @param cy 中心Y @param size LED大小 @param color 颜色(已应用亮度) */
    void drawSingleLed(QPainter& painter, qreal cx, qreal cy, int size, const QColor& color) const;

    int m_rows;                         ///< 网格行数
    int m_cols;                         ///< 网格列数
    int m_interval;                     ///< LED间距(像素)
    int m_minLedSize;                   ///< LED最小尺寸(像素)
    double m_brightness;                ///< 亮度因子(0.0~1.0)
    Shape m_shape;                      ///< LED形状
    QVector<QColor> m_grid;            ///< 一维颜色数组(rows*cols)
    Stats m_stats;                      ///< 运行统计
};

#endif // LEDMATRIXSIMULATOR_H
