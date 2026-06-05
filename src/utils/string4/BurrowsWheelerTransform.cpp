/**
 * @file BurrowsWheelerTransform.cpp
 * @brief Burrows-Wheeler变换与逆变换实现
 * @author Serial Tool Team
 * @date 2026-06-05
 */

#include "utils/string4/BurrowsWheelerTransform.h"

#include <QElapsedTimer>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
BurrowsWheelerTransform::BurrowsWheelerTransform(QObject *parent)
    : QObject(parent)
{
}

/** @brief 对二进制数据执行BWT变换
 *         构造所有循环移位，排序后取最后一列
 *  @param data 输入数据
 *  @return QPair<变换后数据, 原始行索引> */
QPair<QByteArray, int> BurrowsWheelerTransform::transform(const QByteArray &data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    if (n == 0) {
        return {{}, 0};
    }

    /* 构造旋转索引数组 */
    QVector<int> indices(n);
    for (int i = 0; i < n; ++i) {
        indices[i] = i;
    }

    /* 使用自定义比较器对旋转后的字符串排序 */
    const char *ptr = data.constData();
    std::stable_sort(indices.begin(), indices.end(),
        [ptr, n](int a, int b) {
            for (int k = 0; k < n; ++k) {
                char ca = ptr[(a + k) % n];
                char cb = ptr[(b + k) % n];
                if (ca < cb) return true;
                if (ca > cb) return false;
            }
            return false; /* 完全相等 */
        });

    /* 构造输出: 最后一列 = 排序后各行的最后一个字符 */
    QByteArray result(n, Qt::Uninitialized);
    int originalIndex = 0;
    for (int i = 0; i < n; ++i) {
        result[i] = ptr[(indices[i] + n - 1) % n];
        if (indices[i] == 0) {
            originalIndex = i;
        }
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalTransforms + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit transformCompleted(n, originalIndex);
    return {result, originalIndex};
}

/** @brief 对二进制数据执行BWT逆变换(LF映射算法)
 *  @param transformed 变换后数据
 *  @param originalIndex 原始行索引
 *  @return 恢复的原始数据 */
QByteArray BurrowsWheelerTransform::inverseTransform(const QByteArray &transformed,
                                                     int originalIndex)
{
    QElapsedTimer timer;
    timer.start();

    int n = transformed.size();
    if (n == 0 || originalIndex < 0 || originalIndex >= n) {
        return {};
    }

    const char *t = transformed.constData();

    /* 步骤1: 统计各字节出现次数 */
    int count[256] = {};
    for (int i = 0; i < n; ++i) {
        count[static_cast<unsigned char>(t[i])]++;
    }

    /* 步骤2: 计算各字节的起始位置(前缀和) */
    int cumul[256] = {};
    int sum = 0;
    for (int c = 0; c < 256; ++c) {
        cumul[c] = sum;
        sum += count[c];
    }

    /* 步骤3: 构造LF映射 T[i] → 排序后第一列中同一字符的第k次出现 */
    QVector<int> lf(n);
    int occ[256] = {}; /* 各字符出现次数 */
    for (int i = 0; i < n; ++i) {
        unsigned char c = static_cast<unsigned char>(t[i]);
        lf[i] = cumul[c] + occ[c];
        ++occ[c];
    }

    /* 步骤4: 从originalIndex开始，沿LF映射逆推n步恢复原始数据 */
    QByteArray result(n, Qt::Uninitialized);
    int idx = originalIndex;
    for (int i = n - 1; i >= 0; --i) {
        result[i] = t[idx];
        idx = lf[idx];
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalInverse;
    m_timeSum += elapsed;
    quint64 totalOps = m_stats.totalTransforms + m_stats.totalInverse;
    m_stats.avgProcessingTimeMs = (totalOps > 0)
        ? m_timeSum / static_cast<double>(totalOps) : 0.0;

    emit inverseCompleted(n);
    return result;
}

/** @brief 对字符串执行BWT变换(转换为UTF-8后调用二进制接口)
 *  @param str 输入字符串
 *  @return QPair<变换后字符串, 原始行索引> */
QPair<QString, int> BurrowsWheelerTransform::transformString(const QString &str)
{
    if (str.isEmpty()) {
        return {{}, 0};
    }

    QByteArray utf8 = str.toUtf8();
    auto result = transform(utf8);
    return {QString::fromUtf8(result.first), result.second};
}

/** @brief 对字符串执行BWT逆变换
 *  @param transformed 变换后字符串
 *  @param index 原始行索引
 *  @return 恢复的原始字符串 */
QString BurrowsWheelerTransform::inverseTransformString(const QString &transformed,
                                                        int index)
{
    if (transformed.isEmpty()) {
        return {};
    }

    QByteArray utf8 = transformed.toUtf8();
    QByteArray result = inverseTransform(utf8, index);
    return QString::fromUtf8(result);
}

/** @brief 重置统计计数器 */
void BurrowsWheelerTransform::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
