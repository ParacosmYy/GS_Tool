/**
 * @file SpectrumMonitorWidget.h
 * @brief 频谱监控显示控件 — 瀑布图/频谱曲线/峰值标记/网格叠加
 *
 * 设计: QWidget子类、QPainter自绘、连接SpectrumMonitor数据源
 * 协作: SpectrumMonitor(数据源) / SpectrumTypes(数据结构)
 *
 * 渲染:
 *   - 上半区: 实时频谱曲线 + 峰值保持曲线 + 峰值标记
 *   - 下半区: 瀑布图(time=Y向下滚动, freq=X, color=magnitude)
 *   - 网格叠加: 频率轴/时间轴/幅度轴刻度
 */

#ifndef SPECTRUMMONITORWIDGET_H
#define SPECTRUMMONITORWIDGET_H

#include <QWidget>
#include <QImage>

#include "chart/spectrum/SpectrumTypes.h"

class SpectrumMonitor;

/** @brief 频谱监控显示控件 — QPainter瀑布图渲染、峰值标记与网格叠加 */
class SpectrumMonitorWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造频谱监控控件 @param parent 父控件 */
    explicit SpectrumMonitorWidget(QWidget* parent = nullptr);

    // ---- 引擎绑定 ----

    /** @brief 绑定频谱监控引擎(不获取所有权) @param monitor 引擎指针 */
    void setMonitor(SpectrumMonitor* monitor);

    // ---- 显示控制 ----

    /** @brief 设置峰值保持曲线可见性 @param visible 是否显示 */
    void setPeakHoldVisible(bool visible);

    /** @brief 设置瀑布图可见性 @param visible 是否显示 */
    void setSpectrogramVisible(bool visible);

    /** @brief 设置网格可见性 @param visible 是否显示 */
    void setGridVisible(bool visible);

    /** @brief 设置dBFS显示范围 @param minDb 最小dB @param maxDb 最大dB */
    void setDbRange(double minDb, double maxDb);

    /** @brief 设置频率显示范围(Hz) @param minFreq 最小频率 @param maxFreq 最大频率 */
    void setFrequencyRange(double minFreq, double maxFreq);

    /** @brief 设置色图方案名称 @param colormap 色图名("Inferno"/"Viridis"/"Plasma"/"Jet") */
    void setColormap(const QString& colormap);

    // ---- 统计 ----

    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const;

    /** @brief 获取累计峰值保持切换次数 */
    quint64 totalPeakHoldToggles() const;

    /** @brief 重置统计计数器 */
    void resetStatistics();

protected:
    /** @brief 自定义绘制事件 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 窗口尺寸变更事件 */
    void resizeEvent(QResizeEvent* event) override;

private slots:
    /** @brief 频谱更新时刷新显示 */
    void onSpectrumUpdated(const SpectrumSlice& slice);

    /** @brief 瀑布图更新时标记脏区域 */
    void onSpectrogramUpdated();

private:
    /** @brief 绘制频谱曲线区域 */
    void drawSpectrum(QPainter& painter, const QRect& rect);

    /** @brief 绘制瀑布图区域 */
    void drawSpectrogram(QPainter& painter, const QRect& rect);

    /** @brief 绘制网格叠加层 */
    void drawGrid(QPainter& painter, const QRect& specRect, const QRect& sgRect);

    /** @brief 绘制峰值标记 */
    void drawPeakMarker(QPainter& painter, const QRect& specRect);

    /** @brief 将dBFS幅度映射为颜色 @param dbFS 归一化dB值(0~1) @return 颜色 */
    QColor magnitudeColor(double normalized) const;

    /** @brief 构建瀑布图QImage缓存 */
    void rebuildSpectrogramImage();

    SpectrumMonitor* m_monitor = nullptr;           ///< 频谱监控引擎(外部拥有)
    SpectrumSlice    m_currentSlice;                ///< 当前频谱切片缓存
    QVector<double>  m_peakHoldMags;                ///< 峰值保持幅度缓存
    QImage           m_spectrogramImage;             ///< 瀑布图缓存图像

    // 显示参数
    bool   m_peakHoldVisible    = true;              ///< 峰值保持曲线可见性
    bool   m_spectrogramVisible = true;              ///< 瀑布图可见性
    bool   m_gridVisible        = true;              ///< 网格可见性
    double m_minDb              = -120.0;            ///< 最小dBFS
    double m_maxDb              = 0.0;               ///< 最大dBFS
    double m_minFreq            = 0.0;               ///< 最小频率(Hz)
    double m_maxFreq            = 24000.0;           ///< 最大频率(Hz)
    QString m_colormap          = QStringLiteral("Inferno"); ///< 色图方案

    // 布局比例
    double m_spectrumRatio = 0.4;                    ///< 频谱区域高度占比

    // 统计
    quint64 m_totalRepaints       = 0;               ///< 累计重绘次数
    quint64 m_totalPeakHoldToggles = 0;              ///< 累计峰值保持切换次数
};

#endif // SPECTRUMMONITORWIDGET_H
