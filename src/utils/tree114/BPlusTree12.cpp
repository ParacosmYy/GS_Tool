#include "BPlusTree12.h"
#include <QElapsedTimer>
#include <QVariant>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>

/**
 * @brief B+树叶子节点
 */
struct BPLeaf12 {
    QVector<int> keys;
    QVector<QVariant> values;
    BPLeaf12* next;
    explicit BPLeaf12() : next(nullptr) {}
};

/**
 * @brief B+树内部节点
 */
struct BPInternal12 {
    QVector<int> keys;
    QVector<void*> children; /* BPInternal12* or BPLeaf12* */
    bool isLeaf;
    explicit BPInternal12(bool leaf) : isLeaf(leaf) {}
};

static void* bpRoot12 = nullptr;
static bool bpRootIsLeaf12 = true;

/* 在叶子中查找 */
static QVariant bpLeafSearch12(BPLeaf12* leaf, int key)
{
    for (int i = 0; i < leaf->keys.size(); ++i) {
        if (leaf->keys[i] == key) return leaf->values[i];
    }
    return QVariant();
}

/* 在内部节点中查找子节点索引 */
static int bpFindIndex12(const QVector<int>& keys, int key)
{
    int idx = 0;
    while (idx < keys.size() && keys[idx] < key) ++idx;
    return idx;
}

/* 收集范围内的键值对 */
static void bpRangeCollect12(void* node, bool isLeaf, int lo, int hi,
    QVector<QPair<int, QVariant>>& out)
{
    if (isLeaf) {
        BPLeaf12* lf = static_cast<BPLeaf12*>(node);
        for (int i = 0; i < lf->keys.size(); ++i) {
            if (lf->keys[i] >= lo && lf->keys[i] <= hi)
                out.append({lf->keys[i], lf->values[i]});
        }
    } else {
        BPInternal12* nd = static_cast<BPInternal12*>(node);
        for (int i = 0; i <= nd->keys.size(); ++i) {
            int loK = (i == 0) ? lo : qMax(lo, nd->keys[i - 1] + 1);
            int hiK = (i == nd->keys.size()) ? hi : qMin(hi, nd->keys[i]);
            if (loK <= hiK) bpRangeCollect12(nd->children[i], nd->isLeaf, loK, hiK, out);
        }
    }
}

/* 插入到叶子，返回是否溢出 */
static void bpInsertLeaf12(BPLeaf12*& leaf, int key, const QVariant& value, int maxKeys)
{
    int pos = 0;
    while (pos < leaf->keys.size() && leaf->keys[pos] < key) ++pos;
    if (pos < leaf->keys.size() && leaf->keys[pos] == key) { leaf->values[pos] = value; return; }
    leaf->keys.insert(pos, key);
    leaf->values.insert(pos, value);
}

/* 计算叶子数量 */
static int bpCount12(void* node, bool isLeaf)
{
    if (!node) return 0;
    if (isLeaf) return 1;
    BPInternal12* nd = static_cast<BPInternal12*>(node);
    int c = 0;
    for (auto* ch : nd->children) c += bpCount12(ch, nd->isLeaf);
    return c;
}

/* 树高度 */
static int bpHeight12(void* node, bool isLeaf)
{
    if (!node) return 0;
    if (isLeaf) return 1;
    BPInternal12* nd = static_cast<BPInternal12*>(node);
    return 1 + bpHeight12(nd->children[0], nd->isLeaf);
}

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
BPlusTree12::BPlusTree12(QObject* parent) : QObject(parent) {}

/**
 * @brief 重置所有统计信息
 */
void BPlusTree12::resetStatistics() { m_stats = Stats(); m_timeSum = 0.0; }

/**
 * @brief 设置B+树的阶数
 * @param order 阶数
 */
