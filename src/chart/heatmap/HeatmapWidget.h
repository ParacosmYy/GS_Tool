/**
 * @file HeatmapWidget.h
 * @brief 热力图显示控件，支持二维数据矩阵的可视化渲染
 */
// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPixmap>

class QPaintEvent;
class QResizeEvent;
class QMouseEvent;

/**
 * @class HeatmapWidget
 * @brief 热力图控件，将二维数值矩阵渲染为颜色矩阵，支持自动缩放和悬停交互
 */
class HeatmapWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父控件指针 */
    explicit HeatmapWidget(QWidget *parent = nullptr);
    /** @brief 析构函数 */
    ~HeatmapWidget() override;

    /** @brief 设置热力图数据矩阵 @param data 二维数值向量 */
    void setData(const QVector<QVector<double>> &data);
    /** @brief 设置颜色映射的数值范围 @param min 最小值 @param max 最大值 */
    void setColorRange(double min, double max);
    /** @brief 设置单元格像素大小 @param size 单元格边长(像素) */
    void setCellSize(int size);
    /** @brief 设置是否在单元格内显示数值 @param show 是否显示 */
    void setShowValues(bool show);
    /** @brief 设置是否根据数据自动缩放颜色范围 @param enabled 是否启用 */
    void setAutoScale(bool enabled);

    /** @brief 返回控件推荐大小 @return 推荐尺寸 */
    QSize sizeHint() const override;
    /** @brief 返回控件最小推荐大小 @return 最小尺寸 */
    QSize minimumSizeHint() const override;

signals:
    /** @brief 鼠标悬停到单元格时发射 @param row 行索引 @param col 列索引 @param value 单元格值 */
    void cellHovered(int row, int col, double value);
    /** @brief 鼠标点击单元格时发射 @param row 行索引 @param col 列索引 @param value 单元格值 */
    void cellClicked(int row, int col, double value);

protected:
    /** @brief 绘制事件处理 */
    void paintEvent(QPaintEvent *event) override;
    /** @brief 大小改变事件处理 */
    void resizeEvent(QResizeEvent *event) override;
    /** @brief 鼠标移动事件处理 */
    void mouseMoveEvent(QMouseEvent *event) override;
    /** @brief 鼠标按下事件处理 */
    void mousePressEvent(QMouseEvent *event) override;

private:
    /** @brief 重建离屏缓存像素图 */
    void updatePixmap();
    /** @brief 将数值映射为颜色 @param value 数值 @return 对应颜色 */
    QColor valueToColor(double value) const;
    /** @brief 格式化数值为显示文本 @param value 数值 @return 格式化字符串 */
    QString formatValue(double value) const;

    QVector<QVector<double>> m_data;   ///< 二维数据矩阵
    double m_minValue = 0.0;           ///< 颜色映射最小值
    double m_maxValue = 1.0;           ///< 颜色映射最大值
    int m_cellSize = 20;               ///< 单元格像素边长
    bool m_showValues = false;         ///< 是否在单元格内显示数值
    bool m_autoScale = true;           ///< 是否自动缩放颜色范围
    int m_hoverRow = -1;               ///< 当前悬停行索引
    int m_hoverCol = -1;               ///< 当前悬停列索引
    QPixmap m_cache;                   ///< 离屏渲染缓存
    bool m_dirty = true;               ///< 缓存脏标记，需要重绘
};
