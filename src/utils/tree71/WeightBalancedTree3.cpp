/**
 * @file WeightBalancedTree3.cpp
 * @brief 加权平衡树实现
 *
 * 实现基于权重因子的BB[alpha]平衡树，支持插入、删除、
 * 查询和排名操作。通过子树大小约束保持平衡。
 */

#include "utils/tree71/WeightBalancedTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
WeightBalancedTree3::WeightBalancedTree3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置平衡因子alpha
 * @param alpha 平衡因子，范围(0, 0.5)
 */
void WeightBalancedTree3::setBalanceFactor(double alpha)
{
    m_alpha = qBound(0.1, alpha, 0.49);
}

/**
 * @brief 插入键值对
 * @param key 键
 * @param value 值
 */
void WeightBalancedTree3::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    m_root = insertNode(m_root, key, value);

    // 更新高度和大小
    m_size = nodeSize(m_root);
    m_height = 0;
    WBNode* cur = m_root;
    while (cur != nullptr) {
        m_height++;
        cur = cur->left;
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalInserts++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalInserts + m_stats.totalQueries);

    emit inserted(key);
}

/**
 * @brief 删除键
 * @param key 要删除的键
 */
void WeightBalancedTree3::remove(double key)
{
    // 查找并删除节点（简化实现）
    WBNode* cur = m_root;
    WBNode* parent = nullptr;

    while (cur != nullptr) {
        if (key < cur->key) {
            parent = cur;
            cur = cur->left;
        } else if (key > cur->key) {
            parent = cur;
            cur = cur->right;
        } else {
            // 找到节点
            if (cur->left == nullptr && cur->right == nullptr) {
                // 叶节点：直接删除
                if (parent == nullptr) {
                    m_root = nullptr;
                } else if (parent->left == cur) {
                    parent->left = nullptr;
                } else {
                    parent->right = nullptr;
                }
                delete cur;
            } else if (cur->left == nullptr) {
                // 只有右子树
                if (parent == nullptr) m_root = cur->right;
                else if (parent->left == cur) parent->left = cur->right;
                else parent->right = cur->right;
                delete cur;
            } else if (cur->right == nullptr) {
                // 只有左子树
                if (parent == nullptr) m_root = cur->left;
                else if (parent->left == cur) parent->left = cur->left;
                else parent->right = cur->left;
                delete cur;
            } else {
                // 两个子树：用中序后继替换
                WBNode* succ = cur->right;
                WBNode* succParent = cur;
                while (succ->left != nullptr) {
                    succParent = succ;
                    succ = succ->left;
                }
                cur->key = succ->key;
                cur->val = succ->val;
                if (succParent->left == succ) {
                    succParent->left = succ->right;
                } else {
                    succParent->right = succ->right;
                }
                delete succ;
            }
            m_size--;
            break;
        }
    }
}

/**
 * @brief 检查是否包含指定键
 * @param key 待查找的键
 * @return 是否存在
 */
bool WeightBalancedTree3::contains(double key) const
{
    WBNode* cur = m_root;
    while (cur != nullptr) {
        if (key < cur->key) cur = cur->left;
        else if (key > cur->key) cur = cur->right;
        else return true;
    }
    return false;
}

/**
 * @brief 计算键的排名（小于该键的元素数量）
 * @param key 待查询的键
 * @return 排名
 */
int WeightBalancedTree3::rank(double key) const
{
    int r = 0;
    WBNode* cur = m_root;
    while (cur != nullptr) {
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += nodeSize(cur->left) + 1;
            cur = cur->right;
        } else {
            r += nodeSize(cur->left);
            break;
        }
    }
    return r;
}

/**
 * @brief 重置统计信息
 */
void WeightBalancedTree3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 递归插入节点
 * @param n 当前子树根
 * @param key 键
 * @param val 值
 * @return 新的子树根
 */
WeightBalancedTree3::WBNode* WeightBalancedTree3::insertNode(WBNode* n, double key, int val)
{
    if (n == nullptr) {
        return new WBNode{key, val, 1, 1, nullptr, nullptr};
    }

    if (key < n->key) {
        n->left = insertNode(n->left, key, val);
    } else if (key > n->key) {
        n->right = insertNode(n->right, key, val);
    } else {
        n->val = val; // 更新已有键
        return n;
    }

    // 更新计数和高度
    n->count = nodeSize(n->left) + nodeSize(n->right) + 1;
    int lh = (n->left) ? n->left->height : 0;
    int rh = (n->right) ? n->right->height : 0;
    n->height = qMax(lh, rh) + 1;

    return balance(n);
}

/**
 * @brief 平衡节点
 * @param n 待平衡的节点
 * @return 平衡后的子树根
 *
 * 检查左右子树的权重比是否超过alpha阈值，
 * 超过则通过旋转恢复平衡。
 */
WeightBalancedTree3::WBNode* WeightBalancedTree3::balance(WBNode* n)
{
    int ls = nodeSize(n->left);
    int rs = nodeSize(n->right);
    int total = ls + rs + 1;

    if (total <= 2) return n;

    // 检查左子树是否过重
    if (ls > m_alpha * total) {
        int lls = nodeSize(n->left->left);
        int lrs = nodeSize(n->left->right);
        // 判断单旋还是双旋
        if (lls > lrs) {
            return rotateRight(n);
        } else {
            n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
    }

    // 检查右子树是否过重
    if (rs > m_alpha * total) {
        int rls = nodeSize(n->right->left);
        int rrs = nodeSize(n->right->right);
        if (rrs > rls) {
            return rotateLeft(n);
        } else {
            n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
    }

    return n;
}

/**
 * @brief 左旋
 * @param n 旋转节点
 * @return 旋转后的新根
 */
WeightBalancedTree3::WBNode* WeightBalancedTree3::rotateLeft(WBNode* n)
{
    WBNode* r = n->right;
    n->right = r->left;
    r->left = n;

    // 更新计数
    n->count = nodeSize(n->left) + nodeSize(n->right) + 1;
    r->count = nodeSize(r->left) + nodeSize(r->right) + 1;

    // 更新高度
    int lh = (n->left) ? n->left->height : 0;
    int rh = (n->right) ? n->right->height : 0;
    n->height = qMax(lh, rh) + 1;
    int rlh = (r->left) ? r->left->height : 0;
    int rrh = (r->right) ? r->right->height : 0;
    r->height = qMax(rlh, rrh) + 1;

    return r;
}

/**
 * @brief 右旋
 * @param n 旋转节点
 * @return 旋转后的新根
 */
WeightBalancedTree3::WBNode* WeightBalancedTree3::rotateRight(WBNode* n)
{
    WBNode* l = n->left;
    n->left = l->right;
    l->right = n;

    n->count = nodeSize(n->left) + nodeSize(n->right) + 1;
    l->count = nodeSize(l->left) + nodeSize(l->right) + 1;

    int lh = (n->left) ? n->left->height : 0;
    int rh = (n->right) ? n->right->height : 0;
    n->height = qMax(lh, rh) + 1;
    int llh = (l->left) ? l->left->height : 0;
    int lrh = (l->right) ? l->right->height : 0;
    l->height = qMax(llh, lrh) + 1;

    return l;
}

/**
 * @brief 获取节点子树大小
 * @param n 节点指针
 * @return 子树中的节点数量
 */
int WeightBalancedTree3::nodeSize(WBNode* n) const
{
    return (n != nullptr) ? n->count : 0;
}
