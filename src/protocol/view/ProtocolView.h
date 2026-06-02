/**
 * @file ProtocolView.h
 * @brief 协议解析结果展示 - 以表格形式展示解析出的帧数据
 *
 * 增强特性:
 *   - 字段值着色: 数值字段按范围变色(正常/警告/错误)
 *   - 错误行标红: 解析失败的帧以错误色高亮显示
 *   - 列宽自动调整: 根据内容自动适配列宽
 *   - 动态列管理: 字段名自动创建对应列
 *   - 右键上下文菜单: 复制行/复制原始数据/导出JSON
 *
 * 每行一帧，列 = 序号(#) + 时间(Time) + 各字段名
 * 数据源: FrameParser通过onFrameParsed/onFrameError信号推送
 *
 * 协作关系:
 *   - FrameParser: 解析成功/失败时调用addFrame/onFrameError
 *   - ThemeManager: 提供语义色板(正常/警告/错误色)
 *   - HexConverter: 原始数据HEX格式化
 *
 * 设计模式: 观察者模式(监听FrameParser的解析信号)
 */
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

/**
 * @brief 数值字段着色范围配置
 *
 * 定义正常/警告/错误三个区间的边界值，
 * ProtocolView根据字段值所在区间选择对应颜色。
 * 所有数值均为绝对值范围。
 */
struct FieldColorRange {
    double warnLow = -1e9;    ///< 警告下限(低于此值为错误)
    double normalLow = 0.0;   ///< 正常下限
    double normalHigh = 100.0; ///< 正常上限
    double warnHigh = 1e9;    ///< 警告上限(高于此值为错误)
};

/**
 * @brief 协议解析结果表格视图
 *
 * 以表格形式展示解析出的帧数据，支持:
 *   - 动态列管理(字段名自动创建列)
 *   - 数值字段着色(按值范围变色)
 *   - 错误行高亮(整行标红)
 *   - 列宽自动调整
 *   - CSV/JSON导出
 *   - 右键上下文菜单
 */
class ProtocolView : public QWidget {
    Q_OBJECT

public:
    explicit ProtocolView(QWidget* parent = nullptr);

    /** @brief 添加一帧解析结果 */
    void addFrame(const QVariantMap& fields);

    /** @brief 清空所有解析结果 */
    void clear();

    /** @brief 设置显示的最大行数（旧数据自动丢弃） */
    void setMaxRows(int max);

    /** @brief 获取当前行数 */
    int rowCount() const;

    /** @brief 获取所有解析结果（用于导出） */
    QList<QVariantMap> allFrames() const;

    /**
     * @brief 设置数值字段着色范围
     * @param fieldName 字段名
     * @param range 着色范围配置
     */
    void setFieldColorRange(const QString& fieldName, const FieldColorRange& range);

public slots:
    /** @brief 帧解析成功 */
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

    /** @brief 帧解析错误 */
    void onFrameError(const QString& reason, const QByteArray& rawFrame);

private slots:
    /** @brief 右键上下文菜单弹出 */
    void onCustomContextMenu(const QPoint& pos);

private:
    /** @brief 初始化UI布局 */
    void setupUI();
    /** @brief 初始化右键上下文菜单 */
    void setupContextMenu();
    /** @brief 动态更新列头(根据字段名) */
    void updateColumnHeaders(const QVariantMap& fields);

    // ---- 右键菜单操作 ----
    /** @brief 复制选中行的文本(制表符分隔) */
    void copyRow();
    /** @brief 复制选中帧的原始HEX数据 */
    void copyRaw();
    /** @brief 导出所有帧为JSON文件 */
    void exportJson();

    /**
     * @brief 根据字段值返回对应的语义色
     * @param fieldName 字段名
     * @param value 字段值字符串
     * @return 着色后的QStandardItem(已设置前景色)
     *
     * 着色规则:
     *   - 值在normalLow~normalHigh: 正常色(默认前景)
     *   - 值在warnLow~normalLow或normalHigh~warnHigh: 警告色
     *   - 值超出warnLow或warnHigh: 错误色
     *   - 无法转为数值的字段: 默认前景色
     */
    QStandardItem* createColoredItem(const QString& fieldName,
                                      const QString& value);

    /** @brief 自动调整所有列宽(根据内容) */
    void autoResizeColumns();

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
    QList<QVariantMap> m_frames;    ///< 保存所有帧数据(用于导出)

    // ---- 着色配置 ----
    QMap<QString, FieldColorRange> m_colorRanges; ///< 字段名 -> 着色范围

    static constexpr int kFixedColumns = 2; ///< 固定列数: 序号(#) + 时间(Time)
};

#endif // PROTOCOLVIEW_H
