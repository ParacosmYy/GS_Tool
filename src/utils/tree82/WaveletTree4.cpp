/**
 * @file WaveletTree4.cpp
 * @brief 小波树实现 — 基于位向量的rank/select/access操作
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-06
 *
 * 实现小波树(Wavelet Tree)数据结构:
 * - build: 从整数序列递归构建小波树，每个节点存储位向量
 * - rank: 查询序列[0..pos]中值val出现的次数，O(log sigma)
 * - select: 查询值val第k次出现的位置，O(log sigma)
 * - access: 访问指定位置的值，O(log sigma)
 *
 * 小波树原理:
 * 递归地将字符集二分，每个内部节点存储位向量B:
 * B[i]=0表示元素进入左子树(值 <= mid)
 * B[i]=1表示元素进入右子树(值 > mid)
 * 直到叶子节点(值域缩小到单个字符)
 *
 * 应用场景: 全文索引、基因组分析、压缩数据结构
 */

#include "utils/tree82/WaveletTree4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ─── 内部节点结构 ─── */

/**
 * @brief 小波树节点
 *
 * 每个节点维护:
 * - b: 位向量，b[i]=0表示对应元素进入左子树，b[i]=1进入右子树
 * - prefixRank0/prefixRank1: 前缀和数组，加速rank计算
 *   prefixRank0[i] = 前i个位中0的个数
 *   prefixRank1[i] = 前i个位中1的个数
 * - lo/hi: 当前节点覆盖的值域范围 [lo, hi]
 * - left/right: 左右子树
 */
struct WaveletTree4::WTNode {
    QVector<int> b;             ///< 位向量
    QVector<int> prefixRank0;   ///< 前缀和: 前i位中0的个数
    QVector<int> prefixRank1;   ///< 前缀和: 前i位中1的个数
    int lo;                     ///< 值域下界
    int hi;                     ///< 值域上界
    WTNode* left;               ///< 左子树(值 <= mid)
    WTNode* right;              ///< 右子树(值 > mid)

    WTNode() : lo(0), hi(0), left(nullptr), right(nullptr) {}
};

/* 匿名命名空间: 递归释放节点 */
namespace {
    void deleteWTNodes(WaveletTree4::WTNode* node) {
        if (!node) return;
        deleteWTNodes(node->left);
        deleteWTNodes(node->right);
        delete node;
    }
}

/* ─── 构造函数 ─── */

/**
 * @brief 构造函数，初始化空的小波树
 * @param parent 父QObject对象
 */
WaveletTree4::WaveletTree4(QObject* parent)
    : QObject(parent)
    , m_root(nullptr)
    , m_alphabetSize(0)
    , m_sequenceLength(0)
{
}

/* ─── 构建小波树 ─── */

/**
 * @brief 从整数序列构建小波树
 *
 * 构建流程:
 * 1. 确定字符集大小(自动检测或使用指定值)
 * 2. 递归构建: 每个节点将当前值域[lo, hi]二分为[lo, mid]和[mid+1, hi]
 * 3. 为每个节点生成位向量: 元素值 <= mid则bit=0(进左子树)，否则bit=1(进右子树)
 * 4. 构建前缀和数组prefixRank0/prefixRank1用于O(1)的rank操作
 * 5. 递归构建左右子树直到值域缩小到单点(叶子节点)
 *
 * @param sequence 输入整数序列
 * @param alphabetSize 字符集大小(0表示自动检测)
 */
void WaveletTree4::build(const QVector<int>& sequence, int alphabetSize)
{
    QElapsedTimer timer;
    timer.start();

    /* 清理旧树 */
    if (m_root) {
        deleteWTNodes(m_root);
        m_root = nullptr;
    }

    m_sequenceLength = sequence.size();

    if (sequence.isEmpty()) {
        emit treeBuilt(0, 0);
        return;
    }

    /* 确定字符集大小 */
    if (alphabetSize <= 0) {
        int maxVal = 0;
        for (int v : sequence) {
            maxVal = qMax(maxVal, v);
        }
        m_alphabetSize = maxVal + 1;
    } else {
        m_alphabetSize = alphabetSize;
    }

    /* 递归构建小波树 */
    m_root = buildNode(sequence, 0, m_alphabetSize - 1);

    /* 更新统计 */
    m_stats.totalTreesBuilt++;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    int totalOps = m_stats.totalTreesBuilt + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit treeBuilt(m_sequenceLength, m_alphabetSize);
}

/* ─── Rank查询 ─── */

