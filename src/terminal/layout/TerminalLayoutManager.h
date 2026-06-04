/** @file TerminalLayoutManager.h @brief 终端布局管理器 -- 管理终端区域的组件布局和搜索栏动画。支持Mixed/LeftRight/TopBottom三种布局模式 */
#ifndef TERMINALLAYOUTMANAGER_H
#define TERMINALLAYOUTMANAGER_H

#include <QObject>
#include <QSplitter>
#include <QLabel>
#include "shared/AppConstants.h"

class TerminalWidget;
class TerminalModel;
class TerminalSearchBar;
class QBoxLayout;

/** @brief 终端布局管理器。封装Mixed/LeftRight/TopBottom三种布局切换。分栏模式自动创建RX/TX终端，共用同一TerminalModel */
class TerminalLayoutManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造终端布局管理器 @param parent 父对象(通常为MainWindow) */
    explicit TerminalLayoutManager(QObject* parent = nullptr);
    /** @brief 析构函数 */
    ~TerminalLayoutManager() override = default;
    /** @brief 初始化布局管理器 @param mainTerminal 主终端控件指针 @param searchBar 搜索栏控件指针 */
    void initialize(TerminalWidget* mainTerminal, TerminalSearchBar* searchBar);
    /** @brief 设置共享数据模型(须在initialize()后调用) @param model TerminalModel指针 */
    void setTerminalModel(TerminalModel* model);
    /** @brief 获取终端容器widget @return 容器Widget指针 */
    QWidget* container() const;
    /** @brief 获取当前布局模式 @return TerminalLayout枚举值 */
    TerminalLayout layout() const;
    /** @brief 获取当前活动终端widget列表 @return 终端控件指针列表 */
    QList<TerminalWidget*> terminalWidgets() const;
    /** @brief 获取主终端widget @return 主终端指针，分栏模式下返回RX终端 */
    TerminalWidget* primaryTerminal() const;

    // ---- 统计计数器 ----

    /** @brief 获取布局切换总次数 @return 切换总次数 */
    quint64 totalSwitches() const;

    /** @brief 获取清除行数累计 @return 清除行总数 */
    quint64 totalLinesCleared() const;

    /** @brief 获取历史最大可见行数 @return 最大可见行数 */
    quint64 maxVisibleLines() const;

    /** @brief 获取分栏创建总次数 @return 累计分栏次数 */
    quint64 totalSplits() const;

    /** @brief 获取Tab切换总次数 @return 累计切换次数 */
    quint64 totalTabSwitches() const;

    /** @brief 获取布局变更总次数 @return 累计变更次数 */
    quint64 totalLayoutChanges() const;

    /** @brief 获取Tab添加总次数 @return 累计添加次数 */
    quint64 totalTabAdds() const;

    /** @brief 获取Tab移除总次数 @return 累计移除次数 */
    quint64 totalTabRemoves() const;

    /** @brief 获取分栏合并回混合模式总次数 @return 累计合并次数 */
    quint64 totalMerges() const;

    /** @brief 获取视图模式变更总次数(分栏/混合切换) @return 累计变更次数 */
    quint64 totalViewModeChanges() const;

    /** @brief 重置所有统计计数器为零 */
    void resetStats();

    /** @brief 通知行清除事件，累加清除行数 @param lines 本次清除的行数 */
    void notifyLinesCleared(quint64 lines);

    /** @brief 更新最大可见行数记录 @param currentVisible 当前可见行数 */
    void updateMaxVisibleLines(quint64 currentVisible);

signals:
    /** @brief 布局模式变化信号 @param newLayout 新的布局模式 */
    void layoutChanged(TerminalLayout newLayout);

public slots:
    /** @brief 按索引切换布局模式 @param layoutIndex 布局索引(0=混合,1=左右,2=上下) */
    void setLayout(int layoutIndex);
    /** @brief 切换布局模式 @param layout TerminalLayout枚举 */
    void setLayout(TerminalLayout layout);

private:
    /**
     * @brief 创建分栏模式的RX/TX终端widget
     * @param direction 数据方向(RX/TX)
     * @param label 终端标签
     * @return 新创建的终端widget
     */
    TerminalWidget* createSplitTerminal(DataDirection direction, const QString& label);

    /** @brief 应用当前布局: 重建容器内的widget层次 */
    void applyLayout();

    /** @brief 应用混合布局模式(销毁分栏终端，显示主终端) */
    void applyMixedLayout();
    /** @brief 应用分栏布局模式(创建RX/TX终端) */
    void applySplitLayout();

    /**
     * @brief 将主终端的显示设置同步到目标终端
     * @param target 目标终端widget
     */
    void syncDisplaySettings(TerminalWidget* target) const;

    TerminalWidget* m_mainTerminal;      ///< 主终端(混合模式使用)
    TerminalWidget* m_rxTerminal;        ///< 分栏RX终端(仅分栏模式创建)
    TerminalWidget* m_txTerminal;        ///< 分栏TX终端(仅分栏模式创建)
    TerminalSearchBar* m_searchBar;      ///< 搜索栏(三种模式共用)
    QWidget* m_container;               ///< 容器widget(搜索栏+终端/分割器)
    QSplitter* m_splitter;              ///< 分栏分割器(仅分栏模式时有效)

    TerminalLayout m_layout;             ///< 当前布局模式
    TerminalModel* m_model;              ///< 共享的数据模型引用

    // 主终端的显示设置缓存，用于同步到分栏终端
    DisplayMode m_displayMode;           ///< 显示模式缓存
    bool m_showTimestamp;                ///< 时间戳显示开关缓存
    bool m_showDirectionPrefix;          ///< 方向前缀显示开关缓存

    // ---- 统计计数器 ----
    quint64 m_totalSwitches = 0;         ///< 布局切换总次数
    quint64 m_totalLinesCleared = 0;     ///< 清除行数累计
    quint64 m_maxVisibleLines = 0;       ///< 历史最大可见行数
    quint64 m_totalSplits = 0;           ///< 分栏创建总次数
    quint64 m_totalTabSwitches = 0;      ///< Tab切换总次数
    quint64 m_totalLayoutChanges = 0;    ///< 布局变更总次数
    quint64 m_totalTabAdds = 0;          ///< Tab添加总次数
    quint64 m_totalTabRemoves = 0;       ///< Tab移除总次数
    quint64 m_totalMerges = 0;           ///< 分栏合并回混合模式总次数
    quint64 m_totalViewModeChanges = 0;  ///< 视图模式变更总次数(分栏/混合切换)
};

#endif // TERMINALLAYOUTMANAGER_H

