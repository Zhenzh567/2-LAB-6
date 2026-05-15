#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHeaderView>
#include <algorithm>

class Marker {
public:
    QString name;
    QString description;
    QString color;
    int inkLevel;

    Marker() : inkLevel(0) {}
    Marker(const QString& n, const QString& d, const QString& c, int i)
        : name(n), description(d), color(c), inkLevel(i) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["description"] = description;
        obj["color"] = color;
        obj["inkLevel"] = inkLevel;
        return obj;
    }

    static Marker fromJson(const QJsonObject& json) {
        Marker m;
        m.name = json["name"].toString();
        m.description = json["description"].toString();
        m.color = json["color"].toString();
        m.inkLevel = json["inkLevel"].toInt(-1);
        return m;
    }

    bool isValid() const {
        if (name.isEmpty() || description.isEmpty() || color.isEmpty()) return false;
        if (inkLevel < 0 || inkLevel > 100) return false;
        for (const QChar& c : color)
            if (!c.isLetter() && c != ' ' && c != '-') return false;
        return true;
    }

    QString getErrors() const {
        QString errors;
        if (name.isEmpty()) errors += "пустое название; ";
        if (description.isEmpty()) errors += "пустое описание; ";
        if (color.isEmpty()) errors += "пустой цвет; ";
        else {
            bool valid = true;
            for (const QChar& c : color)
                if (!c.isLetter() && c != ' ' && c != '-') valid = false;
            if (!valid) errors += "некорректный цвет; ";
        }
        if (inkLevel < 0 || inkLevel > 100) errors += "неверный уровень; ";
        return errors.isEmpty() ? "OK" : errors;
    }

    static QVector<Marker> loadAll(const QString& filename) {
        QVector<Marker> markers;
        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) return markers;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isArray())
            for (const auto& val : doc.array())
                markers.append(fromJson(val.toObject()));
        return markers;
    }

    static void saveAll(const QString& filename, const QVector<Marker>& markers) {
        QJsonArray arr;
        for (const auto& m : markers)
            arr.append(m.toJson());
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
            file.close();
        }
    }
};

class MainWindow : public QWidget {
public:
    MainWindow() {
        setWindowTitle("Программа 2 - Обработка маркеров");
        resize(800, 600);

        auto* layout = new QVBoxLayout(this);

        auto* fileLayout = new QHBoxLayout();
        fileLayout->addWidget(new QLabel("JSON файл:"));
        fileEdit = new QLineEdit("markers.json");
        fileLayout->addWidget(fileEdit);
        auto* browseBtn = new QPushButton("Обзор");
        fileLayout->addWidget(browseBtn);
        auto* loadBtn = new QPushButton("Загрузить и обработать");
        fileLayout->addWidget(loadBtn);
        layout->addLayout(fileLayout);

        layout->addWidget(new QLabel("ТАБЛИЦА 1 (корректные, сортировка Я→А):"));
        table1 = new QTableWidget();
        table1->setColumnCount(5);
        table1->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Статус"});
        table1->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table1);

        layout->addWidget(new QLabel("ТАБЛИЦА 2 (битые):"));
        table2 = new QTableWidget();
        table2->setColumnCount(5);
        table2->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Ошибки"});
        table2->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table2);

        statsLabel = new QLabel();
        layout->addWidget(statsLabel);

        connect(browseBtn, &QPushButton::clicked, [this]() {
            QString filename = QFileDialog::getOpenFileName(this, "Выберите JSON", "", "JSON (*.json)");
            if (!filename.isEmpty()) fileEdit->setText(filename);
        });

        connect(loadBtn, &QPushButton::clicked, this, &MainWindow::processFile);
    }

private:
    QLineEdit* fileEdit;
    QTableWidget* table1;
    QTableWidget* table2;
    QLabel* statsLabel;

    void processFile() {
        auto all = Marker::loadAll(fileEdit->text());

        QVector<Marker> valid, broken;
        for (const auto& m : all)
            (m.isValid() ? valid : broken).append(m);

        std::sort(valid.begin(), valid.end(), [](const Marker& a, const Marker& b) {
            return a.name > b.name;
        });

        if (!broken.isEmpty())
            Marker::saveAll("broken.json", broken);

        table1->setRowCount(valid.size());
        for (int i = 0; i < valid.size(); ++i) {
            table1->setItem(i, 0, new QTableWidgetItem(valid[i].name));
            table1->setItem(i, 1, new QTableWidgetItem(valid[i].description));
            table1->setItem(i, 2, new QTableWidgetItem(valid[i].color));
            table1->setItem(i, 3, new QTableWidgetItem(QString::number(valid[i].inkLevel)));
            table1->setItem(i, 4, new QTableWidgetItem(valid[i].getErrors()));
        }

        table2->setRowCount(broken.size());
        for (int i = 0; i < broken.size(); ++i) {
            table2->setItem(i, 0, new QTableWidgetItem(broken[i].name));
            table2->setItem(i, 1, new QTableWidgetItem(broken[i].description));
            table2->setItem(i, 2, new QTableWidgetItem(broken[i].color));
            table2->setItem(i, 3, new QTableWidgetItem(QString::number(broken[i].inkLevel)));
            table2->setItem(i, 4, new QTableWidgetItem(broken[i].getErrors()));
        }

        statsLabel->setText(QString("Всего: %1 | Корректных: %2 | Битых: %3")
                           .arg(all.size()).arg(valid.size()).arg(broken.size()));
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}


