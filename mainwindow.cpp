#include "mainwindow.h"
#include <QListWidgetItem>
#include <QStyleFactory>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QMouseEvent>
#include <QShortcut>
#include <QAction>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QBrush>
#include <QPen>
#include <QColor>
#include <QFont>
#include <QClipboard>
#include <QHeaderView>
#include <QStackedWidget>
#include <QGraphicsDropShadowEffect>
#include <QToolButton>
#include <QComboBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <cstdlib>
#include <ctime>
#include <QInputDialog>
#include <QImageReader>
#include <QTransform>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QScrollBar>
#include <QStandardPaths>
#include <QDateTime>
#include <QTextStream>
#include <QCloseEvent>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , gdalDataset(nullptr)
    , hasGeoTransform(false)
    , isGeoTIFFLoaded(false)
    , geoTIFFItem(nullptr)
    , coordinateMarker(nullptr)
    , coordinateTextItem(nullptr)
    , extentsLabel(nullptr)
    , displayInDegrees(true)
    , coordinateModeBtn(nullptr)
    , coordinatesToolBtn(nullptr)
    , coordExtentToggleBtn(nullptr)
    , menuBar(nullptr)
    , fileToolBar(nullptr)
    , mapNavToolBar(nullptr)
    , attributesToolBar(nullptr)
    , labelToolBar(nullptr)
    , databaseToolBar(nullptr)
    , imageToolBar(nullptr)
    , layersDock(nullptr)
    , browserDock(nullptr)
    , processingToolboxDock(nullptr)
    , layerStylingDock(nullptr)
    , imagePropertiesDock(nullptr)
    , mapViewsTabWidget(nullptr)
    , mapView(nullptr)
    , mapScene(nullptr)
    , currentImageItem(nullptr)
    , layersTree(nullptr)
    , browserTree(nullptr)
    , searchLineEdit(nullptr)
    , messageLabel(nullptr)
    , coordinateLabel(nullptr)
    , scaleCombo(nullptr)
    , magnifierLabel(nullptr)
    , rotationLabel(nullptr)
    , projectionLabel(nullptr)
    , imageInfoLabel(nullptr)
    , imageDbConnectionName("")
    ,m_connectionDialogShown(false)
    , dbRefreshBtn(nullptr)
    , saveLocationEdit(nullptr)
    , browseSaveLocationBtn(nullptr)
    , exportFormatCombo(nullptr)
    , projectInfoLabel(nullptr)
    , recentProjectsMenu(nullptr)
    , projectModified(false)
    , currentScale(1.0)
    , rotationAngle(0.0)
    , appSettings(nullptr)
    , newProjectAction(nullptr)
    , openProjectAction(nullptr)
    , saveProjectAction(nullptr)
    , saveAsProjectAction(nullptr)
    , printLayoutAction(nullptr)
    , exitAction(nullptr)
    , newMapViewAction(nullptr)
    , panAction(nullptr)
    , zoomInAction(nullptr)
    , zoomOutAction(nullptr)
    , identifyAction(nullptr)
    , measureAction(nullptr)
    , bookmarkAction(nullptr)
    , toggleEditingAction(nullptr)
    , saveLayerEditsAction(nullptr)
    , openAttributeTableAction(nullptr)
    , addVectorLayerAction(nullptr)
    , addRasterLayerAction(nullptr)
    , addImageLayerAction(nullptr)
    , addDatabaseLayerAction(nullptr)
    , addWfsLayerAction(nullptr)
    , addWmsLayerAction(nullptr)
    , processingAction(nullptr)
    , pythonConsoleAction(nullptr)
    , pluginManagerAction(nullptr)
    , layerPropertiesAction(nullptr)
    , layerStylingAction(nullptr)
    , labelAction(nullptr)
    , loadImageAction(nullptr)
    , clearImageAction(nullptr)
    , fitImageAction(nullptr)
    , rotateLeftAction(nullptr)
    , rotateRightAction(nullptr)
    , zoomImageInAction(nullptr)
    , zoomImageOutAction(nullptr)
    , resetZoomAction(nullptr)
    , exportProjectAction(nullptr)
    , importProjectAction(nullptr)
    , saveLayerAction(nullptr)
    , saveLayerAsAction(nullptr)
    , exportToPdfAction(nullptr)
    , exportToImageAction(nullptr)
    , saveAllLayersAction(nullptr)

{
    // Seed the random number generator
    srand(time(nullptr));

    // Initialize settings
    appSettings = new QSettings("QGISDemo", "Application");

    // Remove default connections
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
    if (QSqlDatabase::contains("main_connection")) {
        QSqlDatabase::removeDatabase("main_connection");
    }

    // Set default project name
    currentProjectName = "Untitled";

    // Set QGIS-like style
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    // Load settings
    loadSettings();

    // Initialize UI components in correct order
    setupMenuBar();
    setupToolBars();        // ← NOW dbConnectionCombo is created!
    setupDockWidgets();
    setupCentralWidget();
    setupStatusBar();

    loadRecentCRS();
    setupCRSSelection();
    setupUI();
    setupConnections();

    // Enable drag and drop
    setAcceptDrops(true);

    // Setup file associations
    setupFileAssociations();

    // Set GDAL configuration
    CPLSetConfigOption("GDAL_PAM_ENABLED", "NO");
    CPLSetConfigOption("GDAL_CACHEMAX", "128");
    CPLSetConfigOption("CPL_DEBUG", "OFF");
    CPLSetConfigOption("CPL_LOG_ERRORS", "OFF");

    // Register GDAL drivers
    GDALAllRegister();

    // Load recent projects
    recentProjects = appSettings->value("recentProjects").toStringList();
    updateRecentProjectsMenu();

    setWindowTitle("PPT GIS Desktop Project - " + currentProjectName);
    resize(1400, 900);

    // Center window
    QTimer::singleShot(0, this, [this]() {
        QScreen *screen = QGuiApplication::primaryScreen();
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    });

    qDebug() << "=== VERIFYING DATABASE ACTIONS ===";
    qDebug() << "connectDBAction:" << connectDBAction;
    qDebug() << "disconnectDBAction:" << disconnectDBAction;
    qDebug() << "storeImageAction:" << storeImageAction;
    qDebug() << "loadFromDBAction:" << loadFromDBAction;
    qDebug() << "dbManagerAction:" << dbManagerAction;
    qDebug() << "dbRefreshBtn:" << dbRefreshBtn;
    qDebug() << "messageLabel:" << messageLabel;
    qDebug() << "dbConnectionCombo:" << dbConnectionCombo;
}
MainWindow::~MainWindow()
{
    // Clean up GDAL dataset
    if (gdalDataset) {
        GDALClose(gdalDataset);
        gdalDataset = nullptr;
    }

    // Clean up settings
    if (appSettings) {
        delete appSettings;
        appSettings = nullptr;
    }

    // Close database connection
    QSqlDatabase db = getImageDb();
    if (db.isValid() && db.isOpen()) {
        QString connName = db.connectionName();
        db.close();
        if (!connName.isEmpty() && QSqlDatabase::contains(connName)) {
            QSqlDatabase::removeDatabase(connName);
        }
        imageDbConnectionName.clear();
    }

    // Remove default database connections
    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
    }
    if (QSqlDatabase::contains("main_connection")) {
        QSqlDatabase::removeDatabase("main_connection");
    }

    qDebug() << "MainWindow destroyed";
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Check for unsaved changes first
    if (projectModified) {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Unsaved Changes",
                    QString("Project '%1' has unsaved changes.\n\nDo you want to save before exiting?")
                    .arg(currentProjectName),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                    QMessageBox::Save
                    );

        if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        } else if (reply == QMessageBox::Save) {
            onSaveProject();
        }
    }

    // Close database BEFORE accepting event
    QSqlDatabase db = getImageDb();
    if (db.isValid() && db.isOpen()) {
        qDebug() << "Closing database in closeEvent";
        QString connName = db.connectionName();
        db.close();

        if (!connName.isEmpty() && QSqlDatabase::contains(connName)) {
            QSqlDatabase::removeDatabase(connName);
        }

        imageDbConnectionName.clear();  // Clear the connection name
    }

    event->accept();
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        // Check if any of the dragged files are supported
        QList<QUrl> urlList = event->mimeData()->urls();
        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);
            QString suffix = fileInfo.suffix().toLower();

            QStringList supportedFormats = {
                "jpg", "jpeg", "png", "gif", "tif", "tiff", "bmp",
                "svg", "ai", "eps", "pdf", "shp", "dbf", "shx", "prj",
                "qgz", "qgs"
            };

            if (supportedFormats.contains(suffix)) {
                event->acceptProposedAction();
                return;
            }
        }
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        bool anyLoaded = false;

        for (const QUrl &url : urlList) {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                loadFile(filePath);
                anyLoaded = true;
            }
        }

        if (anyLoaded) {
            event->acceptProposedAction();
        }
    }
}

void MainWindow::setupUI()
{
    // Set menu bar
    if (menuBar) {
        setMenuBar(menuBar);
    }

    // Add toolbars if they exist
    if (fileToolBar) addToolBar(Qt::TopToolBarArea, fileToolBar);
    if (mapNavToolBar) addToolBar(Qt::TopToolBarArea, mapNavToolBar);
    if (attributesToolBar) addToolBar(Qt::TopToolBarArea, attributesToolBar);
    if (labelToolBar) {
        addToolBarBreak(Qt::TopToolBarArea);
        addToolBar(Qt::TopToolBarArea, labelToolBar);
    }
    if (databaseToolBar) addToolBar(Qt::TopToolBarArea, databaseToolBar);
    if (imageToolBar) addToolBar(Qt::TopToolBarArea, imageToolBar);

    // Add dock widgets if they exist
    if (browserDock) addDockWidget(Qt::LeftDockWidgetArea, browserDock);
    if (layersDock) addDockWidget(Qt::LeftDockWidgetArea, layersDock);
    if (processingToolboxDock) addDockWidget(Qt::RightDockWidgetArea, processingToolboxDock);
    if (layerStylingDock) addDockWidget(Qt::RightDockWidgetArea, layerStylingDock);
    if (imagePropertiesDock) addDockWidget(Qt::RightDockWidgetArea, imagePropertiesDock);

    // Tabify dock widgets
    if (browserDock && layersDock) {
        tabifyDockWidget(browserDock, layersDock);
    }
    if (processingToolboxDock && layerStylingDock) {
        tabifyDockWidget(processingToolboxDock, layerStylingDock);
        tabifyDockWidget(layerStylingDock, imagePropertiesDock);
    }

    // Show browser dock by default (like QGIS)
    if (browserDock) {
        browserDock->raise();
    }
}

void MainWindow::setupMenuBar()
{
    menuBar = new QMenuBar(this);

    // Project Menu
    QMenu *projectMenu = menuBar->addMenu("Project");
    newProjectAction = projectMenu->addAction(QIcon(":/icons/open.png"), "&New Project...", this, &MainWindow::onCreateNewProject, QKeySequence::New);
    openProjectAction = projectMenu->addAction(QIcon(":/icons/folder_open.png"), "&Open Project...", this, &MainWindow::onOpenProject, QKeySequence::Open);
    projectMenu->addSeparator();

    recentProjectsMenu = projectMenu->addMenu(QIcon(":/icons/open_recent.png"), "Open &Recent");
    updateRecentProjectsMenu();

    projectMenu->addSeparator();
    saveProjectAction = projectMenu->addAction(QIcon(":/icons/save.png"), "&Save Project", this, &MainWindow::onSaveProject, QKeySequence::Save);
    saveAsProjectAction = projectMenu->addAction(QIcon(":/icons/saveAs.png"), "Save Project &As...", this, &MainWindow::onSaveAsProject, QKeySequence::SaveAs);

    projectMenu->addSeparator();
    exportProjectAction = projectMenu->addAction(QIcon(":/icons/export.png"), "&Export Project...", this, &MainWindow::onExportProject);
    importProjectAction = projectMenu->addAction(QIcon(":/icons/folder_open.png"), "&Import Project...", this, &MainWindow::onImportProject);

    projectMenu->addSeparator();

    QMenu *importExportMenu = projectMenu->addMenu(QIcon(":/icons/load_image.png"), "Import/Export");
    importExportMenu->addAction(QIcon(":/icons/load_image.png"), "Import Image...", this, &MainWindow::onLoadImage, QKeySequence("Ctrl+I"));
    importExportMenu->addAction(QIcon(":/icons/export.png"), "Export Map...", this, [](){ /* Export map */ });

    saveAllLayersAction = projectMenu->addAction(QIcon(":/icons/save_edit.png"), "Save All Layers...", this, &MainWindow::onSaveAllLayers, QKeySequence("Ctrl+Shift+S"));

    projectMenu->addSeparator();
    printLayoutAction = projectMenu->addAction(QIcon(":/icons/print.png"), "New &Print Layout...", this, &MainWindow::onCreatePrintLayout, QKeySequence("Ctrl+P"));
    projectMenu->addAction(QIcon(":/icons/report.png"), "New &Report...");
    projectMenu->addAction(QIcon(":/icons/layout.png"), "Layout &Manager...");
    projectMenu->addSeparator();
    exitAction = projectMenu->addAction(QIcon(":/icons/exit.png"),"E&xit", this, &QWidget::close, QKeySequence::Quit);

    // Edit Menu
    QMenu *editMenu = menuBar->addMenu("&Edit");
    QAction *undoAction = editMenu->addAction(QIcon(":/icons/undo.png"),"Undo");
    undoAction->setShortcut(QKeySequence::Undo);
    undoAction->setEnabled(false);

    QAction *redoAction = editMenu->addAction(QIcon(":/icons/redo.png"),"Redo");
    redoAction->setShortcut(QKeySequence::Redo);
    redoAction->setEnabled(false);

    editMenu->addSeparator();

    QAction *cutAction = editMenu->addAction(QIcon(":/icons/cutting.png"),"Cut");
    cutAction->setShortcut(QKeySequence::Cut);
    cutAction->setEnabled(false);

    QAction *copyAction = editMenu->addAction(QIcon(":/icons/copy.png"),"Copy");
    copyAction->setShortcut(QKeySequence::Copy);
    copyAction->setEnabled(false);

    QAction *pasteAction = editMenu->addAction(QIcon(":/icons/paste.png"),"Paste");
    pasteAction->setShortcut(QKeySequence::Paste);
    pasteAction->setEnabled(false);

    editMenu->addSeparator();

    QAction *findAction = editMenu->addAction(QIcon(":/icons/find.png"),"Find");
    findAction->setShortcut(QKeySequence::Find);

    // View Menu
    QMenu *viewMenu = menuBar->addMenu("&View");
    newMapViewAction = viewMenu->addAction(QIcon(":/icons/new_map_view.png"), "New &Map View");
    newMapViewAction->setShortcut(QKeySequence("Ctrl+M"));

    viewMenu->addAction(QIcon(":/icons/3d-map.png"), "3D Map Views");
    viewMenu->addSeparator();

    panAction = viewMenu->addAction(QIcon(":/icons/pan.png"), "Pan Map", this, &MainWindow::onPanMap);
    zoomInAction = viewMenu->addAction(QIcon(":/icons/zoom_in.png"), "Zoom In", this, &MainWindow::onZoomIn);
    zoomInAction->setShortcut(QKeySequence("Ctrl++"));

    zoomOutAction = viewMenu->addAction(QIcon(":/icons/zoom_out.png"), "Zoom Out", this, &MainWindow::onZoomOut);
    zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));

    viewMenu->addSeparator();

    // Image view actions
    zoomImageInAction = viewMenu->addAction(QIcon(":/icons/zoom_in.png"), "Zoom Image In", this, &MainWindow::onZoomImageIn);
    zoomImageOutAction = viewMenu->addAction(QIcon(":/icons/zoom_out.png"), "Zoom Image Out", this, &MainWindow::onZoomImageOut);
    resetZoomAction = viewMenu->addAction(QIcon(":/icons/reset_zoom.png"), "Reset Image Zoom", this, &MainWindow::onResetZoom);
    fitImageAction = viewMenu->addAction(QIcon(":/icons/fit_to_view.png"), "Fit Image to View", this, &MainWindow::onFitImage);

    viewMenu->addSeparator();
    rotateLeftAction = viewMenu->addAction(QIcon(":/icons/rotate-left.png"), "Rotate Left", this, &MainWindow::onRotateLeft);
    rotateRightAction = viewMenu->addAction(QIcon(":/icons/rotate_right.png"), "Rotate Right", this, &MainWindow::onRotateRight);

    viewMenu->addSeparator();

    identifyAction = viewMenu->addAction(QIcon(":/icons/identity.png"), "Identify Features");
    identifyAction->setShortcut(QKeySequence("Ctrl+Shift+I"));

    measureAction = viewMenu->addAction(QIcon(":/icons/Measure.png"), "Measure");

    viewMenu->addSeparator();

    bookmarkAction = viewMenu->addAction(QIcon(":/icons/bookmark.png"), "New Spatial Bookmark...", this, &MainWindow::onShowBookmarks);
    bookmarkAction->setShortcut(QKeySequence("Ctrl+B"));

    QAction *showBookmarksAction = viewMenu->addAction(QIcon(":/icons/bookmark.png"), "Show Spatial Bookmarks");
    showBookmarksAction->setShortcut(QKeySequence("Ctrl+Shift+B"));

    viewMenu->addSeparator();

    QAction *zoomFullAction = viewMenu->addAction(QIcon(":/icons/zoom_full.png"), "Zoom Full");
    zoomFullAction->setShortcut(QKeySequence("Ctrl+Shift+F"));

    QAction *zoomToSelectionAction = viewMenu->addAction(QIcon(":/icons/zoom_to_selection.png"), "Zoom to Selection");
    zoomToSelectionAction->setShortcut(QKeySequence("Ctrl+I"));

    QAction *zoomToLayerAction = viewMenu->addAction(QIcon(":/icons/zoom_to_layer.png"), "Zoom to Layer(s)");
    viewMenu->addSeparator();

    QAction *refreshAction = viewMenu->addAction(QIcon(":/icons/refresh.png"), "Refresh");
    refreshAction->setShortcut(QKeySequence("F5"));

    // Layer Menu
    QMenu *layerMenu = menuBar->addMenu("Layer");
    layerMenu->addAction(QIcon(":/icons/new.png"), "Create Layer");

    // UPDATED: Vector layer action with proper connection
    addVectorLayerAction = layerMenu->addAction(QIcon(":/icons/vector_layer.png"),
                                                "Add Vector Layer",
                                                this,
                                                &MainWindow::onAddVectorLayer);
    addVectorLayerAction->setShortcut(QKeySequence("Ctrl+L"));

    addRasterLayerAction = layerMenu->addAction(QIcon(":/icons/raster_layer.png"), "Add Raster Layer", this, &MainWindow::onOpenGeoTIFF);

    // GDAL Menu
    QMenu *gdalMenu = menuBar->addMenu("&GDAL");
    QAction *openGeoTIFFAction = gdalMenu->addAction(QIcon(":/icons/raster_layer.png"),
                                                     "Open GeoTIFF with Coordinates",
                                                     this, &MainWindow::onOpenGeoTIFF);

    addImageLayerAction = layerMenu->addAction(QIcon(":/icons/image_layer.png"), "Add Image Layer", this, &MainWindow::onAddImageLayer);

    layerMenu->addSeparator();

    toggleEditingAction = layerMenu->addAction(QIcon(":/icons/Toggle_editing.png"), "Toggle Editing", this, &MainWindow::onToggleEditing);
    saveLayerEditsAction = layerMenu->addAction(QIcon(":/icons/save_edit.png"), "Save Layer Edits");

    layerMenu->addSeparator();

    saveLayerAction = layerMenu->addAction(QIcon(":/icons/save.png"), "&Save Layer", this, &MainWindow::onSaveLayer, QKeySequence::Save);
    saveLayerAsAction = layerMenu->addAction(QIcon(":/icons/saveAs.png"), "Save Layer &As...", this, &MainWindow::onSaveLayerAs, QKeySequence::SaveAs);

    layerMenu->addSeparator();

    openAttributeTableAction = layerMenu->addAction("Open Attribute Table");
    openAttributeTableAction->setShortcut(QKeySequence("F6"));

    layerMenu->addAction("Filter Attribute Table");
    layerMenu->addSeparator();

    layerPropertiesAction = layerMenu->addAction(QIcon(":/icons/properties.png"), "Layer Properties...", this, &MainWindow::onShowLayerProperties);
    layerMenu->addAction("Filter...");
    layerStylingAction = layerMenu->addAction(QIcon(":/icons/layer_styling.png"), "Styling");
    labelAction = layerMenu->addAction(QIcon(":/icons/label_settings.png"), "Labeling");

    layerMenu->addSeparator();

    exportToPdfAction = layerMenu->addAction(QIcon(":/icons/export.png"), "Export to PDF...", this, &MainWindow::onExportToPdf);
    exportToImageAction = layerMenu->addAction(QIcon(":/icons/export.png"), "Export to Image...", this, &MainWindow::onExportToImage);

    layerMenu->addSeparator();
    layerMenu->addAction("Duplicate Layer(s)");
    layerMenu->addAction("Set CRS of Layer(s)");
    layerMenu->addAction("Set Project CRS from Layer");

    // Settings Menu
    QMenu *settingsMenu = menuBar->addMenu("&Settings");
    settingsMenu->addAction(QIcon(":/icons/layer_styling.png"), "Style Manager...");
    settingsMenu->addAction("Custom Projections...");
    settingsMenu->addAction("Keyboard Shortcuts...");
    settingsMenu->addAction("Interface Customization...");
    settingsMenu->addAction(QIcon(":/icons/properties.png"), "Options...");

    // Plugins Menu
    QMenu *pluginsMenu = menuBar->addMenu("&Plugins");
    pluginsMenu->addAction(QIcon(":/icons/properties.png"), "Manage and Install Plugins...");

    pluginManagerAction = pluginsMenu->addAction(QIcon(":/icons/properties.png"), "Plugin Manager");

    pythonConsoleAction = pluginsMenu->addAction(QIcon(":/icons/python.png"), "Python Console", this, &MainWindow::onShowPythonConsole);
    pythonConsoleAction->setShortcut(QKeySequence("Ctrl+Alt+P"));

    // Vector Menu
    QMenu *vectorMenu = menuBar->addMenu("&Vector");
    vectorMenu->addAction(QIcon(":/icons/processing.png"), "Geoprocessing Tools");
    vectorMenu->addAction(QIcon(":/icons/processing.png"), "Analysis Tools");
    vectorMenu->addAction(QIcon(":/icons/processing.png"), "Research Tools");
    vectorMenu->addAction(QIcon(":/icons/processing.png"), "Data Management Tools");

    // Raster Menu
    QMenu *rasterMenu = menuBar->addMenu("&Raster");
    rasterMenu->addAction(QIcon(":/icons/processing.png"), "Extraction");
    rasterMenu->addAction(QIcon(":/icons/processing.png"), "Analysis");
    rasterMenu->addAction(QIcon(":/icons/processing.png"), "Projections");
    rasterMenu->addAction(QIcon(":/icons/processing.png"), "Miscellaneous");

    // Database Menu
    QMenu *databaseMenu = menuBar->addMenu("&Database");
    databaseMenu->setObjectName("DatabaseMenu");
    databaseMenu->addAction(QIcon(":/icons/DataBase.png"), "DB Manager...");

    // Web Menu
    QMenu *webMenu = menuBar->addMenu("&Web");
    webMenu->addAction("WMS/WMTS...");
    webMenu->addAction("WFS/WCS...");
    webMenu->addAction("WPS...");

    // Mesh Menu
    QMenu *meshMenu = menuBar->addMenu("&Mesh");
    meshMenu->addAction("Mesh Calculator");

    // Processing Menu
    QMenu *processingMenu = menuBar->addMenu("&Processing");
    processingAction = processingMenu->addAction(QIcon(":/icons/processing.png"), "Toolbox", this, &MainWindow::onShowProcessingToolbox);
    processingAction->setShortcut(QKeySequence("Ctrl+Alt+T"));

    processingMenu->addAction(QIcon(":/icons/processing.png"), "Graphical Modeler...");
    processingMenu->addAction(QIcon(":/icons/recent.png"), "History...");
    processingMenu->addAction(QIcon(":/icons/identity.png"), "Results Viewer...");

    // Help Menu
    QMenu *helpMenu = menuBar->addMenu("&Help");
    QAction *helpContentsAction = helpMenu->addAction(QIcon(":/icons/identity.png"), "Help Contents");
    helpContentsAction->setShortcut(QKeySequence::HelpContents);

    helpMenu->addAction(QIcon(":/icons/home.png"), "QGIS Homepage");
    helpMenu->addAction(QIcon(":/icons/refresh.png"), "Check for QGIS Updates");
    helpMenu->addSeparator();
    helpMenu->addAction(QIcon(":/icons/about.png"), "About QGIS");
}
void MainWindow::setupToolBars()
{
    // File Toolbar
    fileToolBar = new QToolBar("File", this);
    fileToolBar->setIconSize(QSize(24, 24));

    if (newProjectAction) {
        newProjectAction->setIcon(QIcon(":/icons/open.png"));
        fileToolBar->addAction(newProjectAction);
    }
    if (openProjectAction) {
        openProjectAction->setIcon(QIcon(":/icons/folder_open.png"));
        fileToolBar->addAction(openProjectAction);
    }
    if (saveProjectAction) {
        saveProjectAction->setIcon(QIcon(":/icons/save.png"));
        fileToolBar->addAction(saveProjectAction);
    }
    fileToolBar->addSeparator();
    if (saveAsProjectAction) {
        saveAsProjectAction->setIcon(QIcon(":/icons/saveAs.png"));
        fileToolBar->addAction(saveAsProjectAction);
    }
    fileToolBar->addSeparator();
    if (printLayoutAction) {
        printLayoutAction->setIcon(QIcon(":/icons/print.png"));
        fileToolBar->addAction(printLayoutAction);
    }
    fileToolBar->addSeparator();
    if (saveAllLayersAction) {
        saveAllLayersAction->setIcon(QIcon(":/icons/save_edit.png"));
        fileToolBar->addAction(saveAllLayersAction);
    }
    if (imageToolBar) {
        imageToolBar->addSeparator();
        QAction *openGeoTIFFActionTB = imageToolBar->addAction(QIcon(":/icons/raster_layer.png"),
                                                               "Open GeoTIFF",
                                                               this, &MainWindow::onOpenGeoTIFF);
    }

    // In setupMenuBar():
    QMenu *databaseMenu = menuBar->findChild<QMenu*>("&Database");
    if (!databaseMenu) {
        databaseMenu = menuBar->addMenu("&Database");
    }

    connectDBAction = databaseMenu->addAction(
                QIcon(":/icons/database.png"),
                "Connect to Database...",
                this,
                &MainWindow::onConnectDatabase);

    disconnectDBAction = databaseMenu->addAction(
                QIcon(":/icons/disconnect.png"),
                "Disconnect",
                this,
                &MainWindow::onDisconnectDatabase);
    disconnectDBAction->setEnabled(false);

    databaseMenu->addSeparator();

    storeImageAction = databaseMenu->addAction(
                QIcon(":/icons/save.png"),
                "Store Current Image...",
                this,
                &MainWindow::onStoreCurrentImage);
    storeImageAction->setEnabled(false);

    loadFromDBAction = databaseMenu->addAction(
                QIcon(":/icons/load_image.png"),
                "Load from Database...",
                this,
                &MainWindow::onLoadFromDatabase);
    loadFromDBAction->setEnabled(false);

    databaseMenu->addSeparator();

    dbManagerAction = databaseMenu->addAction(
                QIcon(":/icons/database.png"),
                "Database Manager...",
                this,
                &MainWindow::onShowDatabaseManager);
    dbManagerAction->setEnabled(false);

    // Map Navigation Toolbar
    mapNavToolBar = new QToolBar("Map Navigation", this);
    mapNavToolBar->setIconSize(QSize(24, 24));

    // Navigation actions
    QAction *panMapAction = mapNavToolBar->addAction(QIcon(":/icons/pan.png"), "Pan", this, &MainWindow::onPanMap);
    panMapAction->setCheckable(true);

    QAction *zoomInActionTB = mapNavToolBar->addAction(QIcon(":/icons/zoom_in.png"), "Zoom In", this, &MainWindow::onZoomIn);
    QAction *zoomOutActionTB = mapNavToolBar->addAction(QIcon(":/icons/zoom_out.png"), "Zoom Out", this, &MainWindow::onZoomOut);

    mapNavToolBar->addSeparator();

    QAction *zoomFullActionTB = mapNavToolBar->addAction(QIcon(":/icons/zoom_full.png"), "Zoom Full");
    QAction *zoomToLayerActionTB = mapNavToolBar->addAction(QIcon(":/icons/zoom_to_layer.png"), "Zoom to Layer");
    QAction *zoomToSelectionActionTB = mapNavToolBar->addAction(QIcon(":/icons/zoom_to_selection.png"), "Zoom to Selection");

    mapNavToolBar->addSeparator();

    QAction *identifyActionTB = mapNavToolBar->addAction(QIcon(":/icons/identity.png"), "Identify");
    QAction *measureActionTB = mapNavToolBar->addAction(QIcon(":/icons/Measure.png"), "Measure");
    QAction *bookmarkActionTB = mapNavToolBar->addAction(QIcon(":/icons/bookmark.png"), "Bookmark", this, &MainWindow::onShowBookmarks);

    // Image Toolbar
    imageToolBar = new QToolBar("Image Tools", this);
    imageToolBar->setIconSize(QSize(24, 24));

    loadImageAction = imageToolBar->addAction(QIcon(":/icons/load_image.png"), "Load Image", this, &MainWindow::onLoadImage);
    clearImageAction = imageToolBar->addAction(QIcon(":/icons/clear_image.png"), "Clear Image", this, &MainWindow::onClearImage);
    imageToolBar->addSeparator();

    zoomImageInAction = imageToolBar->addAction(QIcon(":/icons/zoom_in.png"), "Zoom In", this, &MainWindow::onZoomImageIn);
    zoomImageOutAction = imageToolBar->addAction(QIcon(":/icons/zoom_out.png"), "Zoom Out", this, &MainWindow::onZoomImageOut);
    resetZoomAction = imageToolBar->addAction(QIcon(":/icons/reset_zoom.png"), "Reset Zoom", this, &MainWindow::onResetZoom);
    fitImageAction = imageToolBar->addAction(QIcon(":/icons/fit_to_view.png"), "Fit to View", this, &MainWindow::onFitImage);
    imageToolBar->addSeparator();

    rotateLeftAction = imageToolBar->addAction(QIcon(":/icons/rotate-left.png"), "Rotate Left", this, &MainWindow::onRotateLeft);
    rotateRightAction = imageToolBar->addAction(QIcon(":/icons/rotate_right.png"), "Rotate Right", this, &MainWindow::onRotateRight);
    imageToolBar->addSeparator();

    if (saveLayerAction) {
        saveLayerAction->setIcon(QIcon(":/icons/save.png"));
        imageToolBar->addAction(saveLayerAction);
    }
    if (saveLayerAsAction) {
        saveLayerAsAction->setIcon(QIcon(":/icons/save_layer_As.png"));
        imageToolBar->addAction(saveLayerAsAction);
    }

    // Attributes Toolbar
    attributesToolBar = new QToolBar("Attributes", this);
    attributesToolBar->setIconSize(QSize(24, 24));

    QAction *toggleEditAction = attributesToolBar->addAction(QIcon(":/icons/Toggle_editing.png"), "Toggle Editing", this, &MainWindow::onToggleEditing);
    toggleEditAction->setCheckable(true);

    QAction *saveEditsAction = attributesToolBar->addAction(QIcon(":/icons/save_edit.png"), "Save Edits");
    QAction *cancelEditsAction = attributesToolBar->addAction(QIcon(":/icons/cancel.png"),"Cancel Edits");

    attributesToolBar->addSeparator();

    QAction *addFeatureAction = attributesToolBar->addAction(QIcon(":/icons/add_feature.png"), "Add Feature");
    QAction *moveFeatureAction = attributesToolBar->addAction(QIcon(":/icons/move_feature.png"), "Move Feature");
    QAction *deleteFeatureAction = attributesToolBar->addAction(QIcon(":/icons/delete_feature.png"), "Delete Feature");

    attributesToolBar->addSeparator();

    QAction *openAttrTableAction = attributesToolBar->addAction(QIcon(":/icons/attribute.png"),"Attribute Table");

    // Label Toolbar
    labelToolBar = new QToolBar("Label", this);
    labelToolBar->setIconSize(QSize(24, 24));

    QAction *labelSettingsAction = labelToolBar->addAction(QIcon(":/icons/Label_settings.png"), "Label Settings");

    QComboBox *labelFontCombo = new QComboBox();
    labelFontCombo->addItems({"Arial", "Times New Roman", "Verdana", "Courier New"});
    labelFontCombo->setMaximumWidth(120);
    labelToolBar->addWidget(labelFontCombo);

    QSpinBox *labelSizeSpin = new QSpinBox();
    labelSizeSpin->setRange(6, 72);
    labelSizeSpin->setValue(10);
    labelSizeSpin->setMaximumWidth(60);
    labelToolBar->addWidget(labelSizeSpin);

    // Database Toolbar
    databaseToolBar = new QToolBar("Database", this);
    databaseToolBar->setIconSize(QSize(24, 24));

    QAction *dbManagerAction = databaseToolBar->addAction(QIcon(":/icons/DataBase.png"), "DB Manager");
    databaseToolBar->addSeparator();

    dbConnectionCombo = new QComboBox();
    dbConnectionCombo->addItems({"PostgreSQL", "SpatiaLite", "MS SQL Server", "Oracle"});
    dbConnectionCombo->setMaximumWidth(120);
    databaseToolBar->addWidget(dbConnectionCombo);

    // ADD THIS - Initialize dbRefreshBtn
    dbRefreshBtn = new QToolButton();
    dbRefreshBtn->setIcon(QIcon(":/icons/refresh.png"));
    dbRefreshBtn->setToolTip("Refresh Database Connection");
    dbRefreshBtn->setEnabled(false);
    databaseToolBar->addWidget(dbRefreshBtn);
}

