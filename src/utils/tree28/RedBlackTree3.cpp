/**
 * @file RedBlackTree3.cpp
 * @brief 顺序统计增强红黑树核心实现
 *
 * 在标准红黑树基础上, 每个节点维护subtreeSize字段,
 * 支持O(log n)的按秩选择(select)和排名查询(rank)。
 * 本文件包含: 构造/析构/插入/删除/旋转/修复/移植等核心操作。
 * 查询方法(select/rank/predecessor/successor/rangeCount/traversal)见RedBlackTree3Query.cpp。
 */

#include "utils/tree28/RedBlackTree3.h"

#include <QElapsedTimer>

#include <algorithm>
#include <vector>

/**
 * @brief 构造函数
 * 初始化NIL哨兵节点(黑色), 所有子指针自环, 根指向NIL
 * @param parent 父对象
 */
RedBlackTree3::RedBlackTree3(QObject* parent)
    : QObject(parent)
{
    m_nil = new Node{};
    m_nil->color = Black;
    m_nil->subtreeSize = 0;
    m_nil->left = m_nil;
    m_nil->right = m_nil;
    m_nil->parent = m_nil;
    m_root = m_nil;
}

/** @brief 析构函数 — 递归销毁所有节点(后序遍历), 哨兵节点最后删除 */
RedBlackTree3::~RedBlackTree3()
{
    destroyTree(m_root);
    delete m_nil;
}

/**
 * @brief 插入键值对
 *
 * 标准RB插入流程:
 * 1. BST式向下搜索插入位置, 沿途递增每个节点的subtreeSize
 * 2. 新节点着红色(Red), 子节点指向NIL哨兵, subtreeSize=1
 * 3. 将新节点挂接到父节点的左/右子指针
 * 4. 调用insertFixup修复红黑性质(处理连续红色节点)
 *
 * 时间复杂度: O(log n)
 *
 * @param key 键(允许重复, 相同key按插入顺序排列在右子树)
 * @param value 关联值
 */
void RedBlackTree3::insert(double key, int value)
{
    QElapsedTimer timer;
    timer.start();

    Node* z = new Node{};
    z->key = key;
    z->value = value;
    z->color = Red;
    z->left = m_nil;
    z->right = m_nil;
    z->parent = m_nil;
    z->subtreeSize = 1;

    Node* y = m_nil;
    Node* x = m_root;

    /* BST向下搜索插入位置, 沿途递增subtreeSize */
    while (x != m_nil) {
        y = x;
        x->subtreeSize += 1;
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

    insertFixup(z);
    ++m_size;

    /* 更新统计 */
    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit inserted(key);
}

/**
 * @brief 删除键
 *
 * 标准RB删除流程:
 * 1. BST搜索定位目标节点z
 * 2. 若z至多一个子节点, 直接用子节点替换(transplant)
 * 3. 若z有两个子节点, 找后继y替换z, 用y的右子节点填补y的位置
 * 4. 删除后从x的父节点向上修复subtreeSize
 * 5. 若删除的是黑色节点, 调用deleteFixup恢复黑高平衡
 *
 * 时间复杂度: O(log n)
 *
 * @param key 要删除的键
 * @return true=删除成功, false=键不存在
 */
bool RedBlackTree3::remove(double key)
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
        return false;
    }

    Node* y = z;
    Node* x = nullptr;
    Color yOrigColor = y->color;

    if (z->left == m_nil) {
        x = z->right;
        transplant(z, z->right);
    } else if (z->right == m_nil) {
        x = z->left;
        transplant(z, z->left);
    } else {
        y = treeMinimum(z->right);
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
    }

    /* 修复从x向上到根的subtreeSize */
    Node* cur = x->parent;
    while (cur != m_nil) {
        updateSize(cur);
        cur = cur->parent;
    }

    if (yOrigColor == Black) {
        deleteFixup(x);
    }

    delete z;
    --m_size;

    /* 更新统计 */
    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit removed(key);
    return true;
}

/**
 * @brief 查找键对应的值
 * 标准BST搜索, 沿路径比较key大小决定走向
 * @param key 要查找的键
 * @return 关联值, -1表示键不存在
 */
int RedBlackTree3::find(double key) const
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
    return -1;
}

/**
 * @brief 左旋 — 以x为支点
 *
 * 将x的右子节点y提升为新的子树根, x降为y的左子节点。
 * y的左子树转移为x的右子树。旋转后更新x和y的subtreeSize。
 * 这是RB树平衡操作的基础。
 *
 * @param x 旋转支点(旋转后成为左子节点)
 */
void RedBlackTree3::leftRotate(Node* x)
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
    updateSize(x);
    updateSize(y);
}

