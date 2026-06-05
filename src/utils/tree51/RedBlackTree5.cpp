/**
 * @file RedBlackTree5.cpp
 * @brief 红黑树(Red-Black Tree)实现
 *
 * 实现经典的红黑树数据结构，支持插入、删除、查找和中序遍历。
 * 红黑树通过着色规则和旋转操作保证树的高度平衡，确保
 * 所有操作在最坏情况下的时间复杂度为O(log n)。使用QElapsedTimer计时。
 */

#include "utils/tree51/RedBlackTree5.h"

#include <QElapsedTimer>
#include <algorithm>

/**
 * @class RedBlackTree5
 * @brief 红黑树，支持子树大小(subsize)统计
 *
 * 红黑性质：
 * 1. 每个节点为红色或黑色
 * 2. 根节点为黑色
 * 3. 叶节点(NIL)为黑色
 * 4. 红色节点的子节点必须为黑色
 * 5. 从任一节点到其所有叶节点的路径包含相同数量的黑色节点
 */

/**
 * @brief 构造函数，创建NIL哨兵节点
 * @param parent 父QObject指针
 */
RedBlackTree5::RedBlackTree5(QObject* parent)
    : QObject(parent)
{
    /* 创建NIL哨兵节点（黑色，所有叶节点指向它） */
    m_nil = new Node{};
    m_nil->color = Black;
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_nil->subsize = 0;
    m_root = m_nil;
}

/**
 * @brief 析构函数，释放所有节点
 */
RedBlackTree5::~RedBlackTree5()
{
    destroyTree(m_root);
    delete m_nil;
}

/**
 * @brief 插入键值对
 *
 * 标准BST插入后执行红黑树修复（insertFixup），
 * 通过着色和旋转恢复红黑性质。
 *
 * @param key 查找键
 * @param value 存储值
 */
void RedBlackTree5::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    /* 创建新节点（初始为红色） */
    Node* z = new Node{};
    z->key = key;
    z->value = value;
    z->color = Red;
    z->left = m_nil;
    z->right = m_nil;
    z->parent = m_nil;
    z->subsize = 1;

    /* BST插入：找到插入位置 */
    Node* y = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        y = x;
        x->subsize++;  /* 更新子树大小 */
        if (key < x->key) {
            x = x->left;
        } else {
            x = x->right;
        }
    }

    z->parent = y;
    if (y == m_nil) {
        m_root = z;
    } else if (key < y->key) {
        y->left = z;
    } else {
        y->right = z;
    }

    /* 修复红黑性质 */
    insertFixup(z);
    m_size++;

    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;

    emit treeModified(m_size);
}

/**
 * @brief 删除指定键的节点
 * @param key 要删除的键
 * @return 删除成功返回true，键不存在返回false
 */
bool RedBlackTree5::remove(double key)
{
    QElapsedTimer timer;
    timer.start();

    /* 查找目标节点 */
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
        m_stats.totalDeletions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
            ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;
        return false;
    }

    /* 沿路径减去子树大小计数 */
    Node* path = m_root;
    while (path != m_nil) {
        path->subsize--;
        if (key < path->key) path = path->left;
        else if (key > path->key) path = path->right;
        else break;
    }

    Node* y = z;
    Color yOrigColor = y->color;
    Node* x = nullptr;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = minimum(z->right);
        yOrigColor = y->color;
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
        y->subsize = z->subsize - 1;
    }

    if (yOrigColor == Black) {
        deleteFixup(x);
    }

    delete z;
    m_size--;

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries) : 0.0;

    emit treeModified(m_size);
    return true;
}

/**
 * @brief 查找指定键对应的值
 * @param key 查找键
 * @return 对应的值，键不存在返回0
 */
int RedBlackTree5::find(double key) const
{
    Node* x = m_root;
    while (x != m_nil) {
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            x = x->right;
        } else {
            return x->value;
        }
    }
    return 0;
}

/**
 * @brief 清空整棵树
 */
void RedBlackTree5::clear()
{
    destroyTree(m_root);
    m_root = m_nil;
    m_size = 0;
}

/**
 * @brief 中序遍历返回所有键值对（按键排序）
 * @return 键值对列表，按key升序排列
 */
