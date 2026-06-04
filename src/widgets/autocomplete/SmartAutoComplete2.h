/**
 * @file SmartAutoComplete2.h
 * @brief 智能自动补全组件 - 提供前缀匹配和频率排序的弹出补全列表
 *
 * 职责:
 *   1. 基于用户词典提供前缀匹配补全建议
 *   2. 支持大小写敏感/不敏感匹配模式
 *   3. 可配置最小触发字符数和最大建议数量
 *   4. 绑定QLineEdit后自动监听文本变化并弹出补全列表
 */

#pragma once
#include <QWidget>
#include <QStringList>
#include <QMap>

class QLineEdit;
class QListWidget;

/**
 * @brief 智能自动补全弹出列表组件
 *
 * 绑定到QLineEdit后，用户输入达到最小触发字符数时自动弹出
 * 匹配建议列表，支持键盘导航（上下箭头选择、回车确认、Esc关闭）。
 */
class SmartAutoComplete : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造智能补全组件
     * @param parent 父widget
     */
    explicit SmartAutoComplete(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~SmartAutoComplete() override;

    /**
     * @brief 设置补全词典（替换现有词典）
     * @param words 词汇列表
     */
    void setDictionary(const QStringList &words);

    /**
     * @brief 追加词汇到现有词典
     * @param words 要追加的词汇列表
     */
    void addWords(const QStringList &words);

    /**
     * @brief 设置最大建议数量
     * @param max 最大显示条目数，默认10
     */
    void setMaxSuggestions(int max);

    /**
     * @brief 设置是否区分大小写
     * @param cs true区分大小写，false不区分，默认false
     */
    void setCaseSensitive(bool cs);

    /**
     * @brief 设置触发补全的最小输入字符数
     * @param min 最小字符数，默认2
     */
    void setMinCharsToTrigger(int min);

    /**
     * @brief 绑定到输入框
     * @param edit 目标QLineEdit指针
     */
    void setLineEdit(QLineEdit *edit);

    /**
     * @brief 获取当前补全建议列表
     * @return 当前匹配的词汇列表
     */
    QStringList suggestions() const;

    /** @brief 清空词典 */
    void clearDictionary();

protected:
    /** @brief 键盘事件处理：上下导航、回车确认、Esc关闭 */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /** @brief 输入框文本变更时触发补全计算 @param text 当前输入文本 */
    void onTextChanged(const QString &text);

    /** @brief 显示补全建议弹出列表 @param items 匹配的词汇列表 */
    void showSuggestions(const QStringList &items);

    QLineEdit *m_edit = nullptr;        ///< 绑定的输入框
    QListWidget *m_list = nullptr;      ///< 补全建议弹出列表
    QStringList m_dictionary;           ///< 补全词典
    int m_maxSuggestions = 10;          ///< 最大建议数量
    bool m_caseSensitive = false;       ///< 是否区分大小写
    int m_minChars = 2;                 ///< 触发补全的最小字符数

    // ---- 统计计数器 ----
    quint64 m_totalTriggers = 0;         ///< 总补全触发次数
    quint64 m_totalSuggestions = 0;      ///< 总建议展示次数
    quint64 m_totalAccepts = 0;          ///< 总用户接受次数

public:
    /** @brief 获取总补全触发次数 @return 累计触发次数 */
    quint64 totalTriggers() const { return m_totalTriggers; }
    /** @brief 获取总建议展示次数 @return 累计建议次数 */
    quint64 totalSuggestions() const { return m_totalSuggestions; }
    /** @brief 获取总用户接受次数 @return 累计接受次数 */
    quint64 totalAccepts() const { return m_totalAccepts; }
    /** @brief 重置自动补全统计 */
    void resetAutoComplete2Statistics() { m_totalTriggers = 0; m_totalSuggestions = 0; m_totalAccepts = 0; }
};
