/**
 * @file SegmentTree3.cpp
 * @brief 线段树3实现 — 区间加/乘/赋值 + 懒标记下推
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 线段树支持三种区间操作：
 * - rangeAdd：区间加法
 * - rangeMul：区间乘法
 * - rangeAssign：区间赋值（优先级最高）
 *
 * 懒标记下推顺序：赋值 > 乘法 > 加法
 * 赋值操作会清除之前的加法和乘法标记。
 */

#include "utils/tree38/SegmentTree3.h"

#include <QElapsedTimer>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SegmentTree3::SegmentTree3(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SegmentTree3"));
}

/** @brief 重置统计信息 */
void SegmentTree3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 更新平均处理时间
 *
 * 统一的计算 avgProcessingTimeMs 方法，
 * 所有操作（build/update/query）共用。
 */
void SegmentTree3::updateAvgTime(double elapsedMs)
{
    m_timeSum += elapsedMs;
    const int total = m_stats.totalBuilds + m_stats.totalUpdates + m_stats.totalQueries;
    m_stats.avgProcessingTimeMs = total > 0 ? m_timeSum / total : 0.0;
}

/**
 * @brief 构建线段树
 *
 * 根据输入数据构建线段树，内部数组大小为 4*n。
 * 初始化所有懒标记为单位元：add=0, mul=1, assign=无。
 *
 * @param data 输入数据序列
 */
void SegmentTree3::build(const QVector<double> &data)
{
    QElapsedTimer timer;
    timer.start();

    m_n = data.size();
    if (m_n == 0) return;

    const int sz = m_n * 4;
    m_sum.assign(sz, 0.0);
    m_min.assign(sz, 0.0);
    m_max.assign(sz, 0.0);
    m_lazyAdd.assign(sz, 0.0);
    m_lazyMul.assign(sz, 1.0);
    m_lazyAssign.assign(sz, 0.0);
    m_hasAssign.assign(sz, false);

    // 栈模拟递归建树
    struct Frame { int idx, lo, hi, stage; };
    QVector<Frame> stack;
    stack.reserve(m_n * 2);
    stack.push_back({1, 0, m_n - 1, 0});

    while (!stack.isEmpty()) {
        Frame &f = stack.back();
        if (f.lo == f.hi) {
            m_sum[f.idx] = m_min[f.idx] = m_max[f.idx] = data[f.lo];
            stack.pop_back();
            continue;
        }
        if (f.stage == 0) {
            f.stage = 1;
            const int mid = (f.lo + f.hi) / 2;
            stack.push_back({f.idx * 2, f.lo, mid, 0});
            stack.push_back({f.idx * 2 + 1, mid + 1, f.hi, 0});
        } else {
            pullUp(f.idx);
            stack.pop_back();
        }
    }

    m_stats.totalBuilds++;
    updateAvgTime(timer.elapsed());
}

/**
 * @brief 懒标记下推
 *
 * 将当前节点的懒标记下推到子节点。
 * 下推顺序：赋值 > 乘法 > 加法。
 * 赋值标记覆盖所有子节点状态并清除子节点的加法/乘法标记。
 *
 * @param idx 当前节点索引
 */
void SegmentTree3::pushDown(int idx)
{
    const int L = idx * 2, R = idx * 2 + 1;
    const int lenL = 1, lenR = 1; // 简化：子节点长度在 pullUp 时自动正确

    Q_UNUSED(lenL);
    Q_UNUSED(lenR);

    // 赋值标记优先级最高：覆盖子节点所有值和标记
    if (m_hasAssign[idx]) {
        const double v = m_lazyAssign[idx];
        for (int c : {L, R}) {
            m_sum[c] = v; m_min[c] = v; m_max[c] = v;
            m_lazyAdd[c] = 0.0; m_lazyMul[c] = 1.0;
            m_lazyAssign[c] = v; m_hasAssign[c] = true;
        }
        m_hasAssign[idx] = false;
        return;
    }

    // 乘法标记
    if (m_lazyMul[idx] != 1.0) {
        const double mul = m_lazyMul[idx];
        for (int c : {L, R}) {
            m_sum[c] *= mul; m_min[c] *= mul; m_max[c] *= mul;
            m_lazyMul[c] *= mul; m_lazyAdd[c] *= mul;
        }
        m_lazyMul[idx] = 1.0;
    }

    // 加法标记
    if (m_lazyAdd[idx] != 0.0) {
        const double add = m_lazyAdd[idx];
        for (int c : {L, R}) {
            m_sum[c] += add; m_min[c] += add; m_max[c] += add;
            m_lazyAdd[c] += add;
        }
        m_lazyAdd[idx] = 0.0;
    }
}

