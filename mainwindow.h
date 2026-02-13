#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QFrame>
#include <QGroupBox>
#include <QWidget>
#include <QScrollArea>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QAction>
#include <QSlider>
#include <QDockWidget>
#include <QStackedWidget>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTabWidget>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTextEdit>
#include <QComboBox>
#include <QToolButton>
#include <QStyleFactory>
#include <QApplication>
#include <QProgressBar>
#include <QSpinBox>
#include <QInputDialog>
#include <QImageReader>
#include <QSettings>
#include <QStandardPaths>
#include <QDateTime>
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFormLayout>
#include <QPluginLoader>
#include <QDialogButtonBox>
#include <QVariantMap>
#include <QCloseEvent>
#include <QtSql>
#include <QUuid>
#include <QBuffer>
#include <QSqlQuery>

#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "citysearch.h"

// Forward declaration
class QGraphicsSvgItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void onToggleCoordinateMode(bool isDegrees);
    void onCopyExtentsToClipboard();
    void onCopyCoordinatesToClipboard();
    void setupStatusBarShortcuts();
    void onToggleCoordExtentDisplay(bool showCoordinates, QStackedWidget* stackWidget);
    void flashMarker();
    void highlightAreaAroundPoint(double centerX, double centerY, double radius);
    void addSelectionRectangle(double x, double y, double width, double height);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    // Layer management structure
    struct LayerInfo {
        QString name;
        QString filePath;
        QString type; // "raster", "vector", "image", "geotiff", "shapefile"
        QTreeWidgetItem* treeItem;
        QGraphicsItem* graphicsItem;
        QVariantMap properties;
        QList<QGraphicsItem*> vectorItems; // For vector layers with multiple items

        LayerInfo() : treeItem(nullptr), graphicsItem(nullptr) {}
    };

    struct GeoreferenceInfo {
        QGraphicsPixmapItem *imageItem = nullptr;
        QString filePath;
        bool hasTransform = false;
        double geoTransform[6];
        QString projection;
        QSize imageSize;
    };

    QList<GeoreferenceInfo> georeferencedImagesInfo;
    QList<QGraphicsPixmapItem*> georeferencedImages;

    // GDAL-related members
    GDALDataset *gdalDataset = nullptr;
    double gdalGeoTransform[6];
    bool hasGeoTransform = false;
    bool isGeoTIFFLoaded = false;
    QGraphicsPixmapItem *geoTIFFItem = nullptr;
    QImage geoTIFFImage;
    QSize geoTIFFSize;
    QList<QGraphicsItem*> currentCrosshairItems;
    QVector<QGraphicsItem*> currentVectorItems;
    QMap<QString, QVector<QGraphicsItem*>> layerVectorItems;
    QGraphicsEllipseItem *coordinateMarker = nullptr;
    QGraphicsTextItem *coordinateTextItem = nullptr;
    QList<QGraphicsItem*> coordinateMarkerItems;

    // Add these methods for mouse tracking
    void trackVectorItemHover(QGraphicsItem *item, const QPointF &scenePos);
    QPointF getVectorItemCoordinates(QGraphicsItem *item, const QPointF &scenePos);
    void highlightVectorItem(QGraphicsItem *item, bool highlight);

    void setupUI();
    void setupMenuBar();
    void setupToolBars();
    void setupDockWidgets();
    void setupCentralWidget();
    void setupStatusBar();
    void setupConnections();

    // Project management
    void createNewProjectDialog();
    void saveProject();
    void loadProject(const QString &projectPath);
    void addRecentProject(const QString &projectPath);
    void updateRecentProjectsMenu();

    // Extents and coordinates display
    QLabel *extentsLabel;
    bool displayInDegrees;
    QMap<QString, QRectF> layerExtents;

    QToolButton *coordinateModeBtn;
    QToolButton *coordinatesToolBtn;
    QToolButton *coordExtentToggleBtn;

    // Helper methods for new functionality
    void updateExtentsDisplay();
    void updateMiniExtentsDisplay(QLabel* miniExtentsLabel);
    QString formatCoordinate(double value, bool isDegrees);
    void jumpToLocation(double x, double y);
    void showCoordinatePicker();
    void zoomToExtents();
    QString getCurrentExtentsString();
    void addCoordinateMarker(const QPointF &scenePos, double lon, double lat);
    void removeCoordinateMarker();

    // File management
    void setupFileAssociations();
    QString getSupportedFilesFilter();
    QString getVectorFilesFilter();
    QString getRasterFilesFilter();
    QString getImageFilesFilter();
    QString getGeoTIFFFilesFilter();
    QString getSaveLocation();
    void updateSaveLocation(const QString &path);

    // Layer management
    QList<LayerInfo> loadedLayers;

    CitySearchWidget *citySearchWidget;
    QString citiesCSVPath;

    void setupCitySearch();

    // Add to private methods
    void loadCitiesDatabase();
    void showCitySearch();

    // File operations
    void loadFile(const QString &filePath);
    void loadVectorFile(const QString &filePath);
    void loadRasterFile(const QString &filePath);
    void loadImageFile(const QString &filePath);
    bool saveLayerToFile(const LayerInfo &layer, const QString &savePath);
    void exportProject(const QString &directory);
    void importProject(const QString &directory);
    void saveAllLayers();

    // Layer operations
    void addLayerToScene(const LayerInfo &layer);
    void removeLayer(const QString &layerName);
    void updateLayerVisibility(const QString &layerName, bool visible);
    LayerInfo* getLayerByName(const QString &name);

    // Vector operations
    void drawVectorLayer(const QString &filePath);
    void drawGeometry(OGRGeometry *geom, const QColor &color, double scaleFactor = 100.0);
    void drawPoint(OGRPoint *point, const QColor &color, double scaleFactor = 100.0);
    void drawLine(OGRLineString *line, const QColor &color, double scaleFactor = 100.0);
    void drawPolygon(OGRPolygon *polygon, const QColor &color, double scaleFactor = 100.0);
    void addVectorLayerToTree(const QString &layerName, const QString &filePath, OGRwkbGeometryType geomType);
    void clearVectorItems(const QString &layerName = QString());

    // Image handling
    void clearCurrentImage();
    void fitImageToView();
    void updateImageInfo();
    void fitAllImages();
    void fitAllGeoreferencedImages();
    void clearAllImages();
    void updatePropertiesDisplay(const LayerInfo &layer);

    // Settings management
    void saveSettings();
    void loadSettings();

    // QGIS-like UI components
    QMenuBar *menuBar;

    // Toolbars (like QGIS)
    QToolBar *fileToolBar;
    QToolBar *mapNavToolBar;
    QToolBar *attributesToolBar;
    QToolBar *labelToolBar;
    QToolBar *databaseToolBar;
    QToolBar *imageToolBar;

    // Dock widgets (like QGIS panels)
    QDockWidget *layersDock;
    QDockWidget *browserDock;
    QDockWidget *processingToolboxDock;
    QDockWidget *layerStylingDock;
    QDockWidget *imagePropertiesDock;

    // Central widget components
    QTabWidget *mapViewsTabWidget;
    QGraphicsView *mapView;
    QGraphicsScene *mapScene;
    QGraphicsPixmapItem *currentImageItem;

    // Layer tree (like QGIS Layers panel)
    QTreeWidget *layersTree;

    // Browser tree (like QGIS Browser panel)
    QTreeWidget *browserTree;

    // Status bar components (QGIS-like)
    QLineEdit *searchLineEdit;
    QLabel *messageLabel;
    QLabel *coordinateLabel;
    QComboBox *scaleCombo;
    QLabel *magnifierLabel;
    QLabel *rotationLabel;
    QLabel *projectionLabel;
    QLabel *imageInfoLabel;

    QStringList recentCRS;
    void updateRecentCRS(const QString &crs);
    void loadRecentCRS();
    void saveRecentCRS();

    // Database members
    QString imageDbConnectionName;
    QSqlDatabase getImageDb();
    QString currentDbConnection;
    QMap<QString, QString> dbConnections;
    bool m_connectionDialogShown;


    // Database UI components
    QComboBox *dbConnectionCombo;
    QToolButton *dbRefreshBtn;

    // Database actions
    QAction *connectDBAction;
    QAction *disconnectDBAction;
    QAction *storeImageAction;
    QAction *loadFromDBAction;
    QAction *dbManagerAction;

    // Database methods
    bool connectToPostgreSQL(const QString &host, int port,
                            const QString &dbName,
                            const QString &user,
                            const QString &password);
    void closeDatabaseConnection();
    void setupImageTable();
    bool testDatabaseConnection(const QString &host, int port,
                               const QString &dbName,
                               const QString &user,
                               const QString &password);

    bool storeImageInDatabase(const QString &filePath,
                             const QString &layerName = QString(),
                             const QString &description = QString());
    bool storePixmapInDatabase(const QPixmap &pixmap,
                              const QString &layerName,
                              const QString &description = QString());

    bool loadImageFromDatabase(int imageId);
    bool loadImageFromDatabase(const QString &uuid);
    void deleteImageFromDatabase(int imageId);
    QVector<QPair<int, QString>> getImageListFromDatabase();

    void showDatabaseManager();
    void createDatabaseConnectionDialog();
    void updateDatabaseConnectionsCombo();
    QString extractEPSGFromWKT(const QString &wkt);

    // File management UI
    QLineEdit *saveLocationEdit;
    QPushButton *browseSaveLocationBtn;
    QComboBox *exportFormatCombo;
    QLabel *projectInfoLabel;

    // Recent projects
    QMenu *recentProjectsMenu;
    QStringList recentProjects;

    // Current project state
    QString currentProjectName;
    QString currentProjectPath;
    QString currentImagePath;
    QPixmap currentPixmap;
    bool projectModified;

    // Image zoom/pan state
    qreal currentScale;
    qreal rotationAngle;

    // Settings
    QSettings *appSettings;
    QString defaultSaveLocation;
    QString lastUsedDirectory;

    // Actions
    QAction *newProjectAction;
    QAction *openProjectAction;
    QAction *saveProjectAction;
    QAction *saveAsProjectAction;
    QAction *printLayoutAction;
    QAction *exitAction;

    QAction *newMapViewAction;
    QAction *panAction;
    QAction *zoomInAction;
    QAction *zoomOutAction;
    QAction *identifyAction;
    QAction *measureAction;
    QAction *bookmarkAction;

    QAction *toggleEditingAction;
    QAction *saveLayerEditsAction;
    QAction *openAttributeTableAction;

    QAction *addVectorLayerAction;
    QAction *addRasterLayerAction;
    QAction *addImageLayerAction;
    QAction *addDatabaseLayerAction;
    QAction *addWfsLayerAction;
    QAction *addWmsLayerAction;
    QAction *openGeoTIFFAction;

    QAction *processingAction;
    QAction *pythonConsoleAction;
    QAction *pluginManagerAction;

    QAction *layerPropertiesAction;
    QAction *layerStylingAction;
    QAction *labelAction;

    // Image actions
    QAction *loadImageAction;
    QAction *clearImageAction;
    QAction *fitImageAction;
    QAction *rotateLeftAction;
    QAction *rotateRightAction;
    QAction *zoomImageInAction;
    QAction *zoomImageOutAction;
    QAction *resetZoomAction;

    // New file management actions
    QAction *exportProjectAction;
    QAction *importProjectAction;
    QAction *saveLayerAction;
    QAction *saveLayerAsAction;
    QAction *exportToPdfAction;
    QAction *exportToImageAction;
    QAction *saveAllLayersAction;

    // CRS and projection methods
    void setupCRSSelection();
    void openCRSDialog();
    void populateCRSTree(QTreeWidget *crsTree);
    QString getCRSDisplayName(const QString &crsCode);

    // Helper methods for status bar
    void updateCoordinates(const QPointF &coord);
    void updateScale(double scale);
    void updateMagnifier(int percentage);
    void updateRotation(qreal angle);
    void updateProjection(const QString &crs);

    void updateExtentsDisplayLabel(QLabel *extentsDisplayLabel);
    QPointF sceneToGeographicCoords(const QPointF &scenePoint);
    bool hasAnyGeoreferencedLayer();
    QPointF geographicToSceneCoords(double lon, double lat);

    // Projection and export methods
    void setupProjectionSystem();
    void onExportMap();
    void onOpenAttributeTable();
    QIcon createCRSIcon();
    void showProjectionContextMenu(const QPoint &globalPos);
    void animateCRSChange();
    void createTable();
