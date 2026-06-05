/**
 * @file RedBlackTree3Query.cpp
 * @brief 红黑树顺序统计查询与遍历方法实现
 *
 * 从 RedBlackTree3.cpp 拆分而来, 包含:
 * selectKth(按秩选择), rank(排名查询), predecessor(前驱),
 * successor(后继), rangeCount(区间计数), clear(清空),
 * inOrderTraversal(有序遍历)等只读查询和树维护方法。
 *
 * 所有查询方法依赖节点中的subtreeSize字段实现O(log n)性能。
 */

#include "utils/tree28/RedBlackTree3.h"

#include <vector>

/**
 * @brief 按秩选择 — 查找第k小元素的键
 *
 * 利用subtreeSize快速定位:
 * - 左子树大小 leftSize = x->left->subtreeSize
 * - 若 k <= leftSize, 目标在左子树, 递归进入
 * - 若 k == leftSize + 1, 当前节点即为第k小
 * - 否则目标在右子树, k减去(leftSize + 1)后递归进入
 *
 * 时间复杂度: O(log n)
 *
 * @param k 排名(1-based, 即1=最小元素)
 * @return 第k小元素的键, k越界返回0.0
 */
double RedBlackTree3::selectKth(int k) const
{
    if (k < 1 || k > m_size) {
        return 0.0;
    }
    Node* result = selectKthNode(m_root, k);
    return (result != m_nil) ? result->key : 0.0;
}

/**
 * @brief 排名查询 — 查找key在树中的排名
 *
 * 从根节点向下搜索key, 同时累加排名:
 * - key < 当前: 进入左子树, 排名不变
 * - key > 当前: 排名加上(左子树大小+1), 进入右子树
 * - key == 当前: 返回 (已累加排名 + 左子树大小 + 1)
 *
 * 时间复杂度: O(log n)
 *
 * @param key 要查询排名的键
 * @return 排名(1-based), -1表示键不存在
 */
int RedBlackTree3::rank(double key) const
{
    Node* x = m_root;
    int r = 0;
    while (x != m_nil) {
        int leftSize = x->left->subtreeSize;
        if (key < x->key) {
            x = x->left;
        } else if (key > x->key) {
            r += leftSize + 1;
            x = x->right;
        } else {
            return r + leftSize + 1;
        }
    }
    return -1;
}

/**
 * @brief 前驱 — 查找严格小于key的最大键
 *
 * 沿树向下搜索:
 * - 当前节点key < 目标key → 该节点可能是前驱, 记录并走向右子树(寻找更大但仍小于key的)
 * - 当前节点key >= 目标key → 走向左子树(寻找更小的)
 * 最终记录的就是严格小于key的最大值。
 *
 * 时间复杂度: O(log n)
 *
 * @param key 目标键
 * @return 前驱键, 若无前驱(所有键都>=key)返回NaN
 */
double RedBlackTree3::predecessor(double key) const
{
    Node* result = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        if (x->key < key) {
            result = x;
            x = x->right;
        } else {
            x = x->left;
        }
    }
    return (result != m_nil) ? result->key : qQNaN();
}

/**
 * @brief 后继 — 查找严格大于key的最小键
 *
 * 沿树向下搜索:
 * - 当前节点key > 目标key → 该节点可能是后继, 记录并走向左子树(寻找更小但仍大于key的)
 * - 当前节点key <= 目标key → 走向右子树(寻找更大的)
 * 最终记录的就是严格大于key的最小值。
 *
 * 时间复杂度: O(log n)
 *
 * @param key 目标键
 * @return 后继键, 若无后继(所有键都<=key)返回NaN
 */
double RedBlackTree3::successor(double key) const
{
    Node* result = m_nil;
    Node* x = m_root;
    while (x != m_nil) {
        if (x->key > key) {
            result = x;
            x = x->left;
        } else {
            x = x->right;
        }
    }
    return (result != m_nil) ? result->key : qQNaN();
}

/**
 * @brief 范围计数 — 统计[low, high]区间内的元素数
 *
 * 使用中序遍历, 利用BST有序性:
 * - 跳过key < low的节点
 * - 计数key在[low, high]内的节点
 * - 遇到key > high时提前终止遍历
 *
 * 时间复杂度: O(k + log n), k为区间内元素数
 *
 * @param low 区间下界(包含)
 * @param high 区间上界(包含)
 * @return 区间内元素数
 */
int RedBlackTree3::rangeCount(double low, double high) const
{
    if (low > high || m_root == m_nil) {
        return 0;
    }
    int count = 0;
    std::vector<Node*> stack;
    Node* cur = m_root;
    while (cur != m_nil || !stack.empty()) {
        while (cur != m_nil) {
            stack.push_back(cur);
            cur = cur->left;
        }
        cur = stack.back();
        stack.pop_back();
        if (cur->key >= low && cur->key <= high) {
            ++count;
        }
        if (cur->key > high) break;
        cur = cur->right;
    }
    return count;
}

/**
 * @brief 清空整棵树
 * 递归后序遍历删除所有节点, 根重新指向NIL哨兵, 大小归零。
 * 哨兵节点m_nil不被删除(由析构函数负责)。
 */
void RedBlackTree3::clear()
{
    destroyTree(m_root);
    m_root = m_nil;
    m_size = 0;
}

/**
 * @brief 有序遍历 — 中序遍历返回所有键值对
 *
 * 使用栈模拟递归的中序遍历(左-根-右),
 * 返回按键升序排列的(key, value)列表。
 * 时间复杂度: O(n)
 *
 * @return (键, 值)列表, 按键升序排列
 */
QVector<QPair<double,int>> RedBlackTree3::inOrderTraversal() const
{
    QVector<QPair<double,int>> result;
    std::vector<Node*> stack;
    Node* cur = m_root;
    while (cur != m_nil || !stack.empty()) {
        while (cur != m_nil) {
            stack.push_back(cur);
            cur = cur->left;
        }
        cur = stack.back();
        stack.pop_back();
        result.append({cur->key, cur->value});
        cur = cur->right;
    }
    return result;
}
