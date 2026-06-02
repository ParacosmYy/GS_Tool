/**
 * @file ChartExporter.h
 * @brief 图表导出器，支持将图表数据导出为 CSV/PNG/SVG 格式
 *
 * 提供 F3 多通道数据导出子系统的核心导出能力，
 * 将 ChartModel 数据或 QWidget 渲染结果导出为多种文件格式。
 */

#ifndef CHART_EXPORTER_H
#define CHART_EXPORTER_H

#include <QObject>
#include <QString>

class ChartModel;
class QWidget;

/**
 * @class ChartExporter
 * @brief 图表导出类
 *
 * 无状态导出工具，通过静态方法将图表数据或渲染结果
 * 导出为 CSV（数据）、PNG（位图）或 SVG（矢量图）格式。
 */
class ChartExporter : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit ChartExporter(QObject* parent = nullptr);

    /**
     * @brief 导出图表数据为 CSV 文件（ChartModel 版本）
     * @param filePath 目标文件路径
     * @param model 图表数据模型指针
     * @return true 导出成功
     */
    bool exportToCsv(const QString& filePath, ChartModel* model);

    /**
     * @brief 导出多通道数据为 CSV 文件
     * @param filePath 目标文件路径
     * @param channelNames 通道名称列表，如 {"CH1", "CH2"}
     * @param data 各通道数据列表，外层为通道，内层为采样点
     * @return true 导出成功
     */
    bool exportToCsv(const QString& filePath,
                     const QStringList& channelNames,
                     const QList<QList<double>>& data);

    /**
     * @brief 将控件渲染为 PNG 图片
     * @param filePath 目标文件路径
     * @param widget 待渲染的控件指针
     * @return true 导出成功
     */
    bool exportToPng(const QString& filePath, QWidget* widget);

    /**
     * @brief 将控件渲染为 SVG 矢量图
     * @param filePath 目标文件路径
     * @param widget 待渲染的控件指针
     * @return true 导出成功
     */
    bool exportToSvg(const QString& filePath, QWidget* widget);

    /**
     * @brief 导出多通道数据为 JSON 文件
     * @param filePath 目标文件路径
     * @param channelNames 通道名称列表
     * @param data 各通道数据列表
     * @return true 导出成功
     */
    bool exportToJson(const QString& filePath,
                      const QStringList& channelNames,
                      const QList<QList<double>>& data);

signals:
    /**
     * @brief 导出完成信号
     * @param filePath 导出文件路径
     */
    void exportCompleted(const QString& filePath);

    /**
     * @brief 导出失败信号
     * @param error 失败原因描述
     */
    void exportFailed(const QString& error);
};

#endif // CHART_EXPORTER_H