/**
 * @brief Rank查询: 序列[0..pos-1]中值val出现的次数
 *
 * 递归查询过程:
 * 1. 在当前节点，根据val与mid的比较确定位值bit
 * 2. bit=0(val <= mid): 查询左子树，新位置=前pos位中0的个数
 * 3. bit=1(val > mid): 查询右子树，新位置=前pos位中1的个数
 * 4. 到达叶子节点时返回当前位置数
 *
 * 利用prefixRank前缀和数组，每层O(1)时间计算rank。
 * 总时间: O(log sigma)，sigma为字符集大小。
 *
 * @param pos 查询位置(不含)，即查询[0, pos)范围
 * @param val 待查询的值
 * @return val在前pos个位置中的出现次数
 */
int WaveletTree4::rank(int pos, int val) const
{
    if (!m_root || pos <= 0 || val < 0 || val >= m_alphabetSize) {
        return 0;
    }
    return rankHelper(m_root, pos, val, 0, m_alphabetSize - 1);
}

/* ─── Select查询 ─── */

/**
 * @brief Select查询: 值val第k次出现的位置
 *
 * 递归查询过程(从根到叶确定路径，再从叶到根映射回位置):
 * 1. 确定val的位值bit(val <= mid ? 0 : 1)
 * 2. 递归到对应子树获取子树中的位置
 * 3. 将子树位置映射回当前层: 在位向量中找第subPos个bit值的位置
 * 4. 利用prefixRank辅助二分搜索实现O(log n)映射
 *
 * @param k 第k次出现(1-based)
 * @param val 待查询的值
 * @return 位置索引(0-based)，不足k次返回-1
 */
int WaveletTree4::select(int k, int val) const
{
    if (!m_root || k <= 0 || val < 0 || val >= m_alphabetSize) {
        return -1;
    }
    return selectHelper(m_root, k, val, 0, m_alphabetSize - 1);
}

/* ─── Access查询 ─── */

/**
 * @brief Access查询: 获取序列中位置pos的值
 *
 * 递归查询过程:
 * 1. 检查位向量中pos位置的bit值
 * 2. bit=0: 进入左子树，新位置=pos前0的个数(使用prefixRank0)
 * 3. bit=1: 进入右子树，新位置=pos前1的个数(使用prefixRank1)
 * 4. 到达叶子节点时，lo即为所求的值
 *
 * 每层O(1)利用前缀和，总时间O(log sigma)。
 *
 * @param pos 查询位置(0-based)
 * @return 该位置的值，越界返回-1
 */
int WaveletTree4::access(int pos) const
{
    if (!m_root || pos < 0 || pos >= m_sequenceLength) {
        return -1;
    }

    WTNode* node = m_root;
    int lo = 0;
    int hi = m_alphabetSize - 1;

    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        int bit = node->b[pos];

        if (bit == 0) {
            /* 进入左子树: pos前0的个数 */
            pos = node->prefixRank0[pos];
            node = node->left;
            hi = mid;
        } else {
            /* 进入右子树: pos前1的个数 */
            pos = node->prefixRank1[pos];
            node = node->right;
            lo = mid + 1;
        }

        if (!node) return -1;
    }

    return lo;
}

/* ─── 统计重置 ─── */

/**
 * @brief 重置所有统计数据并释放树内存
 */
void WaveletTree4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    if (m_root) {
        deleteWTNodes(m_root);
        m_root = nullptr;
    }
    m_alphabetSize = 0;
    m_sequenceLength = 0;
}

/* ─── 私有方法: 递归构建节点 ─── */

/**
 * @brief 递归构建小波树节点
 *
 * 构建步骤:
 * 1. 创建节点并设置值域[lo, hi]
 * 2. 如果lo == hi(叶子节点)，位向量全0，直接返回
 * 3. 计算mid = (lo + hi) / 2，将值域二分
 * 4. 遍历数据生成位向量，同时分割数据到左右子集
 * 5. 构建prefixRank0/prefixRank1前缀和数组
 * 6. 递归构建左右子树
 *
 * @param data 当前节点的数据子集
 * @param lo 当前值域下界
 * @param hi 当前值域上界
 * @return 构建的节点指针
 */
