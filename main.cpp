#include "mainwindow.h"
#include <QApplication>
#include <QSplashScreen>
#include <QElapsedTimer>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QScreen>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>

int main(int argc, char *argv[])
{

    if (QSqlDatabase::contains(QSqlDatabase::defaultConnection)) {
           QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
       }

    QApplication a(argc, argv);

    // Create a custom splash screen pixmap
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int width = screenGeometry.width() / 3;
    int height = screenGeometry.height() / 3;

    // Create splash pixmap
    QPixmap splashPixmap(width, height);
    splashPixmap.fill(QColor(30, 40, 60)); // Dark blue background

    // Draw on the pixmap
    QPainter painter(&splashPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw gradient background
    QLinearGradient gradient(0, 0, 0, height);
    gradient.setColorAt(0, QColor(30, 40, 60));
    gradient.setColorAt(1, QColor(10, 20, 40));
    painter.fillRect(QRect(0, 0, width, height), gradient);

    // Draw PPTGIS text
    QFont font("Arial", 48, QFont::Bold);
    painter.setFont(font);

    // Draw shadow
    painter.setPen(QColor(0, 0, 0, 150));
    painter.drawText(QRect(2, 2, width, height), Qt::AlignCenter, "PPTGIS");

    // Draw main text
    painter.setPen(QColor(255, 255, 255));
    painter.drawText(QRect(0, 0, width, height), Qt::AlignCenter, "PPTGIS");

    // Draw version
    QFont versionFont("Arial", 16, QFont::Normal);
    painter.setFont(versionFont);
    painter.setPen(QColor(180, 180, 180));
    painter.drawText(QRect(0, 0, width, height - 40),
                    Qt::AlignBottom | Qt::AlignHCenter, "Version 1.0.0");

    // Create and show splash screen
    QSplashScreen splash(splashPixmap, Qt::WindowStaysOnTopHint);
    splash.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    // Center splash screen
    QPoint center = screenGeometry.center() - splash.rect().center();
    splash.move(center);

    splash.show();

    // Process events to make sure splash is painted
    a.processEvents();

    // Create main window
    MainWindow w;

    // Create a timer to delay showing main window
    QTimer::singleShot(3000, [&splash, &w]() {
        splash.close();
        w.show();
    });

    // Optional: Show splash screen while loading resources
    QTimer loadingTimer;
    loadingTimer.setSingleShot(true);

    // Simulate loading time (3 seconds minimum)
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < 3000) {
        a.processEvents();
    }

    return a.exec();
}
