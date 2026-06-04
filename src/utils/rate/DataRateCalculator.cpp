/**
 * @file DataRateCalculator.cpp
 * @brief 数据速率计算器实现 — 构造、数据记录、滑动窗口计算、CSV导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 统计查询与重置方法见：@see DataRateCalculatorStats.cpp
 */

#include "utils/rate/DataRateCalculator.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>

// ──────────────────────────────────────────────
// 构造与析构
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，启动每秒采样定时器
 * @param parent 父对象
 */
DataRateCalculator::DataRateCalculator(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("DataRateCalculator"));

    connect(&m_tickTimer, &QTimer::timeout,
            this, &DataRateCalculator::onTick);
    m_tickTimer.start(1000);

    m_elapsed.start();
}

/** @brief 析构函数 */
DataRateCalculator::~DataRateCalculator() = default;

// ──────────────────────────────────────────────
// 数据记录接口
// ──────────────────────────────────────────────

/**
 * @brief 记录发送字节数
 *
 * 递增当前窗口内的 TX 累计值和全局发送字节总数。
 * 实际速率在下一秒定时器回调时计算。
 *
 * @param bytes 本次发送的字节数
 */
void DataRateCalculator::recordTx(quint64 bytes)
{
    m_accumTx += bytes;
    m_totalBytesTx += bytes;
}

/**
 * @brief 记录接收字节数
 *
 * 递增当前窗口内的 RX 累计值和全局接收字节总数。
 *
 * @param bytes 本次接收的字节数
 */
void DataRateCalculator::recordRx(quint64 bytes)
{
    m_accumRx += bytes;
    m_totalBytesRx += bytes;
}

// ──────────────────────────────────────────────
// 配置接口
// ──────────────────────────────────────────────

/**
 * @brief 设置滑动窗口大小
 *
 * 变更窗口大小不会清除已有采样数据，
 * 下次定时器回调时将按新窗口大小计算速率。
 *
 * @param size 窗口大小枚举值
 */
void DataRateCalculator::setWindowSize(WindowSize size)
{
    m_windowSize = size;
}

/**
 * @brief 设置 EMA 平滑系数
 *
 * alpha 越大响应越快(越接近瞬时值)，越小越平滑。
 * 有效范围 [0.01, 1.0]，超出范围自动裁剪。
 *
 * @param alpha 平滑系数，推荐 0.1~0.5
 */
void DataRateCalculator::setSmoothingAlpha(double alpha)
{
    m_alpha = qBound(0.01, alpha, 1.0);
}

// ──────────────────────────────────────────────
// 定时器回调 — 核心速率计算
// ──────────────────────────────────────────────

/**
 * @brief 每秒触发，执行滑动窗口速率计算和 EMA 更新
 *
 * 工作流程：
 * 1. 获取当前时间戳
 * 2. 将窗口内累计字节数创建采样点推入环形缓冲区
 * 3. 剪裁超出窗口的过期采样点
 * 4. 计算滑动窗口平均速率
 * 5. EMA 平滑更新
 * 6. 峰值检测与信号发射
 */
