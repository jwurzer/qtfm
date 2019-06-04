#include "qtfm.h"
#include "common.h"

#include <QMdiSubWindow>
#include <QApplication>
#include <QFont>

QtFM::QtFM(QWidget *parent)
    : QMainWindow(parent)
    , mdi(Q_NULLPTR)
    , mimes(Q_NULLPTR)
    , mBar(Q_NULLPTR)
    , sBar(Q_NULLPTR)
    , navBar(Q_NULLPTR)
    , pathEdit(Q_NULLPTR)
    , fileMenu(Q_NULLPTR)
    , editMenu(Q_NULLPTR)
    , viewMenu(Q_NULLPTR)
    , tileAction(Q_NULLPTR)
    , tabViewAction(Q_NULLPTR)
    , newTabAction(Q_NULLPTR)
    , newTermAction(Q_NULLPTR)
    , backButton(Q_NULLPTR)
    , upButton(Q_NULLPTR)
    , homeButton(Q_NULLPTR)
    , tileButton(Q_NULLPTR)
{
    QIcon::setThemeSearchPaths(Common::iconPaths(qApp->applicationDirPath()));
    Common::setupIconTheme(qApp->applicationFilePath());

    setWindowIcon(QIcon::fromTheme("qtfm",
                                   QIcon(":/images/qtfm.png")));
    setWindowTitle("QtFM");

    mBar = new QMenuBar(this);
    sBar = new QStatusBar(this);

    fileMenu = new QMenu(this);
    fileMenu->setTitle(tr("File"));

    editMenu = new QMenu(this);
    editMenu->setTitle(tr("Edit"));
    editMenu->hide();

    viewMenu = new QMenu(this);
    viewMenu->setTitle(tr("View"));

    navBar = new QToolBar(this);
    navBar->setWindowTitle(tr("Navigation"));

    pathEdit = new QComboBox(this);
    pathEdit->setEditable(true);
    pathEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    mdi = new QMdiArea(this);
    mdi->setViewMode(QMdiArea::SubWindowView);
    mdi->setTabPosition(QTabWidget::North);
    mdi->setTabsClosable(true);
    mdi->setTabsMovable(true);

    backButton = new QPushButton(this);
    backButton->setText(tr("Back"));

    upButton = new QPushButton(this);
    upButton->setText(tr("Up"));

    homeButton = new QPushButton(this);
    homeButton->setText(tr("Home"));

    tileButton = new QPushButton(this);
    tileButton->setText(tr("Tile"));

    tileAction = new QAction(this);
    tileAction->setText(tr("Tile"));
    tileAction->setIcon(QIcon::fromTheme("preferences-system-windows"));
    tileAction->setShortcut(QKeySequence(tr("Ctrl+T")));

    newTabAction = new QAction(this);
    newTabAction->setText(tr("Open new folder"));
    newTabAction->setIcon(QIcon::fromTheme("window-new"));
    newTabAction->setShortcut(QKeySequence(tr("Ctrl+N")));

    newTermAction = new QAction(this);
    newTermAction->setText(tr("Open new terminal"));
    newTermAction->setIcon(QIcon::fromTheme("terminal"));


    fileMenu->addAction(newTabAction);
    fileMenu->addAction(newTermAction);
    viewMenu->addAction(tileAction);

    mBar->addMenu(fileMenu);
    //mBar->addMenu(editMenu);
    mBar->addMenu(viewMenu);

    //navBar->addWidget(backButton);
    //navBar->addWidget(upButton);
    //navBar->addWidget(homeButton);
    navBar->addWidget(pathEdit);
    //navBar->addWidget(tileButton);

    setMenuBar(mBar);
    setStatusBar(sBar);
    addToolBar(Qt::TopToolBarArea, navBar);
    setCentralWidget(mdi);

    mimes = new MimeUtils(this);
    mimes->setDefaultsFileName(Common::readSetting("defMimeAppsFile",
                                                   MIME_APPS).toString());

    backButton->hide();
    upButton->hide();
    homeButton->hide();
    tileButton->hide();

    setupConnections();
    loadSettings();
    QTimer::singleShot(10, this, SLOT(parseArgs()));
}

