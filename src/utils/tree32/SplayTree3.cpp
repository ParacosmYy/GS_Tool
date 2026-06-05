/**
 * @file SplayTree3.cpp
 * @brief Splay树增强实现 — 顺序统计/区间操作/懒标记/持久化快照
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/tree32/SplayTree3.h"

#include <QElapsedTimer>

#include <algorithm>
#include <stack>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
SplayTree3::SplayTree3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 析构函数 — 释放所有节点 */
SplayTree3::~SplayTree3()
{
    destroyTree(m_root);
}

/** @brief 递归销毁整棵树 @param n 根节点 */
void SplayTree3::destroyTree(Node* n)
{
    if (!n) return;
    destroyTree(n->left);
    destroyTree(n->right);
    delete n;
}

/** @brief 更新节点的子树大小和区间和 @param n 目标节点 */
void SplayTree3::updateSize(Node* n)
{
    if (!n) return;
    n->size = 1;
    n->sum = static_cast<double>(n->value);
    if (n->left) {
        n->size += n->left->size;
        n->sum += n->left->sum;
    }
    if (n->right) {
        n->size += n->right->size;
        n->sum += n->right->sum;
    }
}

/** @brief 下推懒标记(区间反转) @param n 目标节点 */
void SplayTree3::pushDown(Node* n)
{
    if (!n || !n->lazyReverse) return;
    n->lazyReverse = false;

    /* 交换左右子树 */
    std::swap(n->left, n->right);

    /* 向子节点传播标记 */
    if (n->left) {
        n->left->lazyReverse = !n->left->lazyReverse;
    }
    if (n->right) {
        n->right->lazyReverse = !n->right->lazyReverse;
    }
}

/** @brief 旋转操作(Zig/Zag) @param x 待旋转节点 */
void SplayTree3::rotate(Node* x)
{
    if (!x || !x->parent) return;

    Node* p = x->parent;
    Node* g = p->parent;

    pushDown(p);
    pushDown(x);

    if (p->left == x) {
        /* 右旋(Zig) */
        p->left = x->right;
        if (x->right) x->right->parent = p;
        x->right = p;
        p->parent = x;
    } else {
        /* 左旋(Zag) */
        p->right = x->left;
        if (x->left) x->left->parent = p;
        x->left = p;
        p->parent = x;
    }

    x->parent = g;
    if (g) {
        if (g->left == p) g->left = x;
        else g->right = x;
    }

    updateSize(p);
    updateSize(x);
    if (g) updateSize(g);
}

/** @brief Splay操作: 将节点x旋转到根 @param x 目标节点 */
void SplayTree3::splay(Node* x)
{
    if (!x) return;

    while (x->parent) {
        Node* p = x->parent;
        Node* g = p->parent;

        if (!g) {
            /* Zig或Zag(父节点是根) */
            rotate(x);
        } else if ((g->left == p) == (p->left == x)) {
            /* Zig-Zig或Zag-Zag(同向) */
            rotate(p);
            rotate(x);
        } else {
            /* Zig-Zag或Zag-Zig(异向) */
            rotate(x);
            rotate(x);
        }
    }

    m_root = x;
}

/** @brief 查找key对应的节点(不修改树结构) @param key 查找键 @return 节点指针或nullptr */
SplayTree3::Node* SplayTree3::findNode(double key) const
{
    Node* cur = m_root;
    while (cur) {
        if (key < cur->key && cur->left) {
            cur = cur->left;
        } else if (key > cur->key && cur->right) {
            cur = cur->right;
        } else {
            return cur;
        }
    }
    return nullptr;
}

/** @brief 插入键值对
 *  如果key已存在则更新value，否则创建新节点并splay到根
 *  @param key 键
 *  @param value 值
 */
void SplayTree3::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) {
        m_root = new Node{key, value, 1, static_cast<double>(value),
                          false, nullptr, nullptr, nullptr};
        m_size = 1;
        m_stats.totalInsertions++;
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0) ?
            m_timeSum / m_stats.totalInsertions : 0.0;
        emit inserted(key);
        return;
    }

    Node* cur = m_root;
    Node* parent = nullptr;
    while (cur) {
        pushDown(cur);
        parent = cur;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            /* key已存在，更新value */
            cur->value = value;
            updateSize(cur);
            splay(cur);
            m_stats.totalInsertions++;
            m_timeSum += static_cast<double>(timer.elapsed());
            m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0) ?
                m_timeSum / m_stats.totalInsertions : 0.0;
            emit inserted(key);
            return;
        }
    }

    Node* node = new Node{key, value, 1, static_cast<double>(value),
                          false, nullptr, nullptr, parent};
    if (key < parent->key) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    /* 向上更新size */
    Node* p = parent;
    while (p) {
        updateSize(p);
        p = p->parent;
    }

    splay(node);
    ++m_size;

    m_stats.totalInsertions++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions > 0) ?
        m_timeSum / m_stats.totalInsertions : 0.0;
    emit inserted(key);
}

