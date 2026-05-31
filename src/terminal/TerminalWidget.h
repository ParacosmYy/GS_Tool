/**
 * @file TerminalWidget.h
 * @brief 自绘制终端控件 - 使用QPainter直接绘制文本的高性能终端显示组件
 *
 * 比QTextEdit/QPlainTextEdit性能更好，适合大流量数据显示。
 * 核心渲染逻辑(paintEvent/paintLine/formatToCache)保留在本类中，
 * 选区管理和搜索功能分别委托给 TerminalSelectionManager 和 TerminalSearchManager。
 *
 * 协作关系:
 *   - TerminalModel: 数据源，通过信号通知数据追加/清空
 *   - DirectionFilter: 方向过滤器，分栏模式下过滤显示行
 *   - TerminalSelectionManager: 管理鼠标选择交互和选区文本提取
 *   - TerminalSearchManager: 管理文本搜索、高亮匹配和导航
 *
 * 设计模式:
 *   - 组合模式: 通过持有 SelectionManager/SearchManager 分离职责
 *   - 观察者模式: 通过Qt信号/槽与TerminalModel通信
 */

#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include "terminal/TerminalModel.h"
#include "terminal/DirectionFilter.h"
#include "terminal/TerminalSelectionManager.h"
#include "terminal/TerminalSearchManager.h"
#include "core/Constants.h"
#include "terminal/TerminalTypes.h"

/**
 * @brief 自绘制终端控件 - 使用QPainter直接绘制文本
 *
 * 比QTextEdit/QPlainTextEdit性能更好，适合大流量数据显示。
 * 采用增量缓存策略: 只格式化新增行，避免重复计算。
 * 支持方向过滤(分栏模式)、搜索高亮(F3导航)、多显示模式(Text/Hex/Mixed/Decimal)。
 */
class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    /** @brief 搜索匹配结果变化时发射 @param total 匹配总数 @param current 当前高亮匹配索引 */
    void searchMatchesChanged(int total, int current);

public:
    explicit TerminalWidget(QWidget* parent = nullptr);

    /**
     * @brief 设置数据模型
     * @param model 终端数据模型，传入nullptr断开旧模型连接
     */
    void setModel(TerminalModel* model);

    /**
     * @brief 设置方向过滤器 — 只显示指定方向的数据行
     * @param direction 过滤方向(Rx=只显示接收, Tx=只显示发送)
     *
     * 不设置过滤器时(默认)显示所有方向的数据。
     * 用于分栏模式: 一个TerminalWidget只显示RX，另一个只显示TX。
     */
    void setDirectionFilter(DataDirection direction);

    /** @brief 清除方向过滤，恢复显示所有方向的数据 */
    void clearDirectionFilter();

    /** @brief 设置显示模式(文本/HEX/混合/十进制) */
    void setDisplayMode(DisplayMode mode);

    /** @brief 获取当前显示模式 */
    DisplayMode displayMode() const;

    /** @brief 设置是否显示时间戳 */
    void setShowTimestamp(bool show);
    bool showTimestamp() const;

    /** @brief 设置是否显示方向前缀 [TX:] / [RX:] */
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;

    /** @brief 设置自动滚动(新数据时自动滚动到底部) */
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;

    /** @brief 清空显示内容 */
    void clear();

    /**
     * @brief 获取选中的文本
     * @return 选中的文本内容，无选择时返回空字符串
     */
    QString selectedText() const;

    /**
     * @brief 设置搜索高亮
     * @param pattern 搜索模式串
     * @param regex 是否使用正则表达式
     * @param hex 是否使用HEX搜索
     */
    void setSearchHighlight(const QString& pattern, bool regex, bool hex);

    /** @brief 清除搜索高亮 */
    void clearSearchHighlight();

    /** @brief 获取当前搜索匹配总数 */
    int searchMatchCount() const;

    /** @brief 获取当前高亮的匹配索引 */
    int currentMatchIndex() const;

    /** @brief 导航到下一个搜索匹配 */
    void gotoNextMatch();

    /** @brief 导航到上一个搜索匹配 */
    void gotoPrevMatch();

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    /** @brief 模型数据追加时更新滚动偏移并安排重绘 */
    void onDataAppended(int firstNewLine, int count);

    /** @brief 模型数据清空时重置所有显示状态 */
    void onDataCleared();

private:
    /** @brief 计算可见区域可以显示多少行 */
    void updateVisibleRange();

    /**
     * @brief 滚动到指定行(搜索匹配导航)
     * @param line 目标行号(显示行号)
     */
    void scrollToMatch(int line);

    /**
     * @brief 重新执行搜索(缓存更新后调用)
     *
     * 保存当前搜索参数，调用setSearchHighlight重新扫描。
     */
    void refreshSearch();

    /** @brief 缓存更新后重新执行搜索(paintEvent中使用，先清空pattern避免递归) */
    void refreshSearchAfterCacheUpdate();

    /**
     * @brief 将数据行转换为缓存结构
     * @param line 原始终端数据行
     * @return 格式化后的缓存结构(文本+方向+时间戳)
     */
    CachedLine formatToCache(const TerminalLine& line) const;

    /**
     * @brief 绘制单行数据
     * @param painter 画布对象
     * @param cached 缓存行数据
     * @param y 当前Y坐标
     * @param displayLine 显示行号(用于选择/搜索定位)
     * @return 绘制完该行后的Y坐标(= y + m_lineHeight)
     */
    int paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine);

    TerminalModel* m_model = nullptr;       ///< 终端数据模型
    DisplayMode m_displayMode = DisplayMode::Text;  ///< 当前显示模式
    bool m_showTimestamp = false;           ///< 是否显示时间戳
    bool m_showDirectionPrefix = false;     ///< 是否显示方向前缀
    bool m_autoScroll = true;              ///< 是否自动滚动到底部

    // ---- 滚动和渲染 ----
    int m_scrollOffset = 0;         ///< 当前滚动偏移(行数)
    int m_lineHeight = 18;          ///< 每行像素高度
    int m_visibleLines = 0;         ///< 可见行数
    int m_maxScrollOffset = 0;      ///< 最大滚动偏移
    QFont m_font{"Consolas", 10};               ///< 终端字体(等宽)
    QFontMetrics m_fontMetrics{m_font};         ///< 字体度量缓存，字体变更时同步更新

    // ---- 颜色配置 ----
    QColor m_bgColor;               ///< 背景色
    QColor m_rxColor;               ///< 接收文本颜色
    QColor m_txColor;               ///< 发送文本颜色
    QColor m_timestampColor;        ///< 时间戳颜色

    // ---- 组合持有的管理器 ----
    TerminalSelectionManager* m_selectionManager;   ///< 选区管理器，处理鼠标选择和文本提取
    TerminalSearchManager* m_searchManager;          ///< 搜索管理器，处理搜索高亮和导航
    DirectionFilter* m_directionFilter;              ///< 方向过滤器实例，构造时创建

    // ---- 缓存格式化后的行信息 ----
    mutable QVector<CachedLine> m_cachedLines;       ///< 缓存行数据(文本+方向+时间戳)
    mutable int m_cachedLineCount = 0;               ///< 已缓存的模型行数
};

#endif // TERMINALWIDGET_H
