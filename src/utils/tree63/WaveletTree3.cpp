/**
 * @file WaveletTree3.cpp
 * @brief 小波树实现 (rank/select/access + 范围频率查询)
 *
 * 实现小波树(Wavelet Tree)数据结构，支持高效的序列操作:
 * - access(i): 获取位置i的字符 O(log sigma)
 * - rank(c, i): 前i个位置中字符c的出现次数 O(log sigma)
 * - select(c, k): 字符c第k次出现的位置 O(log sigma)
 * - rangeFreq(lo, hi, a, b): 区间[lo,hi)中值在[a,b]范围内的频率统计
 * 小波树在信息检索和基因组分析中有广泛应用。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/tree63/WaveletTree3.h"

#include <QElapsedTimer>
#include <algorithm>

/* 匿命名空间辅助函数: 递归删除小波树节点 */
namespace {
    void deleteWTNodes(WaveletTree3::WTNode* node) {
        if (!node) return;
        deleteWTNodes(node->left);
        deleteWTNodes(node->right);
        delete node;
    }
}

/**
 * @brief 构造函数，初始化小波树
 * @param parent 父QObject指针
 */
WaveletTree3::WaveletTree3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置字符集大小
 * @param sigma 字符集大小 (默认 256)，即值的范围 [0, sigma)
 */
void WaveletTree3::setAlphabetSize(int sigma)
{
    m_sigma = qMax(2, sigma);
}

/**
 * @brief 构建小波树
 *
 * 构建过程:
 * 1. 递归构建: 每个节点存储当前范围的位向量
 * 2. 位向量: bit[i]=1 表示 data[i] > mid, bit[i]=0 表示 data[i] <= mid
 * 3. 左子树包含值 <= mid 的元素，右子树包含值 > mid 的元素
 * 4. 直到范围缩小到单个值或数据为空
 *
 * @param data 输入整数序列
 */
void WaveletTree3::build(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 清理旧树 */
    if (m_root) {
        deleteWTNodes(m_root);
        m_root = nullptr;
    }

    m_n = data.size();

    if (data.isEmpty()) {
        emit treeBuilt(0, m_sigma);
        return;
    }

    /* 递归构建 */
    m_root = buildNode(data, 0, m_sigma - 1);

    /* 更新统计 */
    m_stats.totalBuilds++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = (m_stats.totalBuilds + m_stats.totalQueries > 0)
        ? m_timeSum / (m_stats.totalBuilds + m_stats.totalQueries) : 0.0;

    emit treeBuilt(m_n, m_sigma);
}

/**
 * @brief rank查询: 计算前pos个位置中symbol出现的次数
 *
 * 递归查询:
 * 1. 在当前节点，根据symbol与mid的比较确定方向
 * 2. 如果 symbol <= mid: rank = 左子树中 rank(symbol, count(0到pos-1中0的个数))
 * 3. 如果 symbol > mid:  rank = 右子树中 rank(symbol, count(0到pos-1中1的个数))
 *
 * @param symbol 待查询的字符
 * @param pos 查询位置 (不含)，即查询 [0, pos)
 * @return symbol在前pos个位置中的出现次数
 */
int WaveletTree3::rank(int symbol, int pos) const
{
    if (!m_root || pos <= 0 || symbol < 0 || symbol >= m_sigma) {
        return 0;
    }
    return rankNode(m_root, symbol, pos, 0, m_sigma - 1);
}

/**
 * @brief select查询: 找到symbol第k次出现的位置
 *
 * 递归查询:
 * 1. 从根到叶确定symbol的路径
 * 2. 在叶子节点开始反向查找
 * 3. 根据位向量中的0/1映射回到原始位置
 *
 * @param symbol 待查询的字符
 * @param k 第k次出现 (1-based)
 * @return 位置索引 (0-based)，如果不足k次返回 -1
 */
int WaveletTree3::select(int symbol, int k) const
{
    if (!m_root || k <= 0 || symbol < 0 || symbol >= m_sigma) {
        return -1;
    }
    return selectNode(m_root, symbol, k, 0, m_sigma - 1);
}

/**
 * @brief access查询: 获取位置pos的字符
 *
 * 递归查询:
 * 1. 在根节点检查位向量中pos位的值
 * 2. 如果为0，进入左子树，新位置为pos前0的个数
 * 3. 如果为1，进入右子树，新位置为pos前1的个数
 * 4. 直到到达叶子节点
 *
 * @param pos 查询位置 (0-based)
 * @return 该位置的字符值
 */
