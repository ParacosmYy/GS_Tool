/**
 * @file TerminalSearchBar.h
 * @brief 终端搜索栏 - 嵌入终端顶部的搜索控件，支持文本/正则/HEX搜索
 */

#ifndef TERMINALSEARCHBAR_H
#define TERMINALSEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

/**
 * @brief 终端搜索栏 - 嵌入终端顶部的搜索控件
 *
 * 支持: 文本搜索、正则搜索、HEX搜索
 * 快捷键: Ctrl+F 打开, Esc 关闭
 *
 * 展开/收起动画实现:
 *   - 展开: maximumHeight 从 0 到 36, 200ms, OutCubic 缓动
 *   - 收起: maximumHeight 从 36 到 0, 150ms, InCubic 缓动
 *   - 使用 QPropertyAnimation 驱动，动画结束后恢复 setFixedHeight(36)
 *
 * 协作关系:
 *   - TerminalWidget: 监听 searchRequested/searchCleared 信号执行搜索
 */
class TerminalSearchBar : public QWidget {
    Q_OBJECT
public:
    explicit TerminalSearchBar(QWidget* parent = nullptr);

    /** @brief 获取当前搜索模式串 */
    QString searchPattern() const;

    /** @brief 是否启用正则模式 */
    bool isRegexMode() const;

    /** @brief 是否启用HEX模式 */
    bool isHexMode() const;

public slots:
    /**
     * @brief 激活搜索栏并聚焦输入框
     *
     * 如果已经可见，仅聚焦并全选文本。
     * 如果不可见，播放展开动画后聚焦。
     */
    void activate();

    /**
     * @brief 关闭搜索栏并清除
     *
     * 清除搜索文本和结果标签，播放收起动画，
     * 动画完成后隐藏并恢复状态。
     */
    void deactivate();

    /**
     * @brief 设置匹配结果显示文本（如 "3/15"）
     * @param text 要显示的结果文本
     */
    void setResultText(const QString& text);

signals:
    /**
     * @brief 搜索触发信号
     * @param pattern 搜索内容
     * @param regex 是否为正则模式
     * @param hex 是否为HEX模式
     */
    void searchRequested(const QString& pattern, bool regex, bool hex);

    /** @brief 搜索清除信号（搜索框为空时发射） */
    void searchCleared();

    /** @brief 搜索栏关闭信号（收起动画完成后发射） */
    void closed();

private slots:
    /**
     * @brief 搜索文本变化时触发搜索或清除
     * @param text 当前搜索框文本
     *
     * HEX模式下会验证输入合法性，非法时显示错误样式。
     */
    void onSearchTextChanged(const QString& text);

    /** @brief 关闭按钮点击，委托给 deactivate() */
    void onCloseClicked();

private:
    /** @brief 构建界面布局和样式，创建所有子控件并连接信号 */
    void setupUI();

    /**
     * @brief 验证HEX输入是否合法
     * @param text 待验证的字符串
     * @return true 如果是合法的十六进制字符串
     */
    bool isValidHex(const QString& text) const;

    QLineEdit* m_searchInput;       ///< 搜索输入框（objectName: searchBarInput）
    QPushButton* m_closeBtn;        ///< 关闭按钮（objectName: searchBarCloseBtn）
    QCheckBox* m_regexCheck;        ///< 正则模式复选框（objectName: searchBarRegexCheck）
    QCheckBox* m_hexCheck;          ///< HEX模式复选框（objectName: searchBarHexCheck）
    QLabel* m_resultLabel;          ///< 结果标签（objectName: searchBarResult），如 "3/15 matches"
    QPropertyAnimation* m_activeAnim = nullptr; ///< 当前活跃的展开/收起动画，防止快速切换时冲突
};

#endif // TERMINALSEARCHBAR_H
