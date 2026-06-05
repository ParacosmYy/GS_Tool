/**
 * @file AvlTree.h
 * @brief AVL平衡二叉搜索树 — 头文件模板实现
 *
 * 功能: 自平衡BST，insert/remove/contains/height，
 *       中序遍历，O(log n)操作保证。
 */
#ifndef AVLTREE_H
#define AVLTREE_H

#include <QVector>
#include <algorithm>

/**
 * @brief AVL平衡二叉搜索树模板
 * @tparam T 可比较类型
 */
template<typename T>
class AvlTree {
public:
    /** @brief 统计 */
    struct Stats {
        int nodeCount = 0;          ///< 节点数
        int treeHeight = 0;         ///< 树高度
        quint64 totalInserts = 0;   ///< 累计插入次数
        quint64 totalRemoves = 0;   ///< 累计删除次数
        quint64 totalRotations = 0; ///< 累计旋转次数
        quint64 totalSearches = 0;  ///< 累计搜索次数
    };
    /** @brief 节点 */
    struct Node {
        T data;                     ///< 数据
        Node* left = nullptr;       ///< 左子节点
        Node* right = nullptr;      ///< 右子节点
        int height = 1;             ///< 节点高度
        explicit Node(const T& v) : data(v) {}
    };

    AvlTree() = default;
    AvlTree(const AvlTree& o) : m_stats(o.m_stats) { m_root = clone(m_root, o.m_root); }
    AvlTree(AvlTree&& o) noexcept : m_root(o.m_root), m_stats(o.m_stats) { o.m_root = nullptr; }
    ~AvlTree() { destroy(m_root); }
    AvlTree& operator=(const AvlTree& o) { if(this!=&o){destroy(m_root);m_root=clone(m_root,o.m_root);m_stats=o.m_stats;} return *this; }
    AvlTree& operator=(AvlTree&& o) noexcept { if(this!=&o){destroy(m_root);m_root=o.m_root;m_stats=o.m_stats;o.m_root=nullptr;} return *this; }

    /** @brief 插入元素 @param value 值 */
    void insert(const T& value) { m_root = ins(m_root, value); updateMeta(); }
    /** @brief 删除元素 @param value 值 @return 是否成功 */
    bool remove(const T& value) { if(!contains(value)) return false; m_root=rem(m_root,value); updateMeta(); return true; }
    /** @brief 是否包含 @param value 值 @return 是否存在 */
    bool contains(const T& value) { m_stats.totalSearches++; return find(m_root,value)!=nullptr; }
    /** @brief 树高度 @return 高度 */
    int height() const { return ht(m_root); }
    /** @brief 节点数 @return 数量 */
    int size() const { return m_stats.nodeCount; }
    /** @brief 是否为空 @return 空判断 */
    bool isEmpty() const { return m_root == nullptr; }
    /** @brief 清空树 */
    void clear() { destroy(m_root); m_root=nullptr; m_stats.nodeCount=0; m_stats.treeHeight=0; }
    /** @brief 中序遍历 @return 有序列表 */
    QVector<T> inorderTraversal() const { QVector<T> r; inord(m_root,r); return r; }
    /** @brief 最小值 @return 指针 */
    const T* findMin() const { Node*n=minN(m_root); return n?&n->data:nullptr; }
    /** @brief 最大值 @return 指针 */
    const T* findMax() const { Node*n=maxN(m_root); return n?&n->data:nullptr; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats.totalInserts=m_stats.totalRemoves=m_stats.totalRotations=m_stats.totalSearches=0; }

private:
    Node* m_root = nullptr;
    Stats m_stats;
    int ht(Node* n) const { return n ? n->height : 0; }
    int bf(Node* n) const { return n ? ht(n->left)-ht(n->right) : 0; }
    void upd(Node* n) { if(n) n->height=1+std::max(ht(n->left),ht(n->right)); }
    void updateMeta() { m_stats.nodeCount=countN(m_root); m_stats.treeHeight=ht(m_root); }
    Node* rotR(Node* y) { Node*x=y->left; y->left=x->right; x->right=y; upd(y); upd(x); m_stats.totalRotations++; return x; }
    Node* rotL(Node* x) { Node*y=x->right; x->right=y->left; y->left=x; upd(x); upd(y); m_stats.totalRotations++; return y; }
    Node* bal(Node* n) {
        upd(n); int b=bf(n);
        if(b>1){ if(bf(n->left)<0) n->left=rotL(n->left); return rotR(n); }
        if(b<-1){ if(bf(n->right)>0) n->right=rotR(n->right); return rotL(n); }
        return n;
    }
    Node* ins(Node* n, const T& v) {
        if(!n){ m_stats.totalInserts++; return new Node(v); }
        if(v<n->data) n->left=ins(n->left,v);
        else if(n->data<v) n->right=ins(n->right,v);
        else return n;
        return bal(n);
    }
    Node* rem(Node* n, const T& v) {
        if(!n) return nullptr;
        if(v<n->data) n->left=rem(n->left,v);
        else if(n->data<v) n->right=rem(n->right,v);
        else { m_stats.totalRemoves++; if(!n->left||!n->right){Node*c=n->left?n->left:n->right;delete n;return c;} Node*s=minN(n->right);n->data=s->data;n->right=rem(n->right,s->data); }
        return bal(n);
    }
    Node* find(Node* n, const T& v) const { while(n){if(v<n->data)n=n->left;else if(n->data<v)n=n->right;else return n;} return nullptr; }
    Node* minN(Node* n) const { if(!n)return nullptr;while(n->left)n=n->left;return n; }
    Node* maxN(Node* n) const { if(!n)return nullptr;while(n->right)n=n->right;return n; }
    void inord(Node* n, QVector<T>& r) const { if(!n)return;inord(n->left,r);r.append(n->data);inord(n->right,r); }
    int countN(Node* n) const { return n?1+countN(n->left)+countN(n->right):0; }
    void destroy(Node* n) { if(!n)return;destroy(n->left);destroy(n->right);delete n; }
    Node* clone(Node*, Node* n) const { if(!n)return nullptr;Node*c=new Node(n->data);c->height=n->height;c->left=clone(c->left,n->left);c->right=clone(c->right,n->right);return c; }
};

#endif // AVLTREE_H
