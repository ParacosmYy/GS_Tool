/**
 * @file FrequencyCounter.cpp
 * @brief 频率计数器实现 — 脉冲计数+门控时间测量+频率/周期计算
 */
#include "widgets/freq/FrequencyCounter.h"
#include <QVBoxLayout>
#include <QDateTime>

/** @brief 构造函数，初始化门控定时器和UI @param parent 父Widget */
FrequencyCounter::FrequencyCounter(QWidget *parent) : QWidget(parent), m_gateTimer(new QTimer(this)) {
    setObjectName("FrequencyCounter");
    setupUi();
    connect(m_gateTimer, &QTimer::timeout, this, &FrequencyCounter::onGateTimeout);
}
/** @brief 析构函数 */
FrequencyCounter::~FrequencyCounter() = default;

/** @brief 初始化UI — 频率显示标签+脉冲计数标签 */
void FrequencyCounter::setupUi() {
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4,4,4,4);
    m_freqLabel = new QLabel(tr("0.000 Hz"), this);
    m_freqLabel->setObjectName("freqValueLabel");
    m_freqLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_freqLabel);
    m_countLabel = new QLabel(tr("Pulses: 0"), this);
    m_countLabel->setObjectName("freqCountLabel");
    m_countLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_countLabel);
}

/** @brief 输入单个采样值，检测上升沿并计数 @param value 采样值 */
void FrequencyCounter::feedSample(double value) {
    bool above = value >= m_triggerLevel;
    if (above && !m_lastAbove) {
        m_pulseCount++;
        m_countLabel->setText(tr("Pulses: %1").arg(m_pulseCount));
        emit pulseCounted(m_pulseCount);
    }
    m_lastAbove = above;
    if (!m_gateTimer->isActive()) {
        m_gateStart = QDateTime::currentMSecsSinceEpoch();
        m_pulseCount = 0;
        m_gateTimer->start(m_gateTime);
    }
}

/** @brief 批量输入采样数据 @param data 原始字节数据 @param sampleSize 单个采样字节数(2=int16, 4=float32) */
void FrequencyCounter::feedBuffer(const QByteArray &data, int sampleSize) {
    for (int i = 0; i + sampleSize <= data.size(); i += sampleSize) {
        double val = 0;
        if (sampleSize == 2) {
            qint16 s; memcpy(&s, data.constData()+i, 2);
            val = static_cast<double>(s);
        } else if (sampleSize == 4) {
            float f; memcpy(&f, data.constData()+i, 4);
            val = static_cast<double>(f);
        }
        feedSample(val);
    }
}

/** @brief 设置门控时间 @param ms 门控时间(毫秒) */
void FrequencyCounter::setGateTime(int ms) { m_gateTime = ms; }
/** @brief 设置触发电平 @param l 触发电平值 */
void FrequencyCounter::setTriggerLevel(double l) { m_triggerLevel = l; }
/** @brief 重置计数器和频率显示 */
void FrequencyCounter::reset() { m_pulseCount = 0; m_frequency = 0; m_freqLabel->setText(tr("0.000 Hz")); m_countLabel->setText(tr("Pulses: 0")); }
/** @brief 获取当前测量频率 @return 频率(Hz) */
double FrequencyCounter::frequency() const { return m_frequency; }
/** @brief 获取当前信号周期 @return 周期(秒) */
double FrequencyCounter::period() const { return m_frequency > 0 ? 1.0 / m_frequency : 0.0; }
/** @brief 获取当前脉冲计数 @return 脉冲数 */
int FrequencyCounter::pulseCount() const { return m_pulseCount; }

/** @brief 门控超时回调 — 计算频率并更新显示 */
void FrequencyCounter::onGateTimeout() {
    m_gateTimer->stop();
    double elapsed = static_cast<double>(QDateTime::currentMSecsSinceEpoch() - m_gateStart) / 1000.0;
    if (elapsed > 0 && m_pulseCount > 0) {
        m_frequency = m_pulseCount / elapsed;
        m_freqLabel->setText(tr("%1 Hz").arg(m_frequency, 0, 'f', 3));
        emit frequencyChanged(m_frequency);
    }
}
