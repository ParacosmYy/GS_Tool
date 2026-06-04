/** @file ProtocolView.h @brief 协议解析结果展示 - 表格形式展示解析帧数据，支持字段着色/错误行标红/列宽自适应/动态列/右键菜单 */
#ifndef PROTOCOLVIEW_H
#define PROTOCOLVIEW_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

/** @brief 数值字段着色范围配置 -- 定义正常/警告/错误三个区间的边界值 */
struct FieldColorRange {
    double warnLow = -1e9;    ///< 警告下限(低于此值为错误)
    double normalLow = 0.0;   ///< 正常下限
    double normalHigh = 100.0; ///< 正常上限
    double warnHigh = 1e9;    ///< 警告上限(高于此值为错误)
};

/**
 * @brief 协议解析结果表格视图
 *
 * 以表格展示解析帧数据: 动态列管理/数值着色/错误行高亮/列宽自适应/CSV+JSON导出/右键菜单
 * 协作: FrameParser(解析信号) / ThemeManager(语义色板) / HexConverter(HEX格式化)
 */
class ProtocolView : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造协议帧视图 @param parent 父控件指针 */
    explicit ProtocolView(QWidget* parent = nullptr);
    /** @brief 添加一帧解析结果到表格 @param fields 帧字段名值对 */
    void addFrame(const QVariantMap& fields);
    /** @brief 清空所有解析结果 */
    void clear();
    /** @brief 设置最大显示行数(旧数据自动丢弃) @param max 最大行数 */
    void setMaxRows(int max);
    /** @brief 获取当前表格行数 @return 行数 */
    int rowCount() const;
    /** @brief 获取所有解析结果(用于导出) @return QVariantMap列表 */
    QList<QVariantMap> allFrames() const;
    /** @brief 设置数值字段着色范围 @param fieldName 字段名 @param range 着色范围 */
    void setFieldColorRange(const QString& fieldName, const FieldColorRange& range);

public slots:
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame); ///< 帧解析成功
    void onFrameError(const QString& reason, const QByteArray& rawFrame);     ///< 帧解析错误

private slots:
    void onCustomContextMenu(const QPoint& pos); ///< 右键上下文菜单弹出

private:
    void setupUI();                 ///< 初始化UI布局
    void setupContextMenu();        ///< 初始化右键上下文菜单
    void updateColumnHeaders(const QVariantMap& fields); ///< 动态更新列头
    // ---- 右键菜单操作 ----
    void copyRow();                 ///< 复制选中行(制表符分隔)
    void copyRaw();                 ///< 复制选中帧的原始HEX数据
    void exportJson();              ///< 导出所有帧为JSON文件
    /** @brief 根据字段值返回着色QStandardItem(正常/警告/错误色) */
    QStandardItem* createColoredItem(const QString& fieldName, const QString& value);
    void autoResizeColumns();       ///< 自动调整所有列宽
    // ---- UI组件 ----
    QTableView* m_table;            ///< 数据表格视图
    QStandardItemModel* m_model;    ///< 表格数据模型
    QLabel* m_statusLabel;          ///< 状态栏标签(帧数/错误数)
    QPushButton* m_clearBtn;        ///< 清空按钮
    QPushButton* m_exportBtn;       ///< 导出按钮
    // ---- 右键菜单 ----
    QMenu* m_contextMenu;           ///< 右键上下文菜单
    QAction* m_copyRowAction;       ///< 复制行
    QAction* m_copyRawAction;       ///< 复制原始数据
    QAction* m_exportJsonAction;    ///< 导出JSON
    QAction* m_clearAction;         ///< 清空
    // ---- 数据 ----
    int m_maxRows = 1000;           ///< 最大显示行数
    quint64 m_totalFrames = 0;      ///< 总帧数(含已丢弃)
    quint64 m_totalErrors = 0;      ///< 总错误数
    QStringList m_fieldNames;       ///< 动态列名列表(不含内部字段)
    QList<QVariantMap> m_frames;    ///< 所有帧数据(用于导出)
    // ---- 着色配置 ----
    QMap<QString, FieldColorRange> m_colorRanges; ///< 字段名->着色范围
    // ---- 统计计数器 ----
    quint64 m_totalFramesDisplayed = 0;    ///< 已展示帧总数
    quint64 m_totalExports = 0;            ///< 导出操作总次数
    quint64 m_totalContextMenuActions = 0; ///< 右键菜单操作总次数
    quint64 m_totalRowsPruned = 0;         ///< 累计因maxRows限制而丢弃的行数
    quint64 m_totalColorRangeMatches = 0;  ///< 累计数值着色匹配次数(警告/错误区间)
    static constexpr int kFixedColumns = 2; ///< 固定列数: 序号(#)+时间(Time)
public:
    /** @brief 获取已展示帧总数 @return 累计展示帧计数 */
    quint64 totalFramesDisplayed() const { return m_totalFramesDisplayed; }
    /** @brief 获取导出操作总次数 @return 导出计数 */
    quint64 totalExports() const { return m_totalExports; }
    /** @brief 获取右键菜单操作总次数 @return 菜单操作计数 */
    quint64 totalContextMenuActions() const { return m_totalContextMenuActions; }
    /** @brief 获取因maxRows限制累计丢弃的行数 @return 丢弃行计数 */
    quint64 totalRowsPruned() const { return m_totalRowsPruned; }
    /** @brief 获取累计数值着色匹配次数(警告/错误区间) @return 着色计数 */
    quint64 totalColorRangeMatches() const { return m_totalColorRangeMatches; }
    /** @brief 重置协议视图统计计数器 */
    void resetViewStatistics();
};

#endif // PROTOCOLVIEW_H
