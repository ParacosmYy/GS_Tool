/**
 * @file TerminalExportOptions.cpp
 * @brief 终端导出路径与格式规范化实现
 */

#include "core/terminal/TerminalExportOptions.h"

#include <QFileInfo>

namespace {
DataExporter::Format formatFromExtension(const QString& suffix)
{
    const QString ext = suffix.toLower();
    if (ext == QStringLiteral("csv")) {
        return DataExporter::Csv;
    }
    if (ext == QStringLiteral("bin")) {
        return DataExporter::Bin;
    }
    return DataExporter::Plain;
}

QString extensionFromFilter(const QString& selectedFilter)
{
    if (selectedFilter.contains(QStringLiteral("*.csv"), Qt::CaseInsensitive)) {
        return QStringLiteral("csv");
    }
    if (selectedFilter.contains(QStringLiteral("*.bin"), Qt::CaseInsensitive)) {
        return QStringLiteral("bin");
    }
    return QStringLiteral("txt");
}
} // namespace

TerminalExportOptions normalizedTerminalExportOptions(const QString& filePath,
                                                      const QString& selectedFilter)
{
    QFileInfo info(filePath);
    const QString suffix = info.suffix();
    if (!suffix.isEmpty()) {
        return {filePath, formatFromExtension(suffix)};
    }

    const QString extension = extensionFromFilter(selectedFilter);
    return {filePath + QLatin1Char('.') + extension, formatFromExtension(extension)};
}