void MainWindow::setupDockWidgets()
{
    // Browser Dock
    browserDock = new QDockWidget("Browser", this);
    browserDock->setObjectName("Browser");
    browserDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *browserWidget = new QWidget();
    QVBoxLayout *browserLayout = new QVBoxLayout(browserWidget);
    browserLayout->setContentsMargins(5, 5, 5, 5);

    // Save Location
    QHBoxLayout *locationLayout = new QHBoxLayout();
    QLabel *locationLabel = new QLabel("Save Location:");
    saveLocationEdit = new QLineEdit(getSaveLocation());
    browseSaveLocationBtn = new QPushButton("Browse...");
    browseSaveLocationBtn->setIcon(QIcon(":/icons/folder_open.png"));

    connect(saveLocationEdit, &QLineEdit::editingFinished, this, &MainWindow::onChangeSaveLocation);
    connect(browseSaveLocationBtn, &QPushButton::clicked, this, &MainWindow::onBrowseSaveLocation);

    locationLayout->addWidget(locationLabel);
    locationLayout->addWidget(saveLocationEdit);
    locationLayout->addWidget(browseSaveLocationBtn);

    // Project Info
    projectInfoLabel = new QLabel("Project: " + currentProjectName + "\nLayers: 0");
    projectInfoLabel->setStyleSheet("padding: 5px; background-color: #f0f8ff; border: 1px solid #ccc; border-radius: 3px;");

    // Quick Actions
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    QPushButton *saveAllBtn = new QPushButton(QIcon(":/icons/save_edit.png"), "Save All");
    QPushButton *exportBtn = new QPushButton(QIcon(":/icons/export.png"), "Export");
    QPushButton *importBtn = new QPushButton(QIcon(":/icons/folder_open.png"), "Import");

    connect(saveAllBtn, &QPushButton::clicked, this, &MainWindow::onSaveAllLayers);
    connect(exportBtn, &QPushButton::clicked, this, &MainWindow::onExportProject);
    connect(importBtn, &QPushButton::clicked, this, &MainWindow::onImportProject);

    actionsLayout->addWidget(saveAllBtn);
    actionsLayout->addWidget(exportBtn);
    actionsLayout->addWidget(importBtn);

    // Search bar
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search in browser...");
    searchEdit->addAction(QIcon(":/icons/identity.png"), QLineEdit::LeadingPosition);
    browserLayout->addWidget(searchEdit);

    // Browser tree
    browserTree = new QTreeWidget();
    browserTree->setHeaderHidden(true);
    browserTree->setColumnCount(1);
    browserTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Add browser items with icons
    QTreeWidgetItem *projectSection = new QTreeWidgetItem(browserTree, QStringList() << "Project");
    projectSection->setIcon(0, QIcon(":/icons/folder.png"));

    QTreeWidgetItem *currentProject = new QTreeWidgetItem(projectSection, QStringList() << "Current: " + currentProjectName);
    currentProject->setIcon(0, QIcon(":/icons/documents.png"));

    QTreeWidgetItem *projectProperties = new QTreeWidgetItem(projectSection, QStringList() << "Properties");
    projectProperties->setIcon(0, QIcon(":/icons/properties.png"));

    QTreeWidgetItem *favorites = new QTreeWidgetItem(browserTree, QStringList() << "Favorites");
    favorites->setIcon(0, QIcon(":/icons/starred.png"));

    QTreeWidgetItem *home = new QTreeWidgetItem(favorites, QStringList() << "Home");
    home->setIcon(0, QIcon(":/icons/home.png"));

    QTreeWidgetItem *desktop = new QTreeWidgetItem(favorites, QStringList() << "Desktop");
    desktop->setIcon(0, QIcon(":/icons/documents.png"));

    QTreeWidgetItem *imagesFolder = new QTreeWidgetItem(favorites, QStringList() << "Images");
    imagesFolder->setIcon(0, QIcon(":/icons/load_image.png"));

    QTreeWidgetItem *projectTemplates = new QTreeWidgetItem(browserTree, QStringList() << "Project Templates");
    projectTemplates->setIcon(0, QIcon(":/icons/folder.png"));

    QTreeWidgetItem *favoriteTemplate = new QTreeWidgetItem(projectTemplates, QStringList() << "Favorite");
    favoriteTemplate->setIcon(0, QIcon(":/icons/starred.png"));

    QTreeWidgetItem *spatialBookmarks = new QTreeWidgetItem(projectTemplates, QStringList() << "Spatial Bookmarks");
    spatialBookmarks->setIcon(0, QIcon(":/icons/bookmark.png"));

    QTreeWidgetItem *homeTemplate = new QTreeWidgetItem(projectTemplates, QStringList() << "Home");
    homeTemplate->setIcon(0, QIcon(":/icons/home.png"));

    QTreeWidgetItem *layersSection = new QTreeWidgetItem(browserTree, QStringList() << "Layers");
    layersSection->setIcon(0, QIcon(":/icons/folder.png"));

    QTreeWidgetItem *geoTIFFLayers = new QTreeWidgetItem(layersSection, QStringList() << "GeoTIFF Layers");
    geoTIFFLayers->setIcon(0, QIcon(":/icons/raster_layer.png"));

    QTreeWidgetItem *rasterLayers = new QTreeWidgetItem(layersSection, QStringList() << "Raster Layers");
    rasterLayers->setIcon(0, QIcon(":/icons/image_layer.png"));

    QTreeWidgetItem *vectorLayers = new QTreeWidgetItem(layersSection, QStringList() << "Vector Layers");
    vectorLayers->setIcon(0, QIcon(":/icons/vector_layer.png"));

    QTreeWidgetItem *databases = new QTreeWidgetItem(browserTree, QStringList() << "Databases");
    databases->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *postgis = new QTreeWidgetItem(databases, QStringList() << "PostGIS");
    postgis->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *spatialite = new QTreeWidgetItem(databases, QStringList() << "SpatiaLite");
    spatialite->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *mssql = new QTreeWidgetItem(databases, QStringList() << "MS SQL Server");
    mssql->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *webServices = new QTreeWidgetItem(browserTree, QStringList() << "Web Services");
    webServices->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *wms = new QTreeWidgetItem(webServices, QStringList() << "WMS/WMTS");
    wms->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *wfs = new QTreeWidgetItem(webServices, QStringList() << "WFS");
    wfs->setIcon(0, QIcon(":/icons/DataBase.png"));

    QTreeWidgetItem *xyzTiles = new QTreeWidgetItem(webServices, QStringList() << "XYZ Tiles");
    xyzTiles->setIcon(0, QIcon(":/icons/DataBase.png"));

    browserTree->expandAll();
    browserLayout->addWidget(browserTree);

    browserDock->setWidget(browserWidget);

    // Layers Dock
    layersDock = new QDockWidget("Layers", this);
    layersDock->setObjectName("Layers");
    layersDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *layersWidget = new QWidget();
    QVBoxLayout *layersLayout = new QVBoxLayout(layersWidget);
    layersLayout->setContentsMargins(5, 5, 5, 5);

    // Layers tree
    layersTree = new QTreeWidget();
    layersTree->setHeaderLabels(QStringList() << "Layer" << "Type");
    layersTree->setColumnCount(2);
    layersTree->setIndentation(15);
    layersTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Set column widths
    layersTree->header()->setStretchLastSection(false);
    layersTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    layersTree->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    layersTree->header()->resizeSection(1, 80);

    // Add sample layer groups with icons
    QTreeWidgetItem *layerGroup1 = new QTreeWidgetItem(layersTree, QStringList() << "Vector Layers");
    layerGroup1->setExpanded(true);
    layerGroup1->setIcon(0, QIcon(":/icons/folder.png"));

    QTreeWidgetItem *layerGroup2 = new QTreeWidgetItem(layersTree, QStringList() << "Raster Layers");
    layerGroup2->setExpanded(true);
    layerGroup2->setIcon(0, QIcon(":/icons/folder.png"));

    QTreeWidgetItem *layerGroup3 = new QTreeWidgetItem(layersTree, QStringList() << "Image Layers");
    layerGroup3->setExpanded(true);
    layerGroup3->setIcon(0, QIcon(":/icons/folder.png"));

    layersLayout->addWidget(layersTree);

    // Layer buttons with icons
    QHBoxLayout *layerButtonsLayout = new QHBoxLayout();
    QPushButton *addLayerBtn = new QPushButton(QIcon(":/icons/add_feature.png"), "Add Layer");
    QPushButton *removeLayerBtn = new QPushButton(QIcon(":/icons/clear_image.png"), "Remove");
    QPushButton *layerPropertiesBtn = new QPushButton(QIcon(":/icons/properties.png"), "Properties");

    connect(addLayerBtn, &QPushButton::clicked, this, &MainWindow::onAddImageLayer);
    connect(removeLayerBtn, &QPushButton::clicked, this, &MainWindow::onRemoveLayer);
    connect(layerPropertiesBtn, &QPushButton::clicked, this, &MainWindow::onShowLayerProperties);

    layerButtonsLayout->addWidget(addLayerBtn);
    layerButtonsLayout->addWidget(removeLayerBtn);
    layerButtonsLayout->addWidget(layerPropertiesBtn);
    layersLayout->addLayout(layerButtonsLayout);

    layersDock->setWidget(layersWidget);

    // Processing Toolbox Dock
    processingToolboxDock = new QDockWidget("Processing Toolbox", this);
    processingToolboxDock->setObjectName("Processing");
    processingToolboxDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *processingWidget = new QWidget();
    QVBoxLayout *processingLayout = new QVBoxLayout(processingWidget);
    processingLayout->setContentsMargins(5, 5, 5, 5);

    QLineEdit *processingSearch = new QLineEdit();
    processingSearch->setPlaceholderText("Search algorithms...");
    processingSearch->addAction(QIcon(":/icons/identity.png"), QLineEdit::LeadingPosition);
    processingLayout->addWidget(processingSearch);

    QTreeWidget *processingTree = new QTreeWidget();
    processingTree->setHeaderHidden(true);

    QTreeWidgetItem *geoProcessing = new QTreeWidgetItem(processingTree, QStringList() << "Geoprocessing");
    geoProcessing->setIcon(0, QIcon(":/icons/processing.png"));
    new QTreeWidgetItem(geoProcessing, QStringList() << "Buffer");
    new QTreeWidgetItem(geoProcessing, QStringList() << "Clip");
    new QTreeWidgetItem(geoProcessing, QStringList() << "Intersection");

    QTreeWidgetItem *analysis = new QTreeWidgetItem(processingTree, QStringList() << "Analysis");
    analysis->setIcon(0, QIcon(":/icons/processing.png"));
    new QTreeWidgetItem(analysis, QStringList() << "Line Intersections");
    new QTreeWidgetItem(analysis, QStringList() << "Sum Line Lengths");

    QTreeWidgetItem *research = new QTreeWidgetItem(processingTree, QStringList() << "Research");
    research->setIcon(0, QIcon(":/icons/processing.png"));
    new QTreeWidgetItem(research, QStringList() << "Random Points");
    new QTreeWidgetItem(research, QStringList() << "Regular Points");

    processingTree->expandAll();
    processingLayout->addWidget(processingTree);

    processingToolboxDock->setWidget(processingWidget);

    // Layer Styling Dock
    layerStylingDock = new QDockWidget("Layer Styling", this);
    layerStylingDock->setObjectName("Styling");
    layerStylingDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *stylingWidget = new QWidget();
    QVBoxLayout *stylingLayout = new QVBoxLayout(stylingWidget);
    stylingLayout->setContentsMargins(5, 5, 5, 5);

    QComboBox *layerCombo = new QComboBox();
    layerCombo->addItems({"Roads", "Buildings", "Parcels", "DEM"});
    layerCombo->setItemIcon(0, QIcon(":/icons/arrow.png"));
    layerCombo->setItemIcon(1, QIcon(":/icons/add_feature.png"));
    layerCombo->setItemIcon(2, QIcon(":/icons/move_feature.png"));
    layerCombo->setItemIcon(3, QIcon(":/icons/raster_layer.png"));
    stylingLayout->addWidget(layerCombo);

    QTabWidget *stylingTabs = new QTabWidget();
    stylingTabs->setIconSize(QSize(16, 16));

    QWidget *symbologyTab = new QWidget();
    QWidget *labelsTab = new QWidget();
    QWidget *masksTab = new QWidget();

    stylingTabs->addTab(symbologyTab, QIcon(":/icons/layer_styling.png"), "Symbology");
    stylingTabs->addTab(labelsTab, QIcon(":/icons/label_settings.png"), "Labels");
    stylingTabs->addTab(masksTab, QIcon(":/icons/layer_styling.png"), "Masks");

    stylingLayout->addWidget(stylingTabs);
    layerStylingDock->setWidget(stylingWidget);

    // Image Properties Dock
    imagePropertiesDock = new QDockWidget("Image Properties", this);
    imagePropertiesDock->setObjectName("ImageProperties");
    imagePropertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *imagePropsWidget = new QWidget();
    QVBoxLayout *imagePropsLayout = new QVBoxLayout(imagePropsWidget);
    imagePropsLayout->setContentsMargins(5, 5, 5, 5);

    QLabel *imageTitle = new QLabel("Image Information");
    imageTitle->setStyleSheet("font-weight: bold; font-size: 14px;");
    imagePropsLayout->addWidget(imageTitle);

    imageInfoLabel = new QLabel("No image loaded");
    imageInfoLabel->setWordWrap(true);
    imageInfoLabel->setStyleSheet("padding: 10px; background-color: #f0f0f0; border-radius: 5px;");
    imagePropsLayout->addWidget(imageInfoLabel);

    QLabel *controlsTitle = new QLabel("Image Controls");
    controlsTitle->setStyleSheet("font-weight: bold; margin-top: 10px;");
    imagePropsLayout->addWidget(controlsTitle);

    // Zoom slider
    QHBoxLayout *zoomLayout = new QHBoxLayout();
    QLabel *zoomLabel = new QLabel("Zoom:");
    QSlider *zoomSlider = new QSlider(Qt::Horizontal);
    zoomSlider->setRange(10, 500);
    zoomSlider->setValue(100);
    QLabel *zoomPercent = new QLabel("100%");

    zoomLayout->addWidget(zoomLabel);
    zoomLayout->addWidget(zoomSlider);
    zoomLayout->addWidget(zoomPercent);
    imagePropsLayout->addLayout(zoomLayout);

    // Rotation controls
    QHBoxLayout *rotateLayout = new QHBoxLayout();
    QLabel *rotateLabel = new QLabel("Rotation:");
    QPushButton *rotateLeftBtn = new QPushButton(QIcon(":/icons/rotate-left.png"), "");
    QPushButton *rotateRightBtn = new QPushButton(QIcon(":/icons/rotate_right.png"), "");
    QLabel *rotateDegrees = new QLabel("0°");

    rotateLeftBtn->setFixedSize(30, 30);
    rotateRightBtn->setFixedSize(30, 30);

    connect(rotateLeftBtn, &QPushButton::clicked, this, &MainWindow::onRotateLeft);
    connect(rotateRightBtn, &QPushButton::clicked, this, &MainWindow::onRotateRight);

    rotateLayout->addWidget(rotateLabel);
    rotateLayout->addWidget(rotateLeftBtn);
    rotateLayout->addWidget(rotateRightBtn);
    rotateLayout->addWidget(rotateDegrees);
    imagePropsLayout->addLayout(rotateLayout);

    // Additional controls
    QHBoxLayout *controlButtonsLayout = new QHBoxLayout();
    QPushButton *fitToViewBtn = new QPushButton(QIcon(":/icons/fit_to_view.png"), "");
    QPushButton *resetZoomBtn = new QPushButton(QIcon(":/icons/reset_zoom.png"), "");

    fitToViewBtn->setFixedSize(30, 30);
    resetZoomBtn->setFixedSize(30, 30);

    fitToViewBtn->setToolTip("Fit to View");
    resetZoomBtn->setToolTip("Reset Zoom");

    connect(fitToViewBtn, &QPushButton::clicked, this, &MainWindow::onFitImage);
    connect(resetZoomBtn, &QPushButton::clicked, this, &MainWindow::onResetZoom);

    controlButtonsLayout->addWidget(fitToViewBtn);
    controlButtonsLayout->addWidget(resetZoomBtn);
    controlButtonsLayout->addStretch();

    imagePropsLayout->addLayout(controlButtonsLayout);
    imagePropsLayout->addStretch();

    imagePropertiesDock->setWidget(imagePropsWidget);
}

void MainWindow::setupCentralWidget()
{
    // Create central widget with map view
    QWidget *centralWidget = new QWidget();
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // Map view tab widget (like QGIS)
    mapViewsTabWidget = new QTabWidget();
    mapViewsTabWidget->setTabsClosable(true);
    mapViewsTabWidget->setMovable(true);

    // Create main map view
    mapScene = new QGraphicsScene(this);
    mapView = new QGraphicsView(mapScene);
    mapView->setRenderHint(QPainter::Antialiasing, true);
    mapView->setDragMode(QGraphicsView::ScrollHandDrag);
    mapView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    mapView->setBackgroundBrush(QBrush(QColor(240, 240, 240)));
    mapView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mapView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    mapView->setContextMenuPolicy(Qt::CustomContextMenu);

    mapViewsTabWidget->addTab(mapView, "Map");

    centralLayout->addWidget(mapViewsTabWidget);
    setCentralWidget(centralWidget);
}

void MainWindow::setupStatusBar()
{
    // Get or create the main status bar
    QStatusBar *statusBar = this->statusBar();
    if (!statusBar) {
        statusBar = new QStatusBar(this);
        setStatusBar(statusBar);
    }

    // Clear any existing widgets
    QList<QLabel*> labels = statusBar->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        statusBar->removeWidget(label);
        delete label;
    }

    QList<QLineEdit*> lineEdits = statusBar->findChildren<QLineEdit*>();
    for (QLineEdit* edit : lineEdits) {
        statusBar->removeWidget(edit);
        delete edit;
    }

    QList<QComboBox*> combos = statusBar->findChildren<QComboBox*>();
    for (QComboBox* combo : combos) {
        statusBar->removeWidget(combo);
        delete combo;
    }

    QList<QToolButton*> buttons = statusBar->findChildren<QToolButton*>();
    for (QToolButton* btn : buttons) {
        statusBar->removeWidget(btn);
        delete btn;
    }

    // Clear any progress bars
    QList<QProgressBar*> progressBars = statusBar->findChildren<QProgressBar*>();
    for (QProgressBar* pb : progressBars) {
        statusBar->removeWidget(pb);
        delete pb;
    }

    // =========== LEFT-EDGE SEARCH BOX ===========
    searchLineEdit = new QLineEdit();
    searchLineEdit->setPlaceholderText("Type to locate (Ctrl+K)");
    searchLineEdit->setMinimumWidth(200);
    searchLineEdit->setMaximumWidth(300);
    searchLineEdit->setClearButtonEnabled(true);
    searchLineEdit->setStyleSheet(
                "QLineEdit { "
                "padding: 2px 8px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: white; "
                "margin: 1px; "
                "}"
                );

    // Add search icon
    searchLineEdit->addAction(QIcon(":/icons/search.png"), QLineEdit::LeadingPosition);
    connect(searchLineEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    statusBar->addWidget(searchLineEdit);

    // =========== MESSAGE AREA ===========
    messageLabel = new QLabel("Ready");
    messageLabel->setMinimumWidth(100);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    messageLabel->setStyleSheet(
                "QLabel { "
                "padding: 2px 8px; "
                "margin: 1px; "
                "}"
                );
    statusBar->addWidget(messageLabel);

    // =========== COORDINATES/EXTENTS TOGGLE DISPLAY ===========
    QWidget *coordExtentWidget = new QWidget();
    QHBoxLayout *coordExtentLayout = new QHBoxLayout(coordExtentWidget);
    coordExtentLayout->setContentsMargins(0, 0, 0, 0);
    coordExtentLayout->setSpacing(2);

    // Toggle button
    coordExtentToggleBtn = new QToolButton();
    coordExtentToggleBtn->setIcon(QIcon(":/icons/coordinates.png"));
    coordExtentToggleBtn->setToolTip("Toggle between Coordinates and Extents\nClick or press Ctrl+E");
    coordExtentToggleBtn->setCheckable(true);
    coordExtentToggleBtn->setChecked(true);
    coordExtentToggleBtn->setStyleSheet(
                "QToolButton { "
                "padding: 3px 5px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: #f0f8ff; "
                "margin-right: 2px; "
                "min-width: 28px; "
                "min-height: 24px; "
                "}"
                "QToolButton:checked { "
                "background-color: #2196F3; "
                "color: white; "
                "}"
                "QToolButton:hover { "
                "background-color: #d0e8ff; "
                "}"
                );

    // Stacked widget for coordinates/extents
    QStackedWidget *coordExtentStack = new QStackedWidget();
    coordExtentStack->setMinimumWidth(250);
    coordExtentStack->setMaximumWidth(320);

    // Coordinate display (page 0)
    coordinateLabel = new QLabel("Coordinates: Move mouse");
    coordinateLabel->setAlignment(Qt::AlignCenter);
    coordinateLabel->setStyleSheet(
                "QLabel { "
                "padding: 3px 8px; "
                "background-color: white; "
                "font-family: monospace; "
                "font-size: 11px; "
                "border: 1px solid #d0d0d0; "
                "border-radius: 3px; "
                "min-height: 22px; "
                "}"
                );
    coordinateLabel->setToolTip("Current cursor position\nClick to copy coordinates");

    // Extents display (page 1)
    extentsLabel = new QLabel("Extents: No data loaded");
    extentsLabel->setAlignment(Qt::AlignCenter);
    extentsLabel->setStyleSheet(
                "QLabel { "
                "padding: 3px 8px; "
                "background-color: white; "
                "font-family: monospace; "
                "font-size: 11px; "
                "border: 1px solid #d0d0d0; "
                "border-radius: 3px; "
                "min-height: 22px; "
                "}"
                );
    extentsLabel->setToolTip("Layer extents\nClick to copy");

    // Add pages
    coordExtentStack->addWidget(coordinateLabel);
    coordExtentStack->addWidget(extentsLabel);

    // Connect toggle button
    connect(coordExtentToggleBtn, &QToolButton::toggled, this,
            [this, coordExtentStack](bool checked) {
        onToggleCoordExtentDisplay(checked, coordExtentStack);
    });

    // Make labels clickable
    connect(coordinateLabel, &QLabel::linkActivated, this,
            [this]() { onCopyCoordinatesToClipboard(); });
    connect(extentsLabel, &QLabel::linkActivated, this,
            [this]() { onCopyExtentsToClipboard(); });

    coordExtentLayout->addWidget(coordExtentToggleBtn);
    coordExtentLayout->addWidget(coordExtentStack);
    coordExtentWidget->setLayout(coordExtentLayout);
    statusBar->addPermanentWidget(coordExtentWidget);

    // =========== JUMP TO LOCATION BUTTON ===========
    QToolButton *jumpBtn = new QToolButton();
    jumpBtn->setIcon(QIcon(":/icons/geo_jump.png"));
    jumpBtn->setToolTip("Jump to coordinates (Ctrl+J)");
    jumpBtn->setStyleSheet(
                "QToolButton { "
                "padding: 3px 5px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: #f0f8ff; "
                "margin: 1px 3px; "
                "min-width: 28px; "
                "min-height: 24px; "
                "}"
                "QToolButton:hover { "
                "background-color: #e0f0ff; "
                "}"
                );
    connect(jumpBtn, &QToolButton::clicked, this, &MainWindow::showCoordinatePicker);
    statusBar->addPermanentWidget(jumpBtn);

    // =========== COORDINATE MODE TOGGLE ===========
    coordinateModeBtn = new QToolButton();
    coordinateModeBtn->setText("Deg");
    coordinateModeBtn->setToolTip("Toggle coordinate format: Degrees/Decimal (Ctrl+D)");
    coordinateModeBtn->setCheckable(true);
    coordinateModeBtn->setChecked(true);
    coordinateModeBtn->setStyleSheet(
                "QToolButton { "
                "padding: 3px 8px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: #f0f8ff; "
                "margin: 1px 3px; "
                "min-width: 35px; "
                "min-height: 24px; "
                "}"
                "QToolButton:checked { "
                "background-color: #4CAF50; "
                "color: white; "
                "}"
                "QToolButton:hover { "
                "background-color: #e0f0ff; "
                "}"
                );

    displayInDegrees = true;
    connect(coordinateModeBtn, &QToolButton::toggled, this,
            [this](bool checked) { onToggleCoordinateMode(checked); });
    statusBar->addPermanentWidget(coordinateModeBtn);

    // =========== SCALE DISPLAY ===========
    QWidget *scaleWidget = new QWidget();
    QHBoxLayout *scaleLayout = new QHBoxLayout(scaleWidget);
    scaleLayout->setContentsMargins(0, 0, 0, 0);
    scaleLayout->setSpacing(2);

    QLabel *scaleTextLabel = new QLabel("Scale:");
    scaleTextLabel->setStyleSheet("QLabel { margin-right: 2px; }");

    scaleCombo = new QComboBox();
    scaleCombo->setEditable(true);
    scaleCombo->setMinimumWidth(120);
    scaleCombo->addItems({
                             "1:500", "1:1000", "1:2500", "1:5000", "1:10000",
                             "1:25000", "1:50000", "1:100000", "1:250000",
                             "1:500000", "1:1000000", "1:2500000"
                         });
    scaleCombo->setCurrentText("1:1000");
    scaleCombo->setStyleSheet(
                "QComboBox { "
                "border: 1px solid #aaa; "
                "padding: 3px 6px; "
                "background-color: white; "
                "min-height: 22px; "
                "}"
                );

    connect(scaleCombo, &QComboBox::currentTextChanged, this,
            [this](const QString &text) { onScaleChanged(text); });

    scaleLayout->addWidget(scaleTextLabel);
    scaleLayout->addWidget(scaleCombo);
    scaleWidget->setLayout(scaleLayout);
    statusBar->addPermanentWidget(scaleWidget);

    // =========== OTHER DISPLAYS ===========
    magnifierLabel = new QLabel("Magnifier: 100%");
    magnifierLabel->setMinimumWidth(100);
    magnifierLabel->setAlignment(Qt::AlignCenter);
    magnifierLabel->setStyleSheet(
                "QLabel { "
                "padding: 3px 8px; "
                "border: 1px solid #aaa; "
                "background-color: white; "
                "margin: 1px; "
                "min-height: 22px; "
                "}"
                );
    statusBar->addPermanentWidget(magnifierLabel);

    rotationLabel = new QLabel("Rotation: 0.0°");
    rotationLabel->setMinimumWidth(100);
    rotationLabel->setAlignment(Qt::AlignCenter);
    rotationLabel->setStyleSheet(
                "QLabel { "
                "padding: 3px 8px; "
                "border: 1px solid #aaa; "
                "background-color: white; "
                "margin: 1px; "
                "min-height: 22px; "
                "}"
                );
    statusBar->addPermanentWidget(rotationLabel);

    projectionLabel = new QLabel("Render: EPSG:4326");
    projectionLabel->setMinimumWidth(150);
    projectionLabel->setAlignment(Qt::AlignCenter);
    projectionLabel->setStyleSheet(
                "QLabel { "
                "padding: 3px 8px; "
                "border: 1px solid #aaa; "
                "border-right: 3px solid #d0d0d0; "
                "background-color: white; "
                "margin: 1px; "
                "margin-right: 3px; "
                "min-height: 22px; "
                "}"
                );
    statusBar->addPermanentWidget(projectionLabel);

    // =========== COORDINATE TOOL BUTTON ===========
    coordinatesToolBtn = new QToolButton();
    coordinatesToolBtn->setIcon(QIcon(":/icons/crs.png"));
    coordinatesToolBtn->setToolTip("Coordinate capture tool (Ctrl+C)");
    coordinatesToolBtn->setCheckable(true);
    coordinatesToolBtn->setStyleSheet(
                "QToolButton { "
                "padding: 3px 5px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: #f0f8ff; "
                "margin: 1px 3px; "
                "min-width: 28px; "
                "min-height: 24px; "
                "}"
                "QToolButton:checked { "
                "background-color: #FF9800; "
                "color: white; "
                "}"
                "QToolButton:hover { "
                "background-color: #e0f0ff; "
                "}"
                );
    connect(coordinatesToolBtn, &QToolButton::toggled, this, &MainWindow::onCoordinatesToolToggled);
    statusBar->addPermanentWidget(coordinatesToolBtn);

    // =========== PROGRESS BAR ===========
    QProgressBar *progressBar = new QProgressBar();
    progressBar->setMaximumWidth(150);
    progressBar->setMinimumWidth(100);
    progressBar->setVisible(false);
    progressBar->setStyleSheet(
                "QProgressBar { "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "padding: 0px; "
                "margin: 1px 5px; "
                "min-height: 22px; "
                "}"
                );
    statusBar->addPermanentWidget(progressBar);

    // =========== SETUP KEYBOARD SHORTCUTS ===========
    setupStatusBarShortcuts();

    // =========== INITIALIZE DISPLAYS ===========
    updateCoordinates(QPointF(0, 0));
    updateExtentsDisplay();
    updateScale(1.0);
    updateMagnifier(100);
    updateRotation(0.0);
    updateProjection("EPSG:4326");

    if (messageLabel) {
        messageLabel->setText("Ready");
    }
}

void MainWindow::updateExtentsDisplayLabel(QLabel* extentsDisplayLabel)
{
    if (!extentsDisplayLabel) return;

    QString displayText = "Ext: ";

    if (isGeoTIFFLoaded && gdalDataset && hasGeoTransform && geoTIFFItem) {
        // Calculate GeoTIFF extents
        double topLeftX = gdalGeoTransform[0];
        double topLeftY = gdalGeoTransform[3];

        double bottomRightX = gdalGeoTransform[0] +
                geoTIFFSize.width() * gdalGeoTransform[1] +
                geoTIFFSize.height() * gdalGeoTransform[2];

        double bottomRightY = gdalGeoTransform[3] +
                geoTIFFSize.width() * gdalGeoTransform[4] +
                geoTIFFSize.height() * gdalGeoTransform[5];

        // Format for compact display
        QString x1 = QString::number(topLeftX, 'f', displayInDegrees ? 3 : 2);
        QString y1 = QString::number(topLeftY, 'f', displayInDegrees ? 3 : 2);
        QString x2 = QString::number(bottomRightX, 'f', displayInDegrees ? 3 : 2);
        QString y2 = QString::number(bottomRightY, 'f', displayInDegrees ? 3 : 2);

        QString suffix = displayInDegrees ? "°" : "";
        displayText += QString("TL(%1%4,%2%5) BR(%3%4,%2%5)")
                .arg(x1).arg(y1).arg(x2).arg(suffix);

    } else if (currentImageItem) {
        // For regular images
        QRectF bounds = currentImageItem->boundingRect();
        displayText += QString("TL(0,0) BR(%1,%2)")
                .arg(bounds.width(), 0, 'f', 0)
                .arg(bounds.height(), 0, 'f', 0);

    } else if (!loadedLayers.isEmpty()) {
        // For vector layers
        QRectF combinedBounds;
        bool first = true;

        for (const LayerInfo &layer : loadedLayers) {
            if (layer.graphicsItem) {
                QRectF bounds = layer.graphicsItem->boundingRect();
                if (first) {
                    combinedBounds = bounds;
                    first = false;
                } else {
                    combinedBounds = combinedBounds.united(bounds);
                }
            }
        }

        if (!combinedBounds.isEmpty()) {
            QString suffix = displayInDegrees ? "°" : "";
            displayText += QString("TL(%1%4,%2%5) BR(%3%4,%2%5)")
                    .arg(combinedBounds.left(), 0, 'f', 1)
                    .arg(combinedBounds.top(), 0, 'f', 1)
                    .arg(combinedBounds.right(), 0, 'f', 1)
                    .arg(suffix);
        } else {
            displayText += "No bounds";
        }

    } else {
        displayText += "No data";
    }

    extentsDisplayLabel->setText(displayText);

    // Make it clickable to copy
    extentsDisplayLabel->setCursor(Qt::PointingHandCursor);
    connect(extentsDisplayLabel, &QLabel::linkActivated, this, [displayText]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(displayText);
    });
}

void MainWindow::onToggleCoordExtentDisplay(bool showCoordinates, QStackedWidget* stackWidget)
{
    if (!stackWidget) return;

    if (showCoordinates) {
        // Show coordinates
        if (coordExtentToggleBtn) {
            coordExtentToggleBtn->setIcon(QIcon(":/icons/coordinates.png"));
            coordExtentToggleBtn->setToolTip("Showing Coordinates\nClick to show Extents");
        }
        stackWidget->setCurrentIndex(0); // Show coordinates page
        updateCoordinates(QPointF(0, 0)); // Refresh display
    } else {
        // Show extents
        if (coordExtentToggleBtn) {
            coordExtentToggleBtn->setIcon(QIcon(":/icons/extent.png"));
            coordExtentToggleBtn->setToolTip("Showing Extents\nClick to show Coordinates");
        }
        stackWidget->setCurrentIndex(1); // Show extents page
        updateExtentsDisplay(); // Refresh display
    }

    if (messageLabel) {
        messageLabel->setText(showCoordinates ? "Showing Coordinates" : "Showing Extents");
    }
}

void MainWindow::onScaleChanged(const QString &text)
{
    if (text.isEmpty()) return;

    // Parse scale from text like "1:1000"
    QStringList parts = text.split(":");
    if (parts.size() == 2 && parts[0] == "1") {
        bool ok;
        double denominator = parts[1].toDouble(&ok);
        if (ok && denominator > 0) {
            double scale = 1.0 / denominator;
            if (mapView) {
                mapView->resetTransform();
                mapView->scale(scale, scale);
                currentScale = scale;

                // Update image info if needed
                updateImageInfo();

                // Update magnifier when scale changes
                if (magnifierLabel) {
                    magnifierLabel->setText(QString("Magnifier: %1%").arg(qRound(currentScale * 100)));
                }

                if (messageLabel) {
                    messageLabel->setText(QString("Scale set to: %1").arg(text));
                }
            }
        }
    } else {
        // Try to parse as a decimal scale (e.g., "0.001")
        bool ok;
        double scale = text.toDouble(&ok);
        if (ok && scale > 0) {
            if (mapView) {
                mapView->resetTransform();
                mapView->scale(scale, scale);
                currentScale = scale;

                // Update image info if needed
                updateImageInfo();

                // Update magnifier
                if (magnifierLabel) {
                    magnifierLabel->setText(QString("Magnifier: %1%").arg(qRound(currentScale * 100)));
                }

                // Convert to 1:XXXX format for display
                int denominator = qRound(1.0 / scale);
                if (scaleCombo && denominator > 0) {
                    QString scaleText = QString("1:%1").arg(denominator);
                    bool found = false;
                    for (int i = 0; i < scaleCombo->count(); i++) {
                        if (scaleCombo->itemText(i) == scaleText) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        scaleCombo->addItem(scaleText);
                    }
                    if (scaleCombo->currentText() != scaleText) {
                        scaleCombo->setCurrentText(scaleText);
                    }
                }

                if (messageLabel) {
                    messageLabel->setText(QString("Scale set to: 1:%1").arg(denominator));
                }
            }
        }
    }
}

