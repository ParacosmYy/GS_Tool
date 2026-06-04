/**
 * @file ScopeWidget.cpp
 * @brief 示波器Widget实现 — 多通道采样缓冲+网格绘制+触发控制
 */
#include "widgets/scope/ScopeWidget.h"
#include "core/theme/ThemeManager.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>

/** @brief 构造函数，默认2通道 @param parent 父Widget */
ScopeWidget::ScopeWidget(QWidget *parent) : QWidget(parent) { setObjectName("ScopeWidget"); setChannelCount(2); }
/** @brief 析构函数 */
ScopeWidget::~ScopeWidget() = default;

/** @brief 设置通道数量 @param c 通道数 */
void ScopeWidget::setChannelCount(int c) { ++m_totalChannelChanges; m_channels.resize(c); for (auto &ch : m_channels) ch.resize(m_bufferSize); }
/** @brief 设置采样缓冲区大小 @param s 缓冲区采样数 */
void ScopeWidget::setSampleBuffer(int s) { m_bufferSize = s; for (auto &ch : m_channels) ch.resize(s); m_writePos = 0; }
/** @brief 添加单个采样值 @param ch 通道索引 @param v 采样值 */
void ScopeWidget::addSample(int ch, double v) { if (ch >= 0 && ch < m_channels.size()) { m_channels[ch][m_writePos % m_bufferSize] = v; ++m_totalSamples; if (ch == 0) { m_writePos++; if (m_writePos >= m_bufferSize) { m_writePos = 0; m_wrapped = true; ++m_totalOverflows; emit dataOverflow(); } if (m_running && qAbs(v - m_triggerLevel) < 0.01 && ch == m_triggerCh) { ++m_totalTriggerFires; emit triggerFired(); } } } }
/** @brief 批量添加采样值 @param ch 通道索引 @param vals 采样值向量 */
void ScopeWidget::addSamples(int ch, const QVector<double> &vals) { for (auto v : vals) addSample(ch, v); }
/** @brief 设置时间轴缩放 @param ms 时间刻度(毫秒) */
void ScopeWidget::setTimeScale(double ms) { m_timeScale = ms; ++m_totalScaleChanges; }
/** @brief 设置电压轴缩放 @param v 电压刻度 */
void ScopeWidget::setVoltageScale(double v) { m_voltScale = v; ++m_totalScaleChanges; }
/** @brief 设置触发通道 @param c 通道索引 */
void ScopeWidget::setTriggerChannel(int c) { m_triggerCh = c; }
/** @brief 设置触发电平 @param l 触发电平值 */
void ScopeWidget::setTriggerLevel(double l) { m_triggerLevel = l; }
/** @brief 启停采集 @param on true启动 */
void ScopeWidget::setRunning(bool on) { if (m_running && !on) ++m_totalPauses; if (!m_running && on) ++m_totalRestarts; m_running = on; }
/** @brief 清空所有通道数据 */
void ScopeWidget::clearData() { ++m_totalClears; for (auto &ch : m_channels) ch.fill(0); m_writePos = 0; m_wrapped = false; }
/** @brief 获取通道数量 @return 通道数 */
int ScopeWidget::channelCount() const { return m_channels.size(); }
/** @brief 查询是否正在采集 @return 运行中返回true */
bool ScopeWidget::isRunning() const { return m_running; }

/** @brief 绘制示波器波形 — 暗色背景+网格+多通道波形路径 */
void ScopeWidget::paintEvent(QPaintEvent *) {
    ++m_totalRepaints;
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int w = width(), h = height();
    p.fillRect(rect(), ThemeManager::instance().color(ThemeManager::SemanticColor::BgPrimary));
    drawGrid(p, w, h);
    static const QColor colors[] = {
        ThemeManager::instance().color(ThemeManager::SemanticColor::Success),      ///< 通道0: 绿色
        ThemeManager::instance().color(ThemeManager::SemanticColor::Warning),      ///< 通道1: 黄色
        ThemeManager::instance().color(ThemeManager::SemanticColor::Accent),       ///< 通道2: 蓝色
        ThemeManager::instance().color(ThemeManager::SemanticColor::Error)         ///< 通道3: 红色
    };
    for (int c = 0; c < m_channels.size(); ++c) {
        p.setPen(QPen(colors[c % 4], 1.5));
        int drawLen = m_wrapped ? m_bufferSize : m_writePos;
        if (drawLen < 2) continue;
        // 电压刻度为0时跳过绘制，避免除零产生inf/nan
        double scale = (qFuzzyIsNull(m_voltScale)) ? 1.0 : m_voltScale;
        QPainterPath path;
        int startIdx = m_wrapped ? m_writePos : 0;
        for (int i = 0; i < drawLen; ++i) {
            int bufIdx = (startIdx + i) % m_bufferSize;
            double x = static_cast<double>(i) / drawLen * w;
            double y = h / 2.0 - m_channels[c][bufIdx] / scale * (h / kDivisions);
            if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
        }
        p.drawPath(path);
    }
}

/** @brief 绘制网格线 — 点状网格+中心十字线 @param p 画笔 @param w 宽度 @param h 高度 */
void ScopeWidget::drawGrid(QPainter &p, int w, int h) {
    QColor gridColor = ThemeManager::instance().color(ThemeManager::SemanticColor::Border);
    QColor crossColor = ThemeManager::instance().color(ThemeManager::SemanticColor::TextSecondary);
    p.setPen(QPen(gridColor, 1, Qt::DotLine));
    for (int i = 1; i < kDivisions; ++i) {
        int x = i * w / kDivisions;
        p.drawLine(x, 0, x, h);
    }
    for (int i = 1; i < kDivisions; ++i) {
        int y = i * h / kDivisions;
        p.drawLine(0, y, w, y);
    }
    p.setPen(QPen(crossColor, 1));
    p.drawLine(w/2, 0, w/2, h);
    p.drawLine(0, h/2, w, h/2);
}

/** @brief 窗口大小变更事件 @param e 重设事件 */
void ScopeWidget::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); update(); }

// ── 统计 Getter ──
quint64 ScopeWidget::totalSamples() const { return m_totalSamples; }
quint64 ScopeWidget::totalRepaints() const { return m_totalRepaints; }
quint64 ScopeWidget::totalTriggers() const { return m_totalTriggers; }
quint64 ScopeWidget::totalOverflows() const { return m_totalOverflows; }
quint64 ScopeWidget::totalClears() const { return m_totalClears; }
quint64 ScopeWidget::totalScaleChanges() const { return m_totalScaleChanges; }
quint64 ScopeWidget::totalChannelChanges() const { return m_totalChannelChanges; }
quint64 ScopeWidget::totalPauses() const { return m_totalPauses; }
quint64 ScopeWidget::totalRestarts() const { return m_totalRestarts; }
quint64 ScopeWidget::totalTriggerFires() const { return m_totalTriggerFires; }
void ScopeWidget::resetScopeStatistics() { m_totalSamples = 0; m_totalRepaints = 0; m_totalTriggers = 0; m_totalOverflows = 0; m_totalClears = 0; m_totalScaleChanges = 0; m_totalChannelChanges = 0; m_totalPauses = 0; m_totalRestarts = 0; m_totalTriggerFires = 0; }
