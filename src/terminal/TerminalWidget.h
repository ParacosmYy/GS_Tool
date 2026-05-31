/**
 * @file TerminalWidget.h
 * @brief auto-draw terminal widget - QPainter high-performance rendering
 */
#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>
#include "terminal/TerminalContextMenuManager.h"
#include "terminal/TerminalModel.h"
#include "terminal/DirectionFilter.h"
#include "terminal/TerminalSelectionManager.h"
#include "terminal/TerminalSearchManager.h"
#include "terminal/TerminalSearchRenderer.h"
#include "core/Constants.h"
#include "terminal/TerminalTypes.h"

class QContextMenuEvent;

class TerminalWidget : public QWidget {
    Q_OBJECT

signals:
    void searchMatchesChanged(int total, int current);
    void searchRequested();
    void pasteRequested(const QString& text);
    void clearRequested();

public:
    explicit TerminalWidget(QWidget* parent = nullptr);
    void setModel(TerminalModel* model);
    void setDirectionFilter(DataDirection direction);
    void clearDirectionFilter();
    void setDisplayMode(DisplayMode mode);
    DisplayMode displayMode() const;
    void setShowTimestamp(bool show);
    bool showTimestamp() const;
    void setShowDirectionPrefix(bool show);
    bool showDirectionPrefix() const;
    void setAutoScroll(bool autoScroll);
    bool autoScroll() const;
    void clear();
    QString selectedText() const;
    void setSearchHighlight(const QString& pattern, bool regex, bool hex);
    void clearSearchHighlight();
    int searchMatchCount() const;
    int currentMatchIndex() const;
    void gotoNextMatch();
    void gotoPrevMatch();
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
    void onDataAppended(int firstNewLine, int count);
    void onDataCleared();

private:
    void updateVisibleRange();
    void scrollToMatch(int line);
    void refreshSearchAfterCacheUpdate();
    CachedLine formatToCache(const TerminalLine& line) const;
    int paintLine(QPainter& painter, const CachedLine& cached, int y, int displayLine);

    TerminalModel* m_model = nullptr;
    DisplayMode m_displayMode = DisplayMode::Text;
    bool m_showTimestamp = false;
    bool m_showDirectionPrefix = false;
    bool m_autoScroll = true;
    int m_scrollOffset = 0;
    int m_lineHeight = 18;
    int m_visibleLines = 0;
    int m_maxScrollOffset = 0;
    QFont m_font{"Consolas", 10};
    QFontMetrics m_fontMetrics{m_font};
    QColor m_bgColor;
    QColor m_rxColor;
    QColor m_txColor;
    QColor m_timestampColor;
    TerminalSelectionManager* m_selectionManager;
    TerminalSearchManager* m_searchManager;
    DirectionFilter* m_directionFilter;
    TerminalContextMenuManager* m_contextMenuManager;
    mutable QVector<CachedLine> m_cachedLines;
    mutable int m_cachedLineCount = 0;
};

#endif // TERMINALWIDGET_H