void MainWindow::setupConnections()
{
    // Connect browser tree clicks
    if (browserTree) {
        connect(browserTree, &QTreeWidget::itemClicked, this, &MainWindow::onBrowserItemClicked);
    }

    // Connect layers tree
    if (layersTree) {
        connect(layersTree, &QTreeWidget::itemChanged, [this](QTreeWidgetItem *item, int column) {
            if (column == 0 && item && messageLabel) {
                // Layer visibility toggled
                bool visible = (item->checkState(0) == Qt::Checked);
                QString layerName = item->text(0);
                updateLayerVisibility(layerName, visible);

                messageLabel->setText(layerName + " visibility: " + (visible ? "ON" : "OFF"));
            }
        });

        connect(layersTree, &QTreeWidget::itemDoubleClicked, this, &MainWindow::onLayerItemDoubleClicked);
        connect(layersTree, &QTreeWidget::customContextMenuRequested, this, &MainWindow::onLayerContextMenuRequested);
    }

    // Map view interactions
    if (mapView) {
        connect(mapView, &QGraphicsView::customContextMenuRequested, [this](const QPoint &pos) {
            QMenu contextMenu;
            if (zoomInAction) {
                zoomInAction->setIcon(QIcon(":/icons/zoom_in.png"));
                contextMenu.addAction(zoomInAction);
            }
            if (zoomOutAction) {
                zoomOutAction->setIcon(QIcon(":/icons/zoom_out.png"));
                contextMenu.addAction(zoomOutAction);
            }
            if (panAction) {
                panAction->setIcon(QIcon(":/icons/pan.png"));
                contextMenu.addAction(panAction);
            }
            contextMenu.addSeparator();
            if (loadImageAction) {
                loadImageAction->setIcon(QIcon(":/icons/load_image.png"));
                contextMenu.addAction(loadImageAction);
            }
            if (clearImageAction) {
                clearImageAction->setIcon(QIcon(":/icons/clear_image.png"));
                contextMenu.addAction(clearImageAction);
            }
            contextMenu.addSeparator();
            if (identifyAction) {
                identifyAction->setIcon(QIcon(":/icons/identity.png"));
                contextMenu.addAction(identifyAction);
            }
            if (measureAction) {
                measureAction->setIcon(QIcon(":/icons/Measure.png"));
                contextMenu.addAction(measureAction);
            }
            if (bookmarkAction) {
                bookmarkAction->setIcon(QIcon(":/icons/bookmark.png"));
                contextMenu.addAction(bookmarkAction);
            }
            contextMenu.addSeparator();
            if (saveAllLayersAction) {
                saveAllLayersAction->setIcon(QIcon(":/icons/save_edit.png"));
                contextMenu.addAction(saveAllLayersAction);
            }
            if (mapView) {
                contextMenu.exec(mapView->mapToGlobal(pos));
            }
        });

        // Install event filter for mouse tracking
        mapView->viewport()->installEventFilter(this);
    }
}

// =========== STATUS BAR HELPER METHODS ===========

//void MainWindow::updateCoordinates(const QPointF &coord)
//{
//    if (coordinateLabel) {
//        QString coordText;

//        if (isGeoTIFFLoaded && gdalDataset && hasGeoTransform) {
//            // For GeoTIFF with geographic coordinates
//            coordText = QString("Coordinate: %1°, %2°")
//                .arg(coord.x(), 0, 'f', 1)
//                .arg(coord.y(), 0, 'f', 1);
//        } else {
//            // For regular images/empty map
//            coordText = QString("Coordinate: %1, %2")
//                .arg(coord.x(), 0, 'f', 1)
//                .arg(coord.y(), 0, 'f', 1);
//        }

//        coordinateLabel->setText(coordText);
//    }
//}

void MainWindow::onToggleCoordinateMode(bool isDegrees)
{
    displayInDegrees = isDegrees;

    if (coordinateModeBtn) {
        coordinateModeBtn->setText(isDegrees ? "Deg" : "Dec");
        coordinateModeBtn->setToolTip(isDegrees ?
                                          "Degrees format\nClick for Decimal" :
                                          "Decimal format\nClick for Degrees");
    }

    // Refresh both displays
    updateCoordinates(QPointF(0, 0));
    updateExtentsDisplay();

    if (messageLabel) {
        messageLabel->setText("Coordinate format: " +
                              QString(isDegrees ? "Degrees" : "Decimal"));
    }
}

void MainWindow::onCopyCoordinatesToClipboard()
{
    if (coordinateLabel) {
        QString text = coordinateLabel->text();
        // Remove "Coordinates: " prefix if present
        if (text.startsWith("Coordinates: ")) {
            text = text.mid(13);
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);

        if (messageLabel) {
            messageLabel->setText("Coordinates copied to clipboard");
        }
    }
}

void MainWindow::onCopyExtentsToClipboard()
{
    if (extentsLabel) {
        QString text = extentsLabel->text();
        // Remove "Extents: " prefix if present
        if (text.startsWith("Extents: ")) {
            text = text.mid(9);
        }

        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(text);

        if (messageLabel) {
            messageLabel->setText("Extents copied to clipboard");
        }
    }
}

void MainWindow::updateScale(double scale)
{
    if (scale <= 0) scale = 1.0;

    // Calculate scale denominator (1:XXXX)
    int denominator = qRound(1.0 / scale);
    if (denominator < 1) denominator = 1;

    // Update scale combo if it exists
    if (scaleCombo) {
        QString scaleText = QString("1:%1").arg(denominator);

        // Check if this scale is in our preset list
        bool found = false;
        for (int i = 0; i < scaleCombo->count(); i++) {
            if (scaleCombo->itemText(i) == scaleText) {
                found = true;
                break;
            }
        }

        if (!found) {
            // Add custom scale
            scaleCombo->addItem(scaleText);
        }

        if (scaleCombo->currentText() != scaleText) {
            scaleCombo->setCurrentText(scaleText);
        }
    }
}

void MainWindow::setupStatusBarShortcuts()
{
    // Jump to coordinates

    // Jump to coordinates
    QShortcut *jumpShortcut = new QShortcut(QKeySequence("Ctrl+J"), this);
    connect(jumpShortcut, &QShortcut::activated, this, &MainWindow::showCoordinatePicker);

    // Clear markers
    QShortcut *clearMarkerShortcut = new QShortcut(QKeySequence("Ctrl+Shift+M"), this);
    connect(clearMarkerShortcut, &QShortcut::activated, this, &MainWindow::removeCoordinateMarker);

    // Zoom to marker
    QShortcut *zoomMarkerShortcut = new QShortcut(QKeySequence("Ctrl+Shift+Z"), this);
    connect(zoomMarkerShortcut, &QShortcut::activated, this, [this]() {
        if (coordinateMarker) {
            QRectF markerRect = coordinateMarker->boundingRect();
            QPointF center = coordinateMarker->mapToScene(markerRect.center());
            mapView->centerOn(center);
        }
    });
    // Toggle coordinate capture tool
    QShortcut *coordToolShortcut = new QShortcut(QKeySequence("Ctrl+C"), this);
    connect(coordToolShortcut, &QShortcut::activated, this, [this]() {
        if (coordinatesToolBtn) {
            coordinatesToolBtn->toggle();
        }
    });

    // Toggle between coordinates and extents
    QShortcut *toggleDisplayShortcut = new QShortcut(QKeySequence("Ctrl+E"), this);
    connect(toggleDisplayShortcut, &QShortcut::activated, this, [this]() {
        if (coordExtentToggleBtn) {
            coordExtentToggleBtn->toggle();
        }
    });

    // Toggle degree/decimal format
    QShortcut *coordModeShortcut = new QShortcut(QKeySequence("Ctrl+D"), this);
    connect(coordModeShortcut, &QShortcut::activated, this, [this]() {
        if (coordinateModeBtn) {
            coordinateModeBtn->toggle();
        }
    });
}
void MainWindow::updateMagnifier(int percentage)
{
    if (magnifierLabel) {
        magnifierLabel->setText(QString("Magnifier: %1%").arg(percentage));
    }
}

void MainWindow::updateRotation(qreal angle)
{
    if (rotationLabel) {
        rotationLabel->setText(QString("Rotation: %1°").arg(angle, 0, 'f', 1));
    }
}

void MainWindow::updateProjection(const QString &crs)
{
    if (projectionLabel) {
        projectionLabel->setText(QString("Render: %1").arg(crs));
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    if (messageLabel) {
        if (text.isEmpty()) {
            messageLabel->setText("Ready");
        } else {
            messageLabel->setText("Searching for: " + text);
            // Here you would implement actual search functionality
            // For now, just update the message
        }
    }
}

// File Management Methods
void MainWindow::setupFileAssociations()
{
    // Register supported MIME types
    QMimeDatabase mimeDb;

    // Raster formats
    QStringList rasterFormats;
    rasterFormats << "image/jpeg" << "image/png" << "image/gif"
                  << "image/tiff" << "image/tif" << "image/x-tiff" << "image/bmp";

    // Vector formats
    QStringList vectorFormats;
    vectorFormats << "image/svg+xml" << "application/postscript"
                  << "application/pdf" << "application/vnd.adobe.illustrator"
                  << "application/x-shapefile";

    // Project formats
    QStringList projectFormats;
    projectFormats << "application/x-qgis-project" << "application/x-gzip";
}

QString MainWindow::getSupportedFilesFilter()
{
    return tr("All Supported Files (*.jpg *.jpeg *.png *.gif *.tif *.tiff *.bmp "
              "*.svg *.ai *.eps *.pdf *.shp *.dbf *.shx *.prj *.qgz *.qgs);;"
              "Raster Files (*.jpg *.jpeg *.png *.gif *.tif *.tiff *.bmp);;"
              "Vector Files (*.svg *.ai *.eps *.pdf *.shp);;"
              "QGIS Projects (*.qgz *.qgs);;"
              "All Files (*)");
}

QString MainWindow::getVectorFilesFilter()
{
    return tr("Vector Files (*.svg *.ai *.eps *.pdf *.shp);;"
              "SVG Files (*.svg);;"
              "Adobe Illustrator (*.ai);;"
              "EPS Files (*.eps);;"
              "PDF Files (*.pdf);;"
              "Shapefiles (*.shp);;"
              "All Files (*)");
}

QString MainWindow::getRasterFilesFilter()
{
    return tr("Raster Files (*.jpg *.jpeg *.png *.gif *.tif *.tiff *.bmp);;"
              "JPEG Files (*.jpg *.jpeg);;"
              "PNG Files (*.png);;"
              "GIF Files (*.gif);;"
              "TIFF Files (*.tif *.tiff);;"
              "BMP Files (*.bmp);;"
              "All Files (*)");
}

QString MainWindow::getImageFilesFilter()
{
    return tr("Image Files (*.jpg *.jpeg *.png *.gif *.bmp *.tif *.tiff);;"
              "All Files (*)");
}

QString MainWindow::getSaveLocation()
{
    if (defaultSaveLocation.isEmpty() || !QDir(defaultSaveLocation).exists()) {
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        defaultSaveLocation = QDir(documentsPath).filePath("QGIS Projects");

        // Create directory if it doesn't exist
        QDir dir(defaultSaveLocation);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
    }

    return defaultSaveLocation;
}

void MainWindow::updateSaveLocation(const QString &path)
{
    if (QDir(path).exists()) {
        defaultSaveLocation = path;
        if (saveLocationEdit) {
            saveLocationEdit->setText(path);
        }
        saveSettings();
    }
}

// Project Management Methods
void MainWindow::createNewProjectDialog()
{
    // First check if we have unsaved changes in current project
    if (projectModified && !currentProjectName.isEmpty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "Unsaved Changes",
                    QString("Project '%1' has unsaved changes.\n\nDo you want to save before creating a new project?")
                    .arg(currentProjectName),
                    QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                    QMessageBox::Save
                    );

        if (reply == QMessageBox::Cancel) {
            return;  // User cancelled
        } else if (reply == QMessageBox::Save) {
            onSaveProject();  // Save current project first
        }
        // If Discard, continue without saving
    }

    // Check if we already have a project (even if not modified)
    if (!currentProjectName.isEmpty() && currentProjectName != "Untitled") {
        QMessageBox::StandardButton reply = QMessageBox::question(
                    this,
                    "New Project Confirmation",
                    QString("Project '%1' is already open.\n\nDo you want to create a new project anyway?")
                    .arg(currentProjectName),
                    QMessageBox::Yes | QMessageBox::No,
                    QMessageBox::No
                    );

        if (reply == QMessageBox::No) {
            return;  // User doesn't want to create new project
        }
    }

    // Get new project name
    bool ok;
    QString projectName = QInputDialog::getText(this,
                                                "New Project",
                                                "Enter project name:",
                                                QLineEdit::Normal,
                                                "My Project",
                                                &ok);

    if (ok && !projectName.isEmpty()) {
        // Clear current content
        clearCurrentImage();

        // Clear loaded layers
        for (int i = loadedLayers.size() - 1; i >= 0; --i) {
            if (loadedLayers[i].graphicsItem) {
                mapScene->removeItem(loadedLayers[i].graphicsItem);
                delete loadedLayers[i].graphicsItem;
            }
        }
        loadedLayers.clear();

        // Clear layers tree (remove only child items, keep groups)
        if (layersTree) {
            for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
                QTreeWidgetItem *group = layersTree->topLevelItem(i);
                while (group->childCount() > 0) {
                    delete group->takeChild(0);
                }
            }
        }

        // Update project state
        currentProjectName = projectName;
        currentProjectPath = "";
        projectModified = false;  // New project, no modifications yet

        // Update window title
        setWindowTitle("PPT GIS Desktop Project - " + projectName);

        // Update browser tree
        if (browserTree && browserTree->topLevelItemCount() > 0) {
            QTreeWidgetItem *projectSection = browserTree->topLevelItem(0);
            if (projectSection && projectSection->childCount() > 0) {
                projectSection->child(0)->setText(0, "Current: " + projectName);
            }
        }

        // Update project info
        if (projectInfoLabel) {
            projectInfoLabel->setText(QString("Project: %1\nLayers: 0").arg(projectName));
        }

        if (messageLabel) {
            messageLabel->setText("Created new project: " + projectName);
        }

        // Update status bar
        updateProjection("EPSG:4326");
        updateCoordinates(QPointF(0, 0));
        updateScale(1.0);
        updateMagnifier(100);
        updateRotation(0.0);
    }
}

void MainWindow::saveProject()
{
    if (currentProjectPath.isEmpty()) {
        onSaveAsProject();
        return;
    }

    // Save all layers
    saveAllLayers();

    // Here you would save the project file
    QMessageBox::information(this, "Save Project",
                             "Project saved: " + currentProjectName);

    projectModified = false;  // Reset modified flag after saving

    if (messageLabel) {
        messageLabel->setText("Project saved: " + currentProjectName);
    }
}

void MainWindow::loadProject(const QString &projectPath)
{
    QFileInfo fileInfo(projectPath);
    currentProjectName = fileInfo.baseName();
    currentProjectPath = projectPath;
    projectModified = false;  // Loaded project is not modified

    setWindowTitle("PPT GIS Desktop Project - " + currentProjectName);

    // Update browser tree
    if (browserTree && browserTree->topLevelItemCount() > 0) {
        QTreeWidgetItem *projectSection = browserTree->topLevelItem(0);
        if (projectSection && projectSection->childCount() > 0) {
            projectSection->child(0)->setText(0, "Current: " + currentProjectName);
        }
    }

    // Update project info
    if (projectInfoLabel) {
        projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                  .arg(currentProjectName)
                                  .arg(loadedLayers.size()));
    }

    if (messageLabel) {
        messageLabel->setText("Loaded project: " + currentProjectName);
    }

    // Add to recent projects
    addRecentProject(projectPath);
}

void MainWindow::addRecentProject(const QString &projectPath)
{
    // Remove if already exists
    recentProjects.removeAll(projectPath);

    // Add to front
    recentProjects.prepend(projectPath);

    // Keep only last 10
    if (recentProjects.size() > 10) {
        recentProjects = recentProjects.mid(0, 10);
    }

    updateRecentProjectsMenu();
}

void MainWindow::updateRecentProjectsMenu()
{
    if (recentProjectsMenu) {
        recentProjectsMenu->clear();

        if (recentProjects.isEmpty()) {
            recentProjectsMenu->addAction("No recent projects")->setEnabled(false);
        } else {
            for (const QString &project : recentProjects) {
                QFileInfo fileInfo(project);
                recentProjectsMenu->addAction(fileInfo.fileName(), this, &MainWindow::onOpenRecentProject);
            }
        }
    }
}

// File Loading Methods
void MainWindow::loadFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    // Check file type and load accordingly
    if (suffix == "shp" || suffix == "svg" || suffix == "pdf" ||
            suffix == "ai" || suffix == "eps") {
        loadVectorFile(filePath);
    } else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" ||
               suffix == "gif" || suffix == "tif" || suffix == "tiff" || suffix == "bmp") {
        loadRasterFile(filePath);
    } else if (suffix == "qgz" || suffix == "qgs") {
        loadProject(filePath);
    } else {
        QMessageBox::warning(this, "Unsupported Format",
                             "File format not supported: " + suffix);
    }
}

void MainWindow::loadVectorFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString layerName = fileInfo.baseName();

    // Check if layer already loaded
    for (int i = 0; i < loadedLayers.size(); ++i) {
        if (loadedLayers[i].name == layerName && loadedLayers[i].type == "vector") {
            QMessageBox::information(this, "Layer Exists",
                                     "Vector layer already loaded: " + layerName);
            return;
        }
    }

    // Create layer info
    LayerInfo layer;
    layer.name = layerName;
    layer.filePath = filePath;
    layer.type = "vector";
    layer.properties["format"] = fileInfo.suffix().toLower();

    // For demonstration, create a graphical representation
    QGraphicsItem *graphicsItem = nullptr;

    if (fileInfo.suffix().toLower() == "shp") {
        // Simulate shapefile - create random polygons
        QGraphicsPolygonItem *polygonItem = new QGraphicsPolygonItem();
        QPolygonF polygon;
        for (int i = 0; i < 6; ++i) {
            double angle = 2 * M_PI * i / 6;
            polygon << QPointF(cos(angle) * 50 + rand() % 200,
                               sin(angle) * 50 + rand() % 200);
        }
        polygonItem->setPolygon(polygon);
        polygonItem->setBrush(QBrush(QColor(100, 150, 200, 100)));
        polygonItem->setPen(QPen(Qt::darkBlue, 2));
        graphicsItem = polygonItem;

    } else if (fileInfo.suffix().toLower() == "svg") {
        // For SVG files, create a placeholder
        QGraphicsRectItem *rectItem = new QGraphicsRectItem(0, 0, 100, 100);
        rectItem->setBrush(QBrush(QColor(150, 200, 150, 100)));
        rectItem->setPen(QPen(Qt::darkGreen, 2));
        QGraphicsTextItem *textItem = new QGraphicsTextItem("SVG");
        textItem->setPos(25, 35);
        textItem->setDefaultTextColor(Qt::darkGreen);
        textItem->setFont(QFont("Arial", 12, QFont::Bold));
        graphicsItem = rectItem;
        mapScene->addItem(textItem);
    } else {
        // For other vector formats, create a placeholder
        QGraphicsRectItem *rectItem = new QGraphicsRectItem(0, 0, 100, 100);
        rectItem->setBrush(QBrush(QColor(200, 100, 100, 100)));
        rectItem->setPen(QPen(Qt::darkRed, 2));
        graphicsItem = rectItem;
    }

    if (graphicsItem) {
        layer.graphicsItem = graphicsItem;
        mapScene->addItem(graphicsItem);

        // Add to layers tree
        QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                    QStringList() << layerName << "Vector");
        layerItem->setCheckState(0, Qt::Checked);
        layer.treeItem = layerItem;

        // Find or create vector group
        QTreeWidgetItem *vectorGroup = nullptr;
        for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
            if (layersTree->topLevelItem(i)->text(0) == "Vector Layers") {
                vectorGroup = layersTree->topLevelItem(i);
                break;
            }
        }

        if (!vectorGroup) {
            vectorGroup = new QTreeWidgetItem(layersTree, QStringList() << "Vector Layers");
            vectorGroup->setExpanded(true);
        }

        vectorGroup->addChild(layerItem);

        loadedLayers.append(layer);
        projectModified = true;  // Mark project as modified

        // Update project info
        if (projectInfoLabel) {
            projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                      .arg(currentProjectName)
                                      .arg(loadedLayers.size()));
        }

        if (messageLabel) {
            messageLabel->setText("Loaded vector layer: " + layerName);
        }

        emit layerLoaded(layerName, "vector");
    }
}

void MainWindow::loadRasterFile(const QString &filePath)
{
    QPixmap pixmap;
    if (!pixmap.load(filePath)) {
        QImage image(filePath);
        if (!image.isNull()) {
            pixmap = QPixmap::fromImage(image);
        }
    }

    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Error", "Cannot load raster file: " + filePath);
        return;
    }

    QFileInfo fileInfo(filePath);
    QString layerName = fileInfo.baseName();

    // Check if layer already exists
    for (const LayerInfo &layer : loadedLayers) {
        if (layer.name == layerName && layer.type.startsWith("georef")) {
            QMessageBox::information(this, "Layer Exists", "Layer already loaded: " + layerName);
            return;
        }
    }

    // Check if file is a GeoTIFF
    bool isGeoTIFF = fileInfo.suffix().toLower() == "tif" ||
            fileInfo.suffix().toLower() == "tiff";

    // Try to get geotransform
    double geoTransform[6] = {0, 1, 0, 0, 0, -1}; // Default identity transform
    bool hasGeoInfo = false;
    QString projection;
    bool isMainGeoTIFF = false;

    if (isGeoTIFF) {
        GDALDataset *dataset = (GDALDataset*)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly);
        if (dataset) {
            hasGeoInfo = (dataset->GetGeoTransform(geoTransform) == CE_None);
            if (hasGeoInfo) {
                const char *wkt = dataset->GetProjectionRef();
                if (wkt && strlen(wkt) > 0) {
                    projection = QString(wkt);
                }

                // Check if this is the first/main GeoTIFF
                if (!isGeoTIFFLoaded) {
                    isMainGeoTIFF = true;
                    isGeoTIFFLoaded = true;
                    hasGeoTransform = true;
                    memcpy(gdalGeoTransform, geoTransform, sizeof(double) * 6);
                    geoTIFFSize = QSize(pixmap.width(), pixmap.height());
                    gdalDataset = dataset;
                }
            }
            if (!isMainGeoTIFF) {
                GDALClose(dataset);
            }
        }
    }

    // Create graphics item
    QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(pixmap);

    // Store georeference information
    GeoreferenceInfo georefInfo;
    georefInfo.imageItem = pixmapItem;
    georefInfo.filePath = filePath;
    georefInfo.hasTransform = hasGeoInfo;
    georefInfo.imageSize = QSize(pixmap.width(), pixmap.height());

    if (hasGeoInfo) {
        memcpy(georefInfo.geoTransform, geoTransform, sizeof(double) * 6);
        georefInfo.projection = projection;

        // Position the image based on geotransform
        double topLeftX = geoTransform[0];
        double topLeftY = geoTransform[3];

        // Convert to scene coordinates
        QPointF scenePos = geographicToSceneCoords(topLeftX, topLeftY);
        if (!scenePos.isNull()) {
            pixmapItem->setPos(scenePos);

            // If it's the main GeoTIFF, store it
            if (isMainGeoTIFF) {
                geoTIFFItem = pixmapItem;
                currentImageItem = pixmapItem;
                currentImagePath = filePath;
                currentPixmap = pixmap;
            }
        }
    }

    // Store in the list
    georeferencedImagesInfo.append(georefInfo);
    mapScene->addItem(pixmapItem);

    // Create layer info
    LayerInfo layer;
    layer.name = layerName;
    layer.filePath = filePath;

    if (isMainGeoTIFF) {
        layer.type = "geotiff";
        layer.properties["is_main_geotiff"] = true;
    } else if (hasGeoInfo) {
        layer.type = "georeferenced";
    } else {
        layer.type = "raster";
    }

    layer.graphicsItem = pixmapItem;
    layer.properties["has_geotransform"] = hasGeoInfo;
    layer.properties["width"] = pixmap.width();
    layer.properties["height"] = pixmap.height();

    if (hasGeoInfo) {
        layer.properties["top_left_x"] = geoTransform[0];
        layer.properties["top_left_y"] = geoTransform[3];
        layer.properties["pixel_width"] = geoTransform[1];
        layer.properties["pixel_height"] = geoTransform[5];
        layer.properties["rotation_x"] = geoTransform[2];
        layer.properties["rotation_y"] = geoTransform[4];
    }

    // Add to layers tree
    QString layerType;
    QString iconPath;

    if (isMainGeoTIFF) {
        layerType = "GeoTIFF";
        iconPath = ":/icons/geotiff.png";
    } else if (hasGeoInfo) {
        layerType = "Georeferenced";
        iconPath = ":/icons/georeferenced.png";
    } else {
        layerType = "Raster";
        iconPath = ":/icons/raster_layer.png";
    }

    QTreeWidgetItem *layerItem = new QTreeWidgetItem(QStringList() << layerName << layerType);
    layerItem->setCheckState(0, Qt::Checked);
    layerItem->setIcon(0, QIcon(iconPath));
    layer.treeItem = layerItem;

    // Find or create appropriate group
    QString groupName;
    if (isMainGeoTIFF) {
        groupName = "GeoTIFF Layers";
    } else if (hasGeoInfo) {
        groupName = "Georeferenced Layers";
    } else {
        groupName = "Raster Layers";
    }

    QTreeWidgetItem *group = nullptr;
    for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
        if (layersTree->topLevelItem(i)->text(0) == groupName) {
            group = layersTree->topLevelItem(i);
            break;
        }
    }

    if (!group) {
        group = new QTreeWidgetItem(layersTree, QStringList() << groupName);
        group->setIcon(0, QIcon(":/icons/folder.png"));
        group->setExpanded(true);
    }

    group->addChild(layerItem);
    loadedLayers.append(layer);
    projectModified = true;

    // Update project info
    if (projectInfoLabel) {
        projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                  .arg(currentProjectName)
                                  .arg(loadedLayers.size()));
    }

    // Update properties display
    updatePropertiesDisplay(layer);

    // Fit all images in view
    fitAllGeoreferencedImages();

    if (messageLabel) {
        messageLabel->setText(QString("Loaded %1: %2").arg(layerType).arg(layerName));
    }

    emit layerLoaded(layerName, layer.type);
}
void MainWindow::fitAllGeoreferencedImages()
{
    if (!mapView || !mapScene || georeferencedImagesInfo.isEmpty()) return;

    // Find bounds of all georeferenced images in scene coordinates
    QRectF sceneBounds;
    bool first = true;

    for (const GeoreferenceInfo &georefInfo : georeferencedImagesInfo) {
        if (georefInfo.imageItem) {
            QRectF itemBounds = georefInfo.imageItem->boundingRect();
            QPointF itemPos = georefInfo.imageItem->pos();
            QRectF bounds(itemPos, itemBounds.size());

            if (first) {
                sceneBounds = bounds;
                first = false;
            } else {
                sceneBounds = sceneBounds.united(bounds);
            }
        }
    }

    // Also include any vector layers
    for (QGraphicsItem *item : currentVectorItems) {
        if (item) {
            sceneBounds = sceneBounds.united(item->boundingRect());
        }
    }

    if (!sceneBounds.isEmpty()) {
        // Add padding
        double padding = qMax(sceneBounds.width(), sceneBounds.height()) * 0.1;
        sceneBounds.adjust(-padding, -padding, padding, padding);

        mapView->fitInView(sceneBounds, Qt::KeepAspectRatio);
        currentScale = mapView->transform().m11();
        updateMagnifier(qRound(currentScale * 100));
        updateScale(currentScale);
    }
}

void MainWindow::loadImageFile(const QString &filePath)
{
    loadRasterFile(filePath);
}

// Layer Management Methods
//LayerInfo* MainWindow::getLayerByName(const QString &name)
//{
//    for (int i = 0; i < loadedLayers.size(); ++i) {
//        if (loadedLayers[i].name == name) {
//            return &loadedLayers[i];
//        }
//    }
//    return nullptr;
//}

void MainWindow::addLayerToScene(const LayerInfo &layer)
{
    //    if (layer.graphicsItem) {
    //        mapScene->addItem(layer.graphicsItem);
    //    }
}

void MainWindow::updateLayerVisibility(const QString &layerName, bool visible)
{
    //    LayerInfo *layer = getLayerByName(layerName);
    //    if (layer && layer->graphicsItem) {
    //        layer->graphicsItem->setVisible(visible);
    //        projectModified = true;  // Mark project as modified
    //    }
}

void MainWindow::removeLayer(const QString &layerName)
{
    for (int i = 0; i < loadedLayers.size(); ++i) {
        if (loadedLayers[i].name == layerName) {
            LayerInfo &layer = loadedLayers[i];

            // Remove from scene
            if (layer.graphicsItem) {
                mapScene->removeItem(layer.graphicsItem);
                delete layer.graphicsItem;
                layer.graphicsItem = nullptr;
            }

            // Remove from tree
            if (layer.treeItem) {
                QTreeWidgetItem *parent = layer.treeItem->parent();
                if (parent) {
                    parent->removeChild(layer.treeItem);
                }
                delete layer.treeItem;
                layer.treeItem = nullptr;
            }

            // Remove from list
            loadedLayers.removeAt(i);
            projectModified = true;  // Mark project as modified

            // Update project info
            if (projectInfoLabel) {
                projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                          .arg(currentProjectName)
                                          .arg(loadedLayers.size()));
            }

            if (messageLabel) {
                messageLabel->setText("Removed layer: " + layerName);
            }

            break;
        }
    }
}

void MainWindow::fitImageToView()
{
    if (!currentImageItem || !mapView) return;

    mapView->fitInView(currentImageItem, Qt::KeepAspectRatio);
    currentScale = mapView->transform().m11();
    updateMagnifier(qRound(currentScale * 100));
    updateScale(currentScale);

    if (messageLabel) {
        messageLabel->setText("Image fitted to view");
    }
}

// File Saving Methods
bool MainWindow::saveLayerToFile(const LayerInfo &layer, const QString &savePath)
{
    if (layer.filePath.isEmpty()) {
        return false;
    }

    // Copy the original file to new location
    if (QFile::copy(layer.filePath, savePath)) {
        if (messageLabel) {
            messageLabel->setText("Saved layer to: " + savePath);
        }
        emit layerSaved(layer.name, savePath);
        return true;
    }

    return false;
}

void MainWindow::saveAllLayers()
{
    QString saveDir = getSaveLocation();

    // Create project directory
    QString projectDir = QDir(saveDir).filePath(currentProjectName);
    QDir dir(projectDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Create layers subdirectory
    QString layersDir = QDir(projectDir).filePath("layers");
    if (!QDir(layersDir).exists()) {
        QDir(layersDir).mkpath(".");
    }

    // Save each layer
    int savedCount = 0;
    for (int i = 0; i < loadedLayers.size(); ++i) {
        const LayerInfo &layer = loadedLayers[i];
        QString savePath = QDir(layersDir).filePath(
                    layer.name + "." + QFileInfo(layer.filePath).suffix());

        if (QFile::copy(layer.filePath, savePath)) {
            savedCount++;

            if (messageLabel) {
                messageLabel->setText("Saved: " + layer.name);
            }
        } else {
            QMessageBox::warning(this, "Save Error",
                                 "Could not save layer: " + layer.name);
        }
    }

    // Save project file
    QString projectFile = QDir(projectDir).filePath(currentProjectName + ".qgz");
    QFile file(projectFile);
    if (file.open(QIODevice::WriteOnly)) {
        // Save project metadata
        QTextStream stream(&file);
        stream << "# QGIS Project File\n";
        stream << "Project: " << currentProjectName << "\n";
        stream << "Created: " << QDateTime::currentDateTime().toString() << "\n";
        stream << "Layers: " << loadedLayers.size() << "\n";
        stream << "\n";

        for (int i = 0; i < loadedLayers.size(); ++i) {
            const LayerInfo &layer = loadedLayers[i];
            stream << "Layer: " << layer.name << "\n";
            stream << "  Type: " << layer.type << "\n";
            stream << "  File: " << layer.filePath << "\n";
            stream << "\n";
        }

        file.close();

        // Add to recent projects
        addRecentProject(projectFile);

        QMessageBox::information(this, "Project Saved",
                                 QString("Project saved to:\n%1\n\n%2 layers saved.")
                                 .arg(projectDir)
                                 .arg(savedCount));
    }
}

QString MainWindow::getGeoTIFFFilesFilter()
{
    return tr("GeoTIFF Files (*.tif *.tiff *.geotiff);;"
              "All Files (*)");
}

void MainWindow::exportProject(const QString &directory)
{
    // Create export directory
    QString exportDir = QDir(directory).filePath(
                currentProjectName + "_export_" +
                QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    QDir dir(exportDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // Create layers subdirectory
    QString layersExportDir = QDir(exportDir).filePath("layers");
    QDir(layersExportDir).mkpath(".");

    // Export layers
    int exportedCount = 0;
    for (int i = 0; i < loadedLayers.size(); ++i) {
        const LayerInfo &layer = loadedLayers[i];
        QString exportPath = QDir(layersExportDir).filePath(
                    layer.name + "." + QFileInfo(layer.filePath).suffix());

        if (QFile::copy(layer.filePath, exportPath)) {
            exportedCount++;
        }
    }

    // Create project metadata file
    QString metaFile = QDir(exportDir).filePath("project_export.txt");
    QFile file(metaFile);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << "QGIS Project Export\n";
        stream << "===================\n";
        stream << "Project: " << currentProjectName << "\n";
        stream << "Export Date: " << QDateTime::currentDateTime().toString() << "\n";
        stream << "Exported Layers: " << exportedCount << "/" << loadedLayers.size() << "\n";
        stream << "Export Location: " << exportDir << "\n";
        stream << "\n";
        stream << "Layer Details:\n";
        stream << "--------------\n";

        for (int i = 0; i < loadedLayers.size(); ++i) {
            const LayerInfo &layer = loadedLayers[i];
            stream << "- " << layer.name << " (" << layer.type << ")\n";
            stream << "  Source: " << layer.filePath << "\n";
            stream << "  Format: " << layer.properties["format"].toString() << "\n";
            if (layer.type == "raster") {
                stream << "  Size: " << layer.properties["width"].toString()
                       << "x" << layer.properties["height"].toString() << " pixels\n";
            }
            stream << "\n";
        }
        file.close();
    }

    // Create README file
    QString readmeFile = QDir(exportDir).filePath("README.txt");
    QFile readme(readmeFile);
    if (readme.open(QIODevice::WriteOnly)) {
        QTextStream stream(&readme);
        stream << "QGIS Project Export Package\n";
        stream << "===========================\n";
        stream << "\n";
        stream << "This package contains exported layers from QGIS.\n";
        stream << "\n";
        stream << "Directory Structure:\n";
        stream << "- layers/: Contains all exported layer files\n";
        stream << "- project_export.txt: Project metadata and layer information\n";
        stream << "- README.txt: This file\n";
        stream << "\n";
        stream << "To import this project:\n";
        stream << "1. Open QGIS\n";
        stream << "2. Go to Project -> Import Project\n";
        stream << "3. Select this directory\n";
        readme.close();
    }

    emit projectExported(exportDir);

    QMessageBox::information(this, "Export Complete",
                             QString("Project exported to:\n%1\n\n"
                                     "%2 layers exported.\n\n"
                                     "Project metadata saved in project_export.txt")
                             .arg(exportDir)
                             .arg(exportedCount));
}

void MainWindow::importProject(const QString &directory)
{
    QDir dir(directory);

    // Check for project file
    QStringList projectFiles = dir.entryList({"*.qgz", "*.qgs"}, QDir::Files);
    if (projectFiles.isEmpty()) {
        QMessageBox::warning(this, "Import Error", "No project file found in directory.");
        return;
    }

    QString projectFile = dir.filePath(projectFiles.first());
    loadProject(projectFile);

    // Look for layers directory
    QString layersDir = dir.filePath("layers");
    if (QDir(layersDir).exists()) {
        // Load all supported files from layers directory
        QStringList filters;
        filters << "*.shp" << "*.svg" << "*.ai" << "*.eps" << "*.pdf"
                << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.tif" << "*.tiff" << "*.bmp";

        QStringList layerFiles = QDir(layersDir).entryList(filters, QDir::Files);

        int loadedCount = 0;
        for (const QString &layerFile : layerFiles) {
            QString filePath = QDir(layersDir).filePath(layerFile);
            loadFile(filePath);
            loadedCount++;
        }

        if (messageLabel) {
            messageLabel->setText(QString("Imported project with %1 layers").arg(loadedCount));
        }
    }
}

// Settings Management
void MainWindow::saveSettings()
{
    appSettings->setValue("defaultSaveLocation", defaultSaveLocation);
    appSettings->setValue("lastUsedDirectory", lastUsedDirectory);
    appSettings->setValue("recentProjects", recentProjects);
    appSettings->setValue("windowGeometry", saveGeometry());
    appSettings->setValue("windowState", saveState());
    appSettings->setValue("currentProject", currentProjectName);
}

void MainWindow::loadSettings()
{
    defaultSaveLocation = appSettings->value("defaultSaveLocation",
                                             QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)
                                             + "/QGIS Projects").toString();

    lastUsedDirectory = appSettings->value("lastUsedDirectory",
                                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

    recentProjects = appSettings->value("recentProjects").toStringList();

    currentProjectName = appSettings->value("currentProject", "Untitled").toString();

    // Create default save location if it doesn't exist
    QDir saveDir(defaultSaveLocation);
    if (!saveDir.exists()) {
        saveDir.mkpath(".");
    }
}

// Slots Implementation
void MainWindow::onCreateNewProject()
{
    createNewProjectDialog();
}

void MainWindow::onOpenProject()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Project or File",
                                                    lastUsedDirectory,
                                                    getSupportedFilesFilter());

    if (!fileName.isEmpty()) {
        lastUsedDirectory = QFileInfo(fileName).path();
        loadFile(fileName);
    }
}

