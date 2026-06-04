/**
 * @file DataAnnotationWidget.h
 * @brief 数据标注控件 — 数据流标注/书签管理
 *
 * 功能: 对数据流中特定位置添加标注(时间戳+颜色+备注)，支持标注列表管理、
 *       搜索过滤、导入导出、颜色分类。用于标记调试关键事件。
 *
 * 协作: TerminalWidget(右键"添加标注") / DataAnnotationWidget(管理) / RecordingMarker(复用标记逻辑)
 */
#ifndef DATAANNOTATIONWIDGET_H
#define DATAANNOTATIONWIDGET_H

#include <QWidget>
#include <QList>
#include <QColor>
#include <QDateTime>

class QTableWidget;
class QPushButton;
class QLineEdit;
class QComboBox;
class QLabel;
class QTextEdit;

/** @brief 单条数据标注 */
struct DataAnnotation {
    int id = 0;                     ///< 标注ID
    qint64 byteOffset = 0;          ///< 字节偏移量
    qint64 timestamp = 0;           ///< 时间戳(ms since epoch)
    QString title;                  ///< 标注标题
    QString note;                   ///< 详细备注
    QColor color;                   ///< 标注颜色
    int category = 0;               ///< 分类(0=信息/1=警告/2=错误/3=自定义)
    QByteArray contextData;         ///< 上下文数据(前后各N字节)
};

/**
 * @brief 数据标注控件 — 管理数据流中的标注和书签
 */
class DataAnnotationWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 标注分类枚举 */
    enum AnnotationCategory {
        Info = 0,       ///< 信息(蓝)
        Warning = 1,    ///< 警告(黄)
        Error = 2,      ///< 错误(红)
        Custom = 3      ///< 自定义(绿)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalAnnotations = 0;    ///< 累计标注数
        quint64 totalImports = 0;        ///< 累计导入次数
        quint64 totalExports = 0;        ///< 累计导出次数
        quint64 totalSearches = 0;       ///< 累计搜索次数
        quint64 totalEdits = 0;          ///< 累计编辑次数
        int     peakAnnotations = 0;     ///< 峰值标注数
        int     activeAnnotations = 0;   ///< 当前活跃标注数
    };

    explicit DataAnnotationWidget(QWidget* parent = nullptr);

    /** @brief 添加标注 @param annotation 标注数据 @return 标注ID */
    int addAnnotation(const DataAnnotation& annotation);

    /** @brief 移除标注 @param id 标注ID @return 是否成功 */
    bool removeAnnotation(int id);

    /** @brief 更新标注 @param id 标注ID @param annotation 新数据 @return 是否成功 */
    bool updateAnnotation(int id, const DataAnnotation& annotation);

    /** @brief 获取标注 @param id 标注ID @return 标注数据 */
    DataAnnotation annotation(int id) const;

    /** @brief 获取所有标注 @return 标注列表 */
    QList<DataAnnotation> allAnnotations() const;

    /** @brief 搜索标注 @param keyword 关键字 @return 匹配的标注 */
    QList<DataAnnotation> search(const QString& keyword) const;

    /** @brief 按分类过滤 @param category 分类 @return 过滤后的标注 */
    QList<DataAnnotation> filterByCategory(int category) const;

    /** @brief 导出标注到JSON @param filePath 目标路径 @return 是否成功 */
    bool exportToJson(const QString& filePath);

    /** @brief 从JSON导入标注 @param filePath JSON文件路径 @return 是否成功 */
    bool importFromJson(const QString& filePath);

    /** @brief 清除所有标注 */
    void clear();

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 标注已添加 @param id 标注ID */
    void annotationAdded(int id);
    /** @brief 标注已移除 @param id 标注ID */
    void annotationRemoved(int id);
    /** @brief 标注已更新 @param id 标注ID */
    void annotationUpdated(int id);
    /** @brief 标注被选中 @param id 标注ID */
    void annotationSelected(int id);

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onEditClicked();
    void onExportClicked();
    void onImportClicked();
    void onSearchChanged(const QString& text);
    void onCategoryFilterChanged(int index);
    void onRowClicked(int row, int col);

private:
    void setupUI();
    void refreshTable(const QList<DataAnnotation>& annotations);
    QString formatTimestamp(qint64 ms) const;
    QColor categoryColor(int category) const;
    QString categoryName(int category) const;
    int nextId();

    QTableWidget* m_table;         ///< 标注列表表格
    QPushButton* m_addBtn;         ///< 添加按钮
    QPushButton* m_removeBtn;      ///< 移除按钮
    QPushButton* m_editBtn;        ///< 编辑按钮
    QPushButton* m_exportBtn;      ///< 导出按钮
    QPushButton* m_importBtn;      ///< 导入按钮
    QLineEdit* m_searchEdit;       ///< 搜索框
    QComboBox* m_categoryCombo;    ///< 分类过滤
    QLabel* m_countLabel;          ///< 计数标签
    QTextEdit* m_detailEdit;       ///< 详情显示

    QList<DataAnnotation> m_annotations; ///< 标注列表
    int m_nextId;                        ///< 下一个ID
    int m_selectedId;                    ///< 当前选中标注ID

    Stats m_stats;                       ///< 运行统计
};

#endif // DATAANNOTATIONWIDGET_H