void DataRateCalculator::onTick()
{
    const qint64 nowMs    = QDateTime::currentMSecsSinceEpoch();
    const qint64 windowMs = static_cast<qint64>(static_cast<int>(m_windowSize)) * 1000LL;
    ++m_totalSamples;

    // ── TX 采样点 ──
    {
        RateSample sample;
        sample.timestamp = nowMs;
        sample.bytes     = m_accumTx;
        const double instantRate = m_accumTx; // 1秒内的字节数 = Bytes/s
        sample.rateBps  = instantRate;
        sample.rateKbps = instantRate * 8.0 / 1000.0;
        pushSample(m_ringTx, sample);
        m_accumTx = 0;

        // 移除过期采样
        pruneExpired(m_ringTx, nowMs - windowMs);

        // 滑动窗口速率
        const double windowRate = calcWindowRate(m_ringTx, nowMs);
        if (windowRate < 0.0) {
            // 无有效数据，保持上一次 EMA
        } else {
            m_emaRateTx = (m_emaRateTx * (1.0 - m_alpha)) + (windowRate * m_alpha);
        }

        // 峰值检测
        if (m_emaRateTx > m_peakTx) {
            m_peakTx = m_emaRateTx;
            emit peakRateChanged(QStringLiteral("TX"), m_peakTx);
        }

        // 欠载检测：窗口内完全无数据
        if (m_ringTx.isEmpty()) {
            ++m_totalUnderruns;
        }
    }

    // ── RX 采样点 ──
    {
        RateSample sample;
        sample.timestamp = nowMs;
        sample.bytes     = m_accumRx;
        const double instantRate = m_accumRx;
        sample.rateBps  = instantRate;
        sample.rateKbps = instantRate * 8.0 / 1000.0;
        pushSample(m_ringRx, sample);
        m_accumRx = 0;

        pruneExpired(m_ringRx, nowMs - windowMs);

        const double windowRate = calcWindowRate(m_ringRx, nowMs);
        if (windowRate >= 0.0) {
            m_emaRateRx = (m_emaRateRx * (1.0 - m_alpha)) + (windowRate * m_alpha);
        }

        if (m_emaRateRx > m_peakRx) {
            m_peakRx = m_emaRateRx;
            emit peakRateChanged(QStringLiteral("RX"), m_peakRx);
        }

        if (m_ringRx.isEmpty()) {
            ++m_totalUnderruns;
        }
    }

    emit rateUpdated(m_emaRateTx, m_emaRateRx);
}

// ──────────────────────────────────────────────
// 环形缓冲区操作
// ──────────────────────────────────────────────

/**
 * @brief 向环形缓冲区推入采样点，满时覆盖最旧数据
 *
 * 使用 QList 模拟环形缓冲区：当元素数量超过 kMaxRingSize 时，
 * 从头部移除最旧的元素并计入溢出计数。
 *
 * @param ring 目标环形缓冲区引用
 * @param sample 要推入的采样数据
 */
void DataRateCalculator::pushSample(QList<RateSample> &ring, const RateSample &sample)
{
    if (ring.size() >= kMaxRingSize) {
        ring.removeFirst();
        ++m_totalOverflows;
    }
    ring.append(sample);
}

/**
 * @brief 从缓冲区头部移除时间戳早于 cutoffMs 的过期采样点
 *
 * 因为采样按时间升序排列，只需从头开始连续移除即可。
 *
 * @param ring 目标环形缓冲区引用
 * @param cutoffMs 截止时间戳(ms)，早于此值的采样被移除
 */
void DataRateCalculator::pruneExpired(QList<RateSample> &ring, qint64 cutoffMs)
{
    while (!ring.isEmpty() && ring.first().timestamp < cutoffMs) {
        ring.removeFirst();
    }
}

/**
 * @brief 计算滑动窗口内的平均速率
 *
 * 基于窗口内采样点的字节增量除以时间跨度。
 * 如果窗口内不足两个采样点，返回 -1.0 表示无法计算。
 *
 * @param ring 采样环形缓冲区
 * @param nowMs 当前时间戳(ms)
 * @return 平均速率(Bytes/s)，无有效数据时返回 -1.0
 */
double DataRateCalculator::calcWindowRate(const QList<RateSample> &ring, qint64 nowMs) const
{
    if (ring.size() < 2) {
        return -1.0;
    }

    const RateSample &oldest = ring.first();
    const RateSample &newest = ring.last();

    const qint64 spanMs = newest.timestamp - oldest.timestamp;
    if (spanMs <= 0) {
        return -1.0;
    }

    // 计算窗口内字节增量(取最新与最旧的差值)
    const quint64 byteDiff = (newest.bytes >= oldest.bytes)
                                 ? (newest.bytes - oldest.bytes)
                                 : newest.bytes; // 防御性处理溢出回绕

    // Bytes/s = 字节增量 / 时间跨度(秒)
    return static_cast<double>(byteDiff) / (static_cast<double>(spanMs) / 1000.0);
}

// ──────────────────────────────────────────────
// 采样查询
// ──────────────────────────────────────────────

/**
 * @brief 获取发送方向的历史采样列表
 * @return 采样列表(按时间升序排列)
 */
QList<DataRateCalculator::RateSample> DataRateCalculator::samplesTx() const
{
    return m_ringTx;
}

