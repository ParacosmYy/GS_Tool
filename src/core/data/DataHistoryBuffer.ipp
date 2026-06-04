/**
 * @file DataHistoryBuffer.ipp
 * @brief DataHistoryBuffer 模板内联实现
 *
 * 包含 DataHistoryBuffer<T> 所有方法的完整实现。
 * 本文件由 DataHistoryBuffer.h 尾部 #include，不应单独编译。
 */

// =============================================================================
// 构造 / 析构
// =============================================================================

template<typename T>
DataHistoryBuffer<T>::DataHistoryBuffer(int capacity, QObject* parent)
    : QObject(parent)
    , m_capacity(qMax(capacity, 1))
    , m_readIdx(0)
    , m_writeIdx(0)
    , m_count(0)
    , m_nextSeq(0)
{
    m_buffer.resize(m_capacity);
}

// =============================================================================
// 数据写入
// =============================================================================

template<typename T>
void DataHistoryBuffer<T>::push(const T& item)
{
    /* 构造条目 */
    Entry entry;
    entry.data = item;
    entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
    entry.sequenceNumber = m_nextSeq++;

    /* 写入环形缓冲区 */
    m_buffer[m_writeIdx] = std::move(entry);
    m_writeIdx = (m_writeIdx + 1) % m_capacity;

    if (m_count == m_capacity) {
        /* 缓冲区已满，覆盖最旧数据，读索引同步前移 */
        m_readIdx = (m_readIdx + 1) % m_capacity;
        ++m_stats.totalDropped;
    } else {
        ++m_count;
    }

    ++m_stats.totalPushed;

    /* 峰值追踪 */
    if (m_count > m_stats.peakSize) {
        m_stats.peakSize = m_count;
    }
    updateStatsDerived();

    /* 应用保留策略(年龄淘汰) */
    applyRetentionPolicy();

    /* 发射信号 */
    emit entryPushed(entry.sequenceNumber);
}

template<typename T>
void DataHistoryBuffer<T>::pushBatch(const QVector<T>& items)
{
    if (items.isEmpty()) {
        return;
    }

    for (const T& item : items) {
        /* 构造条目 */
        Entry entry;
        entry.data = item;
        entry.timestampMs = QDateTime::currentMSecsSinceEpoch();
        entry.sequenceNumber = m_nextSeq++;

        /* 写入环形缓冲区 */
        m_buffer[m_writeIdx] = std::move(entry);
        m_writeIdx = (m_writeIdx + 1) % m_capacity;

        if (m_count == m_capacity) {
            m_readIdx = (m_readIdx + 1) % m_capacity;
            ++m_stats.totalDropped;
        } else {
            ++m_count;
        }

        ++m_stats.totalPushed;
    }

    /* 峰值追踪(批量只检查一次) */
    if (m_count > m_stats.peakSize) {
        m_stats.peakSize = m_count;
    }
    updateStatsDerived();

    /* 批量写入后统一应用保留策略 */
    applyRetentionPolicy();
}

// =============================================================================
// 数据读取
// =============================================================================

template<typename T>
typename DataHistoryBuffer<T>::Entry DataHistoryBuffer<T>::popOldest()
{
    if (m_count == 0) {
        Entry emptyEntry;
        emptyEntry.sequenceNumber = -1;
        emptyEntry.timestampMs = 0;
        return emptyEntry;
    }

    Entry result = m_buffer[m_readIdx];
    m_readIdx = (m_readIdx + 1) % m_capacity;
    --m_count;
    ++m_stats.totalPopped;

    updateStatsDerived();
    return result;
}

template<typename T>
typename DataHistoryBuffer<T>::Entry DataHistoryBuffer<T>::newest() const
{
    if (m_count == 0) {
        Entry emptyEntry;
        emptyEntry.sequenceNumber = -1;
        emptyEntry.timestampMs = 0;
        return emptyEntry;
    }
    /* 最新元素在写索引的前一个位置 */
    int idx = (m_writeIdx - 1 + m_capacity) % m_capacity;
    return m_buffer[idx];
}

template<typename T>
typename DataHistoryBuffer<T>::Entry DataHistoryBuffer<T>::oldest() const
{
    if (m_count == 0) {
        Entry emptyEntry;
        emptyEntry.sequenceNumber = -1;
        emptyEntry.timestampMs = 0;
        return emptyEntry;
    }
    return m_buffer[m_readIdx];
}

// =============================================================================
// 范围查询
// =============================================================================

