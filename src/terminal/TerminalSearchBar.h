#ifndef TERMINALSEARCHBAR_H
#define TERMINALSEARCHBAR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

// 终端搜索栏 - 嵌入终端顶部的搜索控件
// 支持: 文本搜索、正则搜索、HEX搜索
// 快捷键: Ctrl+F 打开, Esc 关闭
class TerminalSearchBar : public QWidget {
    Q_OBJECT
public:
    explicit TerminalSearchBar(QWidget* parent = nullptr);

    // 获取当前搜索模式串
    QString searchPattern() const;

    // 是否启用正则模式
    bool isRegexMode() const;

    // 是否启用HEX模式
    bool isHexMode() const;

public slots:
    // 激活搜索栏并聚焦输入框
    void activate();

    // 关闭搜索栏并清除
    void deactivate();

signals:
    // 搜索触发: pattern=搜索内容, regex=是否正则, hex=是否HEX
    void searchRequested(const QString& pattern, bool regex, bool hex);

    // 搜索清除
    void searchCleared();

    // 搜索栏关闭
    void closed();

private slots:
    // 搜索文本变化时触发搜索或清除
    void onSearchTextChanged(const QString& text);

    // 关闭按钮点击
    void onCloseClicked();

private:
    // 构建界面布局和样式
    void setupUI();

    // 验证HEX输入是否合法
    bool isValidHex(const QString& text) const;

    QLineEdit* m_searchInput;       // 搜索输入框
    QPushButton* m_closeBtn;        // 关闭按钮
    QCheckBox* m_regexCheck;        // 正则模式复选框
    QCheckBox* m_hexCheck;          // HEX模式复选框
    QLabel* m_resultLabel;          // 结果标签, 如 "3/15 matches"
};

#endif // TERMINALSEARCHBAR_H
