#include "citysearch.h"
#include <QMessageBox>
#include <QDebug>
#include <QAbstractItemView>

namespace {
int cityDataTypeId = qRegisterMetaType<CityData>("CityData");
}

CitySearchWidget::CitySearchWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void CitySearchWidget::setupUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Search input - toolbar friendly style
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Type to locate (Ctrl+K)");
    searchEdit->setMinimumWidth(250);
    searchEdit->setMaximumWidth(400);
    searchEdit->setFixedHeight(30);
    searchEdit->setStyleSheet(
                "QLineEdit { "
                "padding: 5px 10px; "
                "border: 1px solid #bbb; "
                "border-radius: 3px; "
                "font-size: 13px; "
                "background-color: white; "
                "}"
                "QLineEdit:focus { "
                "border: 2px solid #4285f4; "
                "}"
                "QLineEdit:hover { "
                "border: 1px solid #888; "
                "}"
                );

    // Locate button - toolbar friendly
    locateBtn = new QPushButton();
    locateBtn->setIcon(QIcon(":/icons/geo_jump.png"));
    locateBtn->setToolTip("Jump to location (Enter)");
    locateBtn->setFixedSize(30, 30);
    locateBtn->setStyleSheet(
                "QPushButton { "
                "border: 1px solid #4285f4; "
                "border-radius: 3px; "
                "background-color: #4285f4; "
                "padding: 5px; "
                "}"
                "QPushButton:hover { "
                "background-color: #357ae8; "
                "}"
                "QPushButton:pressed { "
                "background-color: #2a5db0; "
                "}"
                );

    layout->addWidget(searchEdit);
    layout->addWidget(locateBtn);

    // Setup completer for autocomplete
    completerModel = new QStringListModel(this);
    completer = new QCompleter(completerModel, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setFilterMode(Qt::MatchContains);
    completer->setMaxVisibleItems(15);
    completer->setCompletionMode(QCompleter::PopupCompletion);

    // Style the completer popup
    completer->popup()->setStyleSheet(
                "QListView { "
                "border: 1px solid #ddd; "
                "background-color: white; "
                "selection-background-color: #4285f4; "
                "selection-color: white; "
                "font-size: 12px; "
                "padding: 2px; "
                "}"
                "QListView::item { "
                "padding: 5px; "
                "border-bottom: 1px solid #f0f0f0; "
                "}"
                "QListView::item:hover { "
                "background-color: #e8f0fe; "
                "}"
                );

    searchEdit->setCompleter(completer);

    // Connect signals
    connect(searchEdit, &QLineEdit::returnPressed, this, &CitySearchWidget::onSearchActivated);
    connect(locateBtn, &QPushButton::clicked, this, &CitySearchWidget::onLocateClicked);
    connect(completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CitySearchWidget::onSearchActivated);
}


bool CitySearchWidget::loadCitiesFromCSV(const QString &filePath)
{
    qDebug() << "Loading cities from:" << filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "ERROR: Cannot open file:" << filePath;
        return false;
    }

    cities.clear();
    cityMap.clear();
    QStringList cityNames;

    QTextStream in(&file);

    // Skip header
    if (!in.atEnd()) {
        QString header = in.readLine();
        qDebug() << "CSV Header:" << header;
    }

    int lineNumber = 1;
    int successCount = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;

        if (line.isEmpty()) continue;

        // Parse CSV
        QStringList fields;
        bool inQuotes = false;
        QString currentField;

        for (int i = 0; i < line.length(); ++i) {
            QChar c = line[i];
            if (c == '"') {
                inQuotes = !inQuotes;
            } else if (c == ',' && !inQuotes) {
                fields.append(currentField.trimmed());
                currentField.clear();
            } else {
                currentField.append(c);
            }
        }
        fields.append(currentField.trimmed());

        if (fields.size() >= 4) {
            CityData city;
            city.name = fields[0].remove('"').trimmed();
            city.country = fields[1].remove('"').trimmed();

            bool latOk, lonOk;
            city.latitude = fields[2].toDouble(&latOk);
            city.longitude = fields[3].toDouble(&lonOk);

            if (latOk && lonOk && !city.name.isEmpty()) {
                cities.append(city);
                QString searchKey = city.getSearchString();
                cityMap[searchKey] = city;
                cityNames.append(searchKey);
                successCount++;
            }
        }

        // Progress for large files
        if (lineNumber % 10000 == 0) {
            qDebug() << "Loaded" << successCount << "cities...";
        }
    }

    file.close();

    qDebug() << "Loaded" << successCount << "cities successfully";

    // Update completer
    completerModel->setStringList(cityNames);

    return successCount > 0;
}

CityData CitySearchWidget::findCity(const QString &searchText)
{
    QString search = searchText.trimmed();

    // Exact match
    if (cityMap.contains(search)) {
        return cityMap[search];
    }

    // Case-insensitive search
    for (auto it = cityMap.constBegin(); it != cityMap.constEnd(); ++it) {
        if (it.key().compare(search, Qt::CaseInsensitive) == 0) {
            return it.value();
        }
    }

    // Partial match on city name
    for (const CityData &city : cities) {
        if (city.name.compare(search, Qt::CaseInsensitive) == 0) {
            return city;
        }
    }

    // Return empty city if not found
    return CityData();
}

void CitySearchWidget::onSearchActivated()
{
    QString searchText = searchEdit->text().trimmed();

    if (searchText.isEmpty()) {
        return;
    }

    qDebug() << "Searching for:" << searchText;

    CityData city = findCity(searchText);

    if (!city.name.isEmpty()) {
        qDebug() << "Found:" << city.name << city.country;
        emit citySelected(city);
    } else {
        qDebug() << "City not found:" << searchText;
        searchEdit->setStyleSheet(
                    searchEdit->styleSheet() +
                    "QLineEdit { border: 2px solid #f44336; }"
                    );

        // Reset style after 1 second
        QTimer::singleShot(1000, this, [this]() {
            searchEdit->setStyleSheet(
                        "QLineEdit { "
                        "padding: 10px 15px; "
                        "border: 1px solid #ddd; "
                        "border-radius: 20px; "
                        "font-size: 14px; "
                        "background-color: white; "
                        "}"
                        "QLineEdit:focus { "
                        "border: 2px solid #4285f4; "
                        "}"
                        );
        });
    }
}

void CitySearchWidget::onLocateClicked()
{
    onSearchActivated();
}
