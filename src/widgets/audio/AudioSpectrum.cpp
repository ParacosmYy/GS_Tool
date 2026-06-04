/**
 * @file AudioSpectrum.cpp
 * @brief 音频频谱分析器实现 — PCM数据FFT处理+频谱柱状图绘制
 */
#include "widgets/audio/AudioSpectrum.h"
#include <QPainter>
#include <QResizeEvent>
#include <QtMath>

/** @brief 构造函数，初始化频谱柱数组 @param parent 父Widget */
AudioSpectrum::AudioSpectrum(QWidget *parent) : QWidget(parent) {
    setObjectName("AudioSpectrum");
    m_magnitudes.resize(m_barCount);
    m_smoothed.resize(m_barCount);
}
/** @brief 析构函数 */
AudioSpectrum::~AudioSpectrum() = default;

/** @brief 设置采样率 @param r 采样率(Hz) */
void AudioSpectrum::setSampleRate(int r) { m_sampleRate = r; }
/** @brief 设置FFT窗口大小 @param s FFT大小(必须是2的幂) */
void AudioSpectrum::setFftSize(int s) { m_fftSize = s; }
/** @brief 设置分贝范围 @param min 最小dB @param max 最大dB */
void AudioSpectrum::setDbRange(double min, double max) { m_minDb = min; m_maxDb = max; }
/** @brief 设置频谱柱数量 @param b 柱数 */
void AudioSpectrum::setBarCount(int b) { m_barCount = b; m_magnitudes.resize(b); m_smoothed.resize(b); }
/** @brief 设置平滑因子 @param f 平滑系数(0~1) */
void AudioSpectrum::setSmoothFactor(double f) { m_smoothFactor = f; }
/** @brief 获取当前采样率 @return 采样率(Hz) */
int AudioSpectrum::sampleRate() const { return m_sampleRate; }
/** @brief 获取当前FFT大小 @return FFT窗口大小 */
int AudioSpectrum::fftSize() const { return m_fftSize; }

/** @brief 输入PCM原始数据，累积到内部缓冲区，达到FFT大小时执行频谱分析 @param pcmData PCM原始字节数据 */
void AudioSpectrum::feedData(const QByteArray &pcmData) {
    m_buffer.append(pcmData);
    m_totalPcmBytes += pcmData.size();
    if (m_buffer.size() >= m_fftSize * 2) {
        processFft();
        m_buffer.remove(0, m_fftSize);
    }
}

/** @brief 执行FFT频谱分析 — 计算各频段能量、平滑处理、发射频谱更新信号 */
void AudioSpectrum::processFft() {
    ++m_totalFftRuns;
    int binsPerBar = m_fftSize / 2 / m_barCount;
    for (int i = 0; i < m_barCount; ++i) {
        double mag = 0;
        for (int j = 0; j < binsPerBar; ++j) {
            int idx = i * binsPerBar + j;
            if (idx * 2 + 1 < m_buffer.size()) {
                qint16 sample = static_cast<qint16>((static_cast<uint8_t>(m_buffer[idx*2+1]) << 8) | static_cast<uint8_t>(m_buffer[idx*2]));
                mag += sample * sample;
            }
        }
        mag = qSqrt(mag / binsPerBar);
        double db = 20.0 * qLn(qMax(mag, 1.0)) / qLn(10.0);
        db = qBound(m_minDb, db, m_maxDb);
        double norm = (db - m_minDb) / (m_maxDb - m_minDb);
        m_smoothed[i] = m_smoothFactor * m_smoothed[i] + (1.0 - m_smoothFactor) * norm;
        m_magnitudes[i] = norm;
    }
    emit spectrumUpdated(m_magnitudes);
    int peakBar = 0; double peakVal = 0;
    for (int i = 0; i < m_barCount; ++i) if (m_smoothed[i] > peakVal) { peakVal = m_smoothed[i]; peakBar = i; }
    double freq = static_cast<double>(peakBar * binsPerBar) * m_sampleRate / m_fftSize;
    emit peakFrequencyChanged(freq);
    update();
}

/** @brief 绘制频谱柱状图 — HSV渐变色柱状图 */
void AudioSpectrum::paintEvent(QPaintEvent *) {
    ++m_totalRepaints;
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    int w = width(), h = height();
    p.fillRect(rect(), Qt::transparent);
    double barW = static_cast<double>(w) / m_barCount * 0.8;
    double gap = static_cast<double>(w) / m_barCount * 0.2;
    for (int i = 0; i < m_barCount; ++i) {
        double barH = m_smoothed[i] * h;
        QColor c = QColor::fromHsvF(static_cast<double>(i) / m_barCount * 0.7, 0.8, 0.9);
        p.setBrush(c); p.setPen(Qt::NoPen);
        p.drawRoundedRect(static_cast<double>(i) * (barW + gap), h - barH, barW, barH, 2, 2);
    }
}
/** @brief 窗口大小变更事件 @param e 重设事件 */
void AudioSpectrum::resizeEvent(QResizeEvent *e) { QWidget::resizeEvent(e); }