#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHeaderView>
#include <QRegularExpression>
#include <algorithm>

class Marker {
public:
    QString name;
    QString description;
    QString color;
    int inkLevel;

    Marker() : inkLevel(0) {}
    Marker(const QString& n, const QString& d, const QString& c, int i)
        : name(n), description(d), color(c), inkLevel(i) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["description"] = description;
        obj["color"] = color;
        obj["inkLevel"] = inkLevel;
        return obj;
    }

    static Marker fromJson(const QJsonObject& json) {
        Marker m;
        m.name = json["name"].toString();
        m.description = json["description"].toString();
        m.color = json["color"].toString();
        m.inkLevel = json["inkLevel"].toInt(-1);
        return m;
    }

    bool isValid() const {
        if (name.isEmpty() || description.isEmpty() || color.isEmpty()) return false;
        if (inkLevel < 0 || inkLevel > 100) return false;
        for (const QChar& c : color)
            if (!c.isLetter() && c != ' ' && c != '-') return false;
        return true;
    }

    QString getErrors() const {
        QString errors;
        if (name.isEmpty()) errors += "пустое название; ";
        if (description.isEmpty()) errors += "пустое описание; ";
        if (color.isEmpty()) errors += "пустой цвет; ";
        else {
            bool valid = true;
            for (const QChar& c : color)
                if (!c.isLetter() && c != ' ' && c != '-') valid = false;
            if (!valid) errors += "некорректный цвет; ";
        }
        if (inkLevel < 0 || inkLevel > 100) errors += "неверный уровень; ";
        return errors.isEmpty() ? "OK" : errors;
    }

    static QVector<Marker> loadAll(const QString& filename) {
        QVector<Marker> markers;
        QFile file(filename);
        if (!file.exists()) {
            file.open(QIODevice::WriteOnly);
            file.close();
            return markers;
        }
        if (!file.open(QIODevice::ReadOnly)) return markers;
        QByteArray data = file.readAll();
        file.close();
        if (data.trimmed().isEmpty()) return markers;
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray())
            for (const auto& val : doc.array())
                markers.append(fromJson(val.toObject()));
        return markers;
    }

    static void saveAll(const QString& filename, const QVector<Marker>& markers) {
        QJsonArray arr;
        for (const auto& m : markers)
            arr.append(m.toJson());
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
            file.close();
        }
    }
};

class MainWindow : public QWidget {
public:
    MainWindow() {
        setWindowTitle("Программа 2 - Обработка маркеров");
        resize(800, 600);

        QFile file("broken.json");
        if (!file.exists()) {
            file.open(QIODevice::WriteOnly);
            file.close();
        }

        auto* layout = new QVBoxLayout(this);

        auto* fileLayout = new QHBoxLayout();
        fileLayout->addWidget(new QLabel("JSON файл:"));
        fileEdit = new QLineEdit("markers.json");
        fileLayout->addWidget(fileEdit);
        auto* browseBtn = new QPushButton("Обзор");
        fileLayout->addWidget(browseBtn);
        auto* loadBtn = new QPushButton("Загрузить и обработать");
        fileLayout->addWidget(loadBtn);
        layout->addLayout(fileLayout);

        layout->addWidget(new QLabel("ТАБЛИЦА 1 (корректные, сортировка Я→А):"));
        table1 = new QTableWidget();
        table1->setColumnCount(5);
        table1->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Статус"});
        table1->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table1);

        layout->addWidget(new QLabel("ТАБЛИЦА 2 (битые):"));
        table2 = new QTableWidget();
        table2->setColumnCount(5);
        table2->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Ошибки"});
        table2->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table2);

        statsLabel = new QLabel();
        layout->addWidget(statsLabel);

        connect(browseBtn, &QPushButton::clicked, [this]() {
            QString filename = QFileDialog::getOpenFileName(this, "Выберите JSON", "", "JSON (*.json)");
            if (!filename.isEmpty()) fileEdit->setText(filename);
        });
        connect(loadBtn, &QPushButton::clicked, this, &MainWindow::processFile);
    }