QtFM::~QtFM()
{
    writeSettings();
}

void QtFM::newSubWindow(bool triggered)
{
    Q_UNUSED(triggered)
    newSubWindow();
}

void QtFM::newSubWindow(QString path)
{
    qDebug() << "newSubWindow" << path;
    QFileInfo info(path);
    if (!info.isDir()) { return; }

    QMdiSubWindow *subwindow = new QMdiSubWindow(this);
    FM *fm = new FM(mimes, path);

    connect(fm, SIGNAL(newWindowTitle(QString)),
            subwindow, SLOT(setWindowTitle(QString)));
    connect(fm, SIGNAL(updatedDir(QString)),
            this, SLOT(handleUpdatedDir(QString)));
    connect(fm, SIGNAL(newPath(QString)),
            this, SLOT(handleNewPath(QString)));
    connect(fm, SIGNAL(openFile(QString)),
            this, SLOT(handleOpenFile(QString)));
    connect(fm, SIGNAL(previewFile(QString)),
            this, SLOT(handlePreviewFile(QString)));

    mdi->addSubWindow(subwindow);
    subwindow->setWidget(fm);
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(info.completeBaseName());
    subwindow->setWindowIcon(windowIcon());
    subwindow->setWindowState(Qt::WindowMaximized);

    refreshPath(fm);
    if (mdi->subWindowList().count()>1) {
        mdi->tileSubWindows();
    }
}

void QtFM::parseArgs()
{
    QStringList args = qApp->arguments();
    if (args.count()<2) {
        newSubWindow();
        return;
    }
    for (int i=1;i<args.count();++i) {
        if (!QFile::exists(args.at(i))) { continue; }
        newSubWindow(args.at(i));
    }
}

void QtFM::setupConnections()
{
    connect(tileButton, SIGNAL(released()),
            mdi, SLOT(tileSubWindows()));
    connect(tileAction, SIGNAL(triggered()),
            mdi, SLOT(tileSubWindows()));
    connect(mdi, SIGNAL(subWindowActivated(QMdiSubWindow*)),
            this, SLOT(handleTabActivated(QMdiSubWindow*)));

    connect(pathEdit, SIGNAL(activated(QString)),
            this, SLOT(pathEditChanged(QString)));
    /*
  connect(customComplete, SIGNAL(activated(QString)),
          this, SLOT(pathEditChanged(QString)));
  connect(pathEdit->lineEdit(), SIGNAL(cursorPositionChanged(int,int)),
          this, SLOT(addressChanged(int,int)));
*/

    connect(newTabAction, SIGNAL(triggered(bool)),
            this, SLOT(newSubWindow(bool)));
    connect(newTermAction, SIGNAL(triggered(bool)),
            this, SLOT(handleNewTermAction()));
}

void QtFM::loadSettings()
{
    qDebug() << "load settings";
}

void QtFM::writeSettings()
{
    qDebug() << "write settings";
}

void QtFM::handleNewPath(QString path)
{
    Q_UNUSED(path)
    FM *fm = dynamic_cast<FM*>(sender());
    if (!mdi->currentSubWindow() || !fm) {
        qDebug() << "NO TABS OR FM!";
        return;
    }
    if (fm != dynamic_cast<FM*>(mdi->currentSubWindow()->widget())) { return; }
    qDebug() << "update path for tab";
    refreshPath(fm);
}

void QtFM::handleUpdatedDir(QString path)
{
    qDebug() << "handle updated dir" << path;
}

