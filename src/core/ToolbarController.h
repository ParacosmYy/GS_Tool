#ifndef TOOLBARCONTROLLER_H
#define TOOLBARCONTROLLER_H

#include <QObject>
#include <QStringList>

class QMainWindow;
class QToolBar;
class QComboBox;
class QAction;
class RecordingController;

// 工具栏控制器 - 从MainWindow中提取的工具栏创建和管理逻辑
// 负责工具栏所有控件的创建、布局、objectName设置和信号转发
class ToolbarController : public QObject {
    Q_OBJECT

public:
    explicit ToolbarController(RecordingController* recordingController, QObject* parent = nullptr);

    // 创建并返回工具栏，添加到parent的主窗口
    QToolBar* createToolbar(QMainWindow* parent);

    // 主题管理接口
    void setAvailableThemes(const QStringList& themes);
    void setCurrentTheme(const QString& themeName);
    QString themeNameAt(int index) const;

    // 语言管理接口
    void setCurrentLanguage(const QString& langCode);
    QString languageCodeAt(int index) const;

signals:
    void displayModeChanged(int index);
    void timestampToggled(bool checked);
    void dirPrefixToggled(bool checked);
    void clearRequested();
    void exportRequested();
    void themeChanged(int index);
    void languageChanged(int index);

private:
    RecordingController* m_recordingController;

    // 工具栏控件
    QToolBar* m_toolbar;
    QComboBox* m_displayModeCombo;
    QComboBox* m_themeCombo;
    QComboBox* m_langCombo;
    QAction* m_timestampAction;
    QAction* m_dirPrefixAction;
    QAction* m_clearAction;
    QAction* m_exportAction;
};

#endif // TOOLBARCONTROLLER_H