void MainWindow::onOpenRecentProject()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        // Find the project path from the recent projects list
        QString actionText = action->text();
        for (const QString &projectPath : recentProjects) {
            QFileInfo fileInfo(projectPath);
            if (fileInfo.fileName() == actionText) {
                loadProject(projectPath);
                break;
            }
        }
    }
}

void MainWindow::onSaveProject()
{
    saveProject();
}

void MainWindow::onSaveAsProject()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save Project As",
                                                    QDir(getSaveLocation()).filePath(currentProjectName),
                                                    "QGIS Projects (*.qgz *.qgs);;All Files (*)");

    if (!fileName.isEmpty()) {
        currentProjectPath = fileName;
        QFileInfo fileInfo(fileName);
        currentProjectName = fileInfo.baseName();

        // Update window title
        setWindowTitle("PPT GIS Desktop Project - " + currentProjectName);

        // Update browser tree
        if (browserTree && browserTree->topLevelItemCount() > 0) {
            QTreeWidgetItem *projectSection = browserTree->topLevelItem(0);
            if (projectSection && projectSection->childCount() > 0) {
                projectSection->child(0)->setText(0, "Current: " + currentProjectName);
            }
        }

        // Save the project
        saveProject();

        // Add to recent projects
        addRecentProject(currentProjectPath);
    }
}

void MainWindow::onBrowseSaveLocation()
{
    QString dir = QFileDialog::getExistingDirectory(this,
                                                    "Select Save Location",
                                                    saveLocationEdit ? saveLocationEdit->text() : getSaveLocation(),
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!dir.isEmpty()) {
        updateSaveLocation(dir);
        if (messageLabel) {
            messageLabel->setText("Save location changed to: " + dir);
        }
    }
}

void MainWindow::onChangeSaveLocation()
{
    if (saveLocationEdit) {
        QString path = saveLocationEdit->text();
        if (QDir(path).exists()) {
            updateSaveLocation(path);
            if (messageLabel) {
                messageLabel->setText("Save location updated: " + path);
            }
        } else {
            QMessageBox::warning(this, "Invalid Directory",
                                 "The specified directory does not exist.");
            saveLocationEdit->setText(getSaveLocation());
        }
    }
}

void MainWindow::onExportProject()
{
    QString saveDir = QFileDialog::getExistingDirectory(this,
                                                        "Export Project Directory",
                                                        getSaveLocation(),
                                                        QFileDialog::ShowDirsOnly);

    if (!saveDir.isEmpty()) {
        exportProject(saveDir);
    }
}

void MainWindow::onImportProject()
{
    //    QString dir = QFileDialog->getExistingDirectory(this,
    //        "Import Project Directory",
    //        getSaveLocation(),
    //        QFileDialog::ShowDirsOnly);

    //    if (!dir.isEmpty()) {
    //        importProject(dir);
    //    }
}

void MainWindow::onSaveLayer()
{
    // Save the currently selected layer
    QTreeWidgetItem *currentItem = layersTree->currentItem();
    if (currentItem && currentItem->parent()) {
        QString layerName = currentItem->text(0);
        //        LayerInfo *layer = getLayerByName(layerName);

        //        if (layer) {
        //            QString savePath = QFileDialog::getSaveFileName(this,
        //                "Save Layer As",
        //                QDir(getSaveLocation()).filePath(layerName + "_copy." +
        //                    QFileInfo(layer->filePath).suffix()),
        //                getSupportedFilesFilter());

        //            if (!savePath.isEmpty()) {
        //                saveLayerToFile(*layer, savePath);
        //            }
        //        }
    }
}

void MainWindow::onSaveLayerAs()
{
    onSaveLayer();  // Same functionality for now
}

void MainWindow::onExportToPdf()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export to PDF",
                                                    QDir(getSaveLocation()).filePath(currentProjectName + "_export.pdf"),
                                                    "PDF Files (*.pdf);;All Files (*)");

    if (!fileName.isEmpty()) {
        // In a real QGIS app, this would export the map to PDF
        QMessageBox::information(this, "Export PDF",
                                 "Map would be exported to PDF: " + fileName);

        if (messageLabel) {
            messageLabel->setText("Exported to PDF: " + fileName);
        }
    }
}

void MainWindow::onExportToImage()
{
    //    QString fileName = QFileDialog->getSaveFileName(this,
    //        "Export to Image",
    //        QDir(getSaveLocation()).filePath(currentProjectName + "_export.png"),
    //        "PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;All Files (*)");

    //    if (!fileName.isEmpty()) {
    //        // In a real QGIS app, this would export the map view to an image
    //        QMessageBox::information(this, "Export Image",
    //            "Map would be exported to image: " + fileName);

    //        if (messageLabel) {
    //            messageLabel->setText("Exported to image: " + fileName);
    //        }
    //    }
}

void MainWindow::onSaveAllLayers()
{
    saveAllLayers();
}

void MainWindow::onLoadImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Load Image",
                                                    lastUsedDirectory,
                                                    getImageFilesFilter());

    if (!fileName.isEmpty()) {
        lastUsedDirectory = QFileInfo(fileName).path();
        loadImageFile(fileName);
    }
}

void MainWindow::onClearImage()
{
    if (currentImageItem) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Clear Image",
                                                                  "Are you sure you want to clear the current image?",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            clearCurrentImage();
            if (messageLabel) {
                messageLabel->setText("Image cleared");
            }
        }
    }
}

void MainWindow::onFitImage()
{
    fitImageToView();
}

void MainWindow::onRotateLeft()
{
    if (!currentImageItem) return;

    rotationAngle -= 90;
    if (rotationAngle < 0) rotationAngle += 360;

    QTransform transform;
    transform.rotate(rotationAngle);
    currentImageItem->setTransform(transform);

    // Update status bar
    updateRotation(rotationAngle);

    updateImageInfo();
    if (messageLabel) {
        messageLabel->setText(QString("Image rotated left (now %1°)").arg(qRound(rotationAngle)));
    }
}

void MainWindow::onRotateRight()
{
    if (!currentImageItem) return;

    rotationAngle += 90;
    if (rotationAngle >= 360) rotationAngle -= 360;

    QTransform transform;
    transform.rotate(rotationAngle);
    currentImageItem->setTransform(transform);

    // Update status bar
    updateRotation(rotationAngle);

    updateImageInfo();
    if (messageLabel) {
        messageLabel->setText(QString("Image rotated right (now %1°)").arg(qRound(rotationAngle)));
    }
}

void MainWindow::onZoomImageIn()
{
    if (!mapView) return;

    mapView->scale(1.2, 1.2);
    currentScale *= 1.2;

    // Update status bar
    updateMagnifier(qRound(currentScale * 100));
    updateScale(currentScale);

    updateImageInfo();
    if (messageLabel) {
        messageLabel->setText(QString("Zoomed in to %1%").arg(qRound(currentScale * 100)));
    }
}

void MainWindow::onZoomImageOut()
{
    if (!mapView) return;

    mapView->scale(1/1.2, 1/1.2);
    currentScale /= 1.2;

    // Update status bar
    updateMagnifier(qRound(currentScale * 100));
    updateScale(currentScale);

    updateImageInfo();
    if (messageLabel) {
        messageLabel->setText(QString("Zoomed out to %1%").arg(qRound(currentScale * 100)));
    }
}

void MainWindow::onResetZoom()
{
    if (!mapView || !currentImageItem) return;

    mapView->resetTransform();
    currentScale = 1.0;

    // Re-apply rotation if any
    if (rotationAngle != 0) {
        QTransform transform;
        transform.rotate(rotationAngle);
        currentImageItem->setTransform(transform);
    }

    // Update status bar
    updateMagnifier(100);
    updateScale(1.0);

    updateImageInfo();
    if (messageLabel) {
        messageLabel->setText("Zoom reset to 100%");
    }
}

void MainWindow::onAddRasterLayer()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Add Raster Layer",
                                                    lastUsedDirectory,
                                                    getRasterFilesFilter());

    if (!fileName.isEmpty()) {
        lastUsedDirectory = QFileInfo(fileName).path();
        loadRasterFile(fileName);
    }
}

void MainWindow::onAddImageLayer()
{
    onLoadImage();
}

void MainWindow::onToggleEditing()
{
    static bool editing = false;
    editing = !editing;
    if (messageLabel) {
        messageLabel->setText("Editing mode: " + QString(editing ? "ON" : "OFF"));
    }
}

void MainWindow::onPanMap()
{
    if (mapView) {
        mapView->setDragMode(QGraphicsView::ScrollHandDrag);
    }
    if (messageLabel) {
        messageLabel->setText("Pan mode activated");
    }
}

void MainWindow::onZoomIn()
{
    if (mapView) {
        mapView->scale(1.2, 1.2);
        currentScale *= 1.2;
        updateScale(currentScale);
        updateMagnifier(qRound(currentScale * 100));
    }
}

void MainWindow::onZoomOut()
{
    if (mapView) {
        mapView->scale(1/1.2, 1/1.2);
        currentScale /= 1.2;
        updateScale(currentScale);
        updateMagnifier(qRound(currentScale * 100));
    }
}

void MainWindow::onShowProcessingToolbox()
{
    if (processingToolboxDock) {
        processingToolboxDock->raise();
        processingToolboxDock->show();
    }
    if (messageLabel) {
        messageLabel->setText("Processing Toolbox opened");
    }
}

void MainWindow::onShowPythonConsole()
{
    QMessageBox::information(this, "Python Console",
                             "Python Console would open here (Ctrl+Alt+P)");
    if (messageLabel) {
        messageLabel->setText("Python Console");
    }
}

void MainWindow::onShowLayerProperties()
{
    QTreeWidgetItem *currentItem = layersTree->currentItem();
    if (currentItem && currentItem->parent()) {
        QString layerName = currentItem->text(0);
        // Find layer info
        for (int i = 0; i < loadedLayers.size(); ++i) {
            if (loadedLayers[i].name == layerName) {
                // Create properties dialog
                QDialog *dialog = new QDialog(this);
                dialog->setWindowTitle("Layer Properties - " + layerName);
                dialog->setMinimumSize(600, 500);

                QTabWidget *tabs = new QTabWidget(dialog);

                // Information tab
                QWidget *infoTab = new QWidget();
                QFormLayout *infoLayout = new QFormLayout(infoTab);
                infoLayout->addRow("Name:", new QLabel(layerName));
                infoLayout->addRow("Type:", new QLabel(loadedLayers[i].type));
                infoLayout->addRow("File:", new QLabel(loadedLayers[i].filePath));
                // Add more properties...

                // Symbology tab
                QWidget *symbologyTab = new QWidget();
                // Add symbology controls...

                // Labels tab
                QWidget *labelsTab = new QWidget();
                // Add labeling controls...

                tabs->addTab(infoTab, "Information");
                tabs->addTab(symbologyTab, "Symbology");
                tabs->addTab(labelsTab, "Labels");

                QVBoxLayout *mainLayout = new QVBoxLayout(dialog);
                mainLayout->addWidget(tabs);

                QDialogButtonBox *buttons = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
                connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
                mainLayout->addWidget(buttons);

                dialog->exec();
                delete dialog;
                break;
            }
        }
    }
}

void MainWindow::onCreatePrintLayout()
{
    QMessageBox::information(this, "Print Layout",
                             "Creating new print layout (Ctrl+P)");
    if (messageLabel) {
        messageLabel->setText("Print Layout created");
    }
}

void MainWindow::onShowBookmarks()
{
    QMessageBox::information(this, "Spatial Bookmarks",
                             "Spatial Bookmarks manager would open here (Ctrl+B)");
    if (messageLabel) {
        messageLabel->setText("Spatial Bookmarks");
    }
}

void MainWindow::onLayerItemClicked(QTreeWidgetItem *item, int column)
{
    if (item && messageLabel) {
        QString itemText = item->text(0);
        messageLabel->setText("Selected layer: " + itemText);
    }
}

void MainWindow::onLayerItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (item && item->parent()) {
        QString layerName = item->text(0);
        // Zoom to layer functionality
        //        LayerInfo *layer = getLayerByName(layerName);
        //        if (layer && layer->graphicsItem && mapView) {
        //            mapView->fitInView(layer->graphicsItem, Qt::KeepAspectRatio);
        //            currentScale = mapView->transform().m11();
        //            updateScale(currentScale);
        //            updateMagnifier(qRound(currentScale * 100));
        //        }
    }
}

void MainWindow::onLayerContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem *item = layersTree->itemAt(pos);
    if (item && item->parent()) {
        QMenu contextMenu;
        contextMenu.addAction("Zoom to Layer", this, [this, item]() {
            onLayerItemDoubleClicked(item, 0);
        });
        contextMenu.addSeparator();
        contextMenu.addAction("Save Layer", this, &MainWindow::onSaveLayer);
        contextMenu.addAction("Save Layer As...", this, &MainWindow::onSaveLayerAs);
        contextMenu.addSeparator();
        contextMenu.addAction("Remove Layer", this, &MainWindow::onRemoveLayer);
        contextMenu.addSeparator();
        contextMenu.addAction("Properties...", this, &MainWindow::onShowLayerProperties);

        contextMenu.exec(layersTree->mapToGlobal(pos));
    }
}

void MainWindow::onRemoveLayer()
{
    QTreeWidgetItem *currentItem = layersTree->currentItem();
    if (currentItem && currentItem->parent()) {
        QString layerName = currentItem->text(0);

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Remove Layer",
                                                                  "Are you sure you want to remove layer: " + layerName + "?",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            removeLayer(layerName);
        }
    }
}

void MainWindow::onBrowserItemClicked(QTreeWidgetItem *item, int column)
{
    if (item && messageLabel) {
        QString itemText = item->text(0);
        messageLabel->setText("Selected: " + itemText);

        // Handle different browser items
        if (itemText == "Images" || itemText == "Image Layers") {
            onLoadImage();
        } else if (itemText == "Vector Layers") {
            onAddVectorLayer();
        } else if (itemText == "Raster Layers") {
            onAddRasterLayer();
        } else if (itemText == "Properties") {
            onShowLayerProperties();
        } else if (itemText.contains("Current:")) {
            onCreateNewProject();
        }
    }
}

// =========== GDAL/GeoTIFF METHODS ===========

void MainWindow::onOpenGeoTIFF()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open GeoTIFF",
                                                    lastUsedDirectory,
                                                    "GeoTIFF Files (*.tif *.tiff *.geotiff);;All Files (*)");

    if (!fileName.isEmpty()) {
        lastUsedDirectory = QFileInfo(fileName).path();

        // Close any previously loaded GDAL dataset
        if (gdalDataset) {
            GDALClose(gdalDataset);
            gdalDataset = nullptr;
        }

        // Open the GeoTIFF file
        gdalDataset = (GDALDataset*)GDALOpen(fileName.toUtf8().constData(), GA_ReadOnly);

        if (!gdalDataset) {
            QMessageBox::critical(this, "Error", "Failed to open GeoTIFF file");
            return;
        }

        // Get geotransform
        hasGeoTransform = (gdalDataset->GetGeoTransform(gdalGeoTransform) == CE_None);

        if (!hasGeoTransform) {
            QMessageBox::warning(this, "Warning",
                                 "This GeoTIFF doesn't have geographic transformation information.\n"
                                 "Coordinates will not be available.");
        }

        // Get raster size
        int xSize = gdalDataset->GetRasterXSize();
        int ySize = gdalDataset->GetRasterYSize();
        geoTIFFSize = QSize(xSize, ySize);

        // Get projection info
        const char *wkt = gdalDataset->GetProjectionRef();
        QString projection = QString(wkt);

        // Update projection in status bar
        if (!projection.isEmpty()) {
            QString displayText;

            // Check for WGS84 in projection string
            bool hasWGS84 = false;
            if (projection.contains("WGS84", Qt::CaseInsensitive) ||
                    projection.contains("WGS 84", Qt::CaseInsensitive) ||
                    projection.contains("World Geodetic System 1984", Qt::CaseInsensitive)) {
                hasWGS84 = true;
                displayText += "WGS84 , ";
            }

            // Extract EPSG code from projection string
            QString epsgCode;
            QRegularExpression epsgRegex(R"(EPSG:(\d+))");
            QRegularExpressionMatch match = epsgRegex.match(projection);

            if (match.hasMatch()) {
                epsgCode = "EPSG:" + match.captured(1);
            } else {
                // Try to find any numeric EPSG-like code
                QRegularExpression numberRegex(R"(\b(\d{4,6})\b)");
                match = numberRegex.match(projection);
                if (match.hasMatch()) {
                    epsgCode = "EPSG:" + match.captured(1);
                }
            }

            // Add EPSG if found
            if (!epsgCode.isEmpty()) {
                // If we already added WGS84 from text detection, also check EPSG for WGS84
                if (!hasWGS84 && (epsgCode == "EPSG:4326" || epsgCode == "EPSG:4979")) {
                    displayText += "WGS84 ,";
                }
                displayText += epsgCode;
                updateProjection(displayText);
            } else if (hasWGS84) {
                // If we have WGS84 but no EPSG
                updateProjection(displayText + "(No EPSG)");
            } else {
                updateProjection("GeoTIFF (No EPSG)");
            }
        } else {
            updateProjection("GeoTIFF (No Projection)");
        }

        // Load the raster data
        int bandCount = gdalDataset->GetRasterCount();
        qDebug() << "Band count:" << bandCount;

        if (bandCount >= 3) {
            // RGB image
            geoTIFFImage = QImage(xSize, ySize, QImage::Format_RGB32);
            geoTIFFImage.fill(Qt::black);

            // Read red, green, blue bands
            for (int b = 1; b <= 3 && b <= bandCount; b++) {
                GDALRasterBand *band = gdalDataset->GetRasterBand(b);
                if (!band) continue;

                QVector<unsigned char> buffer(xSize * ySize);
                CPLErr err = band->RasterIO(GF_Read, 0, 0, xSize, ySize,
                                            buffer.data(), xSize, ySize, GDT_Byte, 0, 0);

                if (err == CE_None) {
                    // Copy band data to appropriate color channel
                    for (int y = 0; y < ySize; y++) {
                        for (int x = 0; x < xSize; x++) {
                            QRgb pixel = geoTIFFImage.pixel(x, y);
                            int value = buffer[y * xSize + x];

                            if (b == 1) { // Red
                                pixel = qRgb(value, qGreen(pixel), qBlue(pixel));
                            } else if (b == 2) { // Green
                                pixel = qRgb(qRed(pixel), value, qBlue(pixel));
                            } else if (b == 3) { // Blue
                                pixel = qRgb(qRed(pixel), qGreen(pixel), value);
                            }
                            geoTIFFImage.setPixel(x, y, pixel);
                        }
                    }
                }
            }
        } else if (bandCount >= 1) {
            // Grayscale or single band
            GDALRasterBand *band = gdalDataset->GetRasterBand(1);
            if (band) {
                QVector<unsigned char> buffer(xSize * ySize);
                CPLErr err = band->RasterIO(GF_Read, 0, 0, xSize, ySize,
                                            buffer.data(), xSize, ySize, GDT_Byte, 0, 0);

                if (err == CE_None) {
                    geoTIFFImage = QImage(buffer.data(), xSize, ySize,
                                          xSize, QImage::Format_Grayscale8).copy();
                } else {
                    // Create a blank image if reading fails
                    geoTIFFImage = QImage(xSize, ySize, QImage::Format_Grayscale8);
                    geoTIFFImage.fill(Qt::gray);
                }
            }
        } else {
            QMessageBox::warning(this, "Error", "No raster bands found in file");
            GDALClose(gdalDataset);
            gdalDataset = nullptr;
            return;
        }

        if (geoTIFFImage.isNull()) {
            QMessageBox::warning(this, "Error", "Failed to create image from GeoTIFF");
            GDALClose(gdalDataset);
            gdalDataset = nullptr;
            return;
        }

        // Clear existing items
        if (mapScene) {
            mapScene->clear();
            currentImageItem = nullptr;
            geoTIFFItem = nullptr;
        }

        // Add GeoTIFF to scene
        if (mapScene) {
            QPixmap geoTIFFPixmap = QPixmap::fromImage(geoTIFFImage);
            if (geoTIFFPixmap.isNull()) {
                QMessageBox::warning(this, "Error", "Failed to create pixmap from image");
                GDALClose(gdalDataset);
                gdalDataset = nullptr;
                return;
            }

            geoTIFFItem = mapScene->addPixmap(geoTIFFPixmap);
            currentImageItem = geoTIFFItem;

            // Store the image path
            currentImagePath = fileName;
            currentPixmap = geoTIFFPixmap;

            // Fit in view
            if (mapView) {
                mapView->fitInView(geoTIFFItem, Qt::KeepAspectRatio);
                currentScale = mapView->transform().m11();
                updateMagnifier(qRound(currentScale * 100));
                updateScale(currentScale);
            }

            isGeoTIFFLoaded = true;

            // Update image info
            updateImageInfo();

            // Add to layers tree
            QFileInfo fileInfo(fileName);
            QString layerName = fileInfo.baseName();

            // Check if layer already loaded
            bool alreadyLoaded = false;
            for (int i = 0; i < loadedLayers.size(); ++i) {
                if (loadedLayers[i].name == layerName && loadedLayers[i].type == "geotiff") {
                    alreadyLoaded = true;
                    break;
                }
            }

            if (!alreadyLoaded) {
                // Create layer info
                LayerInfo layer;
                layer.name = layerName;
                layer.filePath = fileName;
                layer.type = "geotiff";
                layer.graphicsItem = geoTIFFItem;
                layer.properties["format"] = "geotiff";
                layer.properties["width"] = geoTIFFImage.width();
                layer.properties["height"] = geoTIFFImage.height();
                layer.properties["has_geotransform"] = hasGeoTransform;

                // Add to layers tree
                QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                            QStringList() << layerName << "GeoTIFF");
                layerItem->setCheckState(0, Qt::Checked);
                layer.treeItem = layerItem;

                // Find or create raster group
                QTreeWidgetItem *rasterGroup = nullptr;
                for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
                    if (layersTree->topLevelItem(i)->text(0) == "Raster Layers") {
                        rasterGroup = layersTree->topLevelItem(i);
                        break;
                    }
                }

                if (!rasterGroup) {
                    rasterGroup = new QTreeWidgetItem(layersTree, QStringList() << "Raster Layers");
                    rasterGroup->setExpanded(true);
                }

                rasterGroup->addChild(layerItem);

                loadedLayers.append(layer);
                projectModified = true;

                // Update project info
                if (projectInfoLabel) {
                    projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                              .arg(currentProjectName)
                                              .arg(loadedLayers.size()));
                }
            }

            if (messageLabel) {
                messageLabel->setText("Loaded GeoTIFF: " + fileInfo.fileName() +
                                      (hasGeoTransform ? " (with coordinates)" : " (no geotransform)"));
            }
        }
    }
}