void QtFM::handleTabActivated(QMdiSubWindow *tab)
{
    if (!tab) {
        qDebug() << "NO TAB ACTIVATED";
        return;
    }
    FM *fm = dynamic_cast<FM*>(tab->widget());
    if (!fm) {
        QTermWidget *console = dynamic_cast<QTermWidget*>(tab->widget());
        if (console) {
            qDebug() << "handle term activated" << console->workingDirectory();
            pathEdit->clear();
            pathEdit->setCurrentIndex(0);
            pathEdit->setCurrentText(console->workingDirectory());
            tab->setWindowTitle(console->workingDirectory());
        }
        return;
    }
    qDebug() << "handle tab activated" << fm->getPath();
    tab->setWindowTitle(fm->getPath());
    refreshPath(fm);
}

void QtFM::handleOpenFile(const QString &file)
{
    qDebug() << "handle open file" << file;
}

void QtFM::handlePreviewFile(const QString &file)
{
    qDebug() << "handle preview file" << file;
}

void QtFM::handleNewTermAction()
{
    qDebug() << "handle new terminal";
    newTerminal(QDir::homePath());
}

void QtFM::handleTermTitleChanged()
{
    qDebug() << "handle term title changed";
    QTermWidget *console = dynamic_cast<QTermWidget*>(sender());
    if (!mdi->currentSubWindow() || !console) {
        qDebug() << "NO TABS OR TERM!";
        return;
    }
    if (console != dynamic_cast<QTermWidget*>(mdi->currentSubWindow()->widget())) { return; }
    mdi->currentSubWindow()->setWindowTitle(console->workingDirectory());
}

void QtFM::refreshPath(FM *fm)
{
    if (!fm) { return; }
    pathEdit->clear();
    pathEdit->setCompleter(fm->getCompleter());
    pathEdit->addItems(*fm->getHistory());
    pathEdit->setCurrentIndex(0);
}

void QtFM::pathEditChanged(const QString &path)
{
    qDebug() << "path edit changed" << path;

    QString info = path;
    if (!QFileInfo(path).exists()) {
        qDebug() << "path does not exists" << path;
        return;
    }

    info.replace(QString("~"), QDir::homePath());
    if (!mdi->currentSubWindow()) {
        qDebug() << "NO TABS?";
        return;
    }
    FM *fm = dynamic_cast<FM*>(mdi->currentSubWindow()->widget());
    if (!fm) { return; }

    qDebug() << "set new path in fm" << path;
    fm->setPath(path);
}

// EXPERIMENTAL!
void QtFM::newTerminal(const QString &path)
{
    QFileInfo info(path);
    if (!info.isDir()) { return; }

    qDebug() << "new terminal" << path;
    QMdiSubWindow *subwindow = new QMdiSubWindow(this);
    QTermWidget *console = new QTermWidget(this);

    QFont font = QApplication::font();
#ifdef Q_OS_MACOS
    font.setFamily("Monaco");
#elif defined(Q_WS_QWS)
    font.setFamily("fixed");
#else
    font.setFamily("Monospace");
#endif
    font.setPointSize(9);
    console->setTerminalFont(font);
    console->setTerminalSizeHint(true);
    //console->setBidiEnabled(true);
    //console->setBlinkingCursor(true);
    if (console->availableColorSchemes().contains("WhiteOnBlack")) {
        console->setColorScheme("WhiteOnBlack");
    }
    console->setScrollBarPosition(QTermWidget::ScrollBarRight);
    console->setWorkingDirectory(path);

    connect(console, SIGNAL(finished()), subwindow, SLOT(close()));
    connect(console, SIGNAL(titleChanged()), this, SLOT(handleTermTitleChanged()));

    mdi->addSubWindow(subwindow);
    subwindow->setWidget(console);
    subwindow->setAttribute(Qt::WA_DeleteOnClose);
    subwindow->setWindowTitle(console->workingDirectory());
    subwindow->setWindowIcon(QIcon::fromTheme("utilities-terminal"));
    subwindow->setWindowState(Qt::WindowMaximized);

    if (mdi->subWindowList().count()>1) {
        mdi->tileSubWindows();
    }
}
