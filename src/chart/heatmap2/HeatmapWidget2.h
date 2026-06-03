/**
 * @file HeatmapWidget2.h
 * @brief 增强版热力图控件，支持行列标签和网格线显示
 */
#pragma once
#include <QWidget>
#include <QVector>
#include <QPair>

/**
 * @class HeatmapWidget
 * @brief 增强版热力图控件，在基础热力图上增加行列标签、可调网格和独立宽高单元格
 */
class HeatmapWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父控件指针 */
    explicit HeatmapWidget(QWidget *parent = nullptr);
    /** @brief 析构函数 */
    ~HeatmapWidget() override;

    /** @brief 设置数据矩阵 @param matrix 二维数值向量 */
    void setData(const QVector<QVector<double>> &matrix);
    /** @brief 设置单元格尺寸 @param w 宽度(像素) @param h 高度(像素) */
    void setCellSize(int w, int h);
    /** @brief 设置颜色映射范围 @param min 最小值 @param max 最大值 */
    void setColorRange(double min, double max);
    /** @brief 设置行列标签 @param rowLabels 行标签列表 @param colLabels 列标签列表 */
    void setLabels(const QStringList &rowLabels, const QStringList &colLabels);
    /** @brief 设置网格线是否可见 @param visible 是否显示 */
    void setGridVisible(bool visible);
    /** @brief 获取数据行数 @return 行数 */
    int rowCount() const;
    /** @brief 获取数据列数 @return 列数 */
    int colCount() const;
    /** @brief 获取指定位置的数值 @param row 行索引 @param col 列索引 @return 单元格值 */
    double valueAt(int row, int col) const;

    // ---- 统计接口 ----

    /** @brief 获取累计数据更新次数(setData调用) */
    quint64 totalDataUpdates() const { return m_totalDataUpdates; }

    /** @brief 获取累计单元格点击次数 */
    quint64 totalCellClicks() const { return m_totalCellClicks; }

    /** @brief 获取累计悬停事件次数 */
    quint64 totalCellHovers() const { return m_totalCellHovers; }

    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const { return m_totalRepaints; }

    /** @brief 获取数据矩阵总单元格数 */
    int totalCells() const;

    /** @brief 重置所有统计计数器 */
    void resetStats();

signals:
    /** @brief 单元格被点击时发射 @param row 行索引 @param col 列索引 @param value 单元格值 */
    void cellClicked(int row, int col, double value);
    /** @brief 鼠标悬停单元格时发射 @param row 行索引 @param col 列索引 @param value 单元格值 */
    void cellHovered(int row, int col, double value);

protected:
    /** @brief 绘制事件处理 */
    void paintEvent(QPaintEvent *event) override;
    /** @brief 鼠标按下事件处理 */
    void mousePressEvent(QMouseEvent *event) override;
    /** @brief 鼠标移动事件处理 */
    void mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 大小改变事件处理 */
    void resizeEvent(QResizeEvent *event) override;

private:
    /** @brief 将数值映射为颜色 @param v 数值 @return 对应颜色 */
    QColor valueToColor(double v) const;
    /** @brief 将像素坐标转换为单元格行列索引 @param pos 像素坐标 @return 行列对 */
    QPair<int,int> posToCell(const QPoint &pos) const;

    QVector<QVector<double>> m_data;   ///< 二维数据矩阵
    int m_cellW = 20;                  ///< 单元格宽度(像素)
    int m_cellH = 20;                  ///< 单元格高度(像素)
    double m_minVal = 0.0;             ///< 颜色映射最小值
    double m_maxVal = 1.0;             ///< 颜色映射最大值
    bool m_gridVisible = true;         ///< 网格线是否可见
    QStringList m_rowLabels;           ///< 行标签列表
    QStringList m_colLabels;           ///< 列标签列表

    // 统计计数器
    quint64 m_totalDataUpdates = 0;    ///< 累计数据更新次数
    quint64 m_totalCellClicks = 0;     ///< 累计单元格点击次数
    quint64 m_totalCellHovers = 0;     ///< 累计悬停事件次数
    quint64 m_totalRepaints = 0;       ///< 累计重绘次数
};
