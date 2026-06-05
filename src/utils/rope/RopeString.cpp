/**
 * @file RopeString.cpp
 * @brief Rope数据结构实现
 */

#include "RopeString.h"
#include <QElapsedTimer>
#include <algorithm>

RopeString::RopeString(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_timeSum(0.0)
    , m_totalOps(0)
{
}

RopeString::RopeString(const QString& text, QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_totalOps(0)
{
    if (text.isEmpty()) {
        m_root = nullptr;
    } else {
        m_root = buildRope(text, 0, text.length());
    }
}

RopeString::~RopeString()
{
    destroyNode(m_root);
}

void RopeString::insert(int pos, const QString& str)
{
    QElapsedTimer timer;
    timer.start();

    if (str.isEmpty()) return;
    int len = length();
    if (pos < 0) pos = 0;
    if (pos > len) pos = len;

    m_root = insertNode(m_root, pos, str);

    /* 分裂过长叶子 */
    m_root = splitLeaf(m_root);

    /* 统计更新 */
    m_stats.totalInserts++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit structureChanged();
}

void RopeString::remove(int pos, int len)
{
    QElapsedTimer timer;
    timer.start();

    if (len <= 0 || m_root == nullptr) return;
    int totalLen = length();
    if (pos < 0) pos = 0;
    if (pos >= totalLen) return;
    if (pos + len > totalLen) len = totalLen - pos;

    m_root = removeRange(m_root, pos, len);

    /* 统计更新 */
    m_stats.totalRemoves++;
    m_timeSum += timer.elapsed();
    m_totalOps++;
    m_stats.avgProcessingTimeMs =
        (m_totalOps > 0) ? m_timeSum / m_totalOps : 0.0;

    emit structureChanged();
}

QString RopeString::substring(int pos, int len) const
{
    if (m_root == nullptr || len <= 0 || pos < 0) return {};

    int totalLen = length();
    if (pos >= totalLen) return {};
    if (pos + len > totalLen) len = totalLen - pos;

    QString result;
    result.reserve(len);
    int skipped = 0;
    substringHelper(m_root, pos, len, skipped, result);
    return result;
}

int RopeString::length() const
{
    if (m_root == nullptr) return 0;

    /* 对叶子节点返回数据长度 */
    if (m_root->isLeaf()) return m_root->data.length();

    /* 对内部节点递归计算右子树长度+weight */
    int total = m_root->weight;
    Node* right = m_root->right;
    while (right != nullptr) {
        if (right->isLeaf()) {
            total += right->data.length();
            break;
        }
        total += right->weight;
        right = right->right;
    }

    /* 实际上weight应该是左子树的总长度
     * 对于内部节点: 总长度 = 左子树长度 + 右子树长度
     * weight存储的是左子树长度 */
    return computeTotalLength(m_root);
}

QString RopeString::toString() const
{
    if (m_root == nullptr) return {};
    QString result;
    toStringHelper(m_root, result);
    return result;
}

void RopeString::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_totalOps = 0;
}

/* ===== 私有方法实现 ===== */

int RopeString::computeTotalLength(Node* node) const
{
    if (node == nullptr) return 0;
    if (node->isLeaf()) return node->data.length();
    return computeTotalLength(node->left) + computeTotalLength(node->right);
}

RopeString::Node* RopeString::buildRope(const QString& text,
                                         int start, int end) const
{
    int len = end - start;
    if (len <= 0) return nullptr;

    if (len <= kLeafSize) {
        Node* leaf = new Node();
        leaf->data = text.mid(start, len);
        leaf->weight = len;
        return leaf;
    }

    int mid = start + len / 2;
    Node* node = new Node();
    node->left = buildRope(text, start, mid);
    node->right = buildRope(text, mid, end);
    node->weight = mid - start;
    return node;
}

RopeString::Node* RopeString::insertNode(Node* node, int pos,
                                          const QString& str)
{
    if (node == nullptr) {
        Node* leaf = new Node();
        leaf->data = str;
        leaf->weight = str.length();
        return leaf;
    }

    if (node->isLeaf()) {
        /* 在叶子节点中插入 */
        int leafLen = node->data.length();
        int insertPos = std::min(pos, leafLen);
        QString newData = node->data.mid(0, insertPos) + str
                          + node->data.mid(insertPos);

        if (newData.length() <= kLeafSize) {
            node->data = newData;
            node->weight = newData.length();
            return node;
        }

        /* 需要分裂 */
        int mid = newData.length() / 2;
        Node* parent = new Node();
        parent->left = new Node();
        parent->left->data = newData.mid(0, mid);
        parent->left->weight = mid;
        parent->right = new Node();
        parent->right->data = newData.mid(mid);
        parent->right->weight = newData.length() - mid;
        parent->weight = mid;
        destroyNode(node);
        return parent;
    }

    /* 内部节点: 根据pos决定走左子树还是右子树 */
    if (pos <= node->weight) {
        node->left = insertNode(node->left, pos, str);
        node->weight += str.length();
    } else {
        node->right = insertNode(node->right, pos - node->weight, str);
    }

    return node;
}

RopeString::Node* RopeString::removeRange(Node* node, int pos, int len)
{
    if (node == nullptr || len <= 0) return node;

    if (node->isLeaf()) {
        int leafLen = node->data.length();
        int startPos = std::min(pos, leafLen);
        int endPos = std::min(pos + len, leafLen);
        node->data = node->data.mid(0, startPos) + node->data.mid(endPos);
        node->weight = node->data.length();
        if (node->data.isEmpty()) {
            delete node;
            return nullptr;
        }
        return node;
    }

    int leftLen = node->weight;
    int rightStart = leftLen;

    /* 删除范围可能跨越左右子树 */
    if (pos + len <= leftLen) {
        /* 全部在左子树 */
        node->left = removeRange(node->left, pos, len);
        node->weight -= len;
    } else if (pos >= leftLen) {
        /* 全部在右子树 */
        node->right = removeRange(node->right, pos - leftLen, len);
    } else {
        /* 跨越左右子树 */
        int leftRemove = leftLen - pos;
        int rightRemove = len - leftRemove;

        node->left = removeRange(node->left, pos, leftRemove);
        node->right = removeRange(node->right, 0, rightRemove);
        node->weight -= leftRemove;
    }

    /* 清理空子树 */
    if (node->left == nullptr && node->right == nullptr) {
        delete node;
        return nullptr;
    }
    if (node->left == nullptr) {
        Node* right = node->right;
        node->right = nullptr;
        delete node;
        return right;
    }
    if (node->right == nullptr) {
        Node* left = node->left;
        node->left = nullptr;
        delete node;
        return left;
    }

    return node;
}

void RopeString::substringHelper(Node* node, int pos, int len,
                                  int& skipped, QString& result) const
{
    if (node == nullptr || result.length() >= len) return;

    if (node->isLeaf()) {
        int leafLen = node->data.length();
        if (skipped + leafLen > pos) {
            int start = std::max(0, pos - skipped);
            int remaining = len - result.length();
            int available = std::min(remaining, leafLen - start);
            if (available > 0) {
                result += node->data.mid(start, available);
            }
        }
        skipped += leafLen;
        return;
    }

    /* 先遍历左子树 */
    substringHelper(node->left, pos, len, skipped, result);

    /* 再遍历右子树 */
    if (result.length() < len) {
        substringHelper(node->right, pos, len, skipped, result);
    }
}

void RopeString::toStringHelper(Node* node, QString& result) const
{
    if (node == nullptr) return;
    if (node->isLeaf()) {
        result += node->data;
        return;
    }
    toStringHelper(node->left, result);
    toStringHelper(node->right, result);
}

void RopeString::updateWeight(Node* node)
{
    if (node == nullptr || node->isLeaf()) return;
    node->weight = 0;
    if (node->left != nullptr) {
        updateWeight(node->left);
        node->weight = computeTotalLength(node->left);
    }
    if (node->right != nullptr) {
        updateWeight(node->right);
    }
}

RopeString::Node* RopeString::splitLeaf(Node* node)
{
    if (node == nullptr) return nullptr;

    if (node->isLeaf()) {
        if (node->data.length() > kLeafSize * 2) {
            int mid = node->data.length() / 2;
            Node* parent = new Node();
            parent->left = new Node();
            parent->left->data = node->data.mid(0, mid);
            parent->left->weight = mid;
            parent->right = new Node();
            parent->right->data = node->data.mid(mid);
            parent->right->weight = node->data.length() - mid;
            parent->weight = mid;
            QString oldData = node->data;
            delete node;
            return parent;
        }
        return node;
    }

    node->left = splitLeaf(node->left);
    node->right = splitLeaf(node->right);
    return node;
}

void RopeString::destroyNode(Node* node)
{
    if (node == nullptr) return;
    destroyNode(node->left);
    destroyNode(node->right);
    delete node;
}

RopeString::Node* RopeString::cloneNode(Node* node) const
{
    if (node == nullptr) return nullptr;
    Node* copy = new Node();
    copy->weight = node->weight;
    copy->data = node->data;
    copy->left = cloneNode(node->left);
    copy->right = cloneNode(node->right);
    return copy;
}
