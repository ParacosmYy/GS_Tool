/**
 * @file RedBlackTree.h
 * @brief 红黑树 — 头文件模板实现
 *
 * 功能: 自平衡BST，insert/remove/contains，
 *       O(log n)操作，红黑性质维护。
 */
#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include <QVector>
#include <algorithm>

/**
 * @brief 红黑树模板
 * @tparam T 可比较类型
 */
template<typename T>
class RedBlackTree {
public:
    /** @brief 统计 */
    struct Stats {
        int nodeCount = 0;          ///< 节点数
        int treeHeight = 0;         ///< 树高度
        quint64 totalInserts = 0;   ///< 累计插入
        quint64 totalRemoves = 0;   ///< 累计删除
        quint64 totalRecolorings = 0; ///< 累计变色
        quint64 totalRotations = 0; ///< 累计旋转
        quint64 totalSearches = 0;  ///< 累计搜索
    };
    enum class Color { Red, Black };
    /** @brief 节点 */
    struct Node {
        T data; Color color=Color::Red; Node*left=nullptr; Node*right=nullptr; Node*parent=nullptr;
        explicit Node(const T& v) : data(v) {}
    };

    RedBlackTree() : m_nil(new Node(T{})) { m_nil->color=Color::Black;m_nil->left=m_nil->right=m_nil->parent=m_nil;m_root=m_nil; }
    RedBlackTree(const RedBlackTree&) = delete;
    RedBlackTree& operator=(const RedBlackTree&) = delete;
    ~RedBlackTree() { destroy(m_root); delete m_nil; }

