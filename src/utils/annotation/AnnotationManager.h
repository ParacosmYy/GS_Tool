/**
 * @file AnnotationManager.h
 * @brief 数据标注管理器 — CRUD、搜索过滤、导入导出、撤销重做
 *
 * 管理数据流标注的完整生命周期：添加/移除/更新标注、按类型/分类/时间范围
 * 过滤、重叠检测、JSON 持久化、CSV 导出、撤销/重做栈（最近 50 步）。
 *
 * 协作: AnnotationWidget(UI) → AnnotationManager(数据) → AnnotationTypes(类型)
 */

#ifndef ANNOTATION_MANAGER_H
#define ANNOTATION_MANAGER_H

#include <QObject>
#include <QList>
#include "utils/annotation/AnnotationTypes.h"

class QJsonDocument;

/**
 * @brief 数据标注管理器 — 数据流标注 CRUD + 搜索 + 持久化
 */
class AnnotationManager : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalAdded    = 0;  ///< 累计添加次数
        quint64 totalRemoved  = 0;  ///< 累计删除次数
        quint64 totalUpdated  = 0;  ///< 累计更新次数
        quint64 totalSearches = 0;  ///< 累计搜索次数
        quint64 totalExports  = 0;  ///< 累计导出次数
        quint64 totalImports  = 0;  ///< 累计导入次数
        int     peakCount     = 0;  ///< 峰值标注数
        int     activeCount   = 0;  ///< 当前活跃标注数
    };

    /** @brief 构造管理器 @param parent 父对象 */
    explicit AnnotationManager(QObject* parent = nullptr);

    /** @brief 添加标注 @param annotation 标注数据 @return 添加后的标注（含生成的 id） */
    DataAnnotation addAnnotation(const DataAnnotation& annotation);

    /** @brief 移除标注 @param id 标注 ID @return 是否成功 */
    bool removeAnnotation(const QString& id);

    /** @brief 更新标注 @param id 标注 ID @param annotation 新数据 @return 是否成功 */
    bool updateAnnotation(const QString& id, const DataAnnotation& annotation);

    /** @brief 获取标注 @param id 标注 ID @return 标注数据（未找到则 id 为空） */
    DataAnnotation annotation(const QString& id) const;

    /** @brief 获取所有标注 @return 标注列表 */
    QList<DataAnnotation> allAnnotations() const;

    /** @brief 按类型过滤 @param type 标注类型 @return 匹配的标注 */
    QList<DataAnnotation> filterByType(AnnotationType type) const;

    /** @brief 按分类过滤 @param category 分类名称 @return 匹配的标注 */
    QList<DataAnnotation> filterByCategory(const QString& category) const;

    /** @brief 按时间范围过滤 @param from 起始时间 @param to 结束时间 @return 匹配的标注 */
    QList<DataAnnotation> filterByTimeRange(
        const QDateTime& from, const QDateTime& to) const;

    /** @brief 关键字搜索 @param keyword 关键字 @return 匹配的标注 */
    QList<DataAnnotation> search(const QString& keyword) const;

    /** @brief 检测与指定标注重叠的标注 @param annotation 目标注 @return 重叠标注列表 */
    QList<DataAnnotation> findOverlaps(const DataAnnotation& annotation) const;

    /** @brief 保存标注到 JSON 文件 @param filePath 文件路径 @return 是否成功 */
    bool saveToJson(const QString& filePath) const;

    /** @brief 从 JSON 文件加载标注 @param filePath 文件路径 @return 是否成功 */
    bool loadFromJson(const QString& filePath);

    /** @brief 导出标注为 CSV @param filePath 文件路径 @return 是否成功 */
    bool exportToCsv(const QString& filePath) const;

    /** @brief 撤销上一步操作 @return 是否成功 */
    bool undo();

    /** @brief 重做上一步撤销 @return 是否成功 */
    bool redo();

    /** @brief 判断是否可撤销 */
    bool canUndo() const;

    /** @brief 判断是否可重做 */
    bool canRedo() const;

    /** @brief 清除所有标注（不可撤销） */
    void clear();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 标注已添加 @param annotation 添加后的标注 */
    void annotationAdded(const DataAnnotation& annotation);
    /** @brief 标注已移除 @param id 被移除的标注 ID */
    void annotationRemoved(const QString& id);
    /** @brief 标注已更新 @param annotation 更新后的标注 */
    void annotationUpdated(const DataAnnotation& annotation);

private:
    void pushUndo(AnnotationAction::Kind action,
                  const DataAnnotation& before,
                  const DataAnnotation& after);

    QList<DataAnnotation> m_annotations;    ///< 标注列表
    QList<AnnotationAction> m_undoStack;    ///< 撤销栈（最多 50 条）
    QList<AnnotationAction> m_redoStack;    ///< 重做栈
    Stats m_stats;                          ///< 运行统计
};

#endif // ANNOTATION_MANAGER_H