template<typename T>
QVector<typename DataHistoryBuffer<T>::Entry> DataHistoryBuffer<T>::range(
    qint64 fromSeq, qint64 toSeq) const
{
    ++m_stats.totalQueries;

    QVector<Entry> result;
    if (m_count == 0 || fromSeq > toSeq) {
        return result;
    }

    /* 线性扫描：从最旧到最新，收集序列号在 [fromSeq, toSeq] 内的条目 */
    result.reserve(qMin(static_cast<qint64>(m_count), toSeq - fromSeq + 1));

    for (int i = 0; i < m_count; ++i) {
        int physIdx = (m_readIdx + i) % m_capacity;
        const Entry& e = m_buffer[physIdx];
        if (e.sequenceNumber > toSeq) {
            /* 已超出范围上界，提前终止 */
            break;
        }
        if (e.sequenceNumber >= fromSeq) {
            result.append(e);
        }
    }

    return result;
}

template<typename T>
QVector<typename DataHistoryBuffer<T>::Entry> DataHistoryBuffer<T>::timeRange(
    qint64 fromMs, qint64 toMs) const
{
    ++m_stats.totalQueries;

    QVector<Entry> result;
    if (m_count == 0 || fromMs > toMs) {
        return result;
    }

    /* 二分查找定位起始位置(>=fromMs的最左元素) */
    int startLogical = binarySearchByTime(fromMs);

    /* 从 startLogical 开始收集，直到超出 toMs 或遍历完所有数据 */
    result.reserve(qMin(m_count, 256)); // 预分配合理上限
    for (int i = startLogical; i < m_count; ++i) {
        int physIdx = (m_readIdx + i) % m_capacity;
        const Entry& e = m_buffer[physIdx];
        if (e.timestampMs > toMs) {
            break;
        }
        result.append(e);
    }

    return result;
}

template<typename T>
QVector<typename DataHistoryBuffer<T>::Entry> DataHistoryBuffer<T>::lastN(
    int count) const
{
    ++m_stats.totalQueries;

    QVector<Entry> result;
    if (m_count == 0 || count <= 0) {
        return result;
    }

    int n = qMin(count, m_count);
    result.reserve(n);

    /* 从倒数第n个元素开始收集 */
    int startLogical = m_count - n;
    for (int i = startLogical; i < m_count; ++i) {
        int physIdx = (m_readIdx + i) % m_capacity;
        result.append(m_buffer[physIdx]);
    }

    return result;
}

// =============================================================================
// 策略与状态
// =============================================================================

template<typename T>
void DataHistoryBuffer<T>::setRetentionPolicy(const RetentionPolicy& policy)
{
    m_policy = policy;

    /* 如果新策略容量小于当前容量，裁剪多余数据 */
    if (m_policy.maxEntries < m_capacity) {
        /* 需要淘汰的超额数量 */
        int excess = m_count - m_policy.maxEntries;
        if (excess > 0) {
            if (m_policy.dropOldest) {
                m_readIdx = (m_readIdx + excess) % m_capacity;
                m_count -= excess;
                m_stats.totalDropped += excess;
                emit bufferOverflow(excess);
            }
            /* dropOldest=false 时拒绝淘汰，保留现有数据 */
        }
    }

    /* 应用年龄策略 */
    applyRetentionPolicy();
    updateStatsDerived();
}

template<typename T>
const typename DataHistoryBuffer<T>::RetentionPolicy&
DataHistoryBuffer<T>::retentionPolicy() const
{
    return m_policy;
}

template<typename T>
int DataHistoryBuffer<T>::size() const
{
    return m_count;
}

template<typename T>
int DataHistoryBuffer<T>::capacity() const
{
    return m_capacity;
}

template<typename T>
bool DataHistoryBuffer<T>::isEmpty() const
{
    return m_count == 0;
}

template<typename T>
void DataHistoryBuffer<T>::clear()
{
    m_readIdx = 0;
    m_writeIdx = 0;
    m_count = 0;
    /* 注意: 不重置 m_nextSeq，保持序列号全局单调递增 */
    updateStatsDerived();
    emit bufferCleared();
}

// =============================================================================
// 数据处理
// =============================================================================

