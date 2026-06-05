/**
 * @file SvdViewerWidgetStats.cpp
 * @brief SVD查看器 — 统计getter和重置方法实现
 *
 * 从 SvdViewerWidget.cpp 拆分而来，包含:
 *   - 所有统计计数器getter方法
 *   - resetStatistics() (含子组件统计重置)
 *   - SvdRegisterTreeModel::resetStatistics()
 *   - SvdBitFieldWidget::resetStatistics()
 */

#include "protocol/svd/SvdViewerWidget.h"
#include "protocol/svd/SvdRegisterTreeModel.h"
#include "protocol/svd/SvdBitFieldWidget.h"

// ============================================================================
// SvdViewerWidget 统计接口
// ============================================================================

/** @brief 获取累计展开操作次数 @return 展开总次数 */
quint64 SvdViewerWidget::totalExpandCount() const
{
    return m_totalExpandCount;
}

/** @brief 获取累计折叠操作次数 @return 折叠总次数 */
quint64 SvdViewerWidget::totalCollapseCount() const
{
    return m_totalCollapseCount;
}

/** @brief 获取累计搜索操作次数 @return 搜索总次数 */
quint64 SvdViewerWidget::totalSearchCount() const
{
    return m_totalSearchCount;
}

/** @brief 获取累计加载SVD文件次数 @return 文件加载总次数 */
quint64 SvdViewerWidget::totalFileLoads() const
{
    return m_totalFileLoads;
}

/** @brief 获取累计字段悬停次数 @return 悬停总次数 */
quint64 SvdViewerWidget::totalFieldHoverCount() const
{
    return m_bitFieldWidget
        ? m_bitFieldWidget->totalFieldHoverCount() : 0;
}

/** @brief 获取累计字段点击次数 @return 点击总次数 */
quint64 SvdViewerWidget::totalFieldClickCount() const
{
    return m_bitFieldWidget
        ? m_bitFieldWidget->totalFieldClickCount() : 0;
}

/** @brief 重置所有统计计数器(含子组件SvdRegisterTreeModel和SvdBitFieldWidget) */
void SvdViewerWidget::resetStatistics()
{
    m_totalExpandCount = 0;
    m_totalCollapseCount = 0;
    m_totalSearchCount = 0;
    m_totalFileLoads = 0;

    if (m_treeModel) {
        m_treeModel->resetStatistics();
    }
    if (m_bitFieldWidget) {
        m_bitFieldWidget->resetStatistics();
    }
}

// ============================================================================
// SvdRegisterTreeModel 统计重置
// ============================================================================

/** @brief 重置SVD寄存器树模型所有统计计数器 */
void SvdRegisterTreeModel::resetStatistics()
{
    m_totalExpandCount = 0;
    m_totalCollapseCount = 0;
    m_totalSearchCount = 0;
    m_totalDeviceLoads = 0;
}

// ============================================================================
// SvdBitFieldWidget 统计重置
// ============================================================================

/** @brief 重置位字段可视化控件所有统计计数器 */
void SvdBitFieldWidget::resetStatistics()
{
    m_totalFieldHoverCount = 0;
    m_totalFieldClickCount = 0;
    m_totalPaintCount = 0;
}
