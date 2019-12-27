#include "im_ex_window.h"

ImExWindow::ImExWindow(QWidget* parent) : AniTabWidget(parent)
{
    disableSwitchAnimation(true);
    setTabEqualWidth(false);
    setMinimumSize(300, 400);

    initLayout();
}

void ImExWindow::initLayout()
{
    // 导出
    export_page = new ExportPage(this);
    addTab(export_page, QIcon(QPixmap(":/icons/export")), "导出");

    // 导入
    import_page = new ImportPage(this);
    addTab(import_page, QIcon(QPixmap(":/icons/import")), "导入");

    // 从码字风云导入
    import_mzfy_page = new ImportMzfyPage(this);
    addTab(import_mzfy_page,  QIcon(QPixmap(":/icons/mzfy")), "从码字风云导入");
}

void ImExWindow::toShow(int kind, QString def_name)
{
    this->setPalette(qApp->palette());
    setStyleSheet("QTabBar::tab{ padding: 5px; 15px; }");

    current_novel_name = def_name;
    if (kind>0 && kind<=count())
    {
        setCurrentIndex(kind-1);
    }
    import_page->setNovelName(def_name);
    adjustSize();
    show();
}

void ImExWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(QRect(0,0,width(),height()),us->getNoOpacityColor(us->mainwin_bg_color));

    return AniTabWidget::paintEvent(event);
}