template<typename T>
void DataHistoryBuffer<T>::compress(int keepEveryN)
{
    ++m_stats.totalCompressions;

    if (keepEveryN <= 1 || m_count <= 1) {
        /* keepEveryN<=1 保留全部，无需压缩 */
        return;
    }

    /* 收集需要保留的条目 */
    QVector<Entry> kept;
    kept.reserve(m_count / keepEveryN + 1);

    for (int i = 0; i < m_count; ++i) {
        if (i % keepEveryN == 0) {
            int physIdx = (m_readIdx + i) % m_capacity;
            kept.append(m_buffer[physIdx]);
        }
    }

    int dropped = m_count - static_cast<int>(kept.size());
    if (dropped <= 0) {
        return;
    }

    /* 将保留的条目重新紧凑排列到缓冲区头部 */
    m_count = 0;
    m_readIdx = 0;
    m_writeIdx = 0;
    for (const Entry& e : kept) {
        m_buffer[m_writeIdx] = e;
        m_writeIdx = (m_writeIdx + 1) % m_capacity;
        ++m_count;
    }

    m_stats.totalDropped += dropped;
    updateStatsDerived();
    emit bufferOverflow(dropped);
}

template<typename T>
qint64 DataHistoryBuffer<T>::exportToCsv(
    const QString& filePath,
    std::function<QString(const T&)> formatter)
{
    ++m_stats.totalExports;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return -1;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    /* 写入CSV头部 */
    stream << "sequence_number,timestamp_ms,datetime,data\n";

    qint64 exported = 0;
    for (int i = 0; i < m_count; ++i) {
        int physIdx = (m_readIdx + i) % m_capacity;
        const Entry& e = m_buffer[physIdx];

        /* 格式化时间戳为可读日期时间 */
        QString datetime = QDateTime::fromMSecsSinceEpoch(e.timestampMs)
                               .toString(Qt::ISODateWithMs);

        stream << e.sequenceNumber << ","
               << e.timestampMs << ","
               << datetime << ","
               << formatter(e.data) << "\n";
        ++exported;
    }

    stream.flush();
    file.close();
    return exported;
}

// =============================================================================
// 统计
// =============================================================================

template<typename T>
const typename DataHistoryBuffer<T>::Stats& DataHistoryBuffer<T>::stats() const
{
    return m_stats;
}

template<typename T>
void DataHistoryBuffer<T>::resetStatistics()
{
    m_stats = Stats{};
    updateStatsDerived();
}

// =============================================================================
// 私有方法
// =============================================================================

template<typename T>
void DataHistoryBuffer<T>::applyRetentionPolicy()
{
    /* ── 年龄淘汰: 移除超过 maxAgeMs 的旧条目 ── */
    if (m_policy.maxAgeMs > 0 && m_count > 0) {
        qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
        qint64 threshold = nowMs - m_policy.maxAgeMs;

        int dropped = 0;
        while (m_count > 0) {
            const Entry& oldest_entry = m_buffer[m_readIdx];
            if (oldest_entry.timestampMs < threshold) {
                m_readIdx = (m_readIdx + 1) % m_capacity;
                --m_count;
                ++dropped;
            } else {
                break;
            }
        }

        if (dropped > 0) {
            m_stats.totalDropped += dropped;
            updateStatsDerived();
            emit bufferOverflow(dropped);
        }
    }

    /* ── 容量淘汰: 如果仍然超过最大条目数 ── */
    if (m_count > m_policy.maxEntries) {
        int excess = m_count - m_policy.maxEntries;
        if (m_policy.dropOldest) {
            m_readIdx = (m_readIdx + excess) % m_capacity;
            m_count = m_policy.maxEntries;
            m_stats.totalDropped += excess;
            updateStatsDerived();
            emit bufferOverflow(excess);
        }
        /* dropOldest=false: 拒绝淘汰，保留超额数据 */
    }
}

template<typename T>
void DataHistoryBuffer<T>::updateStatsDerived()
{
    m_stats.currentSize = m_count;
    m_stats.bytesStored = static_cast<quint64>(m_count) * sizeof(Entry);
    m_stats.fillRatio = (m_capacity > 0)
        ? static_cast<double>(m_count) / static_cast<double>(m_capacity)
        : 0.0;
}

template<typename T>
int DataHistoryBuffer<T>::binarySearchByTime(qint64 targetMs) const
{
    /**
     * 在逻辑有序区间 [0, m_count) 中二分查找，
     * 返回第一个 timestampMs >= targetMs 的逻辑索引。
     * 由于环形缓冲区中数据按时间升序排列(序列号单调递增)，
     * 可以直接在逻辑索引空间上进行二分。
     */
    int lo = 0;
    int hi = m_count;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int physIdx = (m_readIdx + mid) % m_capacity;
        if (m_buffer[physIdx].timestampMs < targetMs) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    return lo; /* lo == m_count 表示所有元素都 < targetMs */
}