void MainWindow::showProjectionContextMenu(const QPoint &globalPos)
{
    QMenu contextMenu;

    // Get current CRS
    QString currentCRS = projectionLabel ? projectionLabel->text().replace("Render: ", "") : "EPSG:4326";

    // Add current CRS as disabled item for reference
    QAction *currentAction = contextMenu.addAction("Current: " + currentCRS);
    currentAction->setEnabled(false);
    contextMenu.addSeparator();

    // Add actions
    QAction *changeAction = contextMenu.addAction(
                QIcon(":/icons/projection.png"),
                "Change CRS...",
                this,
                &MainWindow::showCRSSelectionDialog
                );
    changeAction->setShortcut(QKeySequence("Ctrl+Shift+R"));

    QAction *copyAction = contextMenu.addAction(
                QIcon(":/icons/copy.png"),
                "Copy CRS to Clipboard",
                [currentCRS]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(currentCRS);
    }
    );

    QAction *propertiesAction = contextMenu.addAction(
                QIcon(":/icons/properties.png"),
                "CRS Properties...",
                [this, currentCRS]() {
        QMessageBox::information(this, "CRS Properties",
                                 QString("Current Coordinate Reference System:\n\n"
                                         "● %1\n\n"
                                         "This CRS defines how geographic coordinates\n"
                                         "are projected onto the map display.")
                                 .arg(currentCRS));
    }
    );

    contextMenu.addSeparator();

    QAction *zoomToAction = contextMenu.addAction(
                QIcon(":/icons/zoom_full.png"),
                "Zoom to Full Extent",
                this,
                &MainWindow::zoomToExtents
                );

    // Show the menu
    contextMenu.exec(globalPos);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Check for projection label click
    if (obj == projectionLabel) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                onProjectionLabelClicked();
                return true;
            } else if (mouseEvent->button() == Qt::RightButton) {
                // Show context menu on right-click
                showProjectionContextMenu(mouseEvent->globalPos());
                return true;
            }
        }
    }
    // Check for projection label click
    if (obj == projectionLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onProjectionLabelClicked();
            return true;
        }
    }

    // Existing event filter code for map view...
    if (mapView && mapView->viewport() && obj == mapView->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPointF scenePos = mapView->mapToScene(mouseEvent->pos());
            updateCoordinates(scenePos);
            return true;
        }
        else if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            QTimer::singleShot(1000, this, [this]() {
                if (mapView) {
                    currentScale = mapView->transform().m11();
                    updateMagnifier(qRound(currentScale * 100));
                    updateScale(currentScale);
                }
            });
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (coordinatesToolBtn && coordinatesToolBtn->isChecked() &&
                    mouseEvent->button() == Qt::LeftButton) {
                QPointF scenePos = mapView->mapToScene(mouseEvent->pos());
                QPointF geoCoords = sceneToGeographicCoords(scenePos);
                QString message;
                if (!qIsNaN(geoCoords.x())) {
                    message = QString(
                                "<b>Geographic Coordinates:</b><br>"
                                "Longitude: %1°<br>"
                                "Latitude: %2°<br><br>"
                                "<b>Scene Coordinates:</b><br>"
                                "X: %3<br>"
                                "Y: %4")
                            .arg(geoCoords.x(), 0, 'f', 6)
                            .arg(geoCoords.y(), 0, 'f', 6)
                            .arg(scenePos.x(), 0, 'f', 2)
                            .arg(scenePos.y(), 0, 'f', 2);
                } else {
                    message = QString(
                                "<b>Scene Coordinates:</b><br>"
                                "X: %1<br>"
                                "Y: %2")
                            .arg(scenePos.x(), 0, 'f', 2)
                            .arg(scenePos.y(), 0, 'f', 2);
                }
                addCoordinateMarker(scenePos, geoCoords.x(), geoCoords.y());
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::loadRecentCRS()
{
    if (appSettings) {
        recentCRS = appSettings->value("recentCRS").toStringList();
        // Keep only last 5
        while (recentCRS.size() > 5) {
            recentCRS.removeLast();
        }
    }
}

void MainWindow::saveRecentCRS()
{
    if (appSettings) {
        appSettings->setValue("recentCRS", recentCRS);
    }
}

void MainWindow::updateRecentCRS(const QString &crs)
{
    // Remove if already exists
    recentCRS.removeAll(crs);

    // Add to beginning
    recentCRS.prepend(crs);

    // Keep only last 5
    while (recentCRS.size() > 5) {
        recentCRS.removeLast();
    }
    // Save to settings
    saveRecentCRS();
}

void MainWindow::addCoordinateMarker(const QPointF &scenePos, double lon, double lat)
{
    if (!mapScene) return;

    // Clear previous marker if exists
    if (coordinateMarker) {
        mapScene->removeItem(coordinateMarker);
        delete coordinateMarker;
    }
    if (coordinateTextItem) {
        mapScene->removeItem(coordinateTextItem);
        delete coordinateTextItem;
    }

    // Create marker
    coordinateMarker = mapScene->addEllipse(
                scenePos.x() - 8, scenePos.y() - 8, 16, 16,
                QPen(Qt::blue, 2),
                QBrush(QColor(0, 0, 255, 100))
                );

    // Create text label
    coordinateTextItem = mapScene->addText(
                QString("Lon: %1\nLat: %2")
                .arg(lon, 0, 'f', 6)
                .arg(lat, 0, 'f', 6),
                QFont("Arial", 9, QFont::Bold)
                );
    coordinateTextItem->setDefaultTextColor(Qt::blue);
    coordinateTextItem->setPos(scenePos.x() + 10, scenePos.y() - 20);

    // Bring to front
    coordinateMarker->setZValue(1000);
    coordinateTextItem->setZValue(1000);

    // Auto-remove after 5 seconds
    QTimer::singleShot(5000, this, [this]() {
        if (coordinateMarker) {
            mapScene->removeItem(coordinateMarker);
            delete coordinateMarker;
            coordinateMarker = nullptr;
        }
        if (coordinateTextItem) {
            mapScene->removeItem(coordinateTextItem);
            delete coordinateTextItem;
            coordinateTextItem = nullptr;
        }
    });
}
void MainWindow::updateExtentsDisplay()
{
    if (!extentsLabel) return;

    QString displayText = "Extents: ";

    if (isGeoTIFFLoaded && gdalDataset && hasGeoTransform && geoTIFFItem) {
        // Calculate GeoTIFF extents
        double topLeftX = gdalGeoTransform[0];
        double topLeftY = gdalGeoTransform[3];

        double bottomRightX = gdalGeoTransform[0] +
                geoTIFFSize.width() * gdalGeoTransform[1] +
                geoTIFFSize.height() * gdalGeoTransform[2];

        double bottomRightY = gdalGeoTransform[3] +
                geoTIFFSize.width() * gdalGeoTransform[4] +
                geoTIFFSize.height() * gdalGeoTransform[5];

        // Format coordinates
        QString x1 = formatCoordinate(topLeftX, displayInDegrees);
        QString y1 = formatCoordinate(topLeftY, displayInDegrees);
        QString x2 = formatCoordinate(bottomRightX, displayInDegrees);
        QString y2 = formatCoordinate(bottomRightY, displayInDegrees);

        displayText += QString("TL(%1, %2) BR(%3, %4)")
                .arg(x1).arg(y1).arg(x2).arg(y2);

    } else if (currentImageItem) {
        // For regular images
        QRectF bounds = currentImageItem->boundingRect();
        displayText += QString("TL(0, 0) BR(%1, %2)")
                .arg(bounds.width(), 0, 'f', 0)
                .arg(bounds.height(), 0, 'f', 0);

    } else if (!loadedLayers.isEmpty()) {
        // For vector layers
        QRectF combinedBounds;
        bool first = true;

        for (const LayerInfo &layer : loadedLayers) {
            if (layer.graphicsItem) {
                QRectF bounds = layer.graphicsItem->boundingRect();
                if (first) {
                    combinedBounds = bounds;
                    first = false;
                } else {
                    combinedBounds = combinedBounds.united(bounds);
                }
            }
        }

        if (!combinedBounds.isEmpty()) {
            displayText += QString("TL(%1, %2) BR(%3, %4)")
                    .arg(combinedBounds.left(), 0, 'f', 1)
                    .arg(combinedBounds.top(), 0, 'f', 1)
                    .arg(combinedBounds.right(), 0, 'f', 1)
                    .arg(combinedBounds.bottom(), 0, 'f', 1);
        } else {
            displayText += "No bounds";
        }

    } else {
        displayText += "No data loaded";
    }

    extentsLabel->setText(displayText);

    // Make it clickable for copying
    extentsLabel->setCursor(Qt::PointingHandCursor);
    extentsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
}

void MainWindow::updateMiniExtentsDisplay(QLabel* miniExtentsLabel)
{
    if (!miniExtentsLabel) return;

    QString displayText = "Ext: ";

    if (isGeoTIFFLoaded && gdalDataset && hasGeoTransform && geoTIFFItem) {
        // Calculate GeoTIFF extents
        double topLeftX = gdalGeoTransform[0];
        double topLeftY = gdalGeoTransform[3];

        double bottomRightX = gdalGeoTransform[0] +
                geoTIFFSize.width() * gdalGeoTransform[1];

        double bottomRightY = gdalGeoTransform[3] +
                geoTIFFSize.height() * gdalGeoTransform[5];

        // Format for compact display
        QString x1 = QString::number(topLeftX, 'f', displayInDegrees ? 3 : 2);
        QString y1 = QString::number(topLeftY, 'f', displayInDegrees ? 3 : 2);
        QString x2 = QString::number(bottomRightX, 'f', displayInDegrees ? 3 : 2);
        QString y2 = QString::number(bottomRightY, 'f', displayInDegrees ? 3 : 2);

        QString suffix = displayInDegrees ? "°" : "";
        displayText += QString("TL(%1%4,%2%5) BR(%3%4,%2%5)")
                .arg(x1).arg(y1).arg(x2).arg(suffix);

    } else if (currentImageItem) {
        // For regular images
        QRectF bounds = currentImageItem->boundingRect();
        displayText += QString("TL(0,0) BR(%1,%2)")
                .arg(bounds.width(), 0, 'f', 0)
                .arg(bounds.height(), 0, 'f', 0);

    } else if (!loadedLayers.isEmpty()) {
        // For vector layers
        QRectF combinedBounds;
        bool first = true;

        for (const LayerInfo &layer : loadedLayers) {
            if (layer.graphicsItem) {
                QRectF bounds = layer.graphicsItem->boundingRect();
                if (first) {
                    combinedBounds = bounds;
                    first = false;
                } else {
                    combinedBounds = combinedBounds.united(bounds);
                }
            }
        }

        if (!combinedBounds.isEmpty()) {
            displayText += QString("TL(%1,%2) BR(%3,%4)")
                    .arg(combinedBounds.left(), 0, 'f', 1)
                    .arg(combinedBounds.top(), 0, 'f', 1)
                    .arg(combinedBounds.right(), 0, 'f', 1)
                    .arg(combinedBounds.bottom(), 0, 'f', 1);
        } else {
            displayText += "No bounds";
        }

    } else {
        displayText += "No data";
    }

    miniExtentsLabel->setText(displayText);
}

QString MainWindow::formatCoordinate(double value, bool isDegrees)
{
    if (isDegrees) {
        // Format as degrees with appropriate decimal places
        // For latitude/longitude in extents display, show 2-3 decimal places
        // For regular coordinate display, show 6 decimal places
        if (extentsLabel && coordinateLabel) {
            // Check which label is currently active to determine format
            if (coordExtentToggleBtn && !coordExtentToggleBtn->isChecked()) {
                // Extents display - show 2-3 decimal places
                // Determine based on value magnitude
                if (fabs(value) >= 100) {
                    return QString("%1°").arg(value, 0, 'f', 1);
                } else if (fabs(value) >= 10) {
                    return QString("%1°").arg(value, 0, 'f', 2);
                } else {
                    return QString("%1°").arg(value, 0, 'f', 3);
                }
            } else {
                // Coordinate display - show 6 decimal places
                return QString("%1°").arg(value, 0, 'f', 6);
            }
        } else {
            // Default: 2 decimal places for extents, 6 for coordinates
            return QString("%1°").arg(value, 0, 'f', 2);
        }
    } else {
        // Decimal format - show 2 decimal places
        return QString("%1").arg(value, 0, 'f', 2);
    }
}

void MainWindow::jumpToLocation(double lon, double lat)
{
    if (!mapView || !mapScene) {
        QMessageBox::warning(this, "Error", "Map view not initialized");
        return;
    }

    // Clear any existing markers
    removeCoordinateMarker();

    double sceneX = 0;
    double sceneY = 0;
    QString errorMsg;
    bool isGeographic = false;
    bool withinBounds = true;

    if (isGeoTIFFLoaded && hasGeoTransform && gdalDataset && geoTIFFItem) {
        isGeographic = true;

        qDebug() << "Jumping to coordinates in GeoTIFF:";
        qDebug() << "  Input (lat/lon):" << lat << "," << lon;
        qDebug() << "  Geotransform:"
                 << gdalGeoTransform[0] << gdalGeoTransform[1] << gdalGeoTransform[2]
                 << gdalGeoTransform[3] << gdalGeoTransform[4] << gdalGeoTransform[5];

        // IMPORTANT: GDAL geotransform expects (x=longitude, y=latitude)
        double pixelX, pixelY;

        // Convert geographic to pixel coordinates using the geotransform
        if (fabs(gdalGeoTransform[2]) < 1e-10 && fabs(gdalGeoTransform[4]) < 1e-10) {
            // Simple case: no rotation in the geotransform
            pixelX = (lon - gdalGeoTransform[0]) / gdalGeoTransform[1];
            pixelY = (lat - gdalGeoTransform[3]) / gdalGeoTransform[5];

            qDebug() << "  Simple transform: pixelX =" << pixelX << "pixelY =" << pixelY;
        } else {
            // Complex case: need to solve linear equations
            double det = gdalGeoTransform[1] * gdalGeoTransform[5] -
                    gdalGeoTransform[2] * gdalGeoTransform[4];

            if (fabs(det) < 1e-10) {
                errorMsg = "Geotransform is singular (determinant near zero)";
                QMessageBox::warning(this, "Transformation Error", errorMsg);
                return;
            }

            pixelX = (gdalGeoTransform[5] * (lon - gdalGeoTransform[0]) -
                    gdalGeoTransform[2] * (lat - gdalGeoTransform[3])) / det;
            pixelY = (gdalGeoTransform[1] * (lat - gdalGeoTransform[3]) -
                    gdalGeoTransform[4] * (lon - gdalGeoTransform[0])) / det;

            qDebug() << "  Complex transform: pixelX =" << pixelX << "pixelY =" << pixelY;
        }

        // Check bounds
        withinBounds = (pixelX >= 0 && pixelX < geoTIFFSize.width() &&
                        pixelY >= 0 && pixelY < geoTIFFSize.height());

        if (!withinBounds) {
            errorMsg = QString("Coordinates are outside image bounds\n"
                               "Pixel coordinates: X=%1, Y=%2\n"
                               "Image size: %3 x %4")
                    .arg(pixelX, 0, 'f', 1)
                    .arg(pixelY, 0, 'f', 1)
                    .arg(geoTIFFSize.width())
                    .arg(geoTIFFSize.height());

            QMessageBox::warning(this, "Out of Bounds", errorMsg);
            // Still continue but mark as out of bounds
        }

        // Store the pixel coordinates for scene placement
        sceneX = pixelX;
        sceneY = pixelY;

        qDebug() << "  Final scene coordinates:" << sceneX << sceneY;

    } else if (currentImageItem) {
        // For regular images, use direct coordinates
        sceneX = lon;  // lon becomes X coordinate
        sceneY = lat;  // lat becomes Y coordinate

        // Check if within image bounds
        QRectF bounds = currentImageItem->boundingRect();
        withinBounds = (sceneX >= bounds.left() && sceneX <= bounds.right() &&
                        sceneY >= bounds.top() && sceneY <= bounds.bottom());

        if (!withinBounds) {
            errorMsg = "Coordinates are outside image bounds";
        }
    } else {
        QMessageBox::warning(this, "Error", "No image loaded to jump to");
        return;
    }

    // Create a prominent visual marker
    QColor markerColor = withinBounds ? Qt::red : Qt::yellow; // Red for within bounds, yellow for outside

    // Create outer circle (larger, semi-transparent)
    coordinateMarker = mapScene->addEllipse(
                sceneX - 20, sceneY - 20, 40, 40,
                QPen(markerColor, 3),
                QBrush(QColor(markerColor.red(), markerColor.green(), markerColor.blue(), 50))
                );

    // Create inner circle (solid)
    QGraphicsEllipseItem *innerCircle = mapScene->addEllipse(
                sceneX - 8, sceneY - 8, 16, 16,
                QPen(Qt::black, 2),
                QBrush(markerColor)
                );

    // Create plus sign for better visibility
    QGraphicsLineItem *verticalLine = mapScene->addLine(
                sceneX, sceneY - 15,
                sceneX, sceneY + 15,
                QPen(Qt::white, 3)
                );

    QGraphicsLineItem *horizontalLine = mapScene->addLine(
                sceneX - 15, sceneY,
                sceneX + 15, sceneY,
                QPen(Qt::white, 3)
                );

    // Create crosshair lines that extend across the view
    QGraphicsLineItem *vLine = mapScene->addLine(
                sceneX, mapScene->sceneRect().top(),
                sceneX, mapScene->sceneRect().bottom(),
                QPen(QColor(255, 0, 0, 100), 1, Qt::DashLine)
                );

    QGraphicsLineItem *hLine = mapScene->addLine(
                mapScene->sceneRect().left(), sceneY,
                mapScene->sceneRect().right(), sceneY,
                QPen(QColor(255, 0, 0, 100), 1, Qt::DashLine)
                );

    // Create a highlight rectangle around the area
    QGraphicsRectItem *highlightRect = mapScene->addRect(
                sceneX - 30, sceneY - 30, 60, 60,
                QPen(QColor(255, 255, 0, 150), 2),
                QBrush(QColor(255, 255, 0, 30))
                );

    // Store all marker items for later removal
    currentCrosshairItems.append(innerCircle);
    currentCrosshairItems.append(verticalLine);
    currentCrosshairItems.append(horizontalLine);
    currentCrosshairItems.append(vLine);
    currentCrosshairItems.append(hLine);
    currentCrosshairItems.append(highlightRect);

    // Create text label with blinking effect
    QString labelText;
    if (isGeographic) {
        labelText = QString(" ")
                .arg(lon, 0, 'f', 6)
                .arg(lat, 0, 'f', 6);

        if (isGeoTIFFLoaded) {
            labelText += QString(" ")
                    .arg(sceneX, 0, 'f', 1)
                    .arg(sceneY, 0, 'f', 1);
        }
    } else {
        labelText = QString(" ")
                .arg(sceneX, 0, 'f', 2)
                .arg(sceneY, 0, 'f', 2);
    }

    if (!withinBounds) {
        labelText += "<br><b><font color='orange'>[OUT OF BOUNDS]</font></b>";
    }

    coordinateTextItem = mapScene->addText(labelText, QFont("Arial", 10, QFont::Bold));
    coordinateTextItem->setDefaultTextColor(withinBounds ? Qt::red : Qt::yellow);
    coordinateTextItem->setPos(sceneX + 25, sceneY - 40);

    // Add background to text for better readability
    //    QGraphicsRectItem *textBg = mapScene->addRect(
    //        coordinateTextItem->boundingRect().translated(coordinateTextItem->pos()),
    //        QPen(Qt::black, 1),
    //        QBrush(QColor(255, 255, 255, 220))
    //    );
    //    textBg->setZValue(999);
    //    coordinateTextItem->setZValue(1000);

    //    // Store text background
    //    currentCrosshairItems.append(textBg);

    // Bring all marker items to front
    coordinateMarker->setZValue(1001);
    for (QGraphicsItem *item : currentCrosshairItems) {
        item->setZValue(1000);
    }

    // Center the view on the marker with animation
    QPointF targetCenter(sceneX, sceneY);

    // Animate the zoom and pan
    QTimer::singleShot(50, this, [this, targetCenter, withinBounds]() {
        if (mapView) {
            // Smoothly center on the target
            mapView->centerOn(targetCenter);

            // Zoom in for better visibility
            QTimer::singleShot(100, this, [this, targetCenter, withinBounds]() {
                if (mapView) {
                    // Calculate zoom level
                    double zoomFactor = withinBounds ? 8.0 : 4.0; // Zoom more if within bounds

                    // Get current transform
                    QTransform currentTransform = mapView->transform();

                    // Reset and apply zoom centered on marker
                    mapView->resetTransform();
                    mapView->scale(zoomFactor, zoomFactor);
                    currentScale = mapView->transform().m11();

                    // Update status bar
                    updateMagnifier(qRound(currentScale * 100));
                    updateScale(currentScale);

                    // Re-center on marker
                    mapView->centerOn(targetCenter);

                    // Add a slight animation effect
                    QTimer::singleShot(50, this, [this, targetCenter]() {
                        if (mapView) {
                            // Slight bounce effect
                            mapView->centerOn(targetCenter);

                            // Flash the marker
                            flashMarker();
                        }
                    });
                }
            });
        }
    });

    // Update status message
    if (messageLabel) {
        QString locationType = isGeographic ? "Geographic" : "Scene";
        QString msg = QString("Jumped to %1 location: %2, %3")
                .arg(locationType)
                .arg(lon, 0, 'f', 6)
                .arg(lat, 0, 'f', 6);

        if (!withinBounds) {
            msg += " [Out of bounds]";
        }

        if (!errorMsg.isEmpty()) {
            msg += " - " + errorMsg;
        }

        messageLabel->setText(msg);
    }

    // Update coordinate display
    updateCoordinates(QPointF(lon, lat));

    // Play a sound or visual effect (optional)
    // You could add a beep sound or flash the window
    QApplication::beep();
}

void MainWindow::flashMarker()
{
    if (!coordinateMarker) return;

    // Create a flashing animation
    for (int i = 0; i < 3; i++) {
        QTimer::singleShot(i * 200, this, [this, i]() {
            if (coordinateMarker) {
                // Alternate between visible and invisible
                coordinateMarker->setVisible(i % 2 == 0);

                // Also flash the text
                if (coordinateTextItem) {
                    coordinateTextItem->setVisible(i % 2 == 0);
                }

                // Flash the crosshair lines
                for (QGraphicsItem *item : currentCrosshairItems) {
                    if (item && (item->type() == QGraphicsLineItem::Type ||
                                 item->type() == QGraphicsEllipseItem::Type)) {
                        item->setVisible(i % 2 == 0);
                    }
                }
            }
        });
    }

    // Make sure everything is visible at the end
    QTimer::singleShot(600, this, [this]() {
        if (coordinateMarker) {
            coordinateMarker->setVisible(true);
        }
        if (coordinateTextItem) {
            coordinateTextItem->setVisible(true);
        }
        for (QGraphicsItem *item : currentCrosshairItems) {
            if (item) {
                item->setVisible(true);
            }
        }
    });
}


bool MainWindow::hasAnyGeoreferencedLayer()
{
    // Check if any loaded layer has geographic information
    for (const LayerInfo &layer : loadedLayers) {
        if (layer.type == "geotiff" || layer.properties.contains("has_geotransform") ||
                layer.properties.contains("projection")) {
            return true;
        }
    }
    return !georeferencedImages.isEmpty();
}

QPointF MainWindow::geographicToSceneCoords(double lon, double lat)
{
    // First, try to convert using the main GeoTIFF
    if (isGeoTIFFLoaded && hasGeoTransform && gdalDataset) {
        // Convert geographic to pixel coordinates
        double pixelX, pixelY;

        if (fabs(gdalGeoTransform[2]) < 1e-10 && fabs(gdalGeoTransform[4]) < 1e-10) {
            // Simple case
            pixelX = (lon - gdalGeoTransform[0]) / gdalGeoTransform[1];
            pixelY = (lat - gdalGeoTransform[3]) / gdalGeoTransform[5];
        } else {
            // Complex case
            double det = gdalGeoTransform[1] * gdalGeoTransform[5] -
                    gdalGeoTransform[2] * gdalGeoTransform[4];

            if (fabs(det) < 1e-10) return QPointF();

            pixelX = (gdalGeoTransform[5] * (lon - gdalGeoTransform[0]) -
                    gdalGeoTransform[2] * (lat - gdalGeoTransform[3])) / det;
            pixelY = (gdalGeoTransform[1] * (lat - gdalGeoTransform[3]) -
                    gdalGeoTransform[4] * (lon - gdalGeoTransform[0])) / det;
        }

        // Convert to scene coordinates
        if (geoTIFFItem) {
            return geoTIFFItem->mapToScene(QPointF(pixelX, pixelY));
        }
    }

    // Try to find which georeferenced image contains these coordinates
    for (const GeoreferenceInfo &georefInfo : georeferencedImagesInfo) {
        if (georefInfo.hasTransform) {
            // Calculate pixel coordinates for this image
            double pixelX, pixelY;

            if (fabs(georefInfo.geoTransform[2]) < 1e-10 &&
                    fabs(georefInfo.geoTransform[4]) < 1e-10) {
                // Simple case
                pixelX = (lon - georefInfo.geoTransform[0]) / georefInfo.geoTransform[1];
                pixelY = (lat - georefInfo.geoTransform[3]) / georefInfo.geoTransform[5];
            } else {
                // Complex case
                double det = georefInfo.geoTransform[1] * georefInfo.geoTransform[5] -
                        georefInfo.geoTransform[2] * georefInfo.geoTransform[4];

                if (fabs(det) < 1e-10) continue;

                pixelX = (georefInfo.geoTransform[5] * (lon - georefInfo.geoTransform[0]) -
                        georefInfo.geoTransform[2] * (lat - georefInfo.geoTransform[3])) / det;
                pixelY = (georefInfo.geoTransform[1] * (lat - georefInfo.geoTransform[3]) -
                        georefInfo.geoTransform[4] * (lon - georefInfo.geoTransform[0])) / det;
            }

            // Check if within image bounds
            if (pixelX >= 0 && pixelX < georefInfo.imageSize.width() &&
                    pixelY >= 0 && pixelY < georefInfo.imageSize.height()) {

                if (georefInfo.imageItem) {
                    return georefInfo.imageItem->mapToScene(QPointF(pixelX, pixelY));
                }
            }
        }
    }

    // If no specific image found, create a relative position
    // Find the bounding box of all georeferenced images
    if (!georeferencedImagesInfo.isEmpty()) {
        double minLon = std::numeric_limits<double>::max();
        double maxLon = -std::numeric_limits<double>::max();
        double minLat = std::numeric_limits<double>::max();
        double maxLat = -std::numeric_limits<double>::max();

        for (const GeoreferenceInfo &georefInfo : georeferencedImagesInfo) {
            if (georefInfo.hasTransform) {
                double topLeftX = georefInfo.geoTransform[0];
                double topLeftY = georefInfo.geoTransform[3];
                double bottomRightX = topLeftX + georefInfo.imageSize.width() * georefInfo.geoTransform[1];
                double bottomRightY = topLeftY + georefInfo.imageSize.height() * georefInfo.geoTransform[5];

                minLon = qMin(minLon, qMin(topLeftX, bottomRightX));
                maxLon = qMax(maxLon, qMax(topLeftX, bottomRightX));
                minLat = qMin(minLat, qMin(topLeftY, bottomRightY));
                maxLat = qMax(maxLat, qMax(topLeftY, bottomRightY));
            }
        }

        // Calculate relative position
        double scale = 1000.0; // Scale factor
        double x = (lon - minLon) * scale;
        double y = (maxLat - lat) * scale; // Invert Y axis

        return QPointF(x, y);
    }

    return QPointF();
}

QPointF MainWindow::sceneToGeographicCoords(const QPointF &scenePoint)
{
    // Check all georeferenced images
    for (const GeoreferenceInfo &georefInfo : georeferencedImagesInfo) {
        if (georefInfo.imageItem && georefInfo.hasTransform &&
                georefInfo.imageItem->contains(scenePoint)) {

            QPointF itemPos = georefInfo.imageItem->mapFromScene(scenePoint);

            // Clamp to image bounds
            double imgX = qBound(0.0, itemPos.x(), (double)georefInfo.imageSize.width() - 1);
            double imgY = qBound(0.0, itemPos.y(), (double)georefInfo.imageSize.height() - 1);

            // Convert to geographic coordinates using the image's geotransform
            double geoX = georefInfo.geoTransform[0] +
                    imgX * georefInfo.geoTransform[1] +
                    imgY * georefInfo.geoTransform[2];

            double geoY = georefInfo.geoTransform[3] +
                    imgX * georefInfo.geoTransform[4] +
                    imgY * georefInfo.geoTransform[5];

            return QPointF(geoX, geoY);
        }
    }

    // Fall back to main GeoTIFF if available
    if (isGeoTIFFLoaded && hasGeoTransform && gdalDataset && geoTIFFItem) {
        if (geoTIFFItem->contains(scenePoint)) {
            QPointF itemPos = geoTIFFItem->mapFromScene(scenePoint);
            int imgX = qRound(itemPos.x());
            int imgY = qRound(itemPos.y());

            // Clamp to image bounds
            imgX = qBound(0, imgX, geoTIFFSize.width() - 1);
            imgY = qBound(0, imgY, geoTIFFSize.height() - 1);

            // Convert to geographic coordinates
            double geoX = gdalGeoTransform[0] +
                    imgX * gdalGeoTransform[1] +
                    imgY * gdalGeoTransform[2];

            double geoY = gdalGeoTransform[3] +
                    imgX * gdalGeoTransform[4] +
                    imgY * gdalGeoTransform[5];

            return QPointF(geoX, geoY);
        }
    }

    return QPointF(qQNaN(), qQNaN());
}



void MainWindow::fitAllImages()
{
    if (!mapView || !mapScene || loadedLayers.isEmpty()) return;

    QRectF totalBounds;
    bool first = true;

    for (const LayerInfo &layer : loadedLayers) {
        if (layer.graphicsItem) {
            QRectF bounds = layer.graphicsItem->boundingRect();
            QPointF pos = layer.graphicsItem->pos();
            bounds.translate(pos);

            if (first) {
                totalBounds = bounds;
                first = false;
            } else {
                totalBounds = totalBounds.united(bounds);
            }
        }
    }

    if (!totalBounds.isEmpty()) {
        // Add some padding
        totalBounds.adjust(-50, -50, 50, 50);
        mapView->fitInView(totalBounds, Qt::KeepAspectRatio);
        currentScale = mapView->transform().m11();
        updateMagnifier(qRound(currentScale * 100));
        updateScale(currentScale);
    }
}

void MainWindow::updatePropertiesDisplay(const LayerInfo &layer)
{
    if (imageInfoLabel) {
        QString info = QString("<b>Layer Information</b><hr>");
        info += QString("<b>Name:</b> %1<br>").arg(layer.name);
        info += QString("<b>Type:</b> %1<br>").arg(layer.type);
        info += QString("<b>File:</b> %1<br>").arg(QFileInfo(layer.filePath).fileName());

        if (layer.properties.contains("width") && layer.properties.contains("height")) {
            info += QString("<b>Size:</b> %1 x %2 pixels<br>")
                    .arg(layer.properties["width"].toInt())
                    .arg(layer.properties["height"].toInt());
        }

        if (layer.properties.contains("format")) {
            info += QString("<b>Format:</b> %1<br>").arg(layer.properties["format"].toString());
        }

        if (layer.properties.contains("has_geotransform") && layer.properties["has_geotransform"].toBool()) {
            info += "<b>Georeferenced:</b> Yes<br>";
            if (layer.properties.contains("projection")) {
                QString proj = layer.properties["projection"].toString();
                if (proj.length() > 50) {
                    proj = proj.left(50) + "...";
                }
                info += QString("<b>Projection:</b> %1<br>").arg(proj);
            }
        } else {
            info += "<b>Georeferenced:</b> No<br>";
        }

        if (layer.properties.contains("geometry_type")) {
            info += QString("<b>Geometry Type:</b> %1<br>").arg(layer.properties["geometry_type"].toString());
        }

        if (layer.properties.contains("feature_count")) {
            info += QString("<b>Features:</b> %1<br>").arg(layer.properties["feature_count"].toInt());
        }

        info += QString("<b>Total Layers Loaded:</b> %1").arg(loadedLayers.size());

        imageInfoLabel->setText(info);
    }
}

void MainWindow::clearAllImages()
{
    // Clear GDAL datasets
    if (gdalDataset) {
        GDALClose(gdalDataset);
        gdalDataset = nullptr;
    }

    // Reset GeoTIFF flags
    hasGeoTransform = false;
    isGeoTIFFLoaded = false;
    geoTIFFItem = nullptr;
    geoTIFFImage = QImage();
    geoTIFFSize = QSize();

    // Clear georeference info
    georeferencedImagesInfo.clear();

    // Clear all graphics items from scene
    if (mapScene) {
        mapScene->clear();
        currentImageItem = nullptr;
    }

    // Clear loaded layers
    loadedLayers.clear();
    currentVectorItems.clear();
    layerVectorItems.clear();
    currentCrosshairItems.clear();

    // Clear layers tree but keep groups
    if (layersTree) {
        for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
            QTreeWidgetItem *group = layersTree->topLevelItem(i);
            while (group->childCount() > 0) {
                delete group->takeChild(0);
            }
        }
    }

    // Reset status
    currentImagePath.clear();
    currentPixmap = QPixmap();
    currentScale = 1.0;
    rotationAngle = 0.0;

    // Update displays
    updateMagnifier(100);
    updateScale(1.0);
    updateRotation(0.0);
    updateProjection("EPSG:4326");
    updateCoordinates(QPointF(0, 0));

    // Clear properties display
    if (imageInfoLabel) {
        imageInfoLabel->setText("No image loaded\n\n"
                                "Drag and drop an image file here or use the 'Load Image' button.\n\n"
                                "For GeoTIFF with coordinates, use 'Open GeoTIFF' from the GDAL menu.");
    }

    if (messageLabel) {
        messageLabel->setText("All images cleared");
    }
}
void MainWindow::highlightAreaAroundPoint(double centerX, double centerY, double radius)
{
    // Create a circular highlight
    QGraphicsEllipseItem *areaHighlight = mapScene->addEllipse(
                centerX - radius, centerY - radius,
                radius * 2, radius * 2,
                QPen(QColor(255, 255, 0, 150), 3),
                QBrush(QColor(255, 255, 0, 30))
                );

    // Create radial lines
    for (int i = 0; i < 8; i++) {
        double angle = i * M_PI / 4;
        QGraphicsLineItem *radialLine = mapScene->addLine(
                    centerX, centerY,
                    centerX + cos(angle) * radius,
                    centerY + sin(angle) * radius,
                    QPen(QColor(255, 200, 0, 100), 2)
                    );
        currentCrosshairItems.append(radialLine);
    }

    currentCrosshairItems.append(areaHighlight);
}

void MainWindow::addSelectionRectangle(double x, double y, double width, double height)
{
    // Create a selection rectangle
    QGraphicsRectItem *selectionRect = mapScene->addRect(
                x - width/2, y - height/2, width, height,
                QPen(Qt::green, 2, Qt::DashLine),
                QBrush(QColor(0, 255, 0, 50))
                );

    // Add corner markers
    QList<QPointF> corners = {
        QPointF(x - width/2, y - height/2),
        QPointF(x + width/2, y - height/2),
        QPointF(x - width/2, y + height/2),
        QPointF(x + width/2, y + height/2)
    };

    for (const QPointF &corner : corners) {
        QGraphicsEllipseItem *cornerMarker = mapScene->addEllipse(
                    corner.x() - 5, corner.y() - 5, 10, 10,
                    QPen(Qt::green, 2),
                    QBrush(Qt::green)
                    );
        currentCrosshairItems.append(cornerMarker);
    }

    currentCrosshairItems.append(selectionRect);
}

void MainWindow::removeCoordinateMarker()
{
    if (coordinateMarker) {
        mapScene->removeItem(coordinateMarker);
        delete coordinateMarker;
        coordinateMarker = nullptr;
    }

    if (coordinateTextItem) {
        mapScene->removeItem(coordinateTextItem);
        delete coordinateTextItem;
        coordinateTextItem = nullptr;
    }

    // Remove crosshair items
    for (QGraphicsItem *item : currentCrosshairItems) {
        if (item) {
            mapScene->removeItem(item);
            delete item;
        }
    }
    currentCrosshairItems.clear();

    if (messageLabel) {
        messageLabel->setText("Coordinate marker cleared");
    }
}

void MainWindow::showCoordinatePicker()
{
    // Get current view center for default values
    QPointF center = mapView->mapToScene(mapView->viewport()->rect().center());

    bool ok;

    // Show appropriate labels based on loaded data
    QString latLabel = isGeoTIFFLoaded ? "Latitude (Y coordinate)" : "Y coordinate";
    QString lonLabel = isGeoTIFFLoaded ? "Longitude (X coordinate)" : "X coordinate";

    QString latDefault = QString::number(center.y(), 'f', 6);
    QString lonDefault = QString::number(center.x(), 'f', 6);

    // For GeoTIFF, get actual coordinates at center
    if (isGeoTIFFLoaded && hasGeoTransform && geoTIFFItem) {
        if (geoTIFFItem->contains(center)) {
            QPointF itemPos = geoTIFFItem->mapFromScene(center);
            int imgX = qRound(itemPos.x());
            int imgY = qRound(itemPos.y());

            // Convert to geographic coordinates
            double geoX = gdalGeoTransform[0] +
                    imgX * gdalGeoTransform[1] +
                    imgY * gdalGeoTransform[2];

            double geoY = gdalGeoTransform[3] +
                    imgX * gdalGeoTransform[4] +
                    imgY * gdalGeoTransform[5];

            latDefault = QString::number(geoY, 'f', 6);
            lonDefault = QString::number(geoX, 'f', 6);
        }
    }

    double lat = QInputDialog::getDouble(this, "Jump to Location",
                                         latLabel + ":", latDefault.toDouble(),
                                         -90.0, 90.0, 6, &ok);

    if (!ok) return;

    double lon = QInputDialog::getDouble(this, "Jump to Location",
                                         lonLabel + ":", lonDefault.toDouble(),
                                         -180.0, 180.0, 6, &ok);

    if (!ok) return;

    // Show confirmation with coordinate info
    QString message;
    if (isGeoTIFFLoaded) {
        message = QString("Jumping to geographic coordinates:\n\n"
                          "Latitude: %1°\n"
                          "Longitude: %2°\n\n"
                          "This location will be shown on the GeoTIFF image.")
                .arg(lat, 0, 'f', 6)
                .arg(lon, 0, 'f', 6);
    } else {
        message = QString("Jumping to scene coordinates:\n\n"
                          "X: %1\n"
                          "Y: %2")
                .arg(lon, 0, 'f', 2)
                .arg(lat, 0, 'f', 2);
    }

    QMessageBox::information(this, "Jump to Location", message);

    // Call the jump function (NOTE: lon is X, lat is Y)
    jumpToLocation(lon, lat);
}

// In your menu setup (setupMenuBar() or setupToolBars())
void MainWindow::addMarkerActions()
{
    // In View menu or create a new "Markers" menu
    QMenu *viewMenu = menuBar->addMenu("&View");

    QAction *clearMarkersAction = viewMenu->addAction(QIcon(":/icons/clear_image.png"),
                                                      "Clear Markers",
                                                      this,
                                                      &MainWindow::removeCoordinateMarker);
    clearMarkersAction->setShortcut(QKeySequence("Ctrl+Shift+M"));

    // Add to toolbar too
    if (mapNavToolBar) {
        mapNavToolBar->addSeparator();
        QToolButton *clearMarkersBtn = new QToolButton();
        clearMarkersBtn->setIcon(QIcon(":/icons/clear_image.png"));
        clearMarkersBtn->setToolTip("Clear all markers");
        clearMarkersBtn->setShortcut(QKeySequence("Ctrl+Shift+M"));
        connect(clearMarkersBtn, &QToolButton::clicked, this, &MainWindow::removeCoordinateMarker);
        mapNavToolBar->addWidget(clearMarkersBtn);
    }
}

void MainWindow::zoomToExtents()
{
    if (!mapView || !mapScene) return;

    QRectF bounds;
    if (isGeoTIFFLoaded && geoTIFFItem) {
        bounds = geoTIFFItem->boundingRect();
    } else if (currentImageItem) {
        bounds = currentImageItem->boundingRect();
    } else if (!loadedLayers.isEmpty()) {
        for (const LayerInfo &layer : loadedLayers) {
            if (layer.graphicsItem) {
                bounds = bounds.united(layer.graphicsItem->boundingRect());
            }
        }
    }

    if (!bounds.isEmpty()) {
        mapView->fitInView(bounds, Qt::KeepAspectRatio);
        currentScale = mapView->transform().m11();
        updateMagnifier(qRound(currentScale * 100));
        updateScale(currentScale);

        if (messageLabel) {
            messageLabel->setText("Zoomed to layer extents");
        }
    }
}

QString MainWindow::getCurrentExtentsString()
{
    QString extents;

    if (isGeoTIFFLoaded && hasGeoTransform) {
        double topLeftX = gdalGeoTransform[0];
        double topLeftY = gdalGeoTransform[3];
        double bottomRightX = gdalGeoTransform[0] + geoTIFFSize.width() * gdalGeoTransform[1];
        double bottomRightY = gdalGeoTransform[3] + geoTIFFSize.height() * gdalGeoTransform[5];

        extents = QString("Extents: TL(%1, %2) BR(%3, %4)")
                .arg(topLeftX, 0, 'f', 6)
                .arg(topLeftY, 0, 'f', 6)
                .arg(bottomRightX, 0, 'f', 6)
                .arg(bottomRightY, 0, 'f', 6);
    } else if (currentImageItem) {
        QRectF bounds = currentImageItem->boundingRect();
        extents = QString("Extents: TL(0, 0) BR(%1, %2)")
                .arg(bounds.width(), 0, 'f', 0)
                .arg(bounds.height(), 0, 'f', 0);
    }

    return extents;
}

void MainWindow::onCoordinatesToolToggled(bool enabled)
{
    if (enabled) {
        // Activate coordinate capture mode
        if (mapView) {
            mapView->viewport()->setCursor(Qt::CrossCursor);
        }
        if (messageLabel) {
            messageLabel->setText("Coordinate capture tool: Click on map to get coordinates");
        }
    } else {
        // Deactivate coordinate capture mode
        if (mapView) {
            mapView->viewport()->setCursor(Qt::ArrowCursor);
        }
        if (messageLabel) {
            messageLabel->setText("Coordinate capture tool deactivated");
        }
    }
}

