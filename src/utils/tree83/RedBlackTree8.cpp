#include "RedBlackTree8.h"
#include <QElapsedTimer>
#include <algorithm>

/**
 * @brief 红黑树节点内部结构
 */
struct RedBlackTree8::Node {
    int key;               ///< 节点键值
    QVariant value;        ///< 节点存储的数据
    bool isRed = true;     ///< 颜色标记：true=红色, false=黑色
    Node* left = nullptr;   ///< 左子节点
    Node* right = nullptr;  ///< 右子节点
    Node* parent = nullptr; ///< 父节点

    explicit Node(int k, const QVariant& v) : key(k), value(v) {}
};

/**
 * @brief 构造函数，初始化红黑树
 * @param parent 父QObject对象指针
 */
RedBlackTree8::RedBlackTree8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 插入键值对到红黑树
 *
 * 标准BST插入后通过重新着色和旋转恢复红黑性质：
 * 1. 新节点着红色插入
 * 2. 若父节点为红色则违反性质，需要修复
 * 3. 根据叔节点颜色选择重着色或旋转
 *
 * @param key 插入的键
 * @param value 关联的值
 */
void RedBlackTree8::insert(int key, const QVariant& value)
{
    QElapsedTimer timer;
    timer.start();

    /// 标准BST插入
    Node* newNode = new Node(key, value);
    if (!m_root) {
        m_root = newNode;
        m_root->isRed = false;  ///< 根节点必须为黑色
    } else {
        Node* current = m_root;
        Node* parent = nullptr;
        while (current) {
            parent = current;
            if (key < current->key) {
                current = current->left;
            } else if (key > current->key) {
                current = current->right;
            } else {
                current->value = value;  ///< 更新已有键的值
                delete newNode;
                return;
            }
        }
        newNode->parent = parent;
        if (key < parent->key) parent->left = newNode;
        else parent->right = newNode;

        /// 修复红黑性质
        Node* node = newNode;
        while (node->parent && node->parent->isRed) {
            Node* gp = node->parent->parent;
            if (!gp) break;

            if (node->parent == gp->left) {
                Node* uncle = gp->right;
                if (uncle && uncle->isRed) {
                    /// 情况1：叔节点为红色，重着色
                    node->parent->isRed = false;
                    uncle->isRed = false;
                    gp->isRed = true;
                    node = gp;
                } else {
                    if (node == node->parent->right) {
                        /// 情况2：左旋转换为情况3
                        node = node->parent;
                        rotateLeft(node);
                    }
                    /// 情况3：右旋+重着色
                    node->parent->isRed = false;
                    gp->isRed = true;
                    rotateRight(gp);
                }
            } else {
                Node* uncle = gp->left;
                if (uncle && uncle->isRed) {
                    node->parent->isRed = false;
                    uncle->isRed = false;
                    gp->isRed = true;
                    node = gp;
                } else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rotateRight(node);
                    }
                    node->parent->isRed = false;
                    gp->isRed = true;
                    rotateLeft(gp);
                }
            }
        }
        m_root->isRed = false;
    }

    /// 更新统计信息
    m_stats.totalInsertions++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions;
    m_stats.avgProcessingTimeMs = m_timeSum / totalOps;
}

/**
 * @brief 删除指定键的节点
 * @param key 要删除的键
 * @return true成功删除，false键不存在
 */
bool RedBlackTree8::remove(int key)
{
    QElapsedTimer timer;
    timer.start();

    Node* node = findNode(m_root, key);
    if (!node) return false;

    deleteNode(node);

    m_stats.totalDeletions++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalInsertions + m_stats.totalDeletions;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);
    return true;
}

/**
 * @brief 中序遍历返回所有键值对
 * @return 按键排序的键值对列表
 */
QVector<QPair<int, QVariant>> RedBlackTree8::inOrderTraversal() const
{
    QVector<QPair<int, QVariant>> result;
    inOrderHelper(m_root, result);
    return result;
}

/**
 * @brief 获取当前统计数据
 * @return 包含插入/删除次数和平均耗时的Stats结构
 */
RedBlackTree8::Stats RedBlackTree8::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void RedBlackTree8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/// 左旋操作
void RedBlackTree8::rotateLeft(Node* x)
{
    Node* y = x->right;
    x->right = y->left;
    if (y->left) y->left->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->left) x->parent->left = y;
    else x->parent->right = y;
    y->left = x;
    x->parent = y;
    emit treeRotated(x->key, QStringLiteral("left"));
}

/// 右旋操作
void RedBlackTree8::rotateRight(Node* x)
{
    Node* y = x->left;
    x->left = y->right;
    if (y->right) y->right->parent = x;
    y->parent = x->parent;
    if (!x->parent) m_root = y;
    else if (x == x->parent->right) x->parent->right = y;
    else x->parent->left = y;
    y->right = x;
    x->parent = y;
    emit treeRotated(x->key, QStringLiteral("right"));
}

/// 查找节点辅助函数
RedBlackTree8::Node* RedBlackTree8::findNode(Node* root, int key)
{
    while (root) {
        if (key < root->key) root = root->left;
        else if (key > root->key) root = root->right;
        else return root;
    }
    return nullptr;
}

/// 删除节点辅助函数
void RedBlackTree8::deleteNode(Node* node)
{
    if (!node->left || !node->right) {
        Node* child = node->left ? node->left : node->right;
        transplant(node, child);
        delete node;
    } else {
        Node* successor = node->right;
        while (successor->left) successor = successor->left;
        node->key = successor->key;
        node->value = successor->value;
        Node* child = successor->right;
        transplant(successor, child);
        delete successor;
    }
}

/// 子树替换辅助函数
void RedBlackTree8::transplant(Node* u, Node* v)
{
    if (!u->parent) m_root = v;
    else if (u == u->parent->left) u->parent->left = v;
    else u->parent->right = v;
    if (v) v->parent = u->parent;
}

/// 中序遍历辅助函数
void RedBlackTree8::inOrderHelper(Node* node, QVector<QPair<int, QVariant>>& result) const
{
    if (!node) return;
    inOrderHelper(node->left, result);
    result.append(qMakePair(node->key, node->value));
    inOrderHelper(node->right, result);
}