private:
    QLineEdit* fileEdit;
    QTableWidget* table1;
    QTableWidget* table2;
    QLabel* statsLabel;

    void processFile() {
        auto all = Marker::loadAll(fileEdit->text());
        QVector<Marker> valid, broken;
        for (const auto& m : all)
            (m.isValid() ? valid : broken).append(m);

        std::sort(valid.begin(), valid.end(), [](const Marker& a, const Marker& b) {
            return a.name > b.name;
        });

        if (!broken.isEmpty())
            Marker::saveAll("broken.json", broken);

        table1->setRowCount(valid.size());
        for (int i = 0; i < valid.size(); ++i) {
            table1->setItem(i, 0, new QTableWidgetItem(valid[i].name));
            table1->setItem(i, 1, new QTableWidgetItem(valid[i].description));
            table1->setItem(i, 2, new QTableWidgetItem(valid[i].color));
            table1->setItem(i, 3, new QTableWidgetItem(QString::number(valid[i].inkLevel)));
            QTableWidgetItem* statusItem = new QTableWidgetItem(valid[i].getErrors());
            statusItem->setBackground(QColor(200, 255, 200));
            table1->setItem(i, 4, statusItem);
        }

        table2->setRowCount(broken.size());
        for (int i = 0; i < broken.size(); ++i) {
            QTableWidgetItem* nameItem = new QTableWidgetItem(broken[i].name);
            QTableWidgetItem* descItem = new QTableWidgetItem(broken[i].description);
            QTableWidgetItem* colorItem = new QTableWidgetItem(broken[i].color);
            QTableWidgetItem* inkItem = new QTableWidgetItem(QString::number(broken[i].inkLevel));

            if (broken[i].name.isEmpty()) nameItem->setBackground(QColor(255, 150, 150));
            if (broken[i].description.isEmpty()) descItem->setBackground(QColor(255, 150, 150));
            if (broken[i].color.isEmpty() || !broken[i].color.contains(QRegularExpression("^[a-zA-Zа-яА-Я -]+$")))
                colorItem->setBackground(QColor(255, 150, 150));
            if (broken[i].inkLevel < 0 || broken[i].inkLevel > 100)
                inkItem->setBackground(QColor(255, 150, 150));

            table2->setItem(i, 0, nameItem);
            table2->setItem(i, 1, descItem);
            table2->setItem(i, 2, colorItem);
            table2->setItem(i, 3, inkItem);
            QTableWidgetItem* errorItem = new QTableWidgetItem(broken[i].getErrors());
            errorItem->setBackground(QColor(255, 200, 200));
            table2->setItem(i, 4, errorItem);
        }

        statsLabel->setText(QString("Всего: %1 | Корректных: %2 | Битых: %3")
                           .arg(all.size()).arg(valid.size()).arg(broken.size()));
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHeaderView>
#include <QRegularExpression>
#include <algorithm>

class Marker {
public:
    QString name;
    QString description;
    QString color;
    int inkLevel;

    Marker() : inkLevel(-1) {}
    Marker(const QString& n, const QString& d, const QString& c, int i)
        : name(n), description(d), color(c), inkLevel(i) {}

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["description"] = description;
        obj["color"] = color;
        obj["inkLevel"] = inkLevel;
        return obj;
    }

    static Marker fromJson(const QJsonObject& json) {
        Marker m;
        m.name = json["name"].toString();
        m.description = json["description"].toString();
        m.color = json["color"].toString();
        m.inkLevel = json["inkLevel"].toInt(-1);
        return m;
    }

    bool isValid() const {
        if (name.isEmpty() || description.isEmpty() || color.isEmpty()) return false;
        if (inkLevel < 0 || inkLevel > 100) return false;
        for (const QChar& c : color)
            if (!c.isLetter() && c != ' ' && c != '-') return false;
        return true;
    }

    QString getErrors() const {
        QString errors;
        if (name.isEmpty()) errors += "пустое название; ";
        if (description.isEmpty()) errors += "пустое описание; ";
        if (color.isEmpty()) errors += "пустой цвет; ";
        else {
            bool valid = true;
            for (const QChar& c : color)
                if (!c.isLetter() && c != ' ' && c != '-') valid = false;
            if (!valid) errors += "некорректный цвет; ";
        }
        if (inkLevel < 0 || inkLevel > 100) errors += "неверный уровень; ";
        return errors.isEmpty() ? "OK" : errors;
    }

    static QVector<Marker> loadAll(const QString& filename) {
        QVector<Marker> markers;
        QFile file(filename);
        if (!file.exists()) {
            file.open(QIODevice::WriteOnly);
            file.close();
            return markers;
        }
        if (!file.open(QIODevice::ReadOnly)) return markers;
        QByteArray data = file.readAll();
        file.close();
        if (data.trimmed().isEmpty()) return markers;
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray())
            for (const auto& val : doc.array())
                markers.append(fromJson(val.toObject()));
        return markers;
    }

    static void saveAll(const QString& filename, const QVector<Marker>& markers) {
        QJsonArray arr;
        for (const auto& m : markers)
            arr.append(m.toJson());
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
            file.close();
        }
    }
};