void MainWindow::updateCoordinates(const QPointF &scenePoint)
{
    if (!coordinateLabel) return;

    // Try to convert scene coordinates to geographic coordinates
    QPointF geoCoords = sceneToGeographicCoords(scenePoint);

    QString coordText;

    if (qIsNaN(geoCoords.x()) || qIsNaN(geoCoords.y())) {
        // No geographic conversion available, show scene coordinates
        coordText = QString("Coordinates: X=%1, Y=%2")
                .arg(scenePoint.x(), 0, 'f', 2)
                .arg(scenePoint.y(), 0, 'f', 2);
    } else {
        // Show geographic coordinates
        coordText = QString("Coordinates: Lon=%1°, Lat=%2°")
                .arg(geoCoords.x(), 0, 'f', 6)
                .arg(geoCoords.y(), 0, 'f', 6);
    }

    coordinateLabel->setText(coordText);
}

void MainWindow::clearCurrentImage()
{
    // Close GDAL dataset if open
    if (gdalDataset) {
        GDALClose(gdalDataset);
        gdalDataset = nullptr;
    }

    hasGeoTransform = false;
    isGeoTIFFLoaded = false;
    geoTIFFItem = nullptr;
    geoTIFFImage = QImage();
    geoTIFFSize = QSize();

    // Clear the scene
    if (mapScene) {
        mapScene->clear();
        currentImageItem = nullptr;
    }

    currentImagePath.clear();
    currentPixmap = QPixmap();
    currentScale = 1.0;
    rotationAngle = 0.0;

    // Update status bar
    updateMagnifier(100);
    updateScale(1.0);
    updateRotation(0.0);
    updateProjection("EPSG:4326");
    updateCoordinates(QPointF(0, 0));

    updateImageInfo();
}

void MainWindow::updateImageInfo()
{
    if (imageInfoLabel) {
        if (isGeoTIFFLoaded && geoTIFFItem) {
            QFileInfo fileInfo(currentImagePath);

            QString info = QString(
                        "<b>GeoTIFF:</b> %1<br>"
                        "<b>Size:</b> %2 x %3 pixels<br>"
                        "<b>Format:</b> GeoTIFF<br>"
                        "<b>Georeferenced:</b> %4<br>"
                        "<b>Zoom:</b> %5%<br>"
                        "<b>Rotation:</b> %6°<br>"
                        "<b>Move mouse to see coordinates</b>"
                        ).arg(
                        fileInfo.fileName(),
                        QString::number(geoTIFFImage.width()),
                        QString::number(geoTIFFImage.height()),
                        hasGeoTransform ? "Yes" : "No",
                        QString::number(qRound(currentScale * 100)),
                        QString::number(qRound(rotationAngle))
                        );

            imageInfoLabel->setText(info);
        } else if (currentImageItem) {
            // Original code for regular images
            QFileInfo fileInfo(currentImagePath);
            QPixmap pixmap = currentImageItem->pixmap();

            QString info = QString(
                        "<b>File:</b> %1<br>"
                        "<b>Size:</b> %2 x %3 pixels<br>"
                        "<b>Format:</b> %4<br>"
                        "<b>Zoom:</b> %5%<br>"
                        "<b>Rotation:</b> %6°"
                        ).arg(
                        fileInfo.fileName(),
                        QString::number(pixmap.width()),
                        QString::number(pixmap.height()),
                        fileInfo.suffix().toUpper(),
                        QString::number(qRound(currentScale * 100)),
                        QString::number(qRound(rotationAngle))
                        );

            imageInfoLabel->setText(info);
        } else {
            imageInfoLabel->setText("No image loaded\n\n"
                                    "Drag and drop an image file here or use the 'Load Image' button.\n\n"
                                    "For GeoTIFF with coordinates, use 'Open GeoTIFF' from the GDAL menu.");
        }
    }
}


// =========== VECTOR FILE LOADING METHODS ===========


void MainWindow::onAddVectorLayer()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Add Vector Layer",
                                                    lastUsedDirectory,
                                                    "Vector Files (*.shp *.geojson *.json *.kml *.gml *.gpkg *.csv);;"
                                                    "Shapefiles (*.shp);;"
                                                    "GeoJSON (*.geojson *.json);;"
                                                    "KML Files (*.kml);;"
                                                    "All Files (*)");

    if (!fileName.isEmpty()) {
        lastUsedDirectory = QFileInfo(fileName).path();
        onLoadVectorFile(fileName);
    }
}

void MainWindow::onLoadVectorFile(const QString &filePath)
{
    if (filePath.isEmpty()) return;

    if (messageLabel) {
        messageLabel->setText("Loading vector file: " + QFileInfo(filePath).fileName());
    }

    drawVectorLayer(filePath);

    if (messageLabel) {
        messageLabel->setText("Loaded vector file: " + QFileInfo(filePath).fileName());
    }
}

void MainWindow::drawVectorLayer(const QString &filePath)
{
    // Open vector file using GDAL
    GDALDataset *dataset = (GDALDataset*)GDALOpenEx(
                filePath.toUtf8().constData(),
                GDAL_OF_VECTOR | GDAL_OF_READONLY,
                nullptr, nullptr, nullptr);

    if (!dataset) {
        QString errorMsg = QString("ERROR: Could not open vector file\n%1\n\nGDAL Error: %2")
                .arg(filePath)
                .arg(CPLGetLastErrorMsg());

        QMessageBox::critical(this, "Vector Load Error", errorMsg);

        if (messageLabel) {
            messageLabel->setText("Error loading vector file");
        }
        return;
    }

    // Get layer count
    int layerCount = dataset->GetLayerCount();

    if (layerCount == 0) {
        QMessageBox::information(this, "No Layers", "No layers found in vector file");
        GDALClose(dataset);
        return;
    }

    // Colors for different geometry types
    QColor pointColor(255, 0, 0, 200);      // Red
    QColor lineColor(0, 0, 255, 200);       // Blue
    QColor polygonColor(0, 255, 0, 150);    // Green with transparency
    QColor multiPointColor(255, 165, 0, 200); // Orange
    QColor multiLineColor(75, 0, 130, 200);   // Indigo
    QColor multiPolygonColor(238, 130, 238, 150); // Violet

    // Determine scale factor based on coordinate system
    double scaleFactor = 100.0;
    if (isGeoTIFFLoaded && hasGeoTransform) {
        // If we have a georeferenced image, use appropriate scaling
        scaleFactor = 1000.0;
    }

    // Process each layer
    for (int i = 0; i < layerCount; i++) {
        OGRLayer *layer = dataset->GetLayer(i);
        if (!layer) continue;

        // Reset reading
        layer->ResetReading();

        // Get layer name
        const char *layerName = layer->GetName();
        QString qLayerName = layerName ? QString(layerName) : QString("Layer %1").arg(i + 1);

        // Get geometry type
        OGRwkbGeometryType geomType = wkbFlatten(layer->GetGeomType());
        QColor color;
        QString geomTypeStr;

        switch (geomType) {
        case wkbPoint:
            color = pointColor;
            geomTypeStr = "Point";
            break;
        case wkbLineString:
            color = lineColor;
            geomTypeStr = "Line";
            break;
        case wkbPolygon:
            color = polygonColor;
            geomTypeStr = "Polygon";
            break;
        case wkbMultiPoint:
            color = multiPointColor;
            geomTypeStr = "MultiPoint";
            break;
        case wkbMultiLineString:
            color = multiLineColor;
            geomTypeStr = "MultiLine";
            break;
        case wkbMultiPolygon:
            color = multiPolygonColor;
            geomTypeStr = "MultiPolygon";
            break;
        default:
            color = QColor(128, 128, 128, 200);  // Gray for unknown types
            geomTypeStr = "Unknown";
        }

        // Create layer info
        LayerInfo layerInfo;
        layerInfo.name = qLayerName;
        layerInfo.filePath = filePath;
        layerInfo.type = "vector";
        layerInfo.properties["geometry_type"] = geomTypeStr;
        layerInfo.properties["layer_index"] = i;

        // Add layer to tree
        QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                    QStringList() << qLayerName << "Vector (" + geomTypeStr + ")");
        layerItem->setCheckState(0, Qt::Checked);
        layerItem->setIcon(0, QIcon(":/icons/vector_layer.png"));
        layerInfo.treeItem = layerItem;

        // Find or create vector group
        QTreeWidgetItem *vectorGroup = nullptr;
        for (int j = 0; j < layersTree->topLevelItemCount(); ++j) {
            if (layersTree->topLevelItem(j)->text(0) == "Vector Layers") {
                vectorGroup = layersTree->topLevelItem(j);
                break;
            }
        }

        if (!vectorGroup) {
            vectorGroup = new QTreeWidgetItem(layersTree, QStringList() << "Vector Layers");
            vectorGroup->setIcon(0, QIcon(":/icons/folder.png"));
            vectorGroup->setExpanded(true);
        }

        vectorGroup->addChild(layerItem);

        // Store items for this layer
        QList<QGraphicsItem*> layerItems;

        // Draw features
        OGRFeature *feature;
        int featureCount = 0;
        int maxFeatures = 1000;  // Limit for performance

        while ((feature = layer->GetNextFeature()) != nullptr && featureCount < maxFeatures) {
            OGRGeometry *geometry = feature->GetGeometryRef();
            if (geometry) {
                drawGeometry(geometry, color, scaleFactor);
                // Note: drawGeometry adds items to currentVectorItems
            }
            OGRFeature::DestroyFeature(feature);
            featureCount++;
        }

        // Store layer items
        layerVectorItems[qLayerName] = currentVectorItems;
        currentVectorItems.clear();

        // Get feature count for the entire layer
        layer->ResetReading();
        int totalFeatures = layer->GetFeatureCount();
        layerInfo.properties["feature_count"] = totalFeatures;
        layerInfo.properties["features_drawn"] = qMin(featureCount, maxFeatures);

        // Add layer to loaded layers
        loadedLayers.append(layerInfo);
        projectModified = true;

        // Update project info
        if (projectInfoLabel) {
            projectInfoLabel->setText(QString("Project: %1\nLayers: %2")
                                      .arg(currentProjectName)
                                      .arg(loadedLayers.size()));
        }

        // Update properties display
        updatePropertiesDisplay(layerInfo);

        // Update status
        if (messageLabel) {
            messageLabel->setText(QString("Loaded %1 features from %2").arg(featureCount).arg(qLayerName));
        }
    }

    GDALClose(dataset);

    // Zoom to fit all items
    fitAllImages();
}

void MainWindow::drawGeometry(OGRGeometry *geom, const QColor &color, double scaleFactor)
{
    if (!geom || !mapScene) return;

    OGRwkbGeometryType type = wkbFlatten(geom->getGeometryType());

    switch (type) {
    case wkbPoint:
        drawPoint((OGRPoint*)geom, color, scaleFactor);
        break;
    case wkbLineString:
        drawLine((OGRLineString*)geom, color, scaleFactor);
        break;
    case wkbPolygon:
        drawPolygon((OGRPolygon*)geom, color, scaleFactor);
        break;
    case wkbMultiPoint:
    case wkbMultiLineString:
    case wkbMultiPolygon:
        // Handle multi-geometries recursively
        if (OGRGeometryCollection *collection = dynamic_cast<OGRGeometryCollection*>(geom)) {
            for (int i = 0; i < collection->getNumGeometries(); i++) {
                drawGeometry(collection->getGeometryRef(i), color, scaleFactor);
            }
        }
        break;
    default:
        qDebug() << "Unhandled geometry type:" << type;
        break;
    }
}

void MainWindow::drawPoint(OGRPoint *point, const QColor &color, double scaleFactor)
{
    if (!point || !mapScene) return;

    double x = point->getX();
    double y = point->getY();

    // Scale coordinates
    x = x * scaleFactor;
    y = y * -scaleFactor; // Invert Y for proper display

    // Create a circle for the point
    double pointSize = 6.0;
    QGraphicsEllipseItem *item = mapScene->addEllipse(x - pointSize/2, y - pointSize/2,
                                                      pointSize, pointSize);
    item->setPen(QPen(color, 1));
    item->setBrush(QBrush(color));

    // Store in current vector items
    currentVectorItems.append(item);

    // Optional: Add tooltip with coordinates
    item->setToolTip(QString("Point: %1, %2").arg(x/scaleFactor).arg(-y/scaleFactor));
}

void MainWindow::drawLine(OGRLineString *line, const QColor &color, double scaleFactor)
{
    if (!line || !mapScene) return;

    int pointCount = line->getNumPoints();
    if (pointCount < 2) return;

    QPainterPath path;

    OGRPoint point;
    line->getPoint(0, &point);
    double x = point.getX() * scaleFactor;
    double y = point.getY() * -scaleFactor;

    path.moveTo(x, y);

    for (int i = 1; i < pointCount; i++) {
        line->getPoint(i, &point);
        x = point.getX() * scaleFactor;
        y = point.getY() * -scaleFactor;
        path.lineTo(x, y);
    }

    QGraphicsPathItem *item = mapScene->addPath(path);
    item->setPen(QPen(color, 2));

    // Store in current vector items
    currentVectorItems.append(item);

    // Optional: Add tooltip
    item->setToolTip(QString("Line with %1 points").arg(pointCount));
}

void MainWindow::drawPolygon(OGRPolygon *polygon, const QColor &color, double scaleFactor)
{
    if (!polygon || !mapScene) return;

    // Outer ring
    OGRLinearRing *ring = polygon->getExteriorRing();
    if (!ring || ring->getNumPoints() < 3) return;

    QPainterPath path;

    OGRPoint point;
    ring->getPoint(0, &point);
    double x = point.getX() * scaleFactor;
    double y = point.getY() * -scaleFactor;

    path.moveTo(x, y);

    for (int i = 1; i < ring->getNumPoints(); i++) {
        ring->getPoint(i, &point);
        x = point.getX() * scaleFactor;
        y = point.getY() * -scaleFactor;
        path.lineTo(x, y);
    }
    path.closeSubpath();

    // Draw interior rings (holes) if any
    int interiorRingCount = polygon->getNumInteriorRings();
    for (int r = 0; r < interiorRingCount; r++) {
        OGRLinearRing *interiorRing = polygon->getInteriorRing(r);
        if (interiorRing && interiorRing->getNumPoints() >= 3) {
            OGRPoint interiorPoint;
            interiorRing->getPoint(0, &interiorPoint);
            double ix = interiorPoint.getX() * scaleFactor;
            double iy = interiorPoint.getY() * -scaleFactor;

            path.moveTo(ix, iy);

            for (int i = 1; i < interiorRing->getNumPoints(); i++) {
                interiorRing->getPoint(i, &interiorPoint);
                ix = interiorPoint.getX() * scaleFactor;
                iy = interiorPoint.getY() * -scaleFactor;
                path.lineTo(ix, iy);
            }
            path.closeSubpath();
        }
    }

    QGraphicsPathItem *item = mapScene->addPath(path);
    QPen pen(color, 1);
    item->setPen(pen);

    // Fill polygon with semi-transparent color
    QColor fillColor = color;
    fillColor.setAlpha(100); // 40% opacity
    item->setBrush(QBrush(fillColor));

    // Store in current vector items
    currentVectorItems.append(item);

    // Optional: Add tooltip
    item->setToolTip(QString("Polygon with %1 rings").arg(interiorRingCount + 1));
}

void MainWindow::clearVectorItems(const QString &layerName)
{
    if (layerName.isEmpty()) {
        // Clear all vector items
        for (QGraphicsItem *item : currentVectorItems) {
            if (item && mapScene) {
                mapScene->removeItem(item);
                delete item;
            }
        }
        currentVectorItems.clear();

        // Clear all layer vector items
        for (auto &layerItems : layerVectorItems) {
            for (QGraphicsItem *item : layerItems) {
                if (item && mapScene) {
                    mapScene->removeItem(item);
                    delete item;
                }
            }
        }
        layerVectorItems.clear();
    } else {
        // Clear vector items for specific layer
        if (layerVectorItems.contains(layerName)) {
            for (QGraphicsItem *item : layerVectorItems[layerName]) {
                if (item && mapScene) {
                    mapScene->removeItem(item);
                    delete item;
                }
            }
            layerVectorItems[layerName].clear();
            layerVectorItems.remove(layerName);
        }
    }
}

void MainWindow::setupProjectionSystem()
{
    // You could integrate with PROJ library for coordinate transformations
    // or use GDAL's OGRSpatialReference for CRS management

    // Example: Add CRS selector to status bar
    QToolButton *crsBtn = new QToolButton();
    crsBtn->setText("EPSG:4326");
    crsBtn->setToolTip("Current Coordinate Reference System\nClick to change");
    crsBtn->setStyleSheet(
                "QToolButton { "
                "padding: 3px 8px; "
                "border: 1px solid #aaa; "
                "border-radius: 3px; "
                "background-color: #f0f8ff; "
                "margin: 1px 3px; "
                "}"
                "QToolButton:hover { background-color: #e0f0ff; }"
                );

    connect(crsBtn, &QToolButton::clicked, this, [this]() {
        // Show CRS selection dialog
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("Select Coordinate Reference System");
        dialog->resize(500, 400);

        QVBoxLayout *layout = new QVBoxLayout(dialog);

        QLineEdit *searchEdit = new QLineEdit();
        searchEdit->setPlaceholderText("Search CRS (e.g., WGS84, UTM)");
        layout->addWidget(searchEdit);

        QListWidget *crsList = new QListWidget();
        QStringList commonCRS = {
            "EPSG:4326 - WGS 84",
            "EPSG:3857 - Web Mercator",
            "EPSG:32633 - UTM Zone 33N",
            "EPSG:32634 - UTM Zone 34N",
            "EPSG:27700 - British National Grid",
            "EPSG:2154 - RGF93 / Lambert-93",
            "EPSG:3035 - ETRS89 / LAEA Europe"
        };
        crsList->addItems(commonCRS);
        layout->addWidget(crsList);

        QDialogButtonBox *buttons = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
        layout->addWidget(buttons);

        if (dialog->exec() == QDialog::Accepted) {
            QListWidgetItem *selected = crsList->currentItem();
            if (selected) {
                QString crs = selected->text().split(" - ").first();
                updateProjection(crs);
            }
        }
        delete dialog;
    });

    // Add to status bar
    statusBar()->addPermanentWidget(crsBtn);
}

void MainWindow::onExportMap()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Export Map",
                                                    QDir(getSaveLocation()).filePath(currentProjectName + "_export.pdf"),
                                                    "PDF Files (*.pdf);;PNG Files (*.png);;JPEG Files (*.jpg);;All Files (*)");

    if (!fileName.isEmpty()) {
        // Create a high-resolution render of the map view
        if (mapView && mapScene) {
            QImage image(mapView->viewport()->size(), QImage::Format_ARGB32);
            image.fill(Qt::white);

            QPainter painter(&image);
            mapView->render(&painter);
            painter.end();

            if (image.save(fileName)) {
                if (messageLabel) {
                    messageLabel->setText("Map exported to: " + fileName);
                }
                QMessageBox::information(this, "Export Successful",
                                         QString("Map exported to:\n%1").arg(fileName));
            }
        }
    }
}
void MainWindow::onOpenAttributeTable()
{
    QTreeWidgetItem *currentItem = layersTree->currentItem();
    if (currentItem && currentItem->parent()) {
        QString layerName = currentItem->text(0);

        // Create attribute table dialog
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("Attribute Table - " + layerName);
        dialog->resize(800, 600);

        QVBoxLayout *layout = new QVBoxLayout(dialog);

        // Add filter/search bar
        QLineEdit *searchEdit = new QLineEdit();
        searchEdit->setPlaceholderText("Filter attributes...");
        layout->addWidget(searchEdit);

        // Create table widget
        QTableWidget *table = new QTableWidget();
        table->setAlternatingRowColors(true);
        table->setSortingEnabled(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);

        // For vector layers, you could load actual attributes from GDAL
        // For now, create sample data
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({"ID", "Name", "Type", "Area", "Length"});
        table->setRowCount(50);

        for (int row = 0; row < 50; ++row) {
            for (int col = 0; col < 5; ++col) {
                QTableWidgetItem *item = new QTableWidgetItem(
                            QString("Data %1-%2").arg(row+1).arg(col+1));
                table->setItem(row, col, item);
            }
        }

        layout->addWidget(table);

        // Add statistics
        QLabel *statsLabel = new QLabel(
                    QString("Showing %1 features").arg(table->rowCount()));
        layout->addWidget(statsLabel);

        dialog->exec();
        delete dialog;
    }
}
QIcon MainWindow::createCRSIcon()
{
    // Create a 32x32 pixmap
    QPixmap pixmap(32, 32);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw globe/coordinate system icon
    painter.setPen(QPen(QColor(0, 120, 215), 2));
    painter.setBrush(QBrush(QColor(230, 240, 255)));

    // Draw circle (globe)
    painter.drawEllipse(4, 4, 24, 24);

    // Draw longitude lines
    painter.setPen(QPen(QColor(0, 90, 180), 1));
    painter.drawLine(16, 6, 16, 26);  // Vertical line

    // Draw latitude lines
    painter.drawLine(6, 16, 26, 16);  // Horizontal line

    // Draw diagonal lines for coordinate grid
    painter.setPen(QPen(QColor(0, 90, 180), 1, Qt::DashLine));
    painter.drawLine(8, 8, 24, 24);   // Diagonal 1
    painter.drawLine(24, 8, 8, 24);   // Diagonal 2

    painter.end();

    return QIcon(pixmap);
}

void MainWindow::setupCRSSelection()
{
    QToolButton *crsButton = new QToolButton();
    crsButton->setIcon(createCRSIcon());

    // Make projection label clickable
    if (projectionLabel) {
        projectionLabel->setCursor(Qt::PointingHandCursor);
        projectionLabel->setStyleSheet(
                    "QLabel { "
                    "padding: 3px 8px; "
                    "border: 1px solid #aaa; "
                    "border-right: 3px solid #d0d0d0; "
                    "background-color: #f0f8ff; "
                    "margin: 1px; "
                    "margin-right: 3px; "
                    "min-height: 22px; "
                    "}"
                    "QLabel:hover { "
                    "background-color: #e0f0ff; "
                    "border-color: #888; "
                    "}"
                    );

        // Connect click event using event filter
        projectionLabel->installEventFilter(this);
    }

    // Also add a CRS button to toolbar
    if (mapNavToolBar) {
        mapNavToolBar->addSeparator();
        QToolButton *crsButton = new QToolButton();

        // Try to load icon from resources
        QIcon crsIcon;
        if (QFile::exists(":/icons/projection.png")) {
            crsIcon = QIcon(":/icons/projection.png");
        } else if (QFile::exists(":/icons/crs.pnge")) {
            crsIcon = QIcon(":/icons/crs.png");
        } else {
            // Create a simple icon programmatically as fallback
            crsIcon = createCRSIcon();
        }

        crsButton->setIcon(crsIcon);
        crsButton->setText("CRS");
        crsButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        crsButton->setToolTip("Coordinate Reference System\nClick to select CRS");
        crsButton->setStyleSheet(
                    "QToolButton { "
                    "padding: 3px 8px; "
                    "border: 1px solid #aaa; "
                    "border-radius: 3px; "
                    "background-color: #f0f8ff; "
                    "}"
                    "QToolButton:hover { "
                    "background-color: #e0f0ff; "
                    "}"
                    );
        connect(crsButton, &QToolButton::clicked, this, &MainWindow::showCRSSelectionDialog);
        mapNavToolBar->addWidget(crsButton);
    }

    // Add keyboard shortcut for CRS dialog (Ctrl+Shift+R)
    QShortcut *crsShortcut = new QShortcut(QKeySequence("Ctrl+Shift+R"), this);
    connect(crsShortcut, &QShortcut::activated, this, &MainWindow::showCRSSelectionDialog);
}
void MainWindow::onProjectionLabelClicked()
{
    showCRSSelectionDialog();
}
void MainWindow::showCRSSelectionDialog()
{
    // Create dialog
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Coordinate Reference System Selector");
    dialog->setMinimumSize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Search bar
    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Filter (e.g., 'WGS84', '4326', 'UTM')...");
    searchEdit->addAction(QIcon(":/icons/search.png"), QLineEdit::LeadingPosition);
    QPushButton *clearSearchBtn = new QPushButton("Clear");
    clearSearchBtn->setMaximumWidth(80);
    searchLayout->addWidget(searchEdit);
    searchLayout->addWidget(clearSearchBtn);
    mainLayout->addLayout(searchLayout);

    // Create tab widget like QGIS
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->setIconSize(QSize(16, 16));

    // Tab 1: Predefined Coordinate Reference Systems
    QWidget *predefinedTab = new QWidget();
    QVBoxLayout *predefinedLayout = new QVBoxLayout(predefinedTab);
    predefinedLayout->setContentsMargins(5, 5, 5, 5);

    // Filter options
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(new QLabel("Filter:"));
    QComboBox *filterCombo = new QComboBox();
    filterCombo->addItems({"All", "Recently Used", "Geographic", "Projected", "Custom", "Engineering", "Compound"});
    filterCombo->setMaximumWidth(150);
    filterLayout->addWidget(filterCombo);
    filterLayout->addStretch();
    predefinedLayout->addLayout(filterLayout);

    // Recently used list (like QGIS)
    QGroupBox *recentGroup = new QGroupBox("Recently Used Coordinate Reference Systems");
    recentGroup->setMaximumHeight(120);
    QVBoxLayout *recentLayout = new QVBoxLayout(recentGroup);
    QListWidget *recentList = new QListWidget();
    recentList->addItems({
                             "EPSG:4326 - WGS 84",
                             "EPSG:3857 - WGS 84 / Pseudo-Mercator",
                             "EPSG:32633 - WGS 84 / UTM zone 33N",
                             "EPSG:32634 - WGS 84 / UTM zone 34N"
                         });
    recentLayout->addWidget(recentList);
    predefinedLayout->addWidget(recentGroup);

    // Main CRS list
    QGroupBox *crsGroup = new QGroupBox("Coordinate Reference System");
    QVBoxLayout *crsLayout = new QVBoxLayout(crsGroup);

    QTreeWidget *crsTree = new QTreeWidget();
    crsTree->setHeaderLabels(QStringList() << "Name" << "Authority ID" << "Type");
    crsTree->setColumnCount(3);
    crsTree->setSortingEnabled(true);
    crsTree->setAlternatingRowColors(true);

    // Populate the CRS tree
    populateCRSTree(crsTree);

    crsTree->expandAll();
    crsTree->resizeColumnToContents(0);

    crsLayout->addWidget(crsTree);
    predefinedLayout->addWidget(crsGroup);
    predefinedLayout->setStretchFactor(crsGroup, 1);

    // Tab 2: Project CRS
    QWidget *projectTab = new QWidget();
    QVBoxLayout *projectLayout = new QVBoxLayout(projectTab);
    projectLayout->setContentsMargins(5, 5, 5, 5);

    QGroupBox *currentCrsGroup = new QGroupBox("Current Project CRS");
    QVBoxLayout *currentCrsLayout = new QVBoxLayout(currentCrsGroup);
    QString currentCRS = projectionLabel ? projectionLabel->text().replace("Render: ", "") : "EPSG:4326 - WGS 84";
    QLabel *currentCrsLabel = new QLabel(currentCRS);
    currentCrsLabel->setStyleSheet("font-weight: bold; padding: 10px; background-color: #f0f8ff; border-radius: 3px;");
    currentCrsLabel->setWordWrap(true);
    currentCrsLayout->addWidget(currentCrsLabel);
    projectLayout->addWidget(currentCrsGroup);

    // Set project CRS from layer
    QGroupBox *layerCrsGroup = new QGroupBox("Set Project CRS from Layer");
    QVBoxLayout *layerCrsLayout = new QVBoxLayout(layerCrsGroup);
    QComboBox *layerCombo = new QComboBox();
    layerCombo->addItem("Select a layer...");

    // Add loaded layers to combo
    for (const LayerInfo &layer : loadedLayers) {
        if (layer.properties.contains("has_geotransform") && layer.properties["has_geotransform"].toBool()) {
            layerCombo->addItem(layer.name);
        }
    }

    if (layerCombo->count() == 1) {
        layerCombo->addItem("No georeferenced layers available");
        layerCombo->setEnabled(false);
    }

    QPushButton *setFromLayerBtn = new QPushButton("Set from Selected Layer");
    layerCrsLayout->addWidget(new QLabel("Select layer:"));
    layerCrsLayout->addWidget(layerCombo);
    layerCrsLayout->addWidget(setFromLayerBtn);
    projectLayout->addWidget(layerCrsGroup);

    projectLayout->addStretch();

    // Tab 3: Custom CRS
    QWidget *customTab = new QWidget();
    QVBoxLayout *customLayout = new QVBoxLayout(customTab);
    customLayout->setContentsMargins(5, 5, 5, 5);

    QGroupBox *customCrsGroup = new QGroupBox("Custom Coordinate Reference System");
    QFormLayout *customForm = new QFormLayout(customCrsGroup);

    QLineEdit *nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("Custom CRS Name");
    customForm->addRow("Name:", nameEdit);

    QLineEdit *proj4Edit = new QLineEdit();
    proj4Edit->setPlaceholderText("+proj=longlat +datum=WGS84 +no_defs");
    customForm->addRow("PROJ.4 String:", proj4Edit);

    QTextEdit *wktEdit = new QTextEdit();
    wktEdit->setMaximumHeight(150);
    wktEdit->setPlaceholderText("GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",...");
    customForm->addRow("WKT String:", wktEdit);

    QHBoxLayout *fileLayout = new QHBoxLayout();
    QPushButton *loadFromFileBtn = new QPushButton("Load from File...");
    QPushButton *saveToFileBtn = new QPushButton("Save to File...");
    fileLayout->addWidget(loadFromFileBtn);
    fileLayout->addWidget(saveToFileBtn);
    customForm->addRow("WKT File:", fileLayout);

    customLayout->addWidget(customCrsGroup);
    customLayout->addStretch();

    // Add tabs
    tabWidget->addTab(predefinedTab, QIcon(":/icons/projection.png"), "Predefined CRS");
    tabWidget->addTab(projectTab, QIcon(":/icons/project.png"), "Project CRS");
    tabWidget->addTab(customTab, QIcon(":/icons/settings.png"), "Custom CRS");

    mainLayout->addWidget(tabWidget);

    // Buttons at bottom
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Help | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Apply");

    connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Help), &QPushButton::clicked, []() {
        QMessageBox::information(nullptr, "CRS Selector Help",
                                 "Select a Coordinate Reference System for your project.\n\n"
                                 "• Predefined CRS: Choose from standard EPSG codes\n"
                                 "• Project CRS: View or set the current project CRS\n"
                                 "• Custom CRS: Define custom coordinate systems\n\n"
                                 "Press Ctrl+F to quickly search for CRS.");
    });

    mainLayout->addWidget(buttonBox);

    // Connect signals for search functionality
    connect(searchEdit, &QLineEdit::textChanged, [crsTree, recentList](const QString &text) {
        if (crsTree) {
            QTreeWidgetItemIterator it(crsTree);
            while (*it) {
                bool matches = text.isEmpty() ||
                        (*it)->text(0).contains(text, Qt::CaseInsensitive) ||
                        (*it)->text(1).contains(text, Qt::CaseInsensitive) ||
                        (*it)->text(2).contains(text, Qt::CaseInsensitive);
                (*it)->setHidden(!matches);

                // Show parent if child matches
                if (matches && (*it)->parent()) {
                    (*it)->parent()->setHidden(false);
                }
                ++it;
            }
        }

        if (recentList) {
            for (int i = 0; i < recentList->count(); ++i) {
                QListWidgetItem *item = recentList->item(i);
                bool matches = text.isEmpty() ||
                        item->text().contains(text, Qt::CaseInsensitive);
                item->setHidden(!matches);
            }
        }
    });

    connect(clearSearchBtn, &QPushButton::clicked, [searchEdit]() {
        searchEdit->clear();
        searchEdit->setFocus();
    });

    connect(filterCombo, &QComboBox::currentTextChanged, [crsTree](const QString &filter) {
        if (!crsTree) return;

        QTreeWidgetItemIterator it(crsTree);
        while (*it) {
            if (filter == "All") {
                (*it)->setHidden(false);
            } else if (filter == "Recently Used") {
                (*it)->setHidden(true);
            } else {
                bool matches = (*it)->text(2).contains(filter, Qt::CaseInsensitive);
                (*it)->setHidden(!matches);

                // Show parent if child matches
                if (matches && (*it)->parent()) {
                    (*it)->parent()->setHidden(false);
                }
            }
            ++it;
        }
    });

    connect(crsTree, &QTreeWidget::itemDoubleClicked, dialog, &QDialog::accept);
    connect(recentList, &QListWidget::itemDoubleClicked, dialog, &QDialog::accept);

    connect(setFromLayerBtn, &QPushButton::clicked, [this, dialog, layerCombo]() {
        if (layerCombo->currentIndex() > 0) {
            QString layerName = layerCombo->currentText();
            // In real implementation, you would get the CRS from the layer
            // For now, use a default
            onCRSChanged("EPSG:4326");
            dialog->accept();
        }
    });

    connect(loadFromFileBtn, &QPushButton::clicked, [wktEdit]() {
        QString fileName = QFileDialog::getOpenFileName(nullptr,
                                                        "Load WKT File", "", "WKT Files (*.wkt *.prj);;All Files (*)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                wktEdit->setPlainText(file.readAll());
                file.close();
            }
        }
    });

    connect(saveToFileBtn, &QPushButton::clicked, [wktEdit]() {
        QString fileName = QFileDialog::getSaveFileName(nullptr,
                                                        "Save WKT File", "", "WKT Files (*.wkt);;All Files (*)");
        if (!fileName.isEmpty()) {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                file.write(wktEdit->toPlainText().toUtf8());
                file.close();
            }
        }
    });

    // Search shortcut
    QShortcut *searchShortcut = new QShortcut(QKeySequence::Find, dialog);
    connect(searchShortcut, &QShortcut::activated, [searchEdit]() {
        if (searchEdit) {
            searchEdit->setFocus();
            searchEdit->selectAll();
        }
    });

    // Show dialog and process result
    if (dialog->exec() == QDialog::Accepted) {
        // Get selected CRS
        QString selectedCRS;

        if (tabWidget->currentIndex() == 0) {
            // From predefined CRS tab
            if (crsTree->currentItem()) {
                QTreeWidgetItem *selected = crsTree->currentItem();
                if (selected && !selected->text(1).isEmpty()) {
                    selectedCRS = selected->text(1); // Authority ID
                } else if (recentList->currentItem()) {
                    QString recentText = recentList->currentItem()->text();
                    // Extract EPSG code from recent item
                    QRegularExpression epsgRegex(R"(EPSG:\d+)");
                    QRegularExpressionMatch match = epsgRegex.match(recentText);
                    if (match.hasMatch()) {
                        selectedCRS = match.captured(0);
                    }
                }
            }
        } else if (tabWidget->currentIndex() == 1) {
            // From project CRS tab - use current CRS
            selectedCRS = "EPSG:4326"; // Default
        } else if (tabWidget->currentIndex() == 2 && !proj4Edit->text().isEmpty()) {
            // From custom CRS tab
            selectedCRS = proj4Edit->text();
        }

        if (!selectedCRS.isEmpty()) {
            onCRSChanged(selectedCRS);
        }
    }

    delete dialog;
}


