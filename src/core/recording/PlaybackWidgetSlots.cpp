/**
 * @file PlaybackWidgetSlots.cpp
 * @brief 回放控制面板 — 信号连接实现
 *
 * 从 PlaybackWidget.cpp 拆分而来，包含:
 *   - setupConnections(): 所有按钮/滑块/下拉框的信号连接
 *
 * UI构建和公开方法见 PlaybackWidget.cpp。
 */

#include "core/recording/PlaybackWidget.h"

#include <QPushButton>
#include <QSlider>
#include <QComboBox>

/** @brief 初始化信号连接: 播放/暂停按钮切换、停止按钮、进度滑块拖拽、倍速选择 */
void PlaybackWidget::setupConnections()
{
    // 播放/暂停切换：checked=true → 播放，checked=false → 暂停
    connect(m_playBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            m_playBtn->setText(tr("⏸"));  // ⏸ 暂停图标
            m_playBtn->setToolTip(tr("暂停"));
            ++m_totalPlays;
            emit playRequested();
        } else {
            m_playBtn->setText(tr("▶"));  // ▶ 播放图标
            m_playBtn->setToolTip(tr("播放"));
            ++m_totalPauses;
            emit pauseRequested();
        }
    });

    // 停止按钮：重置播放状态并发出停止信号
    auto* stopBtn = m_playBtn->parent()->findChild<QPushButton*>("stopBtn");
    if (stopBtn) {
        connect(stopBtn, &QPushButton::clicked, this, [this]() {
            if (m_playBtn) m_playBtn->setChecked(false);  // 恢复为播放图标
            ++m_totalStops;
            emit stopRequested();
        });
    }

    // 进度滑块拖拽：将 0-10000 的整数值换算为毫秒时间
    connect(m_slider, &QSlider::sliderMoved, this, [this](int position) {
        if (m_durationMs > 0) {
            const qint64 timeMs = static_cast<qint64>(
                (position / 10000.0) * m_durationMs);
            ++m_totalSeeks;
            emit seekRequested(timeMs);
        }
    });

    // 倍速选择：解析文本中的数值部分
    connect(m_speedCombo, &QComboBox::currentTextChanged,
            this, [this](const QString& text) {
        // 从 "1x" / "0.5x" 等格式中提取数值
        bool ok = false;
        const QString numPart = text.chopped(1);  // 去掉末尾 'x'
        const qreal speed = numPart.toDouble(&ok);
        if (ok && speed > 0.0) {
            ++m_totalSpeedChanges;
            emit speedChangeRequested(speed);
        }
    });
}