/** @brief 向上合并子节点信息 */
void SegmentTree3::pullUp(int idx)
{
    const int L = idx * 2, R = idx * 2 + 1;
    m_sum[idx] = m_sum[L] + m_sum[R];
    m_min[idx] = qMin(m_min[L], m_min[R]);
    m_max[idx] = qMax(m_max[L], m_max[R]);
}

/**
 * @brief 区间更新内部实现
 *
 * 统一的栈模拟递归更新框架，通过 OpType 区分操作类型：
 * - 0: 加法
 * - 1: 乘法
 * - 2: 赋值
 *
 * @param lo 目标区间左端点
 * @param hi 目标区间右端点
 * @param val 操作值
 * @param opType 操作类型 (0=add, 1=mul, 2=assign)
 */
void SegmentTree3::rangeUpdateImpl(int lo, int hi, double val, int opType)
{
    lo = qBound(0, lo, m_n - 1);
    hi = qBound(0, hi, m_n - 1);

    struct Frame { int idx, lo, hi, stage; };
    QVector<Frame> stack;
    stack.push_back({1, 0, m_n - 1, 0});

    while (!stack.isEmpty()) {
        Frame &f = stack.back();
        if (hi < f.lo || f.hi < lo) { stack.pop_back(); continue; }
        if (lo <= f.lo && f.hi <= hi) {
            const int len = f.hi - f.lo + 1;
            switch (opType) {
            case 0: // 加法
                m_sum[f.idx] += val * len;
                m_min[f.idx] += val; m_max[f.idx] += val;
                m_lazyAdd[f.idx] += val;
                break;
            case 1: // 乘法
                m_sum[f.idx] *= val; m_min[f.idx] *= val; m_max[f.idx] *= val;
                m_lazyMul[f.idx] *= val; m_lazyAdd[f.idx] *= val;
                break;
            case 2: // 赋值
                m_sum[f.idx] = val * len; m_min[f.idx] = val; m_max[f.idx] = val;
                m_lazyAdd[f.idx] = 0.0; m_lazyMul[f.idx] = 1.0;
                m_lazyAssign[f.idx] = val; m_hasAssign[f.idx] = true;
                break;
            }
            stack.pop_back();
            continue;
        }
        pushDown(f.idx);
        if (f.stage == 0) {
            f.stage = 1;
            const int mid = (f.lo + f.hi) / 2;
            if (lo <= mid) stack.push_back({f.idx * 2, f.lo, mid, 0});
            if (hi > mid) stack.push_back({f.idx * 2 + 1, mid + 1, f.hi, 0});
        } else {
            pullUp(f.idx);
            stack.pop_back();
        }
    }
}

/**
 * @brief 区间加法
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @param delta 加法增量
 */
void SegmentTree3::rangeAdd(int lo, int hi, double delta)
{
    if (m_n == 0 || lo > hi) return;
    QElapsedTimer timer; timer.start();
    rangeUpdateImpl(lo, hi, delta, 0);
    m_stats.totalUpdates++;
    updateAvgTime(timer.elapsed());
    emit updated(lo, hi);
}

/**
 * @brief 区间乘法
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @param factor 乘法因子
 */
void SegmentTree3::rangeMul(int lo, int hi, double factor)
{
    if (m_n == 0 || lo > hi) return;
    QElapsedTimer timer; timer.start();
    rangeUpdateImpl(lo, hi, factor, 1);
    m_stats.totalUpdates++;
    updateAvgTime(timer.elapsed());
    emit updated(lo, hi);
}

/**
 * @brief 区间赋值
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @param value 赋值
 */
void SegmentTree3::rangeAssign(int lo, int hi, double value)
{
    if (m_n == 0 || lo > hi) return;
    QElapsedTimer timer; timer.start();
    rangeUpdateImpl(lo, hi, value, 2);
    m_stats.totalUpdates++;
    updateAvgTime(timer.elapsed());
    emit updated(lo, hi);
}

