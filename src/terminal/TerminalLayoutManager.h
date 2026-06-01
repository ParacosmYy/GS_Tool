/**
 * @file TerminalLayoutManager.h
 * @brief 终端布局管理器 — 管理终端区域的组件布局和搜索栏动画
 */
#ifndef TERMINALLAYOUTMANAGER_H
#define TERMINALLAYOUTMANAGER_H

#include <QObject>
#include <QSplitter>
#include <QLabel>
#include "core/Constants.h"

class TerminalWidget;
class TerminalModel;
class TerminalSearchBar;
class QBoxLayout;

// 终端布局管理器 - 管理终端的显示布局模式
// 封装了混合/左右分栏/上下分栏三种布局的切换逻辑
// 将分栏逻辑从MainWindow中解耦，避免MainWindow膨胀
class TerminalLayoutManager : public QObject {
    Q_OBJECT

public:
    explicit TerminalLayoutManager(QObject* parent = nullptr);
    ~TerminalLayoutManager() override;

    // 初始化: 传入主终端控件和搜索栏（混合模式下使用）
    // 调用此方法后，manager获得这些控件的所有权管理权
    void initialize(TerminalWidget* mainTerminal, TerminalSearchBar* searchBar);

    // 设置共享的数据模型 — 分栏终端和主终端共用同一个TerminalModel
    // 必须在initialize()之后调用
    void setTerminalModel(TerminalModel* model);

    // 获取终端容器widget，可嵌入到外部布局中
    // 容器内容随布局模式变化: 混合=searchBar+terminal, 分栏=searchBar+splitter(rx+tx)
    QWidget* container() const;

    // 获取当前布局模式
    TerminalLayout layout() const;

    // 获取当前活动的终端widget列表（用于显示模式/时间戳等批量设置）
    // 混合模式返回1个, 分栏模式返回2个
    QList<TerminalWidget*> terminalWidgets() const;

    // 获取主终端widget（混合模式下唯一的一个, 分栏模式下为RX终端）
    TerminalWidget* primaryTerminal() const;

signals:
    // 布局模式变化信号，通知MainWindow更新导航树等引用
    void layoutChanged(TerminalLayout newLayout);

public slots:
    // 切换布局模式
    void setLayout(int layoutIndex);
    void setLayout(TerminalLayout layout);

private:
    // 创建分栏模式的RX/TX终端widget
    TerminalWidget* createSplitTerminal(DataDirection direction, const QString& label);

    // 应用当前布局: 重建容器内的widget层次
    void applyLayout();

    /** @brief 应用混合布局模式(销毁分栏终端，显示主终端) */
    void applyMixedLayout();
    /** @brief 应用分栏布局模式(创建RX/TX终端) */
    void applySplitLayout();

    // 将当前主终端的显示设置同步到分栏终端
    void syncDisplaySettings(TerminalWidget* target) const;

    TerminalWidget* m_mainTerminal;      // 主终端（混合模式使用）
    TerminalWidget* m_rxTerminal;        // 分栏RX终端（仅分栏模式创建）
    TerminalWidget* m_txTerminal;        // 分栏TX终端（仅分栏模式创建）
    TerminalSearchBar* m_searchBar;      // 搜索栏（三种模式共用）
    QWidget* m_container;               // 容器widget，内含搜索栏+终端/分割器
    QSplitter* m_splitter;              // 分栏分割器（仅分栏模式时有效）

    TerminalLayout m_layout;             // 当前布局模式
    TerminalModel* m_model;              // 共享的数据模型引用

    // 主终端的显示设置缓存，用于同步到分栏终端
    DisplayMode m_displayMode;
    bool m_showTimestamp;
    bool m_showDirectionPrefix;
};

#endif // TERMINALLAYOUTMANAGER_H
