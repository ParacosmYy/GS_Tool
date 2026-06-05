/**
 * @file ScalableBloom.cpp
 * @brief 可扩展布隆过滤器实现 — 分层自动扩容
 *
 * 每层使用独立的位数组和哈希函数族，当某层填充率超过阈值时
 * 自动添加新层(容量按scaleFactor增长)，查询时检查所有层。
 */

#include "utils/bloom8/ScalableBloom.h"

#include <QElapsedTimer>
#include <QtMath>

// ── 常量 ──

/** @brief 填充率阈值(超过则扩展新层) */
static constexpr double kFillThreshold = 0.5;

/** @brief ln(2) 用于计算最优哈希数 */
static constexpr double kLn2 = 0.6931471805599453;

// ── 构造函数 ──

/** @brief 构造函数 @param parent 父对象 */
ScalableBloom::ScalableBloom(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ScalableBloom"));
}

// ── 参数设置 ──

/** @brief 设置初始容量 @param capacity 预期元素数量 */
void ScalableBloom::setInitialCapacity(int capacity)
{
    m_initialCapacity = qMax(1, capacity);
}

/** @brief 设置目标误判率 @param rate 误判率(0.0~1.0) */
void ScalableBloom::setFalsePositiveRate(double rate)
{
    m_fpRate = qBound(0.0001, rate, 0.5);
}

/** @brief 设置扩展因子 @param factor 扩展因子(>1.0) */
void ScalableBloom::setScaleFactor(double factor)
{
    m_scaleFactor = qBound(1.5, factor, 10.0);
}

// ── 插入 ──

/**
 * @brief 插入字节串
 * @param data 待插入数据
 *
 * 插入到最后一层，如果该层已满则先创建新层。
 */
void ScalableBloom::insert(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 确保至少有一层 */
    if (m_layers.isEmpty()) {
        addLayer();
    }

    /* 检查最后一层是否需要扩展 */
    checkExpand();

    /* 插入到最后一层 */
    FilterLayer& layer = m_layers.last();
    auto hashes = computeHashes(data, layer.bitCount, layer.hashCount);
    for (int pos : hashes) {
        setBit(layer.bitArray, pos);
    }
    ++layer.itemCount;
    ++m_totalItems;

    /* 更新统计 */
    ++m_stats.totalInsertions;
    m_stats.totalLayers = m_layers.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInsertions
            + m_stats.totalQueries);

    emit itemInserted(m_totalItems);
}

/**
 * @brief 插入字符串
 * @param key 待插入键
 */
void ScalableBloom::insert(const QString& key)
{
    insert(key.toUtf8());
}

// ── 查询 ──

/**
 * @brief 查询字节串是否存在
 * @param data 待查询数据
 * @return true=可能存在, false=一定不存在
 *
 * 遍历所有层，只要有一层全部匹配则返回true。
 */
bool ScalableBloom::contains(const QByteArray& data) const
{
    QElapsedTimer timer;
    timer.start();

    for (const auto& layer : m_layers) {
        auto hashes = computeHashes(data, layer.bitCount, layer.hashCount);
        bool allSet = true;
        for (int pos : hashes) {
            if (!getBit(layer.bitArray, pos)) {
                allSet = false;
                break;
            }
        }
        if (allSet) {
            const_cast<ScalableBloom*>(this)->m_stats.totalQueries++;
            return true;
        }
    }

    const_cast<ScalableBloom*>(this)->m_stats.totalQueries++;
    return false;
}

/**
 * @brief 查询字符串是否存在
 * @param key 待查询键
 * @return true=可能存在, false=一定不存在
 */
bool ScalableBloom::contains(const QString& key) const
{
    return contains(key.toUtf8());
}

// ── 分析 ──

/**
 * @brief 估算当前误判率
 * @return 误判率上界
 *
 * 各层误判率独立，总误判率为(1 - prod(1-fp_i))的近似。
 */
double ScalableBloom::estimatedFalsePositiveRate() const
{
    double totalFp = 1.0;
    for (const auto& layer : m_layers) {
        if (layer.itemCount == 0 || layer.bitCount == 0) continue;

        /* 单层误判率: (1 - e^(-kn/m))^k */
        double load = static_cast<double>(layer.itemCount)
            * static_cast<double>(layer.hashCount)
            / static_cast<double>(layer.bitCount);
        double layerFp = qPow(1.0 - qExp(-load),
            static_cast<double>(layer.hashCount));
        totalFp *= (1.0 - layerFp);
    }
    return 1.0 - totalFp;
}

/**
 * @brief 获取总内存占用(字节)
 * @return 所有层数组大小之和
 */
qint64 ScalableBloom::memoryUsage() const
{
    qint64 total = 0;
    for (const auto& layer : m_layers) {
        total += layer.bitArray.size();
    }
    return total;
}

/** @brief 获取所有层信息 */
QVector<ScalableBloom::FilterLayer> ScalableBloom::layers() const
{
    return m_layers;
}

/** @brief 清除所有数据 */
void ScalableBloom::clear()
{
    m_layers.clear();
    m_totalItems = 0;
    m_stats.totalLayers = 0;
}