void MainWindow::populateCRSTree(QTreeWidget *crsTree)
{
    if (!crsTree) return;

    crsTree->clear();

    // Sample CRS data - in a real application, you would load this from
    // a database or EPSG files
    QMap<QString, QList<QPair<QString, QString>>> crsData = {
        {"Geographic (2D)", {
             {"WGS 84", "EPSG:4326"},
             {"NAD83", "EPSG:4269"},
             {"ETRS89", "EPSG:4258"},
             {"JGD2000", "EPSG:4612"},
             {"GDA94", "EPSG:4283"},
             {"Tokyo", "EPSG:4301"}
         }},
        {"Geographic (3D)", {
             {"WGS 84 3D", "EPSG:4979"},
             {"ETRS89 3D", "EPSG:4937"}
         }},
        {"Projected", {
             {"WGS 84 / Pseudo-Mercator", "EPSG:3857"},
             {"WGS 84 / UTM zone 33N", "EPSG:32633"},
             {"WGS 84 / UTM zone 34N", "EPSG:32634"},
             {"OSGB 1936 / British National Grid", "EPSG:27700"},
             {"NAD83 / UTM zone 17N", "EPSG:26917"},
             {"WGS 84 / World Mercator", "EPSG:3395"}
         }},
        {"Engineering", {
             {"Unknown datum based upon the Clarke 1866 ellipsoid", "EPSG:6500"}
         }},
        {"Compound", {
             {"WGS 84 + EGM96 height", "EPSG:9707"}
         }},
        {"Geocentric", {
             {"WGS 84", "EPSG:4328"}
         }}
    };

    for (auto it = crsData.constBegin(); it != crsData.constEnd(); ++it) {
        QString category = it.key();
        QTreeWidgetItem *categoryItem = new QTreeWidgetItem(crsTree, QStringList() << category);
        categoryItem->setIcon(0, QIcon(":/icons/folder.png"));

        for (const auto &crs : it.value()) {
            QString name = crs.first;
            QString authId = crs.second;

            QTreeWidgetItem *crsItem = new QTreeWidgetItem(categoryItem);
            crsItem->setText(0, name);
            crsItem->setText(1, authId);
            crsItem->setText(2, category);
            crsItem->setIcon(0, QIcon(":/icons/projection.png"));

            // Store full data as tooltip
            crsItem->setToolTip(0, QString("%1\n%2").arg(name).arg(authId));
            crsItem->setToolTip(1, authId);
            crsItem->setToolTip(2, category);
        }
    }
}

QString MainWindow::getCRSDisplayName(const QString &crsCode)
{
    // Map EPSG codes to human-readable names
    static QMap<QString, QString> crsNameMap = {
        {"EPSG:4326", "WGS 84"},
        {"EPSG:3857", "WGS 84 / Pseudo-Mercator"},
        {"EPSG:32633", "WGS 84 / UTM zone 33N"},
        {"EPSG:32634", "WGS 84 / UTM zone 34N"},
        {"EPSG:27700", "OSGB 1936 / British National Grid"},
        {"EPSG:4269", "NAD83"},
        {"EPSG:4258", "ETRS89"},
        {"EPSG:4612", "JGD2000"},
        {"EPSG:4979", "WGS 84 3D"},
        {"EPSG:3857", "WGS 84 / Pseudo-Mercator"},
        {"EPSG:3395", "WGS 84 / World Mercator"},
        {"EPSG:4328", "WGS 84 (Geocentric)"}
    };

    if (crsNameMap.contains(crsCode)) {
        return QString("%1 - %2").arg(crsCode).arg(crsNameMap[crsCode]);
    }

    // Check if it's a PROJ.4 string
    if (crsCode.startsWith("+proj=") || crsCode.contains("datum=")) {
        // Extract projection name from PROJ.4 string
        QRegularExpression projRegex(R"(\+proj=(\w+))");
        QRegularExpressionMatch match = projRegex.match(crsCode);
        if (match.hasMatch()) {
            QString projName = match.captured(1).toUpper();
            return QString("Custom: %1").arg(projName);
        }
        return "Custom CRS";
    }

    // Return the code as-is
    return crsCode;
}

void MainWindow::onCRSChanged(const QString &crs)
{
    // Get display name for the CRS
    QString displayName = getCRSDisplayName(crs);

    // Update projection display in status bar
    if (projectionLabel) {
        projectionLabel->setText("Render: " + displayName);

        // Update tooltip with more information
        QString tooltip = QString("Coordinate Reference System: %1\n"
                                  "Click to change CRS\n"
                                  "Press Ctrl+Shift+R to open CRS selector")
                .arg(displayName);
        projectionLabel->setToolTip(tooltip);
    }

    // Update message in status bar
    if (messageLabel) {
        messageLabel->setText(QString("CRS changed to: %1").arg(displayName));
    }

    // Store the CRS in project settings
    if (appSettings) {
        appSettings->setValue("currentCRS", crs);
        appSettings->setValue("currentCRSDisplay", displayName);
    }
    qDebug() << "CRS changed to:" << crs << "(" << displayName << ")";

    animateCRSChange();
    updateRecentCRS(crs);

}
void MainWindow::animateCRSChange()
{
    if (!projectionLabel) return;

    // Store original style
    QString originalStyle = projectionLabel->styleSheet();

    // Animate with color change
    for (int i = 0; i < 3; i++) {
        QTimer::singleShot(i * 200, this, [this, i, originalStyle]() {
            if (i % 2 == 0) {
                projectionLabel->setStyleSheet(
                            "QLabel { "
                            "padding: 3px 8px; "
                            "border: 2px solid #4CAF50; "
                            "border-right: 3px solid #4CAF50; "
                            "background-color: #E8F5E9; "
                            "margin: 1px; "
                            "margin-right: 3px; "
                            "min-height: 22px; "
                            "}"
                            );
            } else {
                projectionLabel->setStyleSheet(originalStyle);
            }
        });
    }

    // Reset to original after animation
    QTimer::singleShot(600, this, [this, originalStyle]() {
        projectionLabel->setStyleSheet(originalStyle);
    });
}

// =========== TABLE SETUP ===========
void MainWindow::setupImageTable()
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "ERROR: Database not open in setupImageTable";
        return;
    }

    qDebug() << "Setting up gis_files table...";

    QSqlQuery query(db);

    // CHECK IF TABLE EXISTS
    bool tableExists = query.exec("SELECT 1 FROM gis_files LIMIT 1");

    if (!tableExists) {
        QString error = query.lastError().text();
        qDebug() << "Table check error:" << error;

        if (error.contains("does not exist")) {
            qDebug() << "Table does not exist, creating...";
            createTable();
        } else {
            qDebug() << "Unexpected error checking table:" << error;
            QMessageBox::critical(this, "Database Error",
                                  "Error checking table: " + error);
            return;
        }
    } else {
        qDebug() << "Table exists";
    }
    qDebug() << "setupImageTable completed successfully";
}


void MainWindow::createTable()
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        qDebug() << "ERROR: Database not open in createTable";
        return;
    }

    QSqlQuery query(db);

    // Enable UUID extension
    query.exec("CREATE EXTENSION IF NOT EXISTS \"uuid-ossp\"");

    QString createTable = R"(
        CREATE TABLE gis_files (
            id SERIAL PRIMARY KEY,
            file_uuid UUID DEFAULT uuid_generate_v4(),
            layer_name VARCHAR(255) NOT NULL,
            file_name VARCHAR(255),
            file_size BIGINT,
            file_type VARCHAR(50),  -- 'geotiff', 'shapefile', 'image', 'vector', etc.
            mime_type VARCHAR(100), -- MIME type for proper handling
            description TEXT,
            file_data BYTEA NOT NULL,  -- Store the ENTIRE file as binary
            thumbnail BYTEA,          -- For images only
            metadata JSONB,          -- Store ALL metadata as JSON (geotransform, projection, etc.)
            created_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            modified_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            created_by VARCHAR(100),
            source_file_path TEXT
        )
    )";

    if (!query.exec(createTable)) {
        qDebug() << "Failed to create table:" << query.lastError().text();
    } else {
        qDebug() << "Table created successfully";

        // Create indexes
        QStringList indexes = {
            "CREATE INDEX idx_layer_name ON gis_files(layer_name)",
            "CREATE INDEX idx_created_date ON gis_files(created_date)",
            "CREATE INDEX idx_file_type ON gis_files(file_type)",
            "CREATE INDEX idx_file_uuid ON gis_files(file_uuid)"
        };

        for (const QString &indexSQL : indexes) {
            query.exec(indexSQL);
        }
    }
}


// =========== STORE IMAGE ===========
// =========== STORE IMAGE ===========
bool MainWindow::storeImageInDatabase(const QString &filePath,
                                     const QString &layerName,
                                     const QString &description)
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "No Database Connection",
                             "Please connect to a database first.");
        return false;
    }

    qDebug() << "Storing file:" << filePath;

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::critical(this, "File Not Found",
                              "File does not exist: " + filePath);
        return false;
    }

    // Read entire file as binary data
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "File Read Error",
                              "Cannot open file: " + filePath);
        return false;
    }

    QByteArray fileData = file.readAll();
    file.close();

    qDebug() << "File size:" << fileData.size() << "bytes";

    QString suffix = fileInfo.suffix().toLower();
    QString fileType;
    QString mimeType;
    QByteArray thumbnailData;
    QJsonObject metadata;

    // Determine file type and collect metadata
    if (suffix == "tif" || suffix == "tiff" || suffix == "geotiff") {
        fileType = "geotiff";
        mimeType = "image/tiff";

        // Extract GeoTIFF metadata
        GDALDataset *dataset = (GDALDataset*)GDALOpen(
                    filePath.toUtf8().constData(), GA_ReadOnly);
        if (dataset) {
            double geoTransform[6];
            if (dataset->GetGeoTransform(geoTransform) == CE_None) {
                QJsonArray geoArray;
                for (int i = 0; i < 6; i++) {
                    geoArray.append(geoTransform[i]);
                }
                metadata["geotransform"] = geoArray;
                metadata["has_geotransform"] = true;
            } else {
                metadata["has_geotransform"] = false;
            }

            const char *wkt = dataset->GetProjectionRef();
            if (wkt && strlen(wkt) > 0) {
                metadata["projection"] = QString(wkt);
            }

            metadata["width"] = dataset->GetRasterXSize();
            metadata["height"] = dataset->GetRasterYSize();
            metadata["band_count"] = dataset->GetRasterCount();

            GDALClose(dataset);
        }

        // Create thumbnail for GeoTIFF
        QImage image(filePath);
        if (!image.isNull()) {
            QPixmap pixmap = QPixmap::fromImage(image);
            QPixmap thumbnail = pixmap.scaled(200, 200,
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QBuffer buffer(&thumbnailData);
            buffer.open(QIODevice::WriteOnly);
            thumbnail.save(&buffer, "PNG");
        }

    } else if (suffix == "shp" || suffix == "shx" || suffix == "dbf" || suffix == "prj") {
        fileType = "shapefile_component";
        mimeType = "application/x-shapefile";

        // For shapefiles, we might want to read the .shp specifically
        if (suffix == "shp") {
            fileType = "shapefile";

            // Get feature count and geometry type using GDAL
            GDALDataset *dataset = (GDALDataset*)GDALOpenEx(
                        filePath.toUtf8().constData(),
                        GDAL_OF_VECTOR | GDAL_OF_READONLY,
                        nullptr, nullptr, nullptr);
            if (dataset) {
                OGRLayer *layer = dataset->GetLayer(0);
                if (layer) {
                    metadata["feature_count"] = (int)layer->GetFeatureCount();
                    metadata["geometry_type"] = OGRGeometryTypeToName(layer->GetGeomType());
                }
                GDALClose(dataset);
            }
        }

    // ============ ADD ALL VECTOR FORMATS HERE ============
    } else if (suffix == "geojson" || suffix == "json") {
        fileType = "geojson";
        mimeType = "application/geo+json";
    } else if (suffix == "kml") {
        fileType = "kml";
        mimeType = "application/vnd.google-earth.kml+xml";
    } else if (suffix == "gml") {
        fileType = "gml";
        mimeType = "application/gml+xml";
    } else if (suffix == "gpkg") {
        fileType = "gpkg";
        mimeType = "application/geopackage+sqlite3";
    } else if (suffix == "dxf") {
        fileType = "dxf";
        mimeType = "application/dxf";
    } else if (suffix == "dwg") {
        fileType = "dwg";
        mimeType = "application/acad";
    } else if (suffix == "cdr") {
        fileType = "cdr";
        mimeType = "application/vnd.corel-draw";
    } else if (suffix == "wmf") {
        fileType = "wmf";
        mimeType = "image/wmf";
    } else if (suffix == "emf") {
        fileType = "emf";
        mimeType = "image/emf";
    } else if (suffix == "odg") {
        fileType = "odg";
        mimeType = "application/vnd.oasis.opendocument.graphics";
    } else if (suffix == "cgm") {
        fileType = "cgm";
        mimeType = "image/cgm";
    } else if (suffix == "plt" || suffix == "hpgl") {
        fileType = "hpgl";
        mimeType = "application/vnd.hp-hpgl";
    } else if (suffix == "fig") {
        fileType = "fig";
        mimeType = "application/x-xfig";
    } else if (suffix == "vsd") {
        fileType = "vsd";
        mimeType = "application/vnd.visio";
    } else if (suffix == "vsdx") {
        fileType = "vsdx";
        mimeType = "application/vnd.ms-visio.drawing";

    } else if (suffix == "jpg" || suffix == "jpeg" || suffix == "png" ||
               suffix == "gif" || suffix == "bmp") {
        fileType = "image";
        mimeType = "image/" + suffix;

        // Create thumbnail for images
        QImage image(filePath);
        if (!image.isNull()) {
            metadata["width"] = image.width();
            metadata["height"] = image.height();

            QPixmap pixmap = QPixmap::fromImage(image);
            QPixmap thumbnail = pixmap.scaled(200, 200,
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
            QBuffer buffer(&thumbnailData);
            buffer.open(QIODevice::WriteOnly);
            thumbnail.save(&buffer, "PNG");
        }

    } else if (suffix == "svg") {
        fileType = "svg";
        mimeType = "image/svg+xml";
    } else if (suffix == "pdf") {
        fileType = "pdf";
        mimeType = "application/pdf";
    } else {
        fileType = "other";
        mimeType = "application/octet-stream";
    }

    // Add common metadata - ONLY JSON-serializable types
    metadata["file_name"] = fileInfo.fileName();
    metadata["file_size"] = (qint64)fileInfo.size();
    metadata["suffix"] = suffix;
    metadata["last_modified"] = fileInfo.lastModified().toString(Qt::ISODate);

    // Prepare insert query
    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO gis_files (
            layer_name, file_name, file_size, file_type, mime_type,
            description, file_data, thumbnail, metadata, created_by, source_file_path
        ) VALUES (
            :layer_name, :file_name, :file_size, :file_type, :mime_type,
            :description, :file_data, :thumbnail, :metadata::jsonb, :created_by, :source_file_path
        ) RETURNING id
    )");

    QString layer = layerName.isEmpty() ? fileInfo.baseName() : layerName;

    query.bindValue(":layer_name", layer);
    query.bindValue(":file_name", fileInfo.fileName());
    query.bindValue(":file_size", (qint64)fileInfo.size());
    query.bindValue(":file_type", fileType);
    query.bindValue(":mime_type", mimeType);
    query.bindValue(":description", description);
    query.bindValue(":file_data", fileData);

    // Only bind thumbnail if it has data
    if (thumbnailData.isEmpty()) {
        query.bindValue(":thumbnail", QVariant(QVariant::ByteArray));
    } else {
        query.bindValue(":thumbnail", thumbnailData);
    }

    // Convert metadata to JSON string - this is the key fix
    QJsonDocument doc(metadata);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    query.bindValue(":metadata", jsonString);

    query.bindValue(":created_by",
                    qgetenv("USERNAME").isEmpty() ? qgetenv("USER") : qgetenv("USERNAME"));
    query.bindValue(":source_file_path", filePath);

    if (!query.exec()) {
        QString error = QString("Failed to store file:\n%1")
                .arg(query.lastError().text());
        qDebug() << error;
        qDebug() << "JSON that caused error:" << jsonString;
        QMessageBox::critical(this, "Database Error", error);
        return false;
    }

    int fileId = -1;
    if (query.next()) {
        fileId = query.value(0).toInt();
    }

    qDebug() << "File stored successfully with ID:" << fileId;

    if (messageLabel) {
        messageLabel->setText(
                    QString("File stored in database (ID: %1)").arg(fileId));
    }

    QMessageBox::information(this, "Success",
                             QString("File stored successfully!\n\n"
                                     "ID: %1\n"
                                     "Layer: %2\n"
                                     "Type: %3\n"
                                     "Size: %4 KB")
                             .arg(fileId)
                             .arg(layer)
                             .arg(fileType)
                             .arg(fileInfo.size() / 1024));

    emit imageStored(fileId, layer);
    return true;
}

// =========== LOAD IMAGE ===========

bool MainWindow::loadImageFromDatabase(int fileId)
{
    qDebug() << "\n=== LOAD FILE FROM DATABASE ===";
    qDebug() << "File ID:" << fileId;

    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "No Database Connection",
                             "Please connect to a database first.");
        return false;
    }

    QSqlQuery query(db);
    // FIXED: Always use gis_files table
    query.prepare("SELECT file_data, layer_name, file_name, file_type, mime_type, metadata FROM gis_files WHERE id = :id");
    query.bindValue(":id", fileId);

    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to load file: " + query.lastError().text());
        return false;
    }

    QByteArray fileData = query.value(0).toByteArray();
    QString layerName = query.value(1).toString();
    QString fileName = query.value(2).toString();
    QString fileType = query.value(3).toString();
    QString mimeType = query.value(4).toString();
    QJsonObject metadata = QJsonDocument::fromJson(query.value(5).toByteArray()).object();

    qDebug() << "File:" << fileName << "Type:" << fileType << "Size:" << fileData.size() << "bytes";

    if (fileData.isEmpty()) {
        QMessageBox::critical(this, "Error", "File data is empty");
        return false;
    }

    // Save to temporary file
    QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString tempFilePath = QDir(tempDir).filePath(fileName);

    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Cannot create temporary file");
        return false;
    }
    tempFile.write(fileData);
    tempFile.close();

    qDebug() << "Saved to temp file:" << tempFilePath;

    // Clear current content
    clearCurrentImage();
    clearVectorItems();

    bool success = false;
    QString displayName = layerName.isEmpty() ? QString("DB_%1").arg(fileId) : layerName;
    QString suffix = QFileInfo(fileName).suffix().toLower();

    // ============ HANDLE ALL FILE TYPES ============

    // 1. GEOTIFF
    if (fileType == "geotiff" || suffix == "tif" || suffix == "tiff") {
        qDebug() << "Loading GeoTIFF...";
        GDALDataset *dataset = (GDALDataset*)GDALOpen(tempFilePath.toUtf8().constData(), GA_ReadOnly);
        if (dataset) {
            hasGeoTransform = (dataset->GetGeoTransform(gdalGeoTransform) == CE_None);
            const char *wkt = dataset->GetProjectionRef();
            if (wkt && strlen(wkt) > 0) updateProjection(extractEPSGFromWKT(QString(wkt)));

            int xSize = dataset->GetRasterXSize();
            int ySize = dataset->GetRasterYSize();
            geoTIFFSize = QSize(xSize, ySize);

            QImage image;
            int bandCount = dataset->GetRasterCount();

            if (bandCount >= 3) {
                image = QImage(xSize, ySize, QImage::Format_RGB32);
                image.fill(Qt::black);
                for (int b = 1; b <= 3 && b <= bandCount; b++) {
                    GDALRasterBand *band = dataset->GetRasterBand(b);
                    if (!band) continue;
                    QVector<unsigned char> buffer(xSize * ySize);
                    CPLErr err = band->RasterIO(GF_Read, 0, 0, xSize, ySize,
                                                buffer.data(), xSize, ySize, GDT_Byte, 0, 0);
                    if (err == CE_None) {
                        for (int y = 0; y < ySize; y++) {
                            for (int x = 0; x < xSize; x++) {
                                QRgb pixel = image.pixel(x, y);
                                int value = buffer[y * xSize + x];
                                if (b == 1) pixel = qRgb(value, qGreen(pixel), qBlue(pixel));
                                else if (b == 2) pixel = qRgb(qRed(pixel), value, qBlue(pixel));
                                else if (b == 3) pixel = qRgb(qRed(pixel), qGreen(pixel), value);
                                image.setPixel(x, y, pixel);
                            }
                        }
                    }
                }
            } else if (bandCount >= 1) {
                GDALRasterBand *band = dataset->GetRasterBand(1);
                if (band) {
                    QVector<unsigned char> buffer(xSize * ySize);
                    CPLErr err = band->RasterIO(GF_Read, 0, 0, xSize, ySize,
                                                buffer.data(), xSize, ySize, GDT_Byte, 0, 0);
                    if (err == CE_None) {
                        image = QImage(buffer.data(), xSize, ySize,
                                      xSize, QImage::Format_Grayscale8).copy();
                    }
                }
            }
            GDALClose(dataset);

            if (!image.isNull()) {
                QPixmap pixmap = QPixmap::fromImage(image);
                if (mapScene) {
                    QGraphicsPixmapItem *item = mapScene->addPixmap(pixmap);
                    currentImageItem = item;
                    currentPixmap = pixmap;
                    currentImagePath = QString("database://%1/%2").arg(fileId).arg(fileName);
                    isGeoTIFFLoaded = true;
                    geoTIFFItem = item;
                    if (mapView) {
                        mapView->fitInView(item, Qt::KeepAspectRatio);
                        currentScale = mapView->transform().m11();
                        updateMagnifier(qRound(currentScale * 100));
                        updateScale(currentScale);
                    }
                    success = true;
                }
            }
        }
        QFile::remove(tempFilePath);
    }

    // 2. VECTOR GIS files (Shapefile, GeoJSON, KML, GPKG)
    else if (fileType == "shapefile" || fileType == "geojson" || fileType == "kml" ||
             fileType == "gml" || fileType == "gpkg" || suffix == "shp" ||
             suffix == "geojson" || suffix == "kml" || suffix == "gml" || suffix == "gpkg") {
        qDebug() << "Loading vector GIS file...";
        drawVectorLayer(tempFilePath);
        if (!currentVectorItems.isEmpty()) {
            QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                        QStringList() << displayName << "Vector (DB)");
            layerItem->setCheckState(0, Qt::Checked);
            layerItem->setIcon(0, QIcon(":/icons/vector_layer.png"));

            QTreeWidgetItem *vectorGroup = nullptr;
            for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
                if (layersTree->topLevelItem(i)->text(0) == "Vector Layers") {
                    vectorGroup = layersTree->topLevelItem(i);
                    break;
                }
            }
            if (!vectorGroup) {
                vectorGroup = new QTreeWidgetItem(layersTree, QStringList() << "Vector Layers");
                vectorGroup->setIcon(0, QIcon(":/icons/folder.png"));
                vectorGroup->setExpanded(true);
            }
            vectorGroup->addChild(layerItem);

            LayerInfo layer;
            layer.name = displayName;
            layer.filePath = QString("database://%1/%2").arg(fileId).arg(fileName);
            layer.type = "vector_db";
            layer.treeItem = layerItem;
            layer.properties["db_id"] = fileId;
            layer.properties["format"] = suffix;
            loadedLayers.append(layer);
            projectModified = true;
            success = true;
        }
        QFile::remove(tempFilePath);
    }

    // 3. OTHER VECTOR formats (SVG, AI, EPS, PDF, DXF, DWG, CDR, etc.)
    else if (fileType == "svg" || fileType == "pdf" || fileType == "ai" ||
             fileType == "eps" || fileType == "dxf" || fileType == "dwg" ||
             fileType == "cdr" || fileType == "wmf" || fileType == "emf" ||
             fileType == "odg" || fileType == "cgm" || fileType == "hpgl" ||
             fileType == "fig" || fileType == "vsd" || fileType == "vsdx" ||
             suffix == "svg" || suffix == "pdf" || suffix == "ai" || suffix == "eps" ||
             suffix == "dxf" || suffix == "dwg" || suffix == "cdr" || suffix == "wmf" ||
             suffix == "emf" || suffix == "odg" || suffix == "cgm" || suffix == "plt" ||
             suffix == "hpgl" || suffix == "fig" || suffix == "vsd" || suffix == "vsdx") {
        qDebug() << "Loading other vector format:" << suffix;

        QColor color;
        if (suffix == "svg") color = QColor(150, 200, 150);
        else if (suffix == "pdf") color = QColor(255, 0, 0);
        else if (suffix == "ai") color = QColor(255, 165, 0);
        else if (suffix == "eps") color = QColor(128, 0, 128);
        else if (suffix == "dxf" || suffix == "dwg") color = QColor(0, 100, 200);
        else if (suffix == "cdr") color = QColor(0, 150, 150);
        else if (suffix == "wmf" || suffix == "emf") color = QColor(0, 150, 150);
        else if (suffix == "odg") color = QColor(0, 150, 0);
        else if (suffix == "cgm") color = QColor(100, 100, 200);
        else if (suffix == "plt" || suffix == "hpgl") color = QColor(150, 75, 0);
        else if (suffix == "fig") color = QColor(0, 200, 200);
        else if (suffix == "vsd" || suffix == "vsdx") color = QColor(0, 150, 150);
        else color = QColor(128, 128, 128);

        QGraphicsRectItem *rectItem = new QGraphicsRectItem(0, 0, 120, 120);
        rectItem->setBrush(QBrush(color.lighter(160)));
        rectItem->setPen(QPen(color, 2));
        QGraphicsTextItem *textItem = new QGraphicsTextItem(suffix.toUpper());
        textItem->setPos(25, 45);
        textItem->setDefaultTextColor(color.darker());
        textItem->setFont(QFont("Arial", 14, QFont::Bold));
        QGraphicsLineItem *line1 = new QGraphicsLineItem(20, 20, 100, 100);
        line1->setPen(QPen(color, 2, Qt::DashLine));
        QGraphicsLineItem *line2 = new QGraphicsLineItem(20, 100, 100, 20);
        line2->setPen(QPen(color, 2, Qt::DashLine));
        QGraphicsEllipseItem *circle = new QGraphicsEllipseItem(50, 50, 20, 20);
        circle->setPen(QPen(color, 2));
        circle->setBrush(Qt::NoBrush);

        QGraphicsItemGroup *group = mapScene->createItemGroup(
            {rectItem, textItem, line1, line2, circle});

        QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                    QStringList() << displayName << QString("%1 (DB)").arg(suffix.toUpper()));
        layerItem->setCheckState(0, Qt::Checked);
        layerItem->setIcon(0, QIcon(":/icons/vector_layer.png"));

        QTreeWidgetItem *vectorGroup = nullptr;
        for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
            if (layersTree->topLevelItem(i)->text(0) == "Vector Layers") {
                vectorGroup = layersTree->topLevelItem(i);
                break;
            }
        }
        if (!vectorGroup) {
            vectorGroup = new QTreeWidgetItem(layersTree, QStringList() << "Vector Layers");
            vectorGroup->setIcon(0, QIcon(":/icons/folder.png"));
            vectorGroup->setExpanded(true);
        }
        vectorGroup->addChild(layerItem);

        LayerInfo layer;
        layer.name = displayName;
        layer.filePath = QString("database://%1/%2").arg(fileId).arg(fileName);
        layer.type = "vector_db";
        layer.graphicsItem = group;
        layer.treeItem = layerItem;
        layer.properties["db_id"] = fileId;
        layer.properties["format"] = suffix;
        loadedLayers.append(layer);
        projectModified = true;
        success = true;
        QFile::remove(tempFilePath);
    }

    // 4. RASTER IMAGES (JPEG, PNG, GIF, BMP, WEBP, HEIC, PSD, XCF, EXR, HDR, etc.)
    else {
        qDebug() << "Loading raster image...";
        QPixmap pixmap;
        if (pixmap.load(tempFilePath)) {
            if (mapScene) {
                QGraphicsPixmapItem *item = mapScene->addPixmap(pixmap);
                currentImageItem = item;
                currentPixmap = pixmap;
                currentImagePath = QString("database://%1/%2").arg(fileId).arg(fileName);
                if (mapView) {
                    mapView->fitInView(item, Qt::KeepAspectRatio);
                    currentScale = mapView->transform().m11();
                    updateMagnifier(qRound(currentScale * 100));
                    updateScale(currentScale);
                }

                QTreeWidgetItem *layerItem = new QTreeWidgetItem(
                            QStringList() << displayName << "Image (DB)");
                layerItem->setCheckState(0, Qt::Checked);
                layerItem->setIcon(0, QIcon(":/icons/database.png"));

                QTreeWidgetItem *rasterGroup = nullptr;
                for (int i = 0; i < layersTree->topLevelItemCount(); ++i) {
                    if (layersTree->topLevelItem(i)->text(0) == "Raster Layers") {
                        rasterGroup = layersTree->topLevelItem(i);
                        break;
                    }
                }
                if (!rasterGroup) {
                    rasterGroup = new QTreeWidgetItem(layersTree,
                                                      QStringList() << "Raster Layers");
                    rasterGroup->setIcon(0, QIcon(":/icons/folder.png"));
                    rasterGroup->setExpanded(true);
                }
                rasterGroup->addChild(layerItem);

                LayerInfo layer;
                layer.name = displayName;
                layer.filePath = currentImagePath;
                layer.type = "image_db";
                layer.graphicsItem = item;
                layer.treeItem = layerItem;
                layer.properties["db_id"] = fileId;
                layer.properties["file_type"] = fileType;
                loadedLayers.append(layer);
                projectModified = true;
                success = true;
            }
        }
        QFile::remove(tempFilePath);
    }

    if (success) {
        updateImageInfo();
        if (messageLabel) {
            messageLabel->setText(
                        QString("Loaded from database: %1 (ID: %2)")
                        .arg(displayName).arg(fileId));
        }
        QMessageBox::information(this, "Success",
                                 QString("File loaded successfully!\n\n"
                                         "Layer: %1\n"
                                         "Type: %2\n"
                                         "ID: %3")
                                 .arg(displayName)
                                 .arg(fileType.isEmpty() ? suffix.toUpper() : fileType)
                                 .arg(fileId));
        emit imageLoaded(fileId, displayName);
        return true;
    } else {
        QMessageBox::critical(this, "Error",
                              QString("Failed to load file from database.\n"
                                      "File: %1\n"
                                      "Type: %2\n"
                                      "Size: %3 bytes")
                              .arg(fileName)
                              .arg(fileType.isEmpty() ? suffix : fileType)
                              .arg(fileData.size()));
        return false;
    }
}


// =========== DELETE IMAGE ===========


void MainWindow::deleteImageFromDatabase(int fileId)
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "No Database Connection",
                             "Please connect to a database first.");
        return;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "Confirm Delete",
                                                              QString("Are you sure you want to delete file ID %1 from the database?\n\n"
                                                                      "This action cannot be undone.")
                                                              .arg(fileId),
                                                              QMessageBox::Yes | QMessageBox::No,
                                                              QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    QSqlQuery query(db);
    query.prepare("DELETE FROM gis_files WHERE id = :id");
    query.bindValue(":id", fileId);

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to delete file:\n" + query.lastError().text());
        return;
    }

    QMessageBox::information(this, "Success",
                             QString("File ID %1 deleted successfully").arg(fileId));

    if (messageLabel) {
        messageLabel->setText(QString("File ID %1 deleted").arg(fileId));
    }
}


QVector<QPair<int, QString>> MainWindow::getImageListFromDatabase()
{
    QVector<QPair<int, QString>> fileList;

    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        return fileList;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT id, layer_name, file_name, file_type,
               created_date, file_size
        FROM gis_files
        ORDER BY created_date DESC
    )");

    if (query.exec()) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString layerName = query.value(1).toString();
            QString fileName = query.value(2).toString();
            QString fileType = query.value(3).toString();
            QDateTime created = query.value(4).toDateTime();
            qint64 fileSize = query.value(5).toLongLong();

            QString info = QString("%1 - %2 [ID: %3]\n"
                                   "  Type: %4, %5 KB\n"
                                   "  Created: %6")
                    .arg(layerName)
                    .arg(fileName)
                    .arg(id)
                    .arg(fileType)
                    .arg(fileSize / 1024)
                    .arg(created.toString("yyyy-MM-dd hh:mm"));

            fileList.append(qMakePair(id, info));
        }
    }

    return fileList;
}