QVector<QPair<double,int>> RedBlackTree5::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    result.reserve(m_size);

    /* 使用栈实现迭代式中序遍历，避免递归栈溢出 */
    QVector<Node*> stack;
    Node* current = m_root;
    while (current != m_nil || !stack.isEmpty()) {
        while (current != m_nil) {
            stack.append(current);
            current = current->left;
        }
        current = stack.takeLast();
        result.append({current->key, current->value});
        current = current->right;
    }
    return result;
}

/**
 * @brief 重置统计数据
 */
void RedBlackTree5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 左旋操作
 *
 * 以x为支点执行左旋：x的右子y上升为父节点，
 * y的左子变为x的右子。同时更新subsize。
 */
void RedBlackTree5::leftRotate(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left != m_nil) y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == m_nil) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    /* 更新子树大小 */
    y->subsize = x->subsize;
    x->subsize = x->left->subsize + x->right->subsize + 1;
}

/**
 * @brief 右旋操作
 *
 * 以y为支点执行右旋：y的左子x上升为父节点，
 * x的右子变为y的左子。同时更新subsize。
 */
void RedBlackTree5::rightRotate(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == m_nil) m_root = x;
    else if (y == y->parent->left) y->parent->left = x;
    else y->parent->right = x;
    x->right = y;
    y->parent = x;
    /* 更新子树大小 */
    x->subsize = y->subsize;
    y->subsize = y->left->subsize + y->right->subsize + 1;
}

/**
 * @brief 插入后修复红黑性质
 *
 * 新插入的节点为红色，可能违反性质4（红色节点子节点必须为黑色）。
 * 根据叔节点颜色分三种情况处理，通过重着色和旋转恢复性质。
 */
void RedBlackTree5::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* uncle = z->parent->parent->right;
            if (uncle->color == Red) {
                /* 情况1：叔节点为红色 -> 重着色 */
                z->parent->color = Black;
                uncle->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    /* 情况2：z为右子 -> 左旋转为情况3 */
                    z = z->parent;
                    leftRotate(z);
                }
                /* 情况3：z为左子 -> 重着色+右旋 */
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rightRotate(z->parent->parent);
            }
        } else {
            /* 镜像情况 */
            Node* uncle = z->parent->parent->left;
            if (uncle->color == Red) {
                z->parent->color = Black;
                uncle->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                leftRotate(z->parent->parent);
            }
        }
    }
    m_root->color = Black;
}

/**
 * @brief 删除后修复红黑性质
 *
 * 删除黑色节点可能导致黑色节点数量不足（违反性质5）。
 * 通过旋转和重着色将"多余的黑色"向上传递直到消除。
 */
void RedBlackTree5::deleteFixup(Node* x)
{
    while (x != m_root && x->color == Black) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                leftRotate(x->parent);
                w = x->parent->right;
            }
            if (w->left->color == Black && w->right->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->right->color == Black) {
                    w->left->color = Black;
                    w->color = Red;
                    rightRotate(w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->right->color = Black;
                leftRotate(x->parent);
                x = m_root;
            }
        } else {
            Node* w = x->parent->left;
            if (w->color == Red) {
                w->color = Black;
                x->parent->color = Red;
                rightRotate(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == Black && w->left->color == Black) {
                w->color = Red;
                x = x->parent;
            } else {
                if (w->left->color == Black) {
                    w->right->color = Black;
                    w->color = Red;
                    leftRotate(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = Black;
                w->left->color = Black;
                rightRotate(x->parent);
                x = m_root;
            }
        }
    }
    x->color = Black;
}

/**
 * @brief 子树替换：用v替换u的位置
 */
void RedBlackTree5::transplant(Node* u, Node* v)
{
    if (u->parent == m_nil) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    v->parent = u->parent;
}

/**
 * @brief 查找子树中的最小节点
 */
RedBlackTree5::Node* RedBlackTree5::minimum(Node* x) const
{
    while (x->left != m_nil) x = x->left;
    return x;
}

/**
 * @brief 递归销毁子树
 */
void RedBlackTree5::destroyTree(Node* n)
{
    if (n != m_nil) {
        destroyTree(n->left);
        destroyTree(n->right);
        delete n;
    }
}