/** @brief 删除指定key的节点 @param key 待删除的键 @return 是否成功删除 */
bool SplayTree3::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    if (!m_root) return false;

    /* 先splay目标节点到根 */
    Node* target = findNode(key);
    if (!target || target->key != key) {
        return false;
    }
    splay(target);
    pushDown(m_root);

    Node* leftTree = m_root->left;
    Node* rightTree = m_root->right;

    /* 断开左右子树 */
    if (leftTree) leftTree->parent = nullptr;
    if (rightTree) rightTree->parent = nullptr;

    delete m_root;
    --m_size;

    if (!leftTree && !rightTree) {
        m_root = nullptr;
    } else if (!leftTree) {
        m_root = rightTree;
    } else if (!rightTree) {
        m_root = leftTree;
    } else {
        /* 合并: 将左子树最大节点splay到根，然后接上右子树 */
        m_root = leftTree;
        Node* maxNode = leftTree;
        while (maxNode->right) {
            pushDown(maxNode);
            maxNode = maxNode->right;
        }
        splay(maxNode);
        m_root->right = rightTree;
        rightTree->parent = m_root;
        updateSize(m_root);
    }

    m_stats.totalDeletions++;
    m_timeSum += static_cast<double>(timer.elapsed());
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                   m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ?
        m_timeSum / totalOps : 0.0;
    emit removed(key);
    return true;
}

/** @brief 查找key对应的value @param key 查找键 @return 值(未找到返回0) */
int SplayTree3::find(double key) const
{
    Node* node = findNode(key);
    if (node && node->key == key) {
        const_cast<SplayTree3*>(this)->splay(node);
        m_stats.totalQueries++;
        return node->value;
    }
    m_stats.totalQueries++;
    return 0;
}

/** @brief 选择第k小的节点key(1-indexed)
 *  利用子树size信息快速定位
 *  @param k 排名(从1开始)
 *  @return 第k小的key
 */
double SplayTree3::selectKth(int k) const
{
    QElapsedTimer timer;
    timer.start();

    if (k < 1 || k > m_size) return 0.0;

    Node* cur = m_root;
    while (cur) {
        const_cast<SplayTree3*>(this)->pushDown(cur);
        int leftSize = cur->left ? cur->left->size : 0;

        if (k <= leftSize) {
            cur = cur->left;
        } else if (k == leftSize + 1) {
            const_cast<SplayTree3*>(this)->splay(cur);
            m_stats.totalQueries++;
            m_timeSum += static_cast<double>(timer.elapsed());
            int totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                           m_stats.totalQueries;
            m_stats.avgProcessingTimeMs = (totalOps > 0) ?
                m_timeSum / totalOps : 0.0;
            return cur->key;
        } else {
            k -= leftSize + 1;
            cur = cur->right;
        }
    }

    m_stats.totalQueries++;
    return 0.0;
}

/** @brief 计算key的排名(比key小的节点数+1) @param key 查询键 @return 排名 */
int SplayTree3::rank(double key) const
{
    QElapsedTimer timer;
    timer.start();

    int r = 0;
    Node* cur = m_root;
    while (cur) {
        const_cast<SplayTree3*>(this)->pushDown(cur);
        int leftSize = cur->left ? cur->left->size : 0;

        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            r += leftSize + 1;
            cur = cur->right;
        } else {
            r += leftSize + 1;
            const_cast<SplayTree3*>(this)->splay(cur);
            m_stats.totalQueries++;
            m_timeSum += static_cast<double>(timer.elapsed());
            int totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                           m_stats.totalQueries;
            m_stats.avgProcessingTimeMs = (totalOps > 0) ?
                m_timeSum / totalOps : 0.0;
            return r;
        }
    }

    m_stats.totalQueries++;
    return r;
}

/** @brief 计算[lo, hi]区间内所有value之和
 *  通过分裂-查询-合并的方式实现
 *  @param lo 区间左端点
 *  @param hi 区间右端点
 *  @return 区间内value总和
 */
double SplayTree3::rangeSum(double lo, double hi) const
{
    QElapsedTimer timer;
    timer.start();

    if (lo > hi) std::swap(lo, hi);
    double sum = 0.0;

    /* 遍历树收集[lo, hi]范围内的节点值之和 */
    std::stack<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.empty()) {
        while (cur) {
            const_cast<SplayTree3*>(this)->pushDown(cur);
            stack.push(cur);
            if (cur->key >= lo) {
                cur = cur->left;
            } else {
                break;
            }
        }

        if (stack.empty()) break;
        cur = stack.top();
        stack.pop();

        if (cur->key >= lo && cur->key <= hi) {
            sum += static_cast<double>(cur->value);
        }

        if (cur->key > hi) {
            cur = nullptr;
        } else {
            cur = cur->right;
        }
    }

    m_stats.totalQueries++;
    m_timeSum += static_cast<double>(timer.elapsed());
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions +
                   m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ?
        m_timeSum / totalOps : 0.0;
    return sum;
}

/** @brief 清空整棵树 */
void SplayTree3::clear()
{
    destroyTree(m_root);
    m_root = nullptr;
    m_size = 0;
}

/** @brief 中序遍历返回所有键值对(按键升序) @return 键值对列表 */
QVector<QPair<double,int>> SplayTree3::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    if (!m_root) return result;

    std::stack<Node*> stack;
    Node* cur = m_root;
    while (cur || !stack.empty()) {
        while (cur) {
            const_cast<SplayTree3*>(this)->pushDown(cur);
            stack.push(cur);
            cur = cur->left;
        }
        cur = stack.top();
        stack.pop();
        result.append(qMakePair(cur->key, cur->value));
        cur = cur->right;
    }
    return result;
}

/** @brief 重置所有统计计数器 */
void SplayTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