/**
 * @brief 获取接收方向的历史采样列表
 * @return 采样列表(按时间升序排列)
 */
QList<DataRateCalculator::RateSample> DataRateCalculator::samplesRx() const
{
    return m_ringRx;
}

// ──────────────────────────────────────────────
// 窗口重置
// ──────────────────────────────────────────────

/**
 * @brief 重置当前窗口内的采样缓冲区
 *
 * 清空 TX/RX 环形缓冲区和窗口累计计数器，
 * 不影响全局累计字节统计和峰值记录。
 */
void DataRateCalculator::resetWindow()
{
    m_ringTx.clear();
    m_ringRx.clear();
    m_accumTx = 0;
    m_accumRx = 0;
    m_emaRateTx = 0.0;
    m_emaRateRx = 0.0;
}

// ──────────────────────────────────────────────
// CSV 导出
// ──────────────────────────────────────────────

/**
 * @brief 导出完整历史采样数据到 CSV 文件
 *
 * CSV 格式：
 * @code
 * Timestamp,TX_Bytes,TX_Rate_Bps,TX_Rate_Kbps,RX_Bytes,RX_Rate_Bps,RX_Rate_Kbps
 * 2026-06-05T12:00:00.000,1024,1024.00,8192.00,2048,2048.00,16384.00
 * @endcode
 *
 * TX 和 RX 采样按时间戳对齐合并输出。
 *
 * @param filePath 目标文件路径
 * @return true 写入成功，false 文件打开失败
 */
bool DataRateCalculator::exportHistory(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    // CSV 表头
    stream << QStringLiteral("Timestamp,TX_Bytes,TX_Rate_Bps,TX_Rate_Kbps,"
                              "RX_Bytes,RX_Rate_Bps,RX_Rate_Kbps\n");

    // 按时间戳合并 TX 和 RX 采样
    int idxTx = 0;
    int idxRx = 0;
    const int sizeTx = m_ringTx.size();
    const int sizeRx = m_ringRx.size();

    while (idxTx < sizeTx || idxRx < sizeRx) {
        const bool hasTx = idxTx < sizeTx;
        const bool hasRx = idxRx < sizeRx;

        // 选择时间戳较早的一侧推进
        bool useTx;
        if (hasTx && hasRx) {
            useTx = m_ringTx[idxTx].timestamp <= m_ringRx[idxRx].timestamp;
        } else {
            useTx = hasTx;
        }

        qint64 ts;
        QString txFields, rxFields;

        if (useTx) {
            const auto &s = m_ringTx[idxTx];
            ts = s.timestamp;
            txFields = QStringLiteral("%1,%2,%3")
                           .arg(QString::number(s.bytes), 0)
                           .arg(s.rateBps, 0, 'f', 2)
                           .arg(s.rateKbps, 0, 'f', 2);
            ++idxTx;

            // 检查 RX 是否在同一时间戳
            if (hasRx && m_ringRx[idxRx].timestamp == ts) {
                const auto &r = m_ringRx[idxRx];
                rxFields = QStringLiteral("%1,%2,%3")
                               .arg(QString::number(r.bytes), 0)
                               .arg(r.rateBps, 0, 'f', 2)
                               .arg(r.rateKbps, 0, 'f', 2);
                ++idxRx;
            } else {
                rxFields = QStringLiteral("0,0.00,0.00");
            }
        } else {
            const auto &r = m_ringRx[idxRx];
            ts = r.timestamp;
            txFields = QStringLiteral("0,0.00,0.00");
            rxFields = QStringLiteral("%1,%2,%3")
                           .arg(QString::number(r.bytes), 0)
                           .arg(r.rateBps, 0, 'f', 2)
                           .arg(r.rateKbps, 0, 'f', 2);
            ++idxRx;
        }

        // 格式化时间戳为 ISO 8601
        const QDateTime dt = QDateTime::fromMSecsSinceEpoch(ts);
        stream << dt.toString(Qt::ISODateWithMs)
               << QStringLiteral(",") << txFields
               << QStringLiteral(",") << rxFields
               << QStringLiteral("\n");
    }

    file.close();
    return true;
}

// ──────────────────────────────────────────────
// 统计查询 — 见 DataRateCalculatorStats.cpp
// ──────────────────────────────────────────────
