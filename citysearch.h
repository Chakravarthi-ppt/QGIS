#ifndef CITYSEARCH_H
#define CITYSEARCH_H

#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QTimer>

struct CityData {
    QString name;
    QString country;
    double latitude;
    double longitude;

    QString getSearchString() const {
        return QString("%1, %2").arg(name).arg(country);
    }
};

Q_DECLARE_METATYPE(CityData)

class CitySearchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CitySearchWidget(QWidget *parent = nullptr);

    bool loadCitiesFromCSV(const QString &filePath);
    int getCityCount() const { return cities.size(); }

signals:
    void citySelected(const CityData &city);

private slots:
    void onSearchActivated();
    void onLocateClicked();

private:
    void setupUI();
    CityData findCity(const QString &searchText);

    QLineEdit *searchEdit;
    QPushButton *locateBtn;
    QCompleter *completer;
    QStringListModel *completerModel;

    QVector<CityData> cities;
    QMap<QString, CityData> cityMap; // For quick lookup
};

#endif // CITYSEARCH_H