/** @brief 重置统计 */
void ScalableBloom::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ── 私有方法 ──

/**
 * @brief MurmurHash3变体
 * @param data 输入数据
 * @param seed 种子
 * @return 32位哈希值
 */
quint32 ScalableBloom::murmurHash(const QByteArray& data, quint32 seed)
{
    quint32 h = seed;
    int len = data.size();
    const char* ptr = data.constData();

    /* 处理4字节块 */
    int i = 0;
    for (; i + 4 <= len; i += 4) {
        quint32 k = static_cast<quint8>(ptr[i])
            | (static_cast<quint8>(ptr[i + 1]) << 8)
            | (static_cast<quint8>(ptr[i + 2]) << 16)
            | (static_cast<quint8>(ptr[i + 3]) << 24);
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    /* 处理剩余字节 */
    quint32 tail = 0;
    int rem = len - i;
    if (rem >= 3) tail |= static_cast<quint8>(ptr[i + 2]) << 16;
    if (rem >= 2) tail |= static_cast<quint8>(ptr[i + 1]) << 8;
    if (rem >= 1) {
        tail |= static_cast<quint8>(ptr[i]);
        tail *= 0xcc9e2d51;
        tail = (tail << 15) | (tail >> 17);
        tail *= 0x1b873593;
        h ^= tail;
    }

    h ^= static_cast<quint32>(len);
    /* finalizer */
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

/**
 * @brief 双重哈希生成多个哈希值
 * @param data 输入数据
 * @param bitCount 位数组长度
 * @param hashCount 需要的哈希数量
 * @return 哈希位置列表
 *
 * 使用 h_i = h1 + i * h2 模式，避免独立哈希函数的开销。
 */
QVector<int> ScalableBloom::computeHashes(const QByteArray& data,
    int bitCount, int hashCount) const
{
    QVector<int> hashes;
    hashes.reserve(hashCount);

    quint32 h1 = murmurHash(data, 0x9747b28c);
    quint32 h2 = murmurHash(data, 0x5bd1e995);

    for (int i = 0; i < hashCount; ++i) {
        quint32 h = h1 + static_cast<quint32>(i) * h2;
        hashes.append(static_cast<int>(h % static_cast<quint32>(bitCount)));
    }
    return hashes;
}

/**
 * @brief 计算最优位数组大小和哈希函数数量
 * @param capacity 预期元素数
 * @param fpRate 目标误判率
 * @return (bitCount, hashCount)
 */
QPair<int, int> ScalableBloom::optimalSize(int capacity,
    double fpRate) const
{
    /* m = -n * ln(p) / (ln2)^2 */
    double m = -static_cast<double>(capacity) * qLn(fpRate)
        / (kLn2 * kLn2);
    int bitCount = qMax(64, static_cast<int>(qCeil(m)));

    /* k = (m/n) * ln2 */
    double k = static_cast<double>(bitCount)
        / static_cast<double>(capacity) * kLn2;
    int hashCount = qMax(1, static_cast<int>(qRound(k)));

    return {bitCount, hashCount};
}

/** @brief 检查是否需要扩展 */
void ScalableBloom::checkExpand()
{
    if (m_layers.isEmpty()) return;

    const FilterLayer& last = m_layers.last();
    double fillRatio = static_cast<double>(last.itemCount)
        / static_cast<double>(qMax(1, last.capacity));

    if (fillRatio >= kFillThreshold) {
        addLayer();
    }
}

/**
 * @brief 添加新的过滤器层
 *
 * 每层容量按scaleFactor增长，误判率收紧。
 */
void ScalableBloom::addLayer()
{
    int layerIndex = m_layers.size();
    int capacity = static_cast<int>(
        m_initialCapacity * qPow(m_scaleFactor, layerIndex));
    /* 每层误判率收紧: fp * 0.5^layerIndex */
    double layerFp = m_fpRate * qPow(0.5, layerIndex);

    auto [bitCount, hashCount] = optimalSize(capacity, layerFp);

    /* 确保bitCount是8的倍数(字节对齐) */
    int byteCount = (bitCount + 7) / 8;
    bitCount = byteCount * 8;

    FilterLayer layer;
    layer.bitCount = bitCount;
    layer.hashCount = hashCount;
    layer.itemCount = 0;
    layer.capacity = capacity;
    layer.bitArray = QByteArray(byteCount, 0);

    m_layers.append(layer);
    m_stats.totalLayers = m_layers.size();

    emit layerAdded(layerIndex, capacity);
}

/** @brief 获取位数组中某位 */
bool ScalableBloom::getBit(const QByteArray& bits, int index)
{
    int byteIdx = index / 8;
    int bitIdx = 7 - (index % 8);
    if (byteIdx < 0 || byteIdx >= bits.size()) return false;
    return (bits[byteIdx] & (1 << bitIdx)) != 0;
}

/** @brief 设置位数组中某位 */
void ScalableBloom::setBit(QByteArray& bits, int index)
{
    int byteIdx = index / 8;
    int bitIdx = 7 - (index % 8);
    if (byteIdx < 0 || byteIdx >= bits.size()) return;
    bits[byteIdx] |= static_cast<char>(1 << bitIdx);
}