WaveletTree4::WTNode* WaveletTree4::buildNode(
    const QVector<int>& data, int lo, int hi)
{
    if (data.isEmpty() || lo > hi) return nullptr;

    WTNode* node = new WTNode();
    node->lo = lo;
    node->hi = hi;

    /* 叶子节点 */
    if (lo == hi) {
        node->b = QVector<int>(data.size(), 0);
        /* 构建前缀和: 全0 */
        node->prefixRank0.resize(data.size() + 1);
        node->prefixRank1.resize(data.size() + 1);
        for (int i = 0; i <= data.size(); ++i) {
            node->prefixRank0[i] = i;
            node->prefixRank1[i] = 0;
        }
        return node;
    }

    int mid = lo + (hi - lo) / 2;

    /* 生成位向量和分割数据 */
    int n = data.size();
    node->b.resize(n);
    QVector<int> leftData, rightData;
    leftData.reserve(n);
    rightData.reserve(n);

    for (int i = 0; i < n; ++i) {
        if (data[i] <= mid) {
            node->b[i] = 0;
            leftData.append(data[i]);
        } else {
            node->b[i] = 1;
            rightData.append(data[i]);
        }
    }

    /* 构建前缀和数组(1-indexed, prefixRank[0]=0) */
    node->prefixRank0.resize(n + 1);
    node->prefixRank1.resize(n + 1);
    node->prefixRank0[0] = 0;
    node->prefixRank1[0] = 0;

    for (int i = 0; i < n; ++i) {
        node->prefixRank0[i + 1] = node->prefixRank0[i] + (node->b[i] == 0 ? 1 : 0);
        node->prefixRank1[i + 1] = node->prefixRank1[i] + (node->b[i] == 1 ? 1 : 0);
    }

    /* 递归构建子树 */
    node->left = buildNode(leftData, lo, mid);
    node->right = buildNode(rightData, mid + 1, hi);

    return node;
}

/* ─── 私有方法: Rank递归查询 ─── */

/**
 * @brief 递归rank查询
 *
 * 利用prefixRank前缀和数组O(1)计算:
 * - bit=0: 新位置 = prefixRank0[pos](前pos位中0的个数)
 * - bit=1: 新位置 = prefixRank1[pos](前pos位中1的个数)
 *
 * @param node 当前节点
 * @param pos 当前层中的位置
 * @param val 待查询的值
 * @param lo 当前值域下界
 * @param hi 当前值域上界
 * @return val在前pos个位置中的出现次数
 */
int WaveletTree4::rankHelper(WTNode* node, int pos, int val, int lo, int hi) const
{
    if (!node || pos <= 0) return 0;
    if (lo == hi) return pos; /* 叶子节点: 所有元素都是val */

    int mid = lo + (hi - lo) / 2;
    int bit = (val <= mid) ? 0 : 1;

    if (bit == 0) {
        /* 左子树: 新位置 = 前pos位中0的个数 */
        int newPos = node->prefixRank0[pos];
        return rankHelper(node->left, newPos, val, lo, mid);
    } else {
        /* 右子树: 新位置 = 前pos位中1的个数 */
        int newPos = node->prefixRank1[pos];
        return rankHelper(node->right, newPos, val, mid + 1, hi);
    }
}

/* ─── 私有方法: Select递归查询 ─── */

/**
 * @brief 递归select查询
 *
 * 步骤:
 * 1. 确定val的路径方向bit
 * 2. 递归获取子树中的位置subPos
 * 3. 在当前节点的位向量中，找到第subPos个bit值的位置
 *    使用前缀和数组进行二分搜索
 *
 * @param node 当前节点
 * @param k 第k次出现(1-based)
 * @param val 待查询的值
 * @param lo 当前值域下界
 * @param hi 当前值域上界
 * @return 位置索引(0-based)，不存在返回-1
 */
int WaveletTree4::selectHelper(WTNode* node, int k, int val, int lo, int hi) const
{
    if (!node || k <= 0) return -1;
    if (lo == hi) return k - 1; /* 叶子节点: 第k个元素的位置 */

    int mid = lo + (hi - lo) / 2;
    int bit = (val <= mid) ? 0 : 1;

    /* 先递归到子树获取子树内位置 */
    int subPos;
    if (bit == 0) {
        subPos = selectHelper(node->left, k, val, lo, mid);
    } else {
        subPos = selectHelper(node->right, k, val, mid + 1, hi);
    }

    if (subPos < 0) return -1;

    /* 在当前位向量中找到第(subPos+1)个bit的位置 */
    int target = subPos + 1; /* 转为1-based计数 */
    int n = node->b.size();

    /* 使用前缀和二分搜索 */
    int left = 0, right = n;
    while (left < right) {
        int m = (left + right) / 2;
        int count = (bit == 0) ? node->prefixRank0[m] : node->prefixRank1[m];
        if (count < target) {
            left = m + 1;
        } else {
            right = m;
        }
    }

    /* 验证找到的位置确实有正确的bit值 */
    if (left <= n && left > 0 && node->b[left - 1] == bit) {
        return left - 1; /* 转回0-based */
    }

    return -1;
}
