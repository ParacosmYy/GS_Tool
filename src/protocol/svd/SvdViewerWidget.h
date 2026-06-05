/**
 * @file SvdViewerWidget.h
 * @brief SVD寄存器查看器 — 主控件，集成工具栏/树视图/详情面板/位字段可视化
 *
 * 职责: 提供完整的SVD文件查看界面 — 顶部工具栏(打开/搜索/过滤)、
 * 左侧寄存器树视图、右侧详情面板(寄存器信息+位字段图)。
 *
 * 协作: SvdRegisterTreeModel(树数据) / SvdBitFieldWidget(位字段图)
 *        SvdParser(文件解析，由另一Agent实现)
 */
#ifndef SVDVIEWERWIDGET_H
#define SVDVIEWERWIDGET_H

#include <QWidget>
#include <QTreeView>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>

class SvdRegisterTreeModel;
class SvdBitFieldWidget;

/**
 * @brief SVD寄存器查看器主控件
 *
 * 布局: 顶部工具栏(打开/搜索/过滤) + 左侧寄存器树 + 右侧详情(信息+位字段图)。
 * 支持SVD文件加载、寄存器搜索、外设过滤和选中详情展示。
 */
class SvdViewerWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造SVD查看器主控件 @param parent 父控件 */
    explicit SvdViewerWidget(QWidget* parent = nullptr);

    /** @brief 加载SVD文件并更新树模型 @param filePath SVD文件路径 */
    void loadSvdFile(const QString& filePath);

    /** @brief 获取当前加载的SVD文件路径 @return 文件路径，未加载返回空 */
    QString currentFilePath() const;

    // ---- 统计接口 ----
    /** @brief 获取累计展开操作次数 @return 展开总次数 */
    quint64 totalExpandCount() const;
    /** @brief 获取累计折叠操作次数 @return 折叠总次数 */
    quint64 totalCollapseCount() const;
    /** @brief 获取累计搜索操作次数 @return 搜索总次数 */
    quint64 totalSearchCount() const;
    /** @brief 获取累计加载SVD文件次数 @return 文件加载总次数 */
    quint64 totalFileLoads() const;
    /** @brief 获取累计字段悬停次数 @return 悬停总次数 */
    quint64 totalFieldHoverCount() const;
    /** @brief 获取累计字段点击次数 @return 点击总次数 */
    quint64 totalFieldClickCount() const;
    /** @brief 重置所有统计计数器(含子组件) */
    void resetStatistics();

signals:
    /** @brief SVD文件加载完成 @param fileName 文件名 */
    void svdFileLoaded(const QString& fileName);
    /** @brief 寄存器被选中 @param peripheral 外设名 @param register_ 寄存器名 */
    void registerSelected(const QString& peripheral, const QString& register_);
    /** @brief 字段被选中 @param peripheral 外设名 @param register_ 寄存器名 @param field 字段名 */
    void fieldSelected(const QString& peripheral, const QString& register_,
                       const QString& field);

private slots:
    /** @brief 打开文件按钮回调 */
    void onOpenFileClicked();
    /** @brief 搜索文本变化回调 @param text 搜索关键词 */
    void onSearchTextChanged(const QString& text);
    /** @brief 外设过滤器变化回调 @param index 选中索引 */
    void onFilterChanged(int index);
    /** @brief 树选中项变化回调 */
    void onTreeSelectionChanged();
    /** @brief 树节点展开回调 @param index 展开的节点索引 */
    void onTreeExpanded(const QModelIndex& index);
    /** @brief 树节点折叠回调 @param index 折叠的节点索引 */
    void onTreeCollapsed(const QModelIndex& index);

private:
    /** @brief 初始化UI布局 */
    void setupUi();
    /** @brief 初始化信号/槽连接 */
    void setupConnections();
    /** @brief 更新详情面板(根据选中节点) @param peripheralName 外设名 @param registerName 寄存器名 */
    void updateDetailPanel(const QString& peripheralName,
                           const QString& registerName);
    /** @brief 构建外设过滤器下拉列表 */
    void rebuildFilterCombo();

    // ---- UI控件 ----
    QTreeView*             m_treeView;          ///< 寄存器树视图
    QTextEdit*             m_detailView;        ///< 详情文本显示
    SvdBitFieldWidget*     m_bitFieldWidget;    ///< 位字段可视化
    QPushButton*           m_openBtn;           ///< 打开SVD文件按钮
    QLineEdit*             m_searchEdit;        ///< 搜索输入框
    QComboBox*             m_filterCombo;       ///< 外设过滤器
    QLabel*                m_statusLabel;       ///< 状态信息标签
    QLabel*                m_fileLabel;         ///< 当前文件路径标签

    // ---- 数据模型 ----
    SvdRegisterTreeModel*  m_treeModel;         ///< 寄存器树模型

    // ---- 状态 ----
    QString m_currentFilePath;                  ///< 当前加载的SVD文件路径

    // ---- 统计计数器 ----
    quint64 m_totalExpandCount = 0;             ///< 累计展开操作次数
    quint64 m_totalCollapseCount = 0;           ///< 累计折叠操作次数
    quint64 m_totalSearchCount = 0;             ///< 累计搜索操作次数
    quint64 m_totalFileLoads = 0;               ///< 累计加载文件次数
};

#endif // SVDVIEWERWIDGET_H