class MainWindow : public QWidget {
public:
    MainWindow() {
        setWindowTitle("Программа 2 - Обработка маркеров");
        resize(800, 600);

        QFile file("broken.json");
        if (!file.exists()) {
            file.open(QIODevice::WriteOnly);
            file.close();
        }

        auto* layout = new QVBoxLayout(this);

        auto* fileLayout = new QHBoxLayout();
        fileLayout->addWidget(new QLabel("JSON файл:"));
        fileEdit = new QLineEdit("markers.json");
        fileLayout->addWidget(fileEdit);
        auto* browseBtn = new QPushButton("Обзор");
        fileLayout->addWidget(browseBtn);
        auto* loadBtn = new QPushButton("Загрузить и обработать");
        fileLayout->addWidget(loadBtn);
        layout->addLayout(fileLayout);

        layout->addWidget(new QLabel("ТАБЛИЦА 1 (корректные, сортировка Я→А):"));
        table1 = new QTableWidget();
        table1->setColumnCount(5);
        table1->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Статус"});
        table1->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table1);

        layout->addWidget(new QLabel("ТАБЛИЦА 2 (битые):"));
        table2 = new QTableWidget();
        table2->setColumnCount(5);
        table2->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Ошибки"});
        table2->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table2);

        statsLabel = new QLabel();
        layout->addWidget(statsLabel);

        connect(browseBtn, &QPushButton::clicked, [this]() {
            QString filename = QFileDialog::getOpenFileName(this, "Выберите JSON", "", "JSON (*.json)");
            if (!filename.isEmpty()) fileEdit->setText(filename);
        });
        connect(loadBtn, &QPushButton::clicked, this, &MainWindow::processFile);
    }

private:
    QLineEdit* fileEdit;
    QTableWidget* table1;
    QTableWidget* table2;
    QLabel* statsLabel;

    void processFile() {
        auto all = Marker::loadAll(fileEdit->text());
        QVector<Marker> valid, broken;
        for (const auto& m : all)
            (m.isValid() ? valid : broken).append(m);

        std::sort(valid.begin(), valid.end(), [](const Marker& a, const Marker& b) {
            return a.name > b.name;
        });

        if (!broken.isEmpty())
            Marker::saveAll("broken.json", broken);

        table1->setRowCount(valid.size());
        for (int i = 0; i < valid.size(); ++i) {
            table1->setItem(i, 0, new QTableWidgetItem(valid[i].name));
            table1->setItem(i, 1, new QTableWidgetItem(valid[i].description));
            table1->setItem(i, 2, new QTableWidgetItem(valid[i].color));
            table1->setItem(i, 3, new QTableWidgetItem(valid[i].inkLevel >= 0 ? QString::number(valid[i].inkLevel) : ""));
            QTableWidgetItem* statusItem = new QTableWidgetItem(valid[i].getErrors());
            statusItem->setBackground(QColor(200, 255, 200));
            table1->setItem(i, 4, statusItem);
        }

        table2->setRowCount(broken.size());
        for (int i = 0; i < broken.size(); ++i) {
            QTableWidgetItem* nameItem = new QTableWidgetItem(broken[i].name);
            QTableWidgetItem* descItem = new QTableWidgetItem(broken[i].description);
            QTableWidgetItem* colorItem = new QTableWidgetItem(broken[i].color);
            QTableWidgetItem* inkItem = new QTableWidgetItem(broken[i].inkLevel >= 0 ? QString::number(broken[i].inkLevel) : "");

            if (broken[i].name.isEmpty()) nameItem->setBackground(QColor(255, 150, 150));
            if (broken[i].description.isEmpty()) descItem->setBackground(QColor(255, 150, 150));
            if (broken[i].color.isEmpty() || !broken[i].color.contains(QRegularExpression("^[a-zA-Zа-яА-Я -]+$")))
                colorItem->setBackground(QColor(255, 150, 150));
            if (broken[i].inkLevel < 0 || broken[i].inkLevel > 100)
                inkItem->setBackground(QColor(255, 150, 150));

            table2->setItem(i, 0, nameItem);
            table2->setItem(i, 1, descItem);
            table2->setItem(i, 2, colorItem);
            table2->setItem(i, 3, inkItem);
            QTableWidgetItem* errorItem = new QTableWidgetItem(broken[i].getErrors());
            errorItem->setBackground(QColor(255, 200, 200));
            table2->setItem(i, 4, errorItem);
        }

        statsLabel->setText(QString("Всего: %1 | Корректных: %2 | Битых: %3")
                           .arg(all.size()).arg(valid.size()).arg(broken.size()));
    }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
