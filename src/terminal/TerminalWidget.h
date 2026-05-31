/**
 * @file TerminalWidget.h
 * @brief 自绘制终端控件 - QPainter高性能终端显示，支持右键菜单/搜索/分栏
 *
 * 核心渲染逻辑(paintEvent/paintLine/formatToCache)保留在本类中，
 * 选区管理和搜索功能分别委托给 TerminalSelectionManager 和 TerminalSearchManager。
 *
 * 协作关系: TerminalModel(数据源), DirectionFilter(方向过滤),
 * TerminalSelectionManager(选区), TerminalSearchManager(搜索)
 */

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include <QMenu>
#include "terminal/TerminalModel.h"
#include "terminal/DirectionFilter.h"
#include "terminal/TerminalSelectionManager.h"
#include "terminal/TerminalSearchManager.h"
#include "core/Constants.h"
#include "terminal/TerminalTypes.h"

class QContextMenuEvent;

/**
 * @brief 自绘制终端控件 - QPainter高性能终端渲染
 *
 * 支持方向过滤(分栏模式)、搜索高亮(F3导航)、多显示模式(Text/Hex/Mixed/Decimal)。
 * 右键菜单提供复制/粘贴/清屏/全选/搜索(Ctrl+F)操作，样式由QSS主题控制。
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    /** @brief 搜索匹配结果变化 @param total 匹配总数 @param current 当前高亮索引 */
    void searchMatchesChanged(int total, int current);
    /** @brief 右键菜单触发搜索请求，外部应激活 TerminalSearchBar */
    void searchRequested();
    /** @brief 右键菜单触发粘贴请求 @param text 剪贴板文本 */
    void pasteRequested(const QString& text);
    /** @brief 右键菜单触发清屏请求 */
    void clearRequested();

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    /** @brief 设置数据模型，传入nullptr断开旧模型 */
    void setModel(TerminalModel* model);

    /** @brief 设置方向过滤器(Rx/Tx)，用于分栏模式 */
    void setDirectionFilter(DataDirection direction);
    /** @brief 清除方向过滤 */
    void clearDirectionFilter();

    /** @brief 设置/获取显示模式(文本/HEX/混合/十进制) */
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;

    /** @brief 设置/获取是否显示时间戳 */
    void setShowTimestamp(bool show);
    bool showTimestamp() const;

    /** @brief 设置/获取是否显示方向前缀 [TX:]/[RX:] */
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;

    /** @brief 设置/获取自动滚动 */
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;

    /** @brief 清空显示内容 */
    void clear();
    /** @brief 获取选中文本，无选择返回空 */
    QString selectedText() const;

    /** @brief 设置搜索高亮 @param pattern 搜索串 @param regex 正则 @param hex HEX搜索 */
    void setSearchHighlight(const QString& pattern, bool regex, bool hex);
    void clearSearchHighlight();
    int searchMatchCount() const;
    int currentMatchIndex() const;
    void gotoNextMatch();
    void gotoPrevMatch();

    /** @brief 全选终端所有内容 */
    void selectAll();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onDataAppended(int firstNewLine, int count);   ///< 模型数据追加
    void onDataCleared();                                ///< 模型数据清空

private:
    void updateVisibleRange();           ///< 计算可见行数
    void scrollToMatch(int line);        ///< 滚动到搜索匹配行
    void refreshSearchAfterCacheUpdate();///< 缓存更新后重新搜索
    void refreshSearch();                ///< 重新执行搜索(搜索参数不变)
    void createContextMenu();            ///< 创建右键菜单
    CachedLine formatToCache(const TerminalLine& line) const;  ///< 数据行转缓存结构
    int paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine); ///< 绘制单行

    // ---- 数据和模型 ----
    TerminalModel* m_model = nullptr;               ///< 终端数据模型
    DisplayMode m_displayMode = DisplayMode::Text;  ///< 当前显示模式
    bool m_showTimestamp = false;                   ///< 是否显示时间戳
    bool m_showDirectionPrefix = false;             ///< 是否显示方向前缀
    bool m_autoScroll = true;                       ///< 自动滚动到底部

    // ---- 滚动和渲染 ----
    int m_scrollOffset = 0;         ///< 当前滚动偏移(行数)
    int m_lineHeight = 18;          ///< 每行像素高度
    int m_visibleLines = 0;         ///< 可见行数
    int m_maxScrollOffset = 0;      ///< 最大滚动偏移
    QFont m_font{"Consolas", 10};   ///< 终端字体(等宽)
    QFontMetrics m_fontMetrics{m_font}; ///< 字体度量

    // ---- 颜色配置(从ThemeManager加载) ----
    QColor m_bgColor;               ///< 背景色
    QColor m_rxColor;               ///< 接收文本颜色
    QColor m_txColor;               ///< 发送文本颜色
    QColor m_timestampColor;        ///< 时间戳颜色

    // ---- 组合持有的管理器 ----
    TerminalSelectionManager* m_selectionManager;   ///< 选区管理器
    TerminalSearchManager* m_searchManager;         ///< 搜索管理器
    DirectionFilter* m_directionFilter;             ///< 方向过滤器

    // ---- 右键菜单 ----
    QMenu* m_contextMenu;           ///< 终端右键菜单
    QAction* m_copyAction;          ///< 复制菜单项
    QAction* m_pasteAction;         ///< 粘贴菜单项
    QAction* m_clearAction;         ///< 清屏菜单项
    QAction* m_selectAllAction;     ///< 全选菜单项
    QAction* m_searchAction;        ///< 搜索菜单项(Ctrl+F)

    // ---- 缓存 ----
    mutable QVector<CachedLine> m_cachedLines;  ///< 缓存行数据
    mutable int m_cachedLineCount = 0;           ///< 已缓存的模型行数
};

#endif // TERMINALWIDGET_H
