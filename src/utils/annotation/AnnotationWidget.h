/**
 * @file AnnotationWidget.h
 * @brief 数据标注工具控件 — 时间轴 + 标注列表 + 工具栏
 *
 * 提供完整的标注 UI：顶部自定义绘制的时间轴（点击添加标注），中部 QTableWidget
 * 显示标注列表（ID/类型/位置/标签/分类），底部工具栏（添加/编辑/删除/撤销/重做）。
 * 双击表格行可编辑标注，通过 AnnotationManager 管理数据。
 */

#ifndef ANNOTATION_WIDGET_H
#define ANNOTATION_WIDGET_H

#include <QWidget>
#include <QList>
#include "utils/annotation/AnnotationTypes.h"

class AnnotationManager;
class QTableWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;

/**
 * @brief 数据标注工具控件 — 时间轴 + 列表 + 工具栏
 */
class AnnotationWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalAdds        = 0;  ///< 累计添加操作
        quint64 totalEdits       = 0;  ///< 累计编辑操作
        quint64 totalDeletes     = 0;  ///< 累计删除操作
        quint64 totalTimelineClicks = 0;  ///< 时间轴点击次数
        quint64 totalTableDblClicks = 0;  ///< 表格双击次数
        quint64 totalRefreshes   = 0;  ///< 表格刷新次数
    };

    /** @brief 构造控件 @param manager 标注管理器 @param parent 父控件 */
    explicit AnnotationWidget(AnnotationManager* manager,
                               QWidget* parent = nullptr);

    /** @brief 设置时间轴的数据范围 @param minPos 起始位置 @param maxPos 结束位置 */
    void setDataRange(qint64 minPos, qint64 maxPos);

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计计数器 */
    void resetStatistics();

signals:
    /** @brief 标注被选中 @param id 标注 ID */
    void annotationClicked(const QString& id);
    /** @brief 时间轴位置被点击 @param pos 点击位置 */
    void timelineClicked(qint64 pos);

protected:
    /** @brief 时间轴绘制事件 */
    void paintEvent(QPaintEvent* event) override;
    /** @brief 时间轴鼠标点击 */
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onAddClicked();
    void onEditClicked();
    void onRemoveClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onExportClicked();
    void onSearchChanged(const QString& text);
    void onFilterChanged(int index);
    void onTableDoubleClicked(int row, int col);
    void onAnnotationAdded(const DataAnnotation& a);
    void onAnnotationRemoved(const QString& id);
    void onAnnotationUpdated(const DataAnnotation& a);

private:
    void setupUI();
    void refreshTable();
    void addTableRow(const DataAnnotation& a);
    void updateTableRow(int row, const DataAnnotation& a);
    int  findRowById(const QString& id) const;
    QString typeDisplayName(AnnotationType type) const;

    AnnotationManager* m_manager;      ///< 标注管理器（非拥有）
    QTableWidget*      m_table;        ///< 标注列表
    QPushButton*       m_addBtn;       ///< 添加按钮
    QPushButton*       m_editBtn;      ///< 编辑按钮
    QPushButton*       m_removeBtn;    ///< 删除按钮
    QPushButton*       m_undoBtn;      ///< 撤销按钮
    QPushButton*       m_redoBtn;      ///< 重做按钮
    QPushButton*       m_exportBtn;    ///< 导出按钮
    QLineEdit*         m_searchEdit;   ///< 搜索框
    QComboBox*         m_filterCombo;  ///< 类型过滤
    QLabel*            m_countLabel;   ///< 计数标签

    qint64 m_timelineMin = 0;          ///< 时间轴起始位置
    qint64 m_timelineMax = 1000;       ///< 时间轴结束位置
    int    m_timelineHeight = 40;      ///< 时间轴高度
    QString m_selectedId;              ///< 当前选中标注 ID

    Stats m_stats;                     ///< 运行统计
};

#endif // ANNOTATION_WIDGET_H