void BPlusTree12::setOrder(int order)
{
    m_order = qMax(3, order);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void BPlusTree12::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    if (!bpRoot12) {
        BPLeaf12* leaf = new BPLeaf12();
        bpInsertLeaf12(leaf, key, value, m_order - 1);
        bpRoot12 = leaf;
        bpRootIsLeaf12 = true;
    } else if (bpRootIsLeaf12) {
        BPLeaf12* leaf = static_cast<BPLeaf12*>(bpRoot12);
        bpInsertLeaf12(leaf, key, value, m_order - 1);
        /* 简化处理：溢出时创建新根 */
        if (leaf->keys.size() >= m_order) {
            int mid = leaf->keys.size() / 2;
            BPLeaf12* right = new BPLeaf12();
            for (int i = mid; i < leaf->keys.size(); ++i) {
                right->keys.append(leaf->keys[i]);
                right->values.append(leaf->values[i]);
            }
            right->next = leaf->next;
            leaf->keys.resize(mid);
            leaf->values.resize(mid);
            leaf->next = right;

            BPInternal12* root = new BPInternal12(true);
            root->keys.append(right->keys[0]);
            root->children.append(leaf);
            root->children.append(right);
            root->isLeaf = false;
            /* 标记子节点为叶子 */
            bpRoot12 = root;
            bpRootIsLeaf12 = false;
        }
    } else {
        /* 简化：递归到叶子的线性搜索插入 */
        BPInternal12* nd = static_cast<BPInternal12*>(bpRoot12);
        int idx = bpFindIndex12(nd->keys, key);
        if (idx < nd->children.size()) {
            /* 找到最左叶子 */
            void* cur = nd->children[qMin(idx, nd->children.size() - 1)];
            BPLeaf12* leaf = static_cast<BPLeaf12*>(cur);
            bpInsertLeaf12(leaf, key, value, m_order - 1);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("insert"), bpHeight12(bpRoot12, bpRootIsLeaf12));
}

/**
 * @brief 查找指定键
 * @param key 待查找的键
 * @return 关联值
 */
QVariant BPlusTree12::search(int key) const
{
    if (!bpRoot12) return QVariant();
    if (bpRootIsLeaf12) return bpLeafSearch12(static_cast<BPLeaf12*>(bpRoot12), key);
    BPInternal12* nd = static_cast<BPInternal12*>(bpRoot12);
    int idx = bpFindIndex12(nd->keys, key);
    if (idx < nd->children.size())
        return bpLeafSearch12(static_cast<BPLeaf12*>(nd->children[qMin(idx, nd->children.size() - 1)]), key);
    return QVariant();
}

/**
 * @brief 范围查询
 * @param minKey 最小键
 * @param maxKey 最大键
 * @return 范围内的键值对列表
 */
QVector<QPair<int, QVariant>> BPlusTree12::rangeQuery(int minKey, int maxKey) const
{
    QVector<QPair<int, QVariant>> result;
    if (bpRoot12) bpRangeCollect12(bpRoot12, bpRootIsLeaf12, minKey, maxKey, result);
    return result;
}

/**
 * @brief 删除指定键
 * @param key 待删除的键
 * @return 是否删除成功
 */
bool BPlusTree12::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    bool found = false;
    if (bpRootIsLeaf12 && bpRoot12) {
        BPLeaf12* leaf = static_cast<BPLeaf12*>(bpRoot12);
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] == key) {
                leaf->keys.removeAt(i);
                leaf->values.removeAt(i);
                found = true;
                break;
            }
        }
    } else if (bpRoot12) {
        BPInternal12* nd = static_cast<BPInternal12*>(bpRoot12);
        int idx = bpFindIndex12(nd->keys, key);
        BPLeaf12* leaf = static_cast<BPLeaf12*>(nd->children[qMin(idx, nd->children.size() - 1)]);
        for (int i = 0; i < leaf->keys.size(); ++i) {
            if (leaf->keys[i] == key) {
                leaf->keys.removeAt(i);
                leaf->values.removeAt(i);
                found = true;
                break;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalOperations++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOperations;
    emit operationCompleted(QStringLiteral("remove"), bpHeight12(bpRoot12, bpRootIsLeaf12));
    return found;
}