private slots:
    void onLoadVectorFile(const QString &filePath);
    void onCreateNewProject();
    void onOpenProject();
    void onSaveProject();
    void onSaveAsProject();
    void onOpenRecentProject();
    void onAddVectorLayer();
    void onAddRasterLayer();
    void onAddImageLayer();
    void onToggleEditing();
    void onPanMap();
    void onZoomIn();
    void onZoomOut();
    void onShowProcessingToolbox();
    void onShowPythonConsole();
    void onShowLayerProperties();
    void onCreatePrintLayout();
    void onShowBookmarks();
    void addMarkerActions();
    void onScaleChanged(const QString &text);
    void onCoordinatesToolToggled(bool enabled);
    void onProjectionLabelClicked();
    void showCRSSelectionDialog();
    void onCRSChanged(const QString &crs);

    // Image slots
    void onLoadImage();
    void onClearImage();
    void onFitImage();
    void onRotateLeft();
    void onRotateRight();
    void onZoomImageIn();
    void onZoomImageOut();
    void onResetZoom();

    // Database slots
    void onConnectDatabase();
    void onDisconnectDatabase();
    void onStoreCurrentImage();
    void onLoadFromDatabase();
    void onShowDatabaseManager();

    // File management slots
    void onBrowseSaveLocation();
    void onChangeSaveLocation();
    void onExportProject();
    void onImportProject();
    void onSaveLayer();
    void onSaveLayerAs();
    void onExportToPdf();
    void onExportToImage();
    void onSaveAllLayers();

    // Layer management slots
    void onLayerItemClicked(QTreeWidgetItem *item, int column);
    void onLayerItemDoubleClicked(QTreeWidgetItem *item, int column);
    void onLayerContextMenuRequested(const QPoint &pos);
    void onRemoveLayer();

    void onBrowserItemClicked(QTreeWidgetItem *item, int column);
    void onSearchTextChanged(const QString &text);

    // GDAL slots
    void onOpenGeoTIFF();

    void onSearchCity();

signals:
    void projectLoaded(const QString &projectPath);
    void layerAdded(const QString &layerName);
    void layerLoaded(const QString &layerName, const QString &layerType);
    void layerSaved(const QString &layerName, const QString &savePath);
    void projectExported(const QString &directory);

    // Database signals
    void databaseConnected(const QString &dbName);
    void databaseDisconnected();
    void imageStored(int imageId, const QString &layerName);
    void imageLoaded(int imageId, const QString &layerName);
};

#endif // MAINWINDOW_H