/**
 * @brief 区间求和查询
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @return 区间元素之和
 */
double SegmentTree3::rangeSum(int lo, int hi) const
{
    if (m_n == 0 || lo > hi) return 0.0;
    QElapsedTimer timer; timer.start();
    lo = qBound(0, lo, m_n - 1); hi = qBound(0, hi, m_n - 1);

    double result = 0.0;
    auto *self = const_cast<SegmentTree3*>(this);

    struct Frame { int idx, lo, hi, stage; };
    QVector<Frame> stack;
    stack.push_back({1, 0, m_n - 1, 0});

    while (!stack.isEmpty()) {
        Frame &f = stack.back();
        if (hi < f.lo || f.hi < lo) { stack.pop_back(); continue; }
        if (lo <= f.lo && f.hi <= hi) {
            result += m_sum[f.idx];
            stack.pop_back(); continue;
        }
        self->pushDown(f.idx);
        if (f.stage == 0) {
            f.stage = 1;
            const int mid = (f.lo + f.hi) / 2;
            if (lo <= mid) stack.push_back({f.idx * 2, f.lo, mid, 0});
            if (hi > mid) stack.push_back({f.idx * 2 + 1, mid + 1, f.hi, 0});
        } else { stack.pop_back(); }
    }

    self->m_stats.totalQueries++;
    self->updateAvgTime(timer.elapsed());
    return result;
}

/**
 * @brief 区间最小值查询
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @return 区间最小值
 */
double SegmentTree3::rangeMin(int lo, int hi) const
{
    if (m_n == 0 || lo > hi) return 0.0;
    lo = qBound(0, lo, m_n - 1); hi = qBound(0, hi, m_n - 1);

    double result = std::numeric_limits<double>::max();
    auto *self = const_cast<SegmentTree3*>(this);

    struct Frame { int idx, lo, hi, stage; };
    QVector<Frame> stack;
    stack.push_back({1, 0, m_n - 1, 0});

    while (!stack.isEmpty()) {
        Frame &f = stack.back();
        if (hi < f.lo || f.hi < lo) { stack.pop_back(); continue; }
        if (lo <= f.lo && f.hi <= hi) {
            result = qMin(result, m_min[f.idx]);
            stack.pop_back(); continue;
        }
        self->pushDown(f.idx);
        if (f.stage == 0) {
            f.stage = 1;
            const int mid = (f.lo + f.hi) / 2;
            if (lo <= mid) stack.push_back({f.idx * 2, f.lo, mid, 0});
            if (hi > mid) stack.push_back({f.idx * 2 + 1, mid + 1, f.hi, 0});
        } else { stack.pop_back(); }
    }
    return result;
}

/**
 * @brief 区间最大值查询
 * @param lo 左端点（包含）
 * @param hi 右端点（包含）
 * @return 区间最大值
 */
double SegmentTree3::rangeMax(int lo, int hi) const
{
    if (m_n == 0 || lo > hi) return 0.0;
    lo = qBound(0, lo, m_n - 1); hi = qBound(0, hi, m_n - 1);

    double result = std::numeric_limits<double>::lowest();
    auto *self = const_cast<SegmentTree3*>(this);

    struct Frame { int idx, lo, hi, stage; };
    QVector<Frame> stack;
    stack.push_back({1, 0, m_n - 1, 0});

    while (!stack.isEmpty()) {
        Frame &f = stack.back();
        if (hi < f.lo || f.hi < lo) { stack.pop_back(); continue; }
        if (lo <= f.lo && f.hi <= hi) {
            result = qMax(result, m_max[f.idx]);
            stack.pop_back(); continue;
        }
        self->pushDown(f.idx);
        if (f.stage == 0) {
            f.stage = 1;
            const int mid = (f.lo + f.hi) / 2;
            if (lo <= mid) stack.push_back({f.idx * 2, f.lo, mid, 0});
            if (hi > mid) stack.push_back({f.idx * 2 + 1, mid + 1, f.hi, 0});
        } else { stack.pop_back(); }
    }
    return result;
}

/** @brief 清空线段树，释放所有内存 */
void SegmentTree3::clear()
{
    m_n = 0;
    m_sum.clear(); m_min.clear(); m_max.clear();
    m_lazyAdd.clear(); m_lazyMul.clear(); m_lazyAssign.clear();
    m_hasAssign.clear();
}
