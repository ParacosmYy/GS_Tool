/**
 * @file PlaybackWidget.cpp
 * @brief 回放控制面板实现
 *
 * 提供完整的回放控制 UI，包括播放/暂停切换、停止、进度拖拽、
 * 倍速选择和时间显示。所有用户操作通过信号转发给 PlaybackController。
 */

#include "core/recording/PlaybackWidget.h"

#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QStyle>

// ============================================================================
// 辅助函数
// ============================================================================

/**
 * @brief 将毫秒时间转换为 "MM:SS.d" 格式字符串
 * @param ms 时间值（毫秒）
 * @return 格式化后的时间字符串
 */
static QString formatTime(qint64 ms)
{
    if (ms < 0) {
        ms = 0;
    }
    const int totalSeconds = static_cast<int>(ms / 1000);
    const int minutes      = totalSeconds / 60;
    const int seconds      = totalSeconds % 60;
    const int tenths       = static_cast<int>((ms % 1000) / 100);
    return QStringLiteral("%1:%2.%3")
        .arg(minutes, 2, 10, QLatin1Char('0'))
        .arg(seconds, 2, 10, QLatin1Char('0'))
        .arg(tenths, 1, 10, QLatin1Char('0'));
}

// ============================================================================
// 构造 / 析构
// ============================================================================

/**
 * @brief 构造函数，初始化控件并构建 UI
 * @param parent 父控件指针
 */
PlaybackWidget::PlaybackWidget(QWidget* parent)
    : QWidget(parent)
    , m_playBtn(nullptr)
    , m_slider(nullptr)
    , m_speedCombo(nullptr)
    , m_timeLabel(nullptr)
    , m_durationMs(0)
{
    setObjectName("PlaybackWidget");
    setupUI();
}

// ============================================================================
// 公开方法
// ============================================================================

/**
 * @brief 设置回放总时长
 *
 * 更新内部时长存储、滑块启用状态和时间标签。
 * 当时长为 0 时禁用滑块，防止用户拖拽。
 *
 * @param durationMs 总时长（毫秒）
 */
void PlaybackWidget::setDuration(qint64 durationMs)
{
    m_durationMs = durationMs;

    // 时长有效时启用滑块，否则禁用
    m_slider->setEnabled(m_durationMs > 0);

    // 重置当前位置并更新时间显示
    m_timeLabel->setText(
        formatTime(0) + tr(" / ") + formatTime(m_durationMs));
}

/**
 * @brief 设置当前回放时间
 *
 * 根据当前时间与总时长的比例更新滑块位置（阻塞信号防止反馈循环），
 * 同时更新时间标签显示。
 *
 * @param timeMs 当前时间（毫秒）
 */
void PlaybackWidget::setCurrentTime(qint64 timeMs)
{
    if (!m_slider) {
        return;
    }

    // 阻塞信号，防止 sliderMoved 信号触发 seekRequested 反馈循环
    const bool wasBlocked = m_slider->blockSignals(true);

    if (m_durationMs > 0) {
        const int pos = static_cast<int>((timeMs * 10000) / m_durationMs);
        m_slider->setValue(qBound(0, pos, 10000));
    } else {
        m_slider->setValue(0);
    }

    m_slider->blockSignals(wasBlocked);

    // 更新时间标签："当前时间 / 总时长"
    m_timeLabel->setText(
        formatTime(timeMs) + tr(" / ") + formatTime(m_durationMs));
}

// ============================================================================
// UI 构建
// ============================================================================

/**
 * @brief 构建完整的回放控制面板 UI
 *
 * 布局：[播放/暂停] [停止] [进度滑块---] [倍速选择] [时间显示]
 *
 * 滑块范围 0-10000，用于提高拖拽精度，实际时间通过比例换算。
 * 倍速选项：0.25x, 0.5x, 1x, 2x, 4x, 8x，默认 1x。
 */
void PlaybackWidget::setupUI()
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(4);

    // ---- 播放/暂停按钮 ----
    m_playBtn = new QPushButton(tr("\u25B6"), this);  // ▶
    m_playBtn->setObjectName("playBtn");
    m_playBtn->setFixedSize(36, 36);
    m_playBtn->setCheckable(true);
    m_playBtn->setToolTip(tr("播放/暂停"));

    // ---- 停止按钮（局部变量，仅用于 UI 交互） ----
    auto* stopBtn = new QPushButton(tr("\u25A0"), this);  // ■
    stopBtn->setObjectName("stopBtn");
    stopBtn->setFixedSize(36, 36);
    stopBtn->setToolTip(tr("停止"));

    // ---- 进度滑块 ----
    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setObjectName("playbackSlider");
    m_slider->setRange(0, 10000);
    m_slider->setValue(0);
    m_slider->setEnabled(false);  // 默认禁用，setDuration 后启用
    m_slider->setToolTip(tr("回放进度"));

    // ---- 倍速选择 ----
    m_speedCombo = new QComboBox(this);
    m_speedCombo->setObjectName("speedCombo");
    m_speedCombo->setFixedWidth(80);
    m_speedCombo->setToolTip(tr("播放倍速"));

    // 倍速选项列表
    const QStringList speeds = {
        tr("0.25x"),
        tr("0.5x"),
        tr("1x"),
        tr("2x"),
        tr("4x"),
        tr("8x")
    };
    m_speedCombo->addItems(speeds);
    m_speedCombo->setCurrentIndex(2);  // 默认 1x

    // ---- 时间标签 ----
    m_timeLabel = new QLabel(
        formatTime(0) + tr(" / ") + formatTime(0), this);
    m_timeLabel->setObjectName("timeLabel");
    m_timeLabel->setFixedWidth(140);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setToolTip(tr("当前时间 / 总时长"));

    // ---- 添加到布局 ----
    layout->addWidget(m_playBtn);
    layout->addWidget(stopBtn);
    layout->addWidget(m_slider, 1);  // stretch=1 占据剩余空间
    layout->addWidget(m_speedCombo);
    layout->addWidget(m_timeLabel);

    // ====================================================================
    // 信号连接
    // ====================================================================

    // 播放/暂停切换：checked=true → 播放，checked=false → 暂停
    connect(m_playBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) {
            m_playBtn->setText(tr("\u23F8"));  // ⏸ 暂停图标
            m_playBtn->setToolTip(tr("暂停"));
            ++m_totalPlays;
            emit playRequested();
        } else {
            m_playBtn->setText(tr("\u25B6"));  // ▶ 播放图标
            m_playBtn->setToolTip(tr("播放"));
            emit pauseRequested();
        }
    });

    // 停止按钮：重置播放状态并发出停止信号
    connect(stopBtn, &QPushButton::clicked, this, [this]() {
        m_playBtn->setChecked(false);  // 恢复为播放图标
        emit stopRequested();
    });

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
