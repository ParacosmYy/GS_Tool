/**
 * @file IntervalTree5.cpp
 * @brief 增强区间树实现
 *
 * 基于红黑树(或简化平衡策略)的增强区间树，
 * 支持动态插入删除和高效区间重叠查询。
 * 每个节点存储区间[low,high]和最大端点值(maxHigh)，
 * 利用maxHigh剪枝加速查询。
 */

#include "utils/tree76/IntervalTree5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 区间树节点结构
 */
struct IntervalTree5::Node {
    double low, high;  /* 区间端点 */
    int data;          /* 关联数据 */
    double maxHigh;    /* 子树中最大的high值(增强信息) */
    Node* left;
    Node* right;
    bool red;          /* 红黑标记 */

    Node(double l, double h, int d)
        : low(l), high(h), data(d), maxHigh(h), left(nullptr), right(nullptr), red(true) {}
};

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
IntervalTree5::IntervalTree5(QObject* parent)
    : QObject(parent), m_root(nullptr)
{
}

/**
 * @brief 更新节点的maxHigh值
 * @param node 目标节点
 *
 * maxHigh = max(node.high, left.maxHigh, right.maxHigh)
 */
static void updateMaxHigh(IntervalTree5::Node* node)
{
    if (!node) return;
    node->maxHigh = node->high;
    if (node->left && node->left->maxHigh > node->maxHigh) {
        node->maxHigh = node->left->maxHigh;
    }
    if (node->right && node->right->maxHigh > node->maxHigh) {
        node->maxHigh = node->right->maxHigh;
    }
}

/**
 * @brief 插入区间[low, high]并关联数据
 * @param low 区间下界
 * @param high 区间上界
 * @param data 关联数据标识
 *
 * 使用标准BST插入后更新maxHigh增强信息。
 * 简化实现不执行红黑树平衡。
 */
void IntervalTree5::insert(double low, double high, int data)
{
    QElapsedTimer timer;
    timer.start();

    Node* newNode = new Node(low, high, data);

    if (!m_root) {
        m_root = newNode;
        m_root->red = false;

        qint64 elapsed = timer.elapsed();
        m_stats.totalIntervals++;
        m_timeSum += elapsed;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalIntervals;

        emit intervalInserted(low, high);
        return;
    }

    /* BST插入 */
    Node* current = m_root;
    Node* parent = nullptr;

    while (current) {
        parent = current;
        if (low < current->low) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    if (low < parent->low) {
        parent->left = newNode;
    } else {
        parent->right = newNode;
    }

    /* 更新路径上所有节点的maxHigh */
    current = m_root;
    while (current) {
        updateMaxHigh(current);
        if (low < current->low) {
            current = current->left;
        } else if (low > current->low) {
            current = current->right;
        } else {
            break;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalIntervals++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalIntervals;

    emit intervalInserted(low, high);
}

/**
 * @brief 删除指定区间
 * @param low 区间下界
 * @param high 区间上界
 * @return true如果找到并删除
 *
 * 查找并删除匹配(low, high)的节点。
 * 删除后更新maxHigh信息。
 */
bool IntervalTree5::remove(double low, double high)
{
    Node* current = m_root;
    Node* parent = nullptr;
    bool isLeft = false;

    /* 查找目标节点 */
    while (current) {
        if (low == current->low && high == current->high) {
            break;
        }
        parent = current;
        if (low < current->low) {
            current = current->left;
            isLeft = true;
        } else {
            current = current->right;
            isLeft = false;
        }
    }

    if (!current) return false;

    /* 删除节点 */
    Node* replacement = nullptr;
    if (!current->left && !current->right) {
        replacement = nullptr;
    } else if (!current->left) {
        replacement = current->right;
    } else if (!current->right) {
        replacement = current->left;
    } else {
        /* 找中序后继 */
        Node* succ = current->right;
        Node* succParent = current;
        while (succ->left) {
            succParent = succ;
            succ = succ->left;
        }
        current->low = succ->low;
        current->high = succ->high;
        current->data = succ->data;
        if (succParent == current) {
            succParent->right = succ->right;
        } else {
            succParent->left = succ->right;
        }
        updateMaxHigh(current);
        delete succ;
        return true;
    }

    if (!parent) {
        m_root = replacement;
    } else if (isLeft) {
        parent->left = replacement;
    } else {
        parent->right = replacement;
    }

    updateMaxHigh(parent);
    delete current;
    return true;
}

/**
 * @brief 查询与点value重叠的所有区间
 * @param value 查询点
 * @return 重叠区间列表[(low,high), data]
 *
 * 利用maxHigh剪枝: 若子树maxHigh < value，则跳过。
 */
QVector<QPair<QPair<double, double>, int>> IntervalTree5::queryPoint(double value) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double, double>, int>> result;

    /* 迭代DFS查询 */
    QVector<Node*> stack;
    if (m_root) stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* node = stack.takeLast();
        if (!node) continue;

        /* 剪枝: 子树maxHigh < value，不可能有重叠 */
        if (node->maxHigh < value) continue;

        /* 检查当前区间是否包含value */
        if (node->low <= value && node->high >= value) {
            result.append({{node->low, node->high}, node->data});
        }

        /* 继续搜索左右子树 */
        if (node->left) stack.append(node->left);
        if (node->right) stack.append(node->right);
    }

    qint64 elapsed = timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_timeSum += elapsed;
    const_cast<IntervalTree5*>(this)->m_stats.avgProcessingTimeMs =
        const_cast<IntervalTree5*>(this)->m_timeSum / m_stats.totalIntervals;

    return result;
}

/**
 * @brief 查询与区间[low,high]重叠的所有区间
 * @param low 查询区间下界
 * @param high 查询区间上界
 * @return 重叠区间列表
 *
 * 两个区间重叠条件: a.low <= b.high && b.low <= a.high
 * 利用maxHigh剪枝加速。
 */
QVector<QPair<QPair<double, double>, int>> IntervalTree5::queryInterval(double low, double high) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QPair<double, double>, int>> result;

    QVector<Node*> stack;
    if (m_root) stack.append(m_root);

    while (!stack.isEmpty()) {
        Node* node = stack.takeLast();
        if (!node) continue;

        /* 剪枝: 子树maxHigh < low，无重叠可能 */
        if (node->maxHigh < low) continue;

        /* 检查重叠: node.low <= high && low <= node.high */
        if (node->low <= high && low <= node->high) {
            result.append({{node->low, node->high}, node->data});
        }

        if (node->left) stack.append(node->left);
        if (node->right) stack.append(node->right);
    }

    qint64 elapsed = timer.elapsed();
    const_cast<IntervalTree5*>(this)->m_stats.totalQueries++;
    const_cast<IntervalTree5*>(this)->m_timeSum += elapsed;

    return result;
}

/**
 * @brief 获取树中区间总数
 * @return 区间数量
 */
int IntervalTree5::count() const
{
    return m_stats.totalIntervals;
}

/**
 * @brief 重置统计信息
 */
void IntervalTree5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
