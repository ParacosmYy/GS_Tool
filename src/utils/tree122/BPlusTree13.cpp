#include "BPlusTree13.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief B+树叶子节点
 */
struct BPLeaf13 {
    QVector<int> keys;
    QVector<double> values;
    BPLeaf13* next;
    BPLeaf13() : next(nullptr) {}
};

/**
 * @brief B+树内部节点
 */
struct BPInternal13 {
    QVector<int> keys;
    QVector<void*> children;
    bool childIsLeaf;
    BPInternal13() : childIsLeaf(true) {}
};

static void* bpRoot13 = nullptr;
static bool bpRootLeaf13 = true;
static int bpOrder13 = 64;

static int bpSz13(void* n, bool isLeaf)
{
    if (!n) return 0;
    if (isLeaf) return static_cast<BPLeaf13*>(n)->keys.size();
    BPInternal13* nd = static_cast<BPInternal13*>(n);
    int s = 0;
    for (auto* ch : nd->children) s += bpSz13(ch, nd->childIsLeaf);
    return s;
}

static int bpHeight13(void* n, bool isLeaf)
{
    if (!n) return 0;
    if (isLeaf) return 1;
    BPInternal13* nd = static_cast<BPInternal13*>(n);
    return 1 + (nd->children.isEmpty() ? 0 : bpHeight13(nd->children[0], nd->childIsLeaf));
}

static void bpRange13(void* n, bool isLeaf, int lo, int hi, QVector<QPair<int, double>>& out)
{
    if (!n) return;
    if (isLeaf) {
        BPLeaf13* lf = static_cast<BPLeaf13*>(n);
        for (int i = 0; i < lf->keys.size(); ++i)
            if (lf->keys[i] >= lo && lf->keys[i] <= hi)
                out.append({lf->keys[i], lf->values[i]});
        return;
    }
    BPInternal13* nd = static_cast<BPInternal13*>(n);
    for (int i = 0; i <= nd->keys.size(); ++i) {
        int loK = (i == 0) ? lo : qMax(lo, nd->keys[i - 1]);
        int hiK = (i == nd->keys.size()) ? hi : qMin(hi, nd->keys[i]);
        if (loK <= hiK && i < nd->children.size())
            bpRange13(nd->children[i], nd->childIsLeaf, loK, hiK, out);
    }
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
BPlusTree13::BPlusTree13(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void BPlusTree13::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 设置B+树阶数
 * @param order 阶数
 */
void BPlusTree13::setOrder(int order) { bpOrder13 = qMax(3, order); }

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 * @return 是否插入成功
 */
bool BPlusTree13::insert(int key, double value)
{
    QElapsedTimer timer;
    timer.start();

    if (!bpRoot13) {
        BPLeaf13* leaf = new BPLeaf13();
        leaf->keys.append(key);
        leaf->values.append(value);
        bpRoot13 = leaf;
        bpRootLeaf13 = true;
    } else if (bpRootLeaf13) {
        BPLeaf13* leaf = static_cast<BPLeaf13*>(bpRoot13);
        int pos = 0;
        while (pos < leaf->keys.size() && leaf->keys[pos] < key) ++pos;
        if (pos < leaf->keys.size() && leaf->keys[pos] == key) {
            leaf->values[pos] = value;
        } else {
            leaf->keys.insert(pos, key);
            leaf->values.insert(pos, value);
        }
        /* 溢出分裂 */
        if (leaf->keys.size() >= bpOrder13) {
            int mid = leaf->keys.size() / 2;
            BPLeaf13* right = new BPLeaf13();
            for (int i = mid; i < leaf->keys.size(); ++i) {
                right->keys.append(leaf->keys[i]);
                right->values.append(leaf->values[i]);
            }
            right->next = leaf->next;
            leaf->keys.resize(mid);
            leaf->values.resize(mid);
            leaf->next = right;

            BPInternal13* root = new BPInternal13();
            root->keys.append(right->keys[0]);
            root->children.append(leaf);
            root->children.append(right);
            root->childIsLeaf = true;
            bpRoot13 = root;
            bpRootLeaf13 = false;
        }
    } else {
        /* 简化：遍历到第一个叶子插入 */
        BPInternal13* nd = static_cast<BPInternal13*>(bpRoot13);
        int idx = 0;
        while (idx < nd->keys.size() && key >= nd->keys[idx]) ++idx;
        if (idx < nd->children.size()) {
            BPLeaf13* leaf = static_cast<BPLeaf13*>(nd->children[idx]);
            int pos = 0;
            while (pos < leaf->keys.size() && leaf->keys[pos] < key) ++pos;
            if (pos < leaf->keys.size() && leaf->keys[pos] == key) {
                leaf->values[pos] = value;
            } else {
                leaf->keys.insert(pos, key);
                leaf->values.insert(pos, value);
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(bpSz13(bpRoot13, bpRootLeaf13));
    return true;
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool BPlusTree13::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    bool found = false;
    if (bpRootLeaf13 && bpRoot13) {
        BPLeaf13* leaf = static_cast<BPLeaf13*>(bpRoot13);
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] == key) { leaf->keys.removeAt(i); leaf->values.removeAt(i); found = true; break; }
        }
    } else if (bpRoot13) {
        BPInternal13* nd = static_cast<BPInternal13*>(bpRoot13);
        int idx = 0;
        while (idx < nd->keys.size() && key >= nd->keys[idx]) ++idx;
        if (idx < nd->children.size()) {
            BPLeaf13* leaf = static_cast<BPLeaf13*>(nd->children[idx]);
            for (int i = 0; i < leaf->keys.size(); ++i) {
                if (leaf->keys[i] == key) { leaf->keys.removeAt(i); leaf->values.removeAt(i); found = true; break; }
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTreeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTreeOps;
    emit treeOperationCompleted(bpSz13(bpRoot13, bpRootLeaf13));
    return found;
}

/**
 * @brief 精确查找指定键
 * @param key 待查找的键
 * @return 键对应的值，未找到返回0.0
 */
double BPlusTree13::search(int key) const
{
    if (!bpRoot13) return 0.0;
    if (bpRootLeaf13) {
        BPLeaf13* leaf = static_cast<BPLeaf13*>(bpRoot13);
        for (int i = 0; i < leaf->keys.size(); ++i)
            if (leaf->keys[i] == key) return leaf->values[i];
        return 0.0;
    }
    BPInternal13* nd = static_cast<BPInternal13*>(bpRoot13);
    int idx = 0;
    while (idx < nd->keys.size() && key >= nd->keys[idx]) ++idx;
    if (idx < nd->children.size()) {
        BPLeaf13* leaf = static_cast<BPLeaf13*>(nd->children[idx]);
        for (int i = 0; i < leaf->keys.size(); ++i)
            if (leaf->keys[i] == key) return leaf->values[i];
    }
    return 0.0;
}

/**
 * @brief 范围查询 [minKey, maxKey]
 * @param minKey 最小键
 * @param maxKey 最大键
 * @return 范围内的键值对列表
 */
QVector<QPair<int, double>> BPlusTree13::rangeQuery(int minKey, int maxKey) const
{
    QVector<QPair<int, double>> result;
    bpRange13(bpRoot13, bpRootLeaf13, minKey, maxKey, result);
    return result;
}

/**
 * @brief 获取B+树的层级数
 * @return 树的高度
 */
int BPlusTree13::height() const { return bpHeight13(bpRoot13, bpRootLeaf13); }