int WaveletTree3::access(int pos) const
{
    if (!m_root || pos < 0 || pos >= m_n) {
        return -1;
    }

    WTNode* node = m_root;
    int lo = 0;
    int hi = m_sigma - 1;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (pos < node->b.size() && node->b[pos] == 0) {
            /* 进入左子树: 计算pos前0的个数 */
            int zeros = 0;
            for (int i = 0; i < pos; ++i) {
                if (node->b[i] == 0) zeros++;
            }
            pos = zeros;
            node = node->left;
            hi = mid;
        } else {
            /* 进入右子树: 计算pos前1的个数 */
            int ones = 0;
            for (int i = 0; i < qMin(pos, node->b.size()); ++i) {
                if (node->b[i] == 1) ones++;
            }
            pos = ones;
            node = node->right;
            lo = mid + 1;
        }

        if (!node) return -1;
    }

    return lo;
}

/**
 * @brief 范围频率查询
 *
 * 查询区间 [lo, hi) 中值在 [a, b] 范围内的元素个数
 *
 * @param lo 区间起始 (含)
 * @param hi 区间结束 (不含)
 * @param a 值范围下界 (含)
 * @param b 值范围上界 (含)
 * @return 频率统计向量 {范围内的个数, 低于a的个数, 高于b的个数}
 */
QVector<int> WaveletTree3::rangeFreq(int lo, int hi, int a, int b) const
{
    QVector<int> result(3, 0); /* {in_range, below, above} */

    if (!m_root || lo < 0 || hi > m_n || lo >= hi) {
        return result;
    }

    /* 简化实现: 遍历区间统计 */
    for (int i = lo; i < hi; ++i) {
        int val = access(i);
        if (val >= a && val <= b) {
            result[0]++;
        } else if (val < a) {
            result[1]++;
        } else {
            result[2]++;
        }
    }

    return result;
}

/**
 * @brief 重置所有统计数据
 */
void WaveletTree3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
    if (m_root) {
        deleteWTNodes(m_root);
        m_root = nullptr;
    }
    m_n = 0;
}

/**
 * @brief 递归构建小波树节点
 *
 * @param data 当前节点的数据子集
 * @param lo 当前值范围下界
 * @param hi 当前值范围上界
 * @return 构建的节点
 */
WaveletTree3::WTNode* WaveletTree3::buildNode(const QVector<int>& data, int lo, int hi)
{
    if (data.isEmpty() || lo > hi) return nullptr;

    WTNode* node = new WTNode();
    node->lo = lo;
    node->hi = hi;
    node->left = nullptr;
    node->right = nullptr;

    /* 叶子节点 */
    if (lo == hi) {
        node->b = QVector<int>(data.size(), 0);
        return node;
    }

    int mid = lo + (hi - lo) / 2;

    /* 构建位向量 */
    node->b.resize(data.size());
    QVector<int> leftData, rightData;

    for (int i = 0; i < data.size(); ++i) {
        if (data[i] <= mid) {
            node->b[i] = 0;
            leftData.append(data[i]);
        } else {
            node->b[i] = 1;
            rightData.append(data[i]);
        }
    }

    /* 递归构建子树 */
    node->left = buildNode(leftData, lo, mid);
    node->right = buildNode(rightData, mid + 1, hi);

    return node;
}

/**
 * @brief 递归rank查询
 */
int WaveletTree3::rankNode(WTNode* n, int symbol, int pos, int lo, int hi) const
{
    if (!n || pos <= 0) return 0;
    if (lo == hi) return pos; /* 叶子节点 */

    int mid = lo + (hi - lo) / 2;
    int bit = (symbol > mid) ? 1 : 0;

    /* 统计 pos 前与 bit 相同的位数 */
    int count = 0;
    for (int i = 0; i < qMin(pos, n->b.size()); ++i) {
        if (n->b[i] == bit) count++;
    }

    if (bit == 0) {
        return rankNode(n->left, symbol, count, lo, mid);
    } else {
        return rankNode(n->right, symbol, count, mid + 1, hi);
    }
}

/**
 * @brief 递归select查询
 */
int WaveletTree3::selectNode(WTNode* n, int symbol, int k, int lo, int hi) const
{
    if (!n || k <= 0) return -1;
    if (lo == hi) return k - 1; /* 叶子节点: 第k个0/1的位置 */

    int mid = lo + (hi - lo) / 2;
    int bit = (symbol > mid) ? 1 : 0;

    int posInChild;
    if (bit == 0) {
        posInChild = selectNode(n->left, symbol, k, lo, mid);
    } else {
        posInChild = selectNode(n->right, symbol, k, mid + 1, hi);
    }

    if (posInChild < 0) return -1;

    /* 将子树位置映射回当前节点位置 */
    int count = 0;
    for (int i = 0; i < n->b.size(); ++i) {
        if (n->b[i] == bit) {
            count++;
            if (count == posInChild + 1) {
                return i;
            }
        }
    }

    return -1;
}
