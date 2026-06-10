/**
 * @file TerminalExportOptions.h
 * @brief 终端导出路径与格式规范化工具
 */
#ifndef TERMINALEXPORTOPTIONS_H
#define TERMINALEXPORTOPTIONS_H

#include "utils/export/DataExporter.h"

#include <QString>

/** @brief 终端导出最终选项 */
struct TerminalExportOptions {
    QString filePath;             ///< 规范化后的导出路径，必要时自动追加扩展名
    DataExporter::Format format;  ///< 根据扩展名或过滤器推断出的导出格式
};

/**
 * @brief 根据用户路径和文件对话框过滤器推断最终导出路径与格式
 * @param filePath 用户在保存对话框中输入或选择的路径
 * @param selectedFilter QFileDialog 返回的选中过滤器文本
 * @return 规范化后的路径和导出格式
 */
TerminalExportOptions normalizedTerminalExportOptions(const QString& filePath,
                                                      const QString& selectedFilter);

#endif // TERMINALEXPORTOPTIONS_H
