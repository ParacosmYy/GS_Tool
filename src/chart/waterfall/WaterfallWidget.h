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

    // ---- 统计接口 ----

    /** @brief 获取累计添加的频谱帧数 */
    quint64 totalSpectrumsAdded() const { return m_totalSpectrumsAdded; }

    /** @brief 获取累计丢弃的频谱帧数(暂停期间) */
    quint64 totalSpectrumsDropped() const { return m_totalSpectrumsDropped; }

    /** @brief 获取峰值频谱宽度(单帧最大频率bin数) */
    int peakSpectrumWidth() const { return m_peakSpectrumWidth; }

    /** @brief 获取当前频谱历史行数 */
    int historyLineCount() const { return m_history.size(); }

    /** @brief 获取累计鼠标游标查询次数 */
    quint64 totalCursorQueries() const { return m_totalCursorQueries; }

    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const { return m_totalRepaints; }

    /** @brief 获取累计数据更新次数(addSpectrum调用) */
    quint64 totalUpdates() const { return m_totalUpdates; }

    /** @brief 获取累计滚动刷新次数 */
    quint64 totalScrolls() const { return m_totalScrolls; }

    /** @brief 获取累计颜色映射范围变更次数 */
    quint64 totalColorMapChanges() const { return m_totalColorMapChanges; }

    /** @brief 获取峰值数据点数(单帧最大数据点) */
    quint64 peakDataPoints() const { return m_peakDataPoints; }

    /** @brief 获取累计帧更新次数(addSpectrum中实际绘制新行的次数) */
    quint64 totalFrameUpdates() const { return m_totalFrameUpdates; }

    /** @brief 获取累计滚动事件次数(定时器触发scrollImage的次数) */
    quint64 totalScrollEvents() const { return m_totalScrollEvents; }

    /** @brief 获取累计渲染次数(paintEvent中实际绘制帧的次数) */
    quint64 totalRenders() const { return m_totalRenders; }

    /** @brief 获取累计暂停操作次数 */
    quint64 totalPauses() const { return m_totalPauses; }

    /** @brief 获取累计恢复操作次数 */
    quint64 totalResumes() const { return m_totalResumes; }

    /** @brief 获取累计清除操作次数 */
    quint64 totalClears() const { return m_totalClears; }

    /** @brief 重置所有统计计数器 */
    void resetStats();

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
    /** @brief 显示事件 — 恢复滚动定时器 */
    void showEvent(QShowEvent *event) override;
    /** @brief 隐藏事件 — 暂停滚动定时器(节省CPU) */
    void hideEvent(QHideEvent *event) override;

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

    // 统计计数器
    quint64 m_totalSpectrumsAdded = 0;  ///< 累计添加的频谱帧数
    quint64 m_totalSpectrumsDropped = 0;///< 累计丢弃的频谱帧数(暂停期间)
    int m_peakSpectrumWidth = 0;        ///< 峰值频谱宽度(单帧最大频率bin数)
    quint64 m_totalCursorQueries = 0;   ///< 累计鼠标游标查询次数
    quint64 m_totalRepaints = 0;        ///< 累计重绘次数
    quint64 m_totalUpdates = 0;         ///< 累计数据更新次数(addSpectrum调用)
    quint64 m_totalScrolls = 0;         ///< 累计滚动刷新次数
    quint64 m_totalColorMapChanges = 0; ///< 累计颜色映射范围变更次数
    quint64 m_peakDataPoints = 0;       ///< 峰值数据点数(单帧最大数据点)
    quint64 m_totalFrameUpdates = 0;    ///< 累计帧更新次数(addSpectrum中实际绘制新行)
    quint64 m_totalScrollEvents = 0;    ///< 累计滚动事件次数(定时器触发scrollImage)
    quint64 m_totalRenders = 0;         ///< 累计渲染次数(paintEvent中实际绘制帧)
    quint64 m_totalPauses = 0;          ///< 累计暂停操作次数
    quint64 m_totalResumes = 0;         ///< 累计恢复操作次数
    quint64 m_totalClears = 0;          ///< 累计清除操作次数
};
