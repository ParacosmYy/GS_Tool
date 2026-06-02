/**
 * @file PlaybackWidget.cpp
 * @brief 回放控制面板实现
 */

#include "core/recording/PlaybackWidget.h"

#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QTime>

PlaybackWidget::PlaybackWidget(QWidget* parent)
    : QWidget(parent)
    , m_playBtn(nullptr)
    , m_slider(nullptr)
    , m_speedCombo(nullptr)
    , m_timeLabel(nullptr)
{
    setObjectName("PlaybackWidget");
    setupUI();
}

void PlaybackWidget::setDuration(qint64 durationMs)
{
    Q_UNUSED(durationMs)
    // TODO: 设置滑块范围为 0 ~ durationMs
}

void PlaybackWidget::setCurrentTime(qint64 timeMs)
{
    Q_UNUSED(timeMs)
    // TODO: 更新滑块位置和时间标签
}

void PlaybackWidget::setupUI()
{
    auto* layout = new QHBoxLayout(this);

    m_playBtn = new QPushButton(tr("播放"), this);
    m_playBtn->setObjectName("playBtn");

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setObjectName("playbackSlider");
    m_slider->setRange(0, 0);

    m_speedCombo = new QComboBox(this);
    m_speedCombo->setObjectName("speedCombo");
    m_speedCombo->addItem("0.5x", QVariant(0.5));
    m_speedCombo->addItem("1.0x", QVariant(1.0));
    m_speedCombo->addItem("2.0x", QVariant(2.0));
    m_speedCombo->addItem("4.0x", QVariant(4.0));
    m_speedCombo->setCurrentIndex(1);

    m_timeLabel = new QLabel("00:00 / 00:00", this);
    m_timeLabel->setObjectName("timeLabel");

    layout->addWidget(m_playBtn);
    layout->addWidget(m_slider, 1);
    layout->addWidget(m_speedCombo);
    layout->addWidget(m_timeLabel);

    // TODO: 连接信号槽
}
