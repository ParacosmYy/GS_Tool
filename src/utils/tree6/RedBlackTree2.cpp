/**
 * @file RedBlackTree2.cpp
 * @brief 红黑树(顺序统计)实现 — select/rank平衡BST
 */

#include "utils/tree6/RedBlackTree2.h"

#include <QElapsedTimer>
#include <QStack>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RedBlackTree2::RedBlackTree2(QObject* parent)
    : QObject(parent)
{
    /* 初始化NIL哨兵 */
    m_nil = new Node();
    m_nil->color = Color::Black;
    m_nil->size = 0;
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

/** @brief 析构函数 */
RedBlackTree2::~RedBlackTree2()
{
    clear();
    delete m_nil;
}

/** @brief 插入键值对 @param key 键值 @param value 关联值 */
void RedBlackTree2::insert(double key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    /* 标准BST插入 */
    Node* z = new Node();
    z->key = key;
    z->value = value;
    z->color = Color::Red;
    z->left = m_nil;
    z->right = m_nil;
    z->parent = m_nil;
    z->size = 1;

    Node* y = m_nil;
    Node* x = m_root;

    while (x != m_nil) {
        y = x;
        y->size += 1; /* 路径上节点大小+1 */
        if (z->key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == m_nil) {
        m_root = z;
    } else if (z->key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    /* 修复红黑性质 */
    insertFixup(z);

    /* 统计 */
    ++m_stats.totalInserts;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    int ops = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (ops > 0) ? m_timeSum / ops : 0.0;

    int h = height();
    if (h > m_stats.maxHeight) m_stats.maxHeight = h;

    emit nodeInserted(key, size());
}

/** @brief 删除键 @param key 键值 @return 成功返回true */
bool RedBlackTree2::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找节点 */
    Node* z = m_root;
    while (z != m_nil) {
        if (key < z->key) {
            z = z->left;
        } else if (key > z->key) {
            z = z->right;
        } else {
            break;
        }
    }

    if (z == m_nil) {
        emit nodeRemoved(key, false);
        return false;
    }

    /* 删除节点 */
    Node* y = z;
    Node* x = nullptr;
    Color yOriginalColor = y->color;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        yOriginalColor = y->color;
        x = y->right;

        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        transplant(z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    /* 更新路径上的size */
    Node* p = x->parent;
    while (p != m_nil) {
        updateSize(p);
        p = p->parent;
    }

    if (yOriginalColor == Color::Black) {
        deleteFixup(x);
    }

    delete z;

    ++m_stats.totalDeletes;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    int ops = m_stats.totalInserts + m_stats.totalDeletes + m_stats.totalSearches;
    m_stats.avgProcessingTimeMs = (ops > 0) ? m_timeSum / ops : 0.0;

    emit nodeRemoved(key, true);
    return true;
}

/** @brief 查找键 @param key 键值 @return 节点指针 */
const RedBlackTree2::Node* RedBlackTree2::find(double key) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            ++m_stats.totalSearches;
            return x;
        }
    }
    ++m_stats.totalSearches;
    return nullptr;
}

/** @brief 选择第k小 @param k 排名(从1开始) @return 节点 */
const RedBlackTree2::Node* RedBlackTree2::select(int k) const
{
    if (k < 1 || k > size()) return nullptr;

    Node* x = m_root;
    while (x != m_nil) {
        int leftSize = nodeSize(x->left);
        if (k <= leftSize) {
            x = x->left;
        } else if (k == leftSize + 1) {
            return x;
        } else {
            k -= leftSize + 1;
            x = x->right;
        }
    }
    return nullptr;
}

/** @brief 查询排名 @param key 键值 @return 排名(从1开始) */
int RedBlackTree2::rank(double key) const
{
    int r = 0;
    Node* x = m_root;

    while (x != m_nil) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            r += nodeSize(x->left) + 1;
            x = x->right;
        } else {
            r += nodeSize(x->left) + 1;
            return r;
        }
    }
    return -1;
}

/** @brief 树大小 @return 元素数量 */
int RedBlackTree2::size() const
{
    return nodeSize(m_root);
}

/** @brief 树高度 @return 高度 */
int RedBlackTree2::height() const
{
    return heightHelper(m_root);
}

/** @brief 中序遍历 @return 有序列表 */
QVector<QPair<double, QVariant>> RedBlackTree2::inOrderTraversal() const
{
    QVector<QPair<double, QVariant>> result;
    result.reserve(size());
    inOrderHelper(m_root, result);
    return result;
}

/** @brief 范围查询 @param lo 下界 @param hi 上界 @return 结果 */
QVector<QPair<double, QVariant>> RedBlackTree2::rangeQuery(double lo, double hi) const
{
    QVector<QPair<double, QVariant>> result;

    /* 迭代中序遍历 + 范围过滤 */
    QStack<Node*> stack;
    Node* current = m_root;

    while (current != m_nil || !stack.isEmpty()) {
        while (current != m_nil) {
            stack.push(current);
            current = current->left;
        }
        current = stack.pop();
        if (current->key >= lo && current->key <= hi) {
            result.append({current->key, current->value});
        }
        if (current->key > hi) break;
        current = current->right;
    }
    return result;
}