void MainWindow::showDatabaseManager()
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                  "No Database Connection",
                                                                  "You are not connected to a database.\n\n"
                                                                  "Would you like to connect now?",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            createDatabaseConnectionDialog();
        }

        db = getImageDb();  // Try again after connection dialog
        if (!db.isValid() || !db.isOpen()) {
            return;
        }
    }

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Database Image Manager - " + db.databaseName());
    dialog->resize(1000, 700);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Header with connection info
    QHBoxLayout *headerLayout = new QHBoxLayout();

    QLabel *connectionLabel = new QLabel(
                QString("<b>Connected to:</b> %1@%2/%3")
                .arg(db.userName())
                .arg(db.hostName())
                .arg(db.databaseName()));
    connectionLabel->setStyleSheet("padding: 5px; background-color: #e8f5e9;");
    headerLayout->addWidget(connectionLabel);


    headerLayout->addStretch();

    QPushButton *refreshBtn = new QPushButton(QIcon(":/icons/refresh.png"), "Refresh");
    refreshBtn->setToolTip("Refresh image list");
    headerLayout->addWidget(refreshBtn);

    QPushButton *disconnectBtn = new QPushButton(
                QIcon(":/icons/disconnect.png"), "Disconnect");
    disconnectBtn->setToolTip("Close database connection");
    headerLayout->addWidget(disconnectBtn);

    layout->addLayout(headerLayout);

    // Search bar
    QLineEdit *searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Search images by layer name, file name...");
    searchEdit->addAction(QIcon(":/icons/search.png"), QLineEdit::LeadingPosition);
    searchEdit->setClearButtonEnabled(true);
    layout->addWidget(searchEdit);

    // Statistics
    QLabel *statsLabel = new QLabel();
    statsLabel->setStyleSheet("padding: 5px; background-color: #f5f5f5;");
    layout->addWidget(statsLabel);

    // Image list table
    QTreeWidget *imageList = new QTreeWidget();
    imageList->setHeaderLabels(QStringList()
                               << "ID" << "Layer Name" << "File Name"
                               << "Size" << "Type" << "Created");
    imageList->setColumnCount(6);
    imageList->setAlternatingRowColors(true);
    imageList->setSelectionBehavior(QAbstractItemView::SelectRows);
    imageList->setSortingEnabled(true);
    imageList->setRootIsDecorated(false);

    // Set column widths
    imageList->setColumnWidth(0, 50);   // ID
    imageList->setColumnWidth(1, 200);  // Layer Name
    imageList->setColumnWidth(2, 200);  // File Name
    imageList->setColumnWidth(3, 100);  // Size
    imageList->setColumnWidth(4, 80);   // Type
    imageList->setColumnWidth(5, 150);  // Created

    layout->addWidget(imageList);

    // Button bar
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *loadBtn = new QPushButton(
                QIcon(":/icons/load_image.png"), "Load Selected");
    loadBtn->setToolTip("Load selected image into map view");

    QPushButton *deleteBtn = new QPushButton(
                QIcon(":/icons/delete.png"), "Delete Selected");
    deleteBtn->setToolTip("Delete selected image from database");

    QPushButton *exportBtn = new QPushButton(
                QIcon(":/icons/export.png"), "Export Selected");
    exportBtn->setToolTip("Export selected image to file");

    QPushButton *propertiesBtn = new QPushButton(
                QIcon(":/icons/properties.png"), "Properties");
    propertiesBtn->setToolTip("View image properties");

    buttonLayout->addWidget(loadBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(exportBtn);
    buttonLayout->addWidget(propertiesBtn);
    buttonLayout->addStretch();

    QPushButton *closeBtn = new QPushButton("Close");
    buttonLayout->addWidget(closeBtn);

    layout->addLayout(buttonLayout);

    // IMPORTANT FIX: Use imageDb directly, don't create a local copy
    // The local copy can cause crashes when it goes out of scope

    // Function to load image list - capture imageDb by reference
    // Function to load file list - use gis_files table
    auto loadImageList = [this, imageList, statsLabel]() {
        imageList->clear();

        QSqlDatabase db = getImageDb();
        if (!db.isValid() || !db.isOpen()) {
            QMessageBox::critical(nullptr, "Connection Error",
                                  "Database connection is not open");
            return;
        }

        QSqlQuery query(db);
        query.prepare(R"(
            SELECT id, layer_name, file_name, file_type,
                   created_date, file_size
            FROM gis_files
            ORDER BY created_date DESC
        )");

        if (!query.exec()) {
            QMessageBox::critical(nullptr, "Query Error",
                                  "Failed to load files:\n" + query.lastError().text());
            return;
        }

        int totalCount = 0;
        qint64 totalSize = 0;
        int geoTiffCount = 0;

        while (query.next()) {
            QTreeWidgetItem *item = new QTreeWidgetItem(imageList);

            int id = query.value(0).toInt();
            QString layerName = query.value(1).toString();
            QString fileName = query.value(2).toString();
            QString fileType = query.value(3).toString();
            QDateTime created = query.value(4).toDateTime();
            qint64 fileSize = query.value(5).toLongLong();

            item->setText(0, QString::number(id));
            item->setText(1, layerName);
            item->setText(2, fileName);
            item->setText(3, QString("%1 KB").arg(fileSize / 1024));
            item->setText(4, fileType);
            item->setText(5, created.toString("yyyy-MM-dd hh:mm"));

            item->setData(0, Qt::UserRole, id);

            if (fileType == "geotiff") {
                item->setIcon(4, QIcon(":/icons/geotiff.png"));
                geoTiffCount++;
            } else if (fileType == "image") {
                item->setIcon(4, QIcon(":/icons/raster_layer.png"));
            } else if (fileType == "shapefile") {
                item->setIcon(4, QIcon(":/icons/vector_layer.png"));
            } else {
                item->setIcon(4, QIcon(":/icons/file.png"));
            }

            totalCount++;
            totalSize += fileSize;
        }

        // Update statistics
        statsLabel->setText(QString(
                                "<b>Total Files:</b> %1  |  "
                                "<b>GeoTIFFs:</b> %2  |  "
                                "<b>Total Size:</b> %3 MB")
                            .arg(totalCount)
                            .arg(geoTiffCount)
                            .arg(totalSize / (1024.0 * 1024.0), 0, 'f', 2));
    };

    // Initial load
    loadImageList();

    // Connect signals
    connect(refreshBtn, &QPushButton::clicked, loadImageList);

    connect(searchEdit, &QLineEdit::textChanged,
            [imageList](const QString &text) {
        for (int i = 0; i < imageList->topLevelItemCount(); ++i) {
            QTreeWidgetItem *item = imageList->topLevelItem(i);
            bool match = text.isEmpty() ||
                    item->text(1).contains(text, Qt::CaseInsensitive) ||
                    item->text(2).contains(text, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    });

    connect(loadBtn, &QPushButton::clicked,
            [this, dialog, imageList]() {
        QTreeWidgetItem *item = imageList->currentItem();
        if (item) {
            int id = item->data(0, Qt::UserRole).toInt();
            if (loadImageFromDatabase(id)) {
                dialog->accept();
            }
        } else {
            QMessageBox::information(dialog, "No Selection",
                                     "Please select an image to load");
        }
    });

    connect(deleteBtn, &QPushButton::clicked,
            [this, imageList, loadImageList]() {
        QTreeWidgetItem *item = imageList->currentItem();
        if (item) {
            int id = item->data(0, Qt::UserRole).toInt();
            deleteImageFromDatabase(id);
            loadImageList();
        } else {
            QMessageBox::information(nullptr, "No Selection",
                                     "Please select an image to delete");
        }
    });

    // Update all other lambdas that use imageDb
    connect(exportBtn, &QPushButton::clicked,
            [this, imageList]() {
        QTreeWidgetItem *item = imageList->currentItem();
        if (!item) {
            QMessageBox::information(nullptr, "No Selection",
                                     "Please select an image to export");
            return;
        }

        int id = item->data(0, Qt::UserRole).toInt();
        QString fileName = item->text(2);

        QString savePath = QFileDialog::getSaveFileName(this,
                                                        "Export Image",
                                                        QDir::home().filePath(fileName),
                                                        "Images (*.png *.jpg *.tif *.tiff);;All Files (*)");

        if (savePath.isEmpty()) return;

        QSqlDatabase db = getImageDb();
        if (!db.isValid() || !db.isOpen()) {
            QMessageBox::critical(this, "Connection Error", "Database connection lost");
            return;
        }

        QSqlQuery query(db);  // Changed from imageDb
        query.prepare("SELECT image_data FROM gis_images WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec() && query.next()) {
            QByteArray imageData = query.value(0).toByteArray();
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(imageData);
                file.close();
                QMessageBox::information(this, "Success",
                                         "Image exported successfully to:\n" + savePath);
            } else {
                QMessageBox::critical(this, "Error",
                                      "Failed to write file:\n" + file.errorString());
            }
        } else {
            QMessageBox::critical(this, "Database Error",
                                  "Failed to read image data:\n" + query.lastError().text());
        }
    });

    // In showDatabaseManager(), REPLACE the exportBtn connection with:
    connect(exportBtn, &QPushButton::clicked,
            [this, imageList]() {
        QTreeWidgetItem *item = imageList->currentItem();
        if (!item) {
            QMessageBox::information(nullptr, "No Selection",
                                     "Please select a file to export");
            return;
        }

        int id = item->data(0, Qt::UserRole).toInt();
        QString fileName = item->text(2);
        QString savePath = QFileDialog::getSaveFileName(this,
                                                        "Export File",
                                                        QDir::home().filePath(fileName),
                                                        "All Files (*.*)");

        if (savePath.isEmpty()) return;

        QSqlDatabase db = getImageDb();
        if (!db.isValid() || !db.isOpen()) {
            QMessageBox::critical(this, "Connection Error", "Database connection lost");
            return;
        }

        // FIXED: Use gis_files and file_data
        QSqlQuery query(db);
        query.prepare("SELECT file_data FROM gis_files WHERE id = :id");
        query.bindValue(":id", id);

        if (query.exec() && query.next()) {
            QByteArray fileData = query.value(0).toByteArray();
            QFile file(savePath);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(fileData);
                file.close();
                QMessageBox::information(this, "Success",
                                         "File exported successfully to:\n" + savePath);
            } else {
                QMessageBox::critical(this, "Error",
                                      "Failed to write file:\n" + file.errorString());
            }
        } else {
            QMessageBox::critical(this, "Database Error",
                                  "Failed to read file data:\n" + query.lastError().text());
        }
    });

    connect(disconnectBtn, &QPushButton::clicked,
            [this, dialog]() {
        QMessageBox::StandardButton reply = QMessageBox::question(dialog,
                                                                  "Confirm Disconnect",
                                                                  "Are you sure you want to disconnect from the database?",
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            closeDatabaseConnection();
            dialog->accept();
        }
    });

    connect(closeBtn, &QPushButton::clicked, dialog, &QDialog::accept);

    connect(imageList, &QTreeWidget::itemDoubleClicked,
            [this, dialog](QTreeWidgetItem *item) {
        int id = item->data(0, Qt::UserRole).toInt();
        if (loadImageFromDatabase(id)) {
            dialog->accept();
        }
    });

    dialog->exec();
    delete dialog;
}

// =========== CONNECTION DIALOG ===========

void MainWindow::createDatabaseConnectionDialog()
{
    static bool isDialogOpen = false;
    if (isDialogOpen) {
        qDebug() << "Connection dialog already open, ignoring duplicate";
        return;
    }
    isDialogOpen = true;

    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Connect to PostgreSQL Database");
    dialog->resize(500, 400);

    QVBoxLayout *layout = new QVBoxLayout(dialog);

    // Title
    QLabel *titleLabel = new QLabel("<h2>PostgreSQL Connection</h2>");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("padding: 10px; background-color: #e3f2fd;");
    layout->addWidget(titleLabel);

    // Connection group
    QGroupBox *connectionGroup = new QGroupBox("Connection Settings");
    QFormLayout *formLayout = new QFormLayout(connectionGroup);

    // Host
    QComboBox *hostCombo = new QComboBox();
    hostCombo->setEditable(true);
    hostCombo->addItems({"127.0.0.1", "localhost", "192.168.1.100"});
    QString lastHost = appSettings ?
                appSettings->value("lastDBHost", "127.0.0.1").toString() : "127.0.0.1";
    hostCombo->setCurrentText(lastHost);
    formLayout->addRow("Host:", hostCombo);

    // Port
    QSpinBox *portSpin = new QSpinBox();
    portSpin->setRange(1, 65535);
    int lastPort = appSettings ?
                appSettings->value("lastDBPort", 5432).toInt() : 5432;
    portSpin->setValue(lastPort);
    formLayout->addRow("Port:", portSpin);

    // Database name
    QLineEdit *dbNameEdit = new QLineEdit();
    QString lastDB = appSettings ?
                appSettings->value("lastDBName", "qt_db").toString() : "qt_db";
    dbNameEdit->setText(lastDB);
    dbNameEdit->setPlaceholderText("Enter database name");
    formLayout->addRow("Database:", dbNameEdit);

    // Username
    QLineEdit *userEdit = new QLineEdit();
    QString lastUser = appSettings ?
                appSettings->value("lastDBUser", "postgres").toString() : "postgres";
    userEdit->setText(lastUser);
    userEdit->setPlaceholderText("Enter username");
    formLayout->addRow("Username:", userEdit);

    // Password
    QLineEdit *passwordEdit = new QLineEdit();
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setPlaceholderText("Enter password");
    formLayout->addRow("Password:", passwordEdit);

    // Remember password checkbox
    QCheckBox *rememberCheck = new QCheckBox("Remember connection settings");
    rememberCheck->setChecked(true);
    formLayout->addRow("", rememberCheck);

    layout->addWidget(connectionGroup);

    // Test connection button
    QPushButton *testBtn = new QPushButton(
                QIcon(":/icons/database.png"), "Test Connection");
    testBtn->setStyleSheet("padding: 8px; font-weight: bold;");
    layout->addWidget(testBtn);

    // Status label
    QLabel *statusLabel = new QLabel("");
    statusLabel->setWordWrap(true);
    statusLabel->setMinimumHeight(40);
    layout->addWidget(statusLabel);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Connect");
    buttonBox->button(QDialogButtonBox::Ok)->setIcon(QIcon(":/icons/connect.png"));
    layout->addWidget(buttonBox);

    // Connect test button
    connect(testBtn, &QPushButton::clicked,
            [this, hostCombo, portSpin, dbNameEdit, userEdit, passwordEdit, statusLabel]() {
        statusLabel->setText("Testing connection...");
        statusLabel->setStyleSheet("color: blue; padding: 5px;");

        QApplication::processEvents();

        qDebug() << "=== CONNECTION ATTEMPT ===";
        qDebug() << "Host:" << hostCombo->currentText();
        qDebug() << "Port:" << portSpin->value();
        qDebug() << "Database:" << dbNameEdit->text();
        qDebug() << "Username:" << userEdit->text();  // <-- ADD THIS
        qDebug() << "Password length:" << passwordEdit->text().length();

        if (connectToPostgreSQL(hostCombo->currentText(),
                                portSpin->value(),
                                dbNameEdit->text(),
                                userEdit->text(),
                                passwordEdit->text())) {
            statusLabel->setText("✓ Connection successful!");
            statusLabel->setStyleSheet(
                        "color: green; font-weight: bold; padding: 5px; "
                        "background-color: #e8f5e9;");
        } else {
            statusLabel->setText("✗ Connection failed! Check your settings.");
            statusLabel->setStyleSheet(
                        "color: red; font-weight: bold; padding: 5px; "
                        "background-color: #ffebee;");
        }
    });

    // Connect OK button
    connect(buttonBox, &QDialogButtonBox::accepted,
            [this, dialog, hostCombo, portSpin, dbNameEdit,
            userEdit, passwordEdit, rememberCheck]() {

        if (dbNameEdit->text().isEmpty()) {
            QMessageBox::warning(dialog, "Missing Information",
                                 "Please enter a database name");
            return;
        }

        if (userEdit->text().isEmpty()) {
            QMessageBox::warning(dialog, "Missing Information",
                                 "Please enter a username");
            return;
        }

        if (connectToPostgreSQL(hostCombo->currentText(),
                                portSpin->value(),
                                dbNameEdit->text(),
                                userEdit->text(),
                                passwordEdit->text())) {
            dialog->accept();
        }
    });

    connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

    // Set focus to password field if other fields are filled
    if (!lastHost.isEmpty() && !lastDB.isEmpty() && !lastUser.isEmpty()) {
        passwordEdit->setFocus();
    }

    dialog->exec();
    isDialogOpen = false;
    delete dialog;
}

// =========== HELPER FUNCTIONS ===========

void MainWindow::updateDatabaseConnectionsCombo()
{
    qDebug() << "=== updateDatabaseConnectionsCombo START ===";

    if (!dbConnectionCombo) {
        qDebug() << "ERROR: dbConnectionCombo is null, returning";
        return;
    }

    qDebug() << "Step 1: Clearing combo";
    dbConnectionCombo->clear();

    qDebug() << "Step 2: Checking if dbConnections is empty";
    if (dbConnections.isEmpty()) {
        qDebug() << "  dbConnections is empty";
        dbConnectionCombo->addItem("No active connections");
        dbConnectionCombo->setEnabled(false);
        qDebug() << "=== updateDatabaseConnectionsCombo END (empty) ===";
        return;
    }

    qDebug() << "Step 3: Building valid connections list";
    QStringList validConnections;

    for (auto it = dbConnections.begin(); it != dbConnections.end(); ++it) {
        QString connName = it.key();
        qDebug() << "  Checking connection:" << connName;

        if (connName.isEmpty()) {
            qDebug() << "    Skipping empty connection name";
            continue;
        }

        if (!QSqlDatabase::contains(connName)) {
            qDebug() << "    Connection not found in QSqlDatabase";
            continue;
        }

        qDebug() << "  Getting database for connection:" << connName;
        QSqlDatabase db = QSqlDatabase::database(connName, false); // DON'T create if missing

        if (!db.isValid()) {
            qDebug() << "    Database is not valid";
            continue;
        }

        if (!db.isOpen()) {
            qDebug() << "    Database is not open";
            continue;
        }

        qDebug() << "  Connection is valid and open";
        QString displayText = QString("%1@%2/%3")
                .arg(db.userName())
                .arg(db.hostName())
                .arg(db.databaseName());

        qDebug() << "  Adding to combo:" << displayText;
        dbConnectionCombo->addItem(displayText, connName);
        validConnections.append(connName);
    }

    qDebug() << "Step 4: Setting combo enabled state";
    dbConnectionCombo->setEnabled(dbConnectionCombo->count() > 0);

    qDebug() << "Step 5: Setting current connection";
    if (!currentDbConnection.isEmpty() && validConnections.contains(currentDbConnection)) {
        int index = dbConnectionCombo->findData(currentDbConnection);
        qDebug() << "  Current connection index:" << index;
        if (index >= 0) {
            dbConnectionCombo->setCurrentIndex(index);
        }
    }

    qDebug() << "=== updateDatabaseConnectionsCombo END ===";
}

QString MainWindow::extractEPSGFromWKT(const QString &wkt)
{
    // Try to extract EPSG code from WKT string
    //    QRegularExpression epsgRegex(R"(AUTHORITY\["EPSG","(\d+)"\)");
    //    QRegularExpressionMatch match = epsgRegex.match(wkt);

    //    if (match.hasMatched()) {
    //        return "EPSG:" + match.captured(1);
    //    }

    // Default to WGS84 if not found
    return "EPSG:4326";
}

// =========== SLOTS ===========

void MainWindow::onConnectDatabase()
{
    // Block ONLY the very first automatic call at startup
    if (!m_connectionDialogShown && sender() == connectDBAction) {
        qDebug() << "==========================================";
        qDebug() << "Blocking automatic connectDBAction trigger at startup!";
        qDebug() << "==========================================";
        m_connectionDialogShown = true;
        return;
    }

    m_connectionDialogShown = true;
    createDatabaseConnectionDialog();
}

void MainWindow::onDisconnectDatabase()
{
    closeDatabaseConnection();
}

void MainWindow::onStoreCurrentImage()
{
    // Check if we have ANY layer loaded
    if (loadedLayers.isEmpty()) {
        QMessageBox::information(this, "No Layer",
                                 "Please load a layer first before storing it in the database.");
        return;
    }

    // Get currently selected layer from layers tree
    QTreeWidgetItem *currentItem = layersTree->currentItem();
    QString selectedLayerName;

    if (currentItem && currentItem->parent()) {
        selectedLayerName = currentItem->text(0);
    } else {
        // If no layer selected, use the first loaded layer
        if (!loadedLayers.isEmpty()) {
            selectedLayerName = loadedLayers.first().name;
        } else {
            QMessageBox::information(this, "No Layer",
                                     "Please select a layer to store in the database.");
            return;
        }
    }

    // Find the layer info
    LayerInfo *layerToStore = nullptr;
    for (int i = 0; i < loadedLayers.size(); ++i) {
        if (loadedLayers[i].name == selectedLayerName) {
            layerToStore = &loadedLayers[i];
            break;
        }
    }

    if (!layerToStore) {
        QMessageBox::critical(this, "Error", "Could not find selected layer.");
        return;
    }

    // Ask for layer name (with current name as default)
    bool ok;
    QString layerName = QInputDialog::getText(this,
                                              "Store Layer",
                                              "Enter layer name for database:",
                                              QLineEdit::Normal,
                                              layerToStore->name,
                                              &ok);

    if (!ok || layerName.isEmpty()) {
        return;
    }

    // Ask for description
    QString description = QInputDialog::getMultiLineText(this,
                                                         "Store Layer",
                                                         "Enter description (optional):",
                                                         "",
                                                         &ok);

    // ============ FIX: Handle Shapefile components ============
    QString filePath = layerToStore->filePath;
    QString suffix = QFileInfo(filePath).suffix().toLower();

    // Check if this is a Shapefile
    if (suffix == "shp") {
        QFileInfo fi(filePath);
        QString basePath = fi.path() + "/" + fi.completeBaseName();

        // Check for all shapefile components
        QStringList extensions = {"shp", "shx", "dbf", "prj", "qpj", "cpg", "sbn", "sbx", "fbn", "fbx", "ain", "aih", "ixs", "mxs", "atx", "xml"};
        bool allComponentsStored = true;

        for (const QString &ext : extensions) {
            QString componentPath = basePath + "." + ext;
            if (QFile::exists(componentPath)) {
                qDebug() << "Storing shapefile component:" << componentPath;
                // Store each component with a special naming convention
                QString componentLayerName = layerName + "_" + ext;
                storeImageInDatabase(componentPath, componentLayerName, description + " (" + ext + " component)");
            }
        }

        QMessageBox::information(this, "Shapefile Stored",
                                 QString("Shapefile '%1' components stored in database.\n"
                                         "Use 'Load from Database' to load all components.")
                                 .arg(layerName));
        return;
    }

    // ============ Regular file (non-shapefile) ============
    if (QFile::exists(filePath)) {
        storeImageInDatabase(filePath, layerName, description);
    } else {
        QMessageBox::critical(this, "File Not Found",
                              "Original file not found:\n" + filePath);
    }
}

// Add this function implementation after storeImageInDatabase:

bool MainWindow::storePixmapInDatabase(const QPixmap &pixmap,
                                       const QString &layerName,
                                       const QString &description)
{
    QSqlDatabase db = getImageDb();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "No Database Connection",
                             "Please connect to a database first.");
        return false;
    }

    if (pixmap.isNull()) {
        QMessageBox::warning(this, "Invalid Image", "No image to store.");
        return false;
    }

    // Convert pixmap to byte array
    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");

    // Create thumbnail
    QPixmap thumbnail = pixmap.scaled(200, 200,
                                      Qt::KeepAspectRatio, Qt::SmoothTransformation);
    QByteArray thumbnailData;
    QBuffer thumbnailBuffer(&thumbnailData);
    thumbnailBuffer.open(QIODevice::WriteOnly);
    thumbnail.save(&thumbnailBuffer, "PNG");

    QString layer = layerName.isEmpty() ? "Untitled" : layerName;
    QString fileName = "capture_" +
            QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".png";

    // Create metadata
    QJsonObject metadata;
    metadata["width"] = pixmap.width();
    metadata["height"] = pixmap.height();
    metadata["format"] = "PNG";
    metadata["source"] = "capture";

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO gis_files (
            layer_name, file_name, file_size, file_type, mime_type,
            description, file_data, thumbnail, metadata, created_by, source_file_path
        ) VALUES (
            :layer_name, :file_name, :file_size, :file_type, :mime_type,
            :description, :file_data, :thumbnail, :metadata::jsonb, :created_by, :source_file_path
        ) RETURNING id
    )");

    query.bindValue(":layer_name", layer);
    query.bindValue(":file_name", fileName);
    query.bindValue(":file_size", (qint64)imageData.size());
    query.bindValue(":file_type", "image");
    query.bindValue(":mime_type", "image/png");
    query.bindValue(":description", description);
    query.bindValue(":file_data", imageData);
    query.bindValue(":thumbnail", thumbnailData);

    // Convert metadata to JSON string
    QJsonDocument doc(metadata);
    QString jsonString = doc.toJson(QJsonDocument::Compact);
    query.bindValue(":metadata", jsonString);

    query.bindValue(":created_by",
                    qgetenv("USERNAME").isEmpty() ? qgetenv("USER") : qgetenv("USERNAME"));
    query.bindValue(":source_file_path", "capture");

    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error",
                              "Failed to store image:\n" + query.lastError().text());
        return false;
    }

    int fileId = -1;
    if (query.next()) {
        fileId = query.value(0).toInt();
    }

    if (messageLabel) {
        messageLabel->setText(
                    QString("Image stored in database (ID: %1)").arg(fileId));
    }

    QMessageBox::information(this, "Success",
                             QString("Image stored successfully!\n\n"
                                     "ID: %1\n"
                                     "Layer: %2\n"
                                     "Type: PNG\n"
                                     "Size: %3 KB")
                             .arg(fileId)
                             .arg(layer)
                             .arg(imageData.size() / 1024));

    emit imageStored(fileId, layer);
    return true;
}
void MainWindow::onLoadFromDatabase()
{
    showDatabaseManager();
}

void MainWindow::onShowDatabaseManager()
{
    showDatabaseManager();
}

QSqlDatabase MainWindow::getImageDb()
{
    if (imageDbConnectionName.isEmpty()) {
        return QSqlDatabase();
    }
    if (!QSqlDatabase::contains(imageDbConnectionName)) {
        imageDbConnectionName.clear();
        return QSqlDatabase();
    }
    return QSqlDatabase::database(imageDbConnectionName, false);
}

bool MainWindow::connectToPostgreSQL(const QString &host, int port,
                                     const QString &dbName,
                                     const QString &user,
                                     const QString &password)
{
    try {
        qDebug() << "=== connectToPostgreSQL START ===";

        if (!dbConnectionCombo) {
            qDebug() << "WARNING: dbConnectionCombo is NULL!";
        }

        // Generate a unique connection name
        QString connectionName = QString("main_postgres_%1_%2_%3_%4")
                .arg(host)
                .arg(port)
                .arg(dbName)
                .arg(QDateTime::currentMSecsSinceEpoch());

        qDebug() << "Step 1: Closing existing connection";
        // Close existing connection if any
        try {
            QSqlDatabase existingDb = getImageDb();
            if (existingDb.isValid() && existingDb.isOpen()) {
                QString oldName = existingDb.connectionName();
                qDebug() << "Closing existing connection:" << oldName;
                existingDb.close();
                if (QSqlDatabase::contains(oldName)) {
                    QSqlDatabase::removeDatabase(oldName);
                }
                imageDbConnectionName.clear();
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 1:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception in Step 1";
        }

        qDebug() << "Step 2: Removing old connection if exists";
        try {
            if (QSqlDatabase::contains(connectionName)) {
                QSqlDatabase::removeDatabase(connectionName);
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 2:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception in Step 2";
        }

        qDebug() << "Step 3: Creating new database connection";
        QSqlDatabase db;
        try {
            db = QSqlDatabase::addDatabase("QPSQL", connectionName);
            db.setHostName(host);
            db.setPort(port);
            db.setDatabaseName(dbName);
            db.setUserName(user);
            db.setPassword(password);
            db.setConnectOptions("connect_timeout=10");
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 3:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception in Step 3";
            return false;
        }

        qDebug() << "Step 4: Opening database";
        try {
            if (!db.open()) {
                QString errorMsg = QString("Failed to connect to PostgreSQL:\n\n"
                                           "Host: %1\nPort: %2\nDatabase: %3\nUser: %4\n\n"
                                           "Error: %5")
                        .arg(host).arg(port).arg(dbName).arg(user)
                        .arg(db.lastError().text());

                qDebug() << "Connection failed:" << db.lastError().text();
                QMessageBox::critical(this, "Connection Failed", errorMsg);

                if (QSqlDatabase::contains(connectionName)) {
                    QSqlDatabase::removeDatabase(connectionName);
                }
                return false;
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 4:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception in Step 4";
            return false;
        }

        qDebug() << "Step 5: Database opened successfully";
        // IMPORTANT: Store the connection name, not the database object
        imageDbConnectionName = connectionName;

        qDebug() << "Step 6: Testing connection with query";
        try {
            QSqlQuery testQuery(db);
            if (!testQuery.exec("SELECT version()")) {
                qDebug() << "Warning: Query failed:" << testQuery.lastError().text();
            } else {
                if (testQuery.next()) {
                    qDebug() << "PostgreSQL Version:" << testQuery.value(0).toString();
                }
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 6:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception in Step 6";
        }

        qDebug() << "Step 7: Storing connection name";
        currentDbConnection = connectionName;

        qDebug() << "Step 8: Adding to dbConnections map";
        try {
            if (dbConnections.contains(connectionName)) {
                dbConnections.remove(connectionName);
            }
            dbConnections[connectionName] = connectionName;
        } catch (const std::exception& e) {
            qDebug() << "Exception in Step 8:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception in Step 8";
        }

        qDebug() << "Step 9: About to call setupImageTable()";
        try {
            setupImageTable();
            qDebug() << "Step 10: Returned from setupImageTable()";
        } catch (const std::exception& e) {
            qDebug() << "Exception in setupImageTable:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception in setupImageTable";
            return false;
        }

        qDebug() << "Step 11: About to call updateDatabaseConnectionsCombo()";
        try {
            updateDatabaseConnectionsCombo();
            qDebug() << "Step 12: Returned from updateDatabaseConnectionsCombo()";
        } catch (const std::exception& e) {
            qDebug() << "Exception in updateDatabaseConnectionsCombo:" << e.what();
            return false;
        } catch (...) {
            qDebug() << "Unknown exception in updateDatabaseConnectionsCombo";
            return false;
        }

        qDebug() << "Step 13: Updating UI elements - START";

        // CRASH POINT 1 - Check each action before using it
        qDebug() << "  Checking disconnectDBAction:" << disconnectDBAction;
        try {
            if (disconnectDBAction) {
                qDebug() << "  Setting disconnectDBAction enabled";
                disconnectDBAction->setEnabled(true);
            } else {
                qDebug() << "  WARNING: disconnectDBAction is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting disconnectDBAction:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting disconnectDBAction";
        }

        qDebug() << "  Checking storeImageAction:" << storeImageAction;
        try {
            if (storeImageAction) {
                qDebug() << "  Setting storeImageAction enabled";
                storeImageAction->setEnabled(true);
            } else {
                qDebug() << "  WARNING: storeImageAction is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting storeImageAction:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting storeImageAction";
        }

        qDebug() << "  Checking loadFromDBAction:" << loadFromDBAction;
        try {
            if (loadFromDBAction) {
                qDebug() << "  Setting loadFromDBAction enabled";
                loadFromDBAction->setEnabled(true);
            } else {
                qDebug() << "  WARNING: loadFromDBAction is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting loadFromDBAction:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting loadFromDBAction";
        }

        qDebug() << "  Checking dbManagerAction:" << dbManagerAction;
        try {
            if (dbManagerAction) {
                qDebug() << "  Setting dbManagerAction enabled";
                dbManagerAction->setEnabled(true);
            } else {
                qDebug() << "  WARNING: dbManagerAction is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting dbManagerAction:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting dbManagerAction";
        }

        qDebug() << "  Checking dbRefreshBtn:" << dbRefreshBtn;
        try {
            if (dbRefreshBtn) {
                qDebug() << "  Setting dbRefreshBtn enabled";
                dbRefreshBtn->setEnabled(true);
            } else {
                qDebug() << "  WARNING: dbRefreshBtn is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting dbRefreshBtn:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting dbRefreshBtn";
        }

        qDebug() << "Step 14: Updating message label";
        try {
            if (messageLabel) {
                qDebug() << "  Setting messageLabel text";
                messageLabel->setText("Connected to database: " + dbName);
            } else {
                qDebug() << "  WARNING: messageLabel is NULL!";
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception setting messageLabel:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception setting messageLabel";
        }

        qDebug() << "Step 15: Saving settings";
        try {
            if (appSettings) {
                appSettings->setValue("lastDBHost", host);
                appSettings->setValue("lastDBPort", port);
                appSettings->setValue("lastDBName", dbName);
                appSettings->setValue("lastDBUser", user);
                appSettings->sync();
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception saving settings:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception saving settings";
        }

        qDebug() << "Step 16: Emitting signal";
        try {
            emit databaseConnected(dbName);
            qDebug() << "  Signal emitted successfully";
        } catch (const std::exception& e) {
            qDebug() << "Exception emitting signal:" << e.what();
        } catch (...) {
            qDebug() << "Unknown exception emitting signal";
        }

        qDebug() << "=== connectToPostgreSQL END ===";
        return true;

    } catch (const std::exception& e) {
        qDebug() << "!!! TOP-LEVEL EXCEPTION in connectToPostgreSQL:" << e.what();
        return false;
    } catch (...) {
        qDebug() << "!!! TOP-LEVEL UNKNOWN EXCEPTION in connectToPostgreSQL";
        return false;
    }
}

bool MainWindow::testDatabaseConnection(const QString &host, int port,
                                        const QString &dbName,
                                        const QString &user,
                                        const QString &password)
{
    QString connectionName = "test_" + QString::number(QDateTime::currentMSecsSinceEpoch());

    bool ok = false;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL", connectionName);
        db.setHostName(host);
        db.setPort(port);
        db.setDatabaseName(dbName);
        db.setUserName(user);
        db.setPassword(password);  // MAKE SURE PASSWORD IS SET
        db.setConnectOptions("connect_timeout=5");

        ok = db.open();

        if (ok) {
            qDebug() << "Test connection successful";

            // Verify with a simple query
            QSqlQuery testQuery(db);
            if (!testQuery.exec("SELECT 1")) {
                qDebug() << "Test query failed:" << testQuery.lastError().text();
                ok = false;
            }

            db.close();
        } else {
            qDebug() << "Test connection failed:" << db.lastError().text();
        }
    }

    if (QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::removeDatabase(connectionName);
    }

    return ok;
}

void MainWindow::closeDatabaseConnection()
{
    qDebug() << "Closing database connection...";

    QSqlDatabase db = getImageDb();
    if (db.isValid() && db.isOpen()) {
        QString connectionName = db.connectionName();
        qDebug() << "Closing main connection:" << connectionName;

        db.close();

        if (!currentDbConnection.isEmpty()) {
            dbConnections.remove(currentDbConnection);
            currentDbConnection.clear();
        }

        if (QSqlDatabase::contains(connectionName)) {
            QSqlDatabase::removeDatabase(connectionName);
        }

        imageDbConnectionName.clear();
    }

    // Disable database actions
    if (disconnectDBAction) disconnectDBAction->setEnabled(false);
    if (storeImageAction) storeImageAction->setEnabled(false);
    if (loadFromDBAction) loadFromDBAction->setEnabled(false);
    if (dbManagerAction) dbManagerAction->setEnabled(false);
    if (dbRefreshBtn) dbRefreshBtn->setEnabled(false);

    updateDatabaseConnectionsCombo();

    if (messageLabel) {
        messageLabel->setText("Disconnected from database");
    }

    QMessageBox::information(this, "Disconnected",
                             "Database connection closed successfully");

    emit databaseDisconnected();
}
