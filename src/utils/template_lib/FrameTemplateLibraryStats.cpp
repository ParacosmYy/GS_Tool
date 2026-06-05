/**
 * @file FrameTemplateLibraryStats.cpp
 * @brief 帧模板库统计接口 — 重置计数器
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/template_lib/FrameTemplateLibrary.h"

/**
 * @brief 重置所有统计计数器
 *
 * 将模板总数、帧构建数、导入导出次数、分类数、
 * 最大字段数全部归零。
 */
void FrameTemplateLibrary::resetStatistics()
{
    m_stats.totalTemplates = 0;
    m_stats.totalFramesBuilt = 0;
    m_stats.totalImports = 0;
    m_stats.totalExports = 0;
    m_stats.totalCategories = 0;
    m_stats.maxFieldsInTemplate = 0;
}