/** @brief 清空树 */
void RedBlackTree2::clear()
{
    destroyTree(m_root);
    m_root = m_nil;
}

/** @brief 重置统计 */
void RedBlackTree2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 左旋 @param x 旋转中心 */
void RedBlackTree2::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;

    if (y->left != m_nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == m_nil) {
        m_root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;

    /* 更新size: 先更新x再更新y */
    updateSize(x);
    updateSize(y);

    ++m_stats.totalRotations;
}

/** @brief 右旋 @param x 旋转中心 */
void RedBlackTree2::rotateRight(Node* x)
{
    Node* y = x->left;
    x->left = y->right;

    if (y->right != m_nil) {
        y->right->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == m_nil) {
        m_root = y;
    } else if (x == x->parent->right) {
        x->parent->right = y;
    } else {
        x->parent->left = y;
    }

    y->right = x;
    x->parent = y;

    updateSize(x);
    updateSize(y);

    ++m_stats.totalRotations;
}

/** @brief 插入修复 @param z 新节点 */
void RedBlackTree2::insertFixup(Node* z)
{
    while (z->parent->color == Color::Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == Color::Red) {
                z->parent->color = Color::Black;
                y->color = Color::Black;
                z->parent->parent->color = Color::Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    rotateLeft(z);
                }
                z->parent->color = Color::Black;
                z->parent->parent->color = Color::Red;
                rotateRight(z->parent->parent);
            }
        } else {
            Node* y = z->parent->parent->left;
            if (y->color == Color::Red) {
                z->parent->color = Color::Black;
                y->color = Color::Black;
                z->parent->parent->color = Color::Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rotateRight(z);
                }
                z->parent->color = Color::Black;
                z->parent->parent->color = Color::Red;
                rotateLeft(z->parent->parent);
            }
        }
    }
    m_root->color = Color::Black;
}

/** @brief 删除修复 @param x 替换节点 */
void RedBlackTree2::deleteFixup(Node* x)
{
    while (x != m_root && x->color == Color::Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Color::Red) {
                w->color = Color::Black;
                x->parent->color = Color::Red;
                rotateLeft(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Color::Black && w->right->color == Color::Black) {
                w->color = Color::Red;
                x = x->parent;
            } else {
                if (w->right->color == Color::Black) {
                    w->left->color = Color::Black;
                    w->color = Color::Red;
                    rotateRight(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Color::Black;
                w->right->color = Color::Black;
                rotateLeft(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == Color::Red) {
                w->color = Color::Black;
                x->parent->color = Color::Red;
                rotateRight(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Color::Black && w->left->color == Color::Black) {
                w->color = Color::Red;
                x = x->parent;
            } else {
                if (w->left->color == Color::Black) {
                    w->right->color = Color::Black;
                    w->color = Color::Red;
                    rotateLeft(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Color::Black;
                w->left->color = Color::Black;
                rotateRight(x->parent);
                x = m_root;
            }
        }
    }
    x->color = Color::Black;
}

/** @brief 用v替换u @param u 被替换 @param v 替换者 */
void RedBlackTree2::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) {
        m_root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

/** @brief 查找子树最小 @param x 根 @return 最小节点 */
RedBlackTree2::Node* RedBlackTree2::minimum(Node* x) const
{
    while (x->left != m_nil) {
        x = x->left;
    }
    return x;
}

/** @brief 更新节点大小 @param x 目标 */
void RedBlackTree2::updateSize(Node* x)
{
    if (x != m_nil) {
        x->size = 1 + nodeSize(x->left) + nodeSize(x->right);
    }
}

/** @brief 获取节点大小 @param x 目标 @return 子树大小 */
int RedBlackTree2::nodeSize(Node* x) const
{
    return (x == m_nil) ? 0 : x->size;
}

/** @brief 递归删除 @param node 当前节点 */
void RedBlackTree2::destroyTree(Node* node)
{
    if (node == m_nil) return;
    QStack<Node*> stack;
    stack.push(node);
    while (!stack.isEmpty()) {
        Node* n = stack.pop();
        if (n->left != m_nil) stack.push(n->left);
        if (n->right != m_nil) stack.push(n->right);
        delete n;
    }
}

/** @brief 递归中序 @param node 当前 @param result 结果 */
void RedBlackTree2::inOrderHelper(Node* node,
                                   QVector<QPair<double, QVariant>>& result) const
{
    if (node == m_nil) return;
    inOrderHelper(node->left, result);
    result.append({node->key, node->value});
    inOrderHelper(node->right, result);
}

/** @brief 递归高度 @param node 当前 @return 高度 */
int RedBlackTree2::heightHelper(Node* node) const
{
    if (node == m_nil) return 0;
    return 1 + qMax(heightHelper(node->left), heightHelper(node->right));
}