    /** @brief 插入元素 @param value 值 */
    void insert(const T& value) {
        Node*nn=new Node(value); nn->left=m_nil; nn->right=m_nil; nn->parent=m_nil;
        Node*p=m_nil,*c=m_root;
        while(c!=m_nil){p=c;if(value<c->data)c=c->left;else if(c->data<value)c=c->right;else{delete nn;return;}}
        nn->parent=p;
        if(p==m_nil)m_root=nn; else if(value<p->data)p->left=nn; else p->right=nn;
        m_stats.totalInserts++; fixInsert(nn); updateMeta();
    }
    /** @brief 删除元素 @param value 值 @return 是否成功 */
    bool remove(const T& value) {
        Node*n=findN(m_root,value); if(n==m_nil)return false;
        m_stats.totalRemoves++; Node*y=n,*x=m_nil; Color yc=y->color;
        if(n->left==m_nil){x=n->right;transplant(n,n->right);}
        else if(n->right==m_nil){x=n->left;transplant(n,n->left);}
        else{y=minN(n->right);yc=y->color;x=y->right;if(y->parent==n)x->parent=y;else{transplant(y,y->right);y->right=n->right;y->right->parent=y;}
            transplant(n,y);y->left=n->left;y->left->parent=y;y->color=n->color;}
        delete n; if(yc==Color::Black)fixDel(x); updateMeta(); return true;
    }
    /** @brief 是否包含 @param value 值 @return 是否存在 */
    bool contains(const T& value) { m_stats.totalSearches++; return findN(m_root,value)!=m_nil; }
    int height() const { return compH(m_root); }
    int size() const { return m_stats.nodeCount; }
    bool isEmpty() const { return m_root==m_nil; }
    void clear() { destroy(m_root); m_root=m_nil; m_stats.nodeCount=0; m_stats.treeHeight=0; }
    /** @brief 中序遍历 @return 有序列表 */
    QVector<T> inorderTraversal() const { QVector<T>r; inord(m_root,r); return r; }
    const Stats& stats() const { return m_stats; }
    void resetStatistics() { m_stats.totalInserts=m_stats.totalRemoves=m_stats.totalRecolorings=m_stats.totalRotations=m_stats.totalSearches=0; }

private:
    Node*m_root,*m_nil; Stats m_stats;
    void updateMeta(){m_stats.nodeCount=countN(m_root);m_stats.treeHeight=compH(m_root);}
    void rotL(Node*x){
        Node*y=x->right;x->right=y->left;if(y->left!=m_nil)y->left->parent=x;y->parent=x->parent;
        if(x->parent==m_nil)m_root=y;else if(x==x->parent->left)x->parent->left=y;else x->parent->right=y;
        y->left=x;x->parent=y;m_stats.totalRotations++;
    }
    void rotR(Node*y){
        Node*x=y->left;y->left=x->right;if(x->right!=m_nil)x->right->parent=y;x->parent=y->parent;
        if(y->parent==m_nil)m_root=x;else if(y==y->parent->right)y->parent->right=x;else y->parent->left=x;
        x->right=y;y->parent=x;m_stats.totalRotations++;
    }
    void fixInsert(Node*z){
        while(z->parent->color==Color::Red){
            if(z->parent==z->parent->parent->left){Node*u=z->parent->parent->right;
                if(u->color==Color::Red){z->parent->color=Color::Black;u->color=Color::Black;z->parent->parent->color=Color::Red;m_stats.totalRecolorings+=3;z=z->parent->parent;}
                else{if(z==z->parent->right){z=z->parent;rotL(z);}z->parent->color=Color::Black;z->parent->parent->color=Color::Red;m_stats.totalRecolorings+=2;rotR(z->parent->parent);}}
            else{Node*u=z->parent->parent->left;
                if(u->color==Color::Red){z->parent->color=Color::Black;u->color=Color::Black;z->parent->parent->color=Color::Red;m_stats.totalRecolorings+=3;z=z->parent->parent;}
                else{if(z==z->parent->left){z=z->parent;rotR(z);}z->parent->color=Color::Black;z->parent->parent->color=Color::Red;m_stats.totalRecolorings+=2;rotL(z->parent->parent);}}
        }
        if(m_root->color!=Color::Black){m_root->color=Color::Black;m_stats.totalRecolorings++;}
    }
    void transplant(Node*u,Node*v){if(u->parent==m_nil)m_root=v;else if(u==u->parent->left)u->parent->left=v;else u->parent->right=v;v->parent=u->parent;}
    void fixDel(Node*x){
        while(x!=m_root&&x->color==Color::Black){
            if(x==x->parent->left){Node*w=x->parent->right;
                if(w->color==Color::Red){w->color=Color::Black;x->parent->color=Color::Red;m_stats.totalRecolorings+=2;rotL(x->parent);w=x->parent->right;}
                if(w->left->color==Color::Black&&w->right->color==Color::Black){w->color=Color::Red;m_stats.totalRecolorings++;x=x->parent;}
                else{if(w->right->color==Color::Black){w->left->color=Color::Black;w->color=Color::Red;m_stats.totalRecolorings+=2;rotR(w);w=x->parent->right;}
                    w->color=x->parent->color;x->parent->color=Color::Black;w->right->color=Color::Black;m_stats.totalRecolorings+=3;rotL(x->parent);x=m_root;}}
            else{Node*w=x->parent->left;
                if(w->color==Color::Red){w->color=Color::Black;x->parent->color=Color::Red;m_stats.totalRecolorings+=2;rotR(x->parent);w=x->parent->left;}
                if(w->right->color==Color::Black&&w->left->color==Color::Black){w->color=Color::Red;m_stats.totalRecolorings++;x=x->parent;}
                else{if(w->left->color==Color::Black){w->right->color=Color::Black;w->color=Color::Red;m_stats.totalRecolorings+=2;rotL(w);w=x->parent->left;}
                    w->color=x->parent->color;x->parent->color=Color::Black;w->left->color=Color::Black;m_stats.totalRecolorings+=3;rotR(x->parent);x=m_root;}}
        }
        if(x->color!=Color::Black){x->color=Color::Black;m_stats.totalRecolorings++;}
    }
    Node*findN(Node*n,const T&v)const{while(n!=m_nil){if(v<n->data)n=n->left;else if(n->data<v)n=n->right;else return n;}return m_nil;}
    Node*minN(Node*n)const{while(n->left!=m_nil)n=n->left;return n;}
    void inord(Node*n,QVector<T>&r)const{if(n==m_nil)return;inord(n->left,r);r.append(n->data);inord(n->right,r);}
    int compH(Node*n)const{return n==m_nil?0:1+std::max(compH(n->left),compH(n->right));}
    int countN(Node*n)const{return n==m_nil?0:1+countN(n->left)+countN(n->right);}
    void destroy(Node*n){if(n==m_nil)return;destroy(n->left);destroy(n->right);delete n;}
};

#endif // REDBLACKTREE_H