/**
 * @brief 右旋 — 以y为支点
 *
 * 将y的左子节点x提升为新的子树根, y降为x的右子节点。
 * 镜像对称于leftRotate, 用于处理对称情况。
 *
 * @param y 旋转支点(旋转后成为右子节点)
 */
void RedBlackTree3::rightRotate(Node* y)
{
    Node* x = y->left;
    y->left = x->right;
    if (x->right != m_nil) {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == m_nil) {
        m_root = x;
    } else if (y == y->parent->right) {
        y->parent->right = x;
    } else {
        y->parent->left = x;
    }
    x->right = y;
    y->parent = x;
    updateSize(y);
    updateSize(x);
}

/**
 * @brief 插入修复 — 维护红黑性质
 *
 * 新插入的红色节点可能违反性质4"红色节点不能有红色子节点"。
 * 根据叔节点颜色分三种情况(以父是祖父左子为例):
 * Case 1: 叔叔红色 → 将父/叔变黑, 祖父变红, z上移到祖父
 * Case 2: 叔叔黑色, z是右子 → 左旋父节点, 转化为Case 3
 * Case 3: 叔叔黑色, z是左子 → 父变黑, 祖父变红, 右旋祖父
 * 父是祖父右子时为镜像对称。
 *
 * @param z 新插入的节点
 */
void RedBlackTree3::insertFixup(Node* z)
{
    while (z->parent->color == Red) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
                z->parent->parent->color = Red;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(z);
                }
                z->parent->color = Black;
                z->parent->parent->color = Red;
                rightRotate(z->parent->parent);
            }
        } else {
            /* 镜像: 父节点是祖父的右子 */
            Node* y = z->parent->parent->left;
            if (y->color == Red) {
                z->parent->color = Black;
                y->color = Black;
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
 * @brief 删除修复 — 维护红黑性质
 *
 * 删除黑色节点可能导致经过x的路径黑高减少1。
 * 以x是父的左子为例(右子为镜像):
 * Case 1: 兄弟w红色 → 旋转使w变黑, 转化为Case 2/3/4
 * Case 2: 兄弟w黑色, 两个侄子都黑 → w变红, x上移到父
 * Case 3: 兄弟w黑色, 远侄子黑近侄子红 → 旋转w, 转化为Case 4
 * Case 4: 兄弟w黑色, 远侄子红 → 旋转父+重着色, 修复完成
 *
 * @param x 替代被删除节点的节点(可能为NIL哨兵)
 */
void RedBlackTree3::deleteFixup(Node* x)
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

/** @brief 移植 — 用子树v替换子树u, 将u的父节点直接指向v @param u 被替换的子树根 @param v 替换的子树根 */
void RedBlackTree3::transplant(Node* u, Node* v)
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

/** @brief 查找子树最小节点 — 沿左子链一直向下直到最左叶节点 @param x 子树根 @return 最小节点 */
RedBlackTree3::Node* RedBlackTree3::treeMinimum(Node* x) const
{
    while (x->left != m_nil) {
        x = x->left;
    }
    return x;
}

/** @brief 更新节点的subtreeSize = 1 + left->subtreeSize + right->subtreeSize @param n 需要更新的节点 */
void RedBlackTree3::updateSize(Node* n)
{
    if (n != m_nil) {
        n->subtreeSize = 1 + n->left->subtreeSize + n->right->subtreeSize;
    }
}

/** @brief 按秩选择节点(递归) — 利用子树大小快速定位第k小节点 @param x 当前子树根 @param k 目标排名(1-based) @return 目标节点 */
RedBlackTree3::Node* RedBlackTree3::selectKthNode(Node* x, int k) const
{
    if (x == m_nil) return m_nil;
    int leftSize = x->left->subtreeSize;
    if (k <= leftSize) {
        return selectKthNode(x->left, k);
    } else if (k == leftSize + 1) {
        return x;
    } else {
        return selectKthNode(x->right, k - leftSize - 1);
    }
}

/** @brief 计算节点的排名(内部) — 从节点向上走到根, 累加左兄弟的subtreeSize @param x 目标节点 @return 排名(1-based) */
int RedBlackTree3::rankOf(Node* x) const
{
    int r = x->left->subtreeSize + 1;
    Node* y = x;
    while (y != m_root) {
        if (y == y->parent->right) {
            r += y->parent->left->subtreeSize + 1;
        }
        y = y->parent;
    }
    return r;
}

/** @brief 递归销毁子树 — 后序遍历删除所有节点(不包括NIL哨兵) @param n 子树根 */
void RedBlackTree3::destroyTree(Node* n)
{
    if (n != m_nil) {
        destroyTree(n->left);
        destroyTree(n->right);
        delete n;
    }
}

/** @brief 重置统计计数器 — 将插入/删除/查询计数和平均时间归零 */
void RedBlackTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
