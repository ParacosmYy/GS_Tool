/**
 * @file WaterfallWidget.h
 * @brief 瀑布图控件，用于时频数据的滚动式可视化显示
 */
// Copyright 2024 EmbedDebug Project
// SPDX-License-Identifier: MIT
#pragma once
#include <QWidget>
#include <QVector>
#include <QColor>
#include <QPixmap>
#include <QTimer>

class QPaintEvent;
class QResizeEvent;

/**
 * @class WaterfallWidget
 * @brief 瀑布图控件，将频谱数据逐行向下滚动显示，适用于频谱监测和时变信号分析
 */
class WaterfallWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父控件指针 */
    explicit WaterfallWidget(QWidget *parent = nullptr);
    /** @brief 析构函数 */
    ~WaterfallWidget() override;

    /** @brief 添加一条频谱数据到瀑布图 @param spectrum 频谱幅度向量 */
    void addSpectrum(const QVector<double> &spectrum);
    /** @brief 设置最大保留行数 @param lines 最大行数 */
    void setMaxLines(int lines);
    /** @brief 设置颜色映射范围 @param min 最小值(dB) @param max 最大值(dB) */
    void setColorRange(double min, double max);
    /** @brief 设置自动滚动刷新间隔 @param ms 间隔毫秒数 */
    void setScrollSpeed(int ms);
    /** @brief 清除所有历史数据 */
    void clear();
    /** @brief 暂停滚动更新 */
    void pause();
    /** @brief 恢复滚动更新 */
    void resume();

    /** @brief 获取最大保留行数 @return 最大行数 */
    int maxLines() const { return m_maxLines; }
    /** @brief 查询是否处于暂停状态 @return 是否暂停 */
    bool isPaused() const { return m_paused; }

signals:
    /** @brief 新频谱数据添加后发射 @param lineCount 当前行数 */
    void spectrumAdded(int lineCount);
    /** @brief 鼠标游标位置的数值 @param index 频率索引 @param value 幅度值 */
    void valueAtCursor(int index, double value);

protected:
    /** @brief 绘制事件处理 */
    void paintEvent(QPaintEvent *event) override;
    /** @brief 大小改变事件处理 */
    void resizeEvent(QResizeEvent *event) override;
    /** @brief 鼠标移动事件处理 */
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    /** @brief 执行图像滚动，将旧数据上移 */
    void scrollImage();
    /** @brief 将数值映射为颜色 @param value 数值 @return 对应颜色 */
    QColor valueToColor(double value) const;

    QVector<QVector<double>> m_history; ///< 频谱历史数据
    int m_maxLines = 200;               ///< 最大保留行数
    double m_minValue = -100.0;         ///< 颜色映射最小值
    double m_maxValue = 0.0;            ///< 颜色映射最大值
    bool m_paused = false;              ///< 是否暂停滚动
    int m_scrollSpeed = 50;             ///< 滚动刷新间隔(毫秒)
    QPixmap m_waterfall;                ///< 瀑布图离屏缓存
    int m_currentLine = 0;              ///< 当前写入行号
    QTimer m_scrollTimer;               ///< 自动滚动定时器
};
