#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHeaderView>
#include <QStackedWidget>
#include <QGroupBox>

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
        if (inkLevel < 0 || inkLevel > 100) errors += "уровень 0-100; ";
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

    static Marker fromTxtLine(const QString& line) {
        Marker m;
        QStringList parts = line.split('/');
        if (parts.size() >= 4) {
            m.name = parts[0];
            m.description = parts[1];
            m.color = parts[2];
            m.inkLevel = parts[3].toInt();
        }
        return m;
    }
};

class Program1Widget : public QWidget {
    Q_OBJECT
public:
    Program1Widget(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);

        auto* formGroup = new QGroupBox("Данные маркера");
        auto* formLayout = new QVBoxLayout(formGroup);

        auto* nameLayout = new QHBoxLayout();
        nameLayout->addWidget(new QLabel("Название:"));
        nameEdit = new QLineEdit();
        nameLayout->addWidget(nameEdit);
        formLayout->addLayout(nameLayout);

        auto* descLayout = new QHBoxLayout();
        descLayout->addWidget(new QLabel("Описание:"));
        descEdit = new QLineEdit();
        descLayout->addWidget(descEdit);
        formLayout->addLayout(descLayout);

        auto* colorLayout = new QHBoxLayout();
        colorLayout->addWidget(new QLabel("Цвет:"));
        colorEdit = new QLineEdit();
        colorLayout->addWidget(colorEdit);
        formLayout->addLayout(colorLayout);

        auto* inkLayout = new QHBoxLayout();
        inkLayout->addWidget(new QLabel("Уровень чернил:"));
        inkSpin = new QSpinBox();
        inkSpin->setRange(0, 100);
        inkLayout->addWidget(inkSpin);
        formLayout->addLayout(inkLayout);

        layout->addWidget(formGroup);

        auto* buttonsLayout = new QHBoxLayout();
        auto* addBtn = new QPushButton("Добавить в JSON");
        auto* importBtn = new QPushButton("Импорт из TXT");
        auto* viewBtn = new QPushButton("Показать все");
        buttonsLayout->addWidget(addBtn);
        buttonsLayout->addWidget(importBtn);
        buttonsLayout->addWidget(viewBtn);
        layout->addLayout(buttonsLayout);

        table = new QTableWidget();
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Уровень чернил"});
        table->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table);

        connect(addBtn, &QPushButton::clicked, this, &Program1Widget::addMarker);
        connect(importBtn, &QPushButton::clicked, this, &Program1Widget::importFromTxt);
        connect(viewBtn, &QPushButton::clicked, this, &Program1Widget::showAll);
    }

private slots:
    void addMarker() {
        Marker m(nameEdit->text(), descEdit->text(), colorEdit->text(), inkSpin->value());
        auto markers = Marker::loadAll("markers.json");
        markers.append(m);
        Marker::saveAll("markers.json", markers);
        QMessageBox::information(this, "Готово", "Маркер добавлен");
        showAll();
        nameEdit->clear();
        descEdit->clear();
        colorEdit->clear();
        inkSpin->setValue(0);
    }

    void importFromTxt() {
        QString filename = QFileDialog::getOpenFileName(this, "Выберите TXT файл", "", "Текстовые файлы (*.txt)");
        if (filename.isEmpty()) return;

        QFile file(filename);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
            return;
        }
        QString line = file.readLine().trimmed();
        file.close();

        Marker m = Marker::fromTxtLine(line);
        nameEdit->setText(m.name);
        descEdit->setText(m.description);
        colorEdit->setText(m.color);
        inkSpin->setValue(m.inkLevel);

        QMessageBox::information(this, "Импорт", "Данные загружены. Отредактируйте и нажмите Добавить");
    }

    void showAll() {
        auto markers = Marker::loadAll("markers.json");
        table->setRowCount(markers.size());
        for (int i = 0; i < markers.size(); ++i) {
            table->setItem(i, 0, new QTableWidgetItem(markers[i].name));
            table->setItem(i, 1, new QTableWidgetItem(markers[i].description));
            table->setItem(i, 2, new QTableWidgetItem(markers[i].color));
            table->setItem(i, 3, new QTableWidgetItem(QString::number(markers[i].inkLevel)));
        }
    }

private:
    QLineEdit* nameEdit;
    QLineEdit* descEdit;
    QLineEdit* colorEdit;
    QSpinBox* inkSpin;
    QTableWidget* table;
};

class Program2Widget : public QWidget {
    Q_OBJECT
public:
    Program2Widget(QWidget* parent = nullptr) : QWidget(parent) {
        auto* layout = new QVBoxLayout(this);

        auto* fileLayout = new QHBoxLayout();
        fileLayout->addWidget(new QLabel("JSON файл:"));
        fileEdit = new QLineEdit("markers.json");
        fileLayout->addWidget(fileEdit);
        auto* browseBtn = new QPushButton("Обзор");
        fileLayout->addWidget(browseBtn);
        auto* loadBtn = new QPushButton("Загрузить");
        fileLayout->addWidget(loadBtn);
        layout->addLayout(fileLayout);

        table1 = new QTableWidget();
        table1->setColumnCount(5);
        table1->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Статус"});
        table1->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(new QLabel("ТАБЛИЦА 1 (корректные, сортировка Я→А):"));
        layout->addWidget(table1);

        table2 = new QTableWidget();
        table2->setColumnCount(5);
        table2->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Ур.чернил", "Ошибки"});
        table2->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(new QLabel("ТАБЛИЦА 2 (битые):"));
        layout->addWidget(table2);

        auto* statsLabel = new QLabel();
        layout->addWidget(statsLabel);

        connect(browseBtn, &QPushButton::clicked, [this]() {
            QString filename = QFileDialog::getOpenFileName(this, "Выберите JSON", "", "JSON (*.json)");
            if (!filename.isEmpty()) fileEdit->setText(filename);
        });

        connect(loadBtn, &QPushButton::clicked, [this, statsLabel]() {
            auto all = Marker::loadAll(fileEdit->text());

            QVector<Marker> valid, broken;
            for (const auto& m : all)
                (m.isValid() ? valid : broken).append(m);

            std::sort(valid.begin(), valid.end(), [](const Marker& a, const Marker& b) {
                return a.name > b.name;
            });

            if (!broken.isEmpty())
                Marker::saveAll("broken.json", broken);

            auto fillTable = [](QTableWidget* table, const QVector<Marker>& markers, bool showErrors) {
                table->setRowCount(markers.size());
                for (int i = 0; i < markers.size(); ++i) {
                    table->setItem(i, 0, new QTableWidgetItem(markers[i].name));
                    table->setItem(i, 1, new QTableWidgetItem(markers[i].description));
                    table->setItem(i, 2, new QTableWidgetItem(markers[i].color));
                    table->setItem(i, 3, new QTableWidgetItem(QString::number(markers[i].inkLevel)));
                    table->setItem(i, 4, new QTableWidgetItem(markers[i].getErrors()));
                }
            };

            fillTable(table1, valid, false);
            fillTable(table2, broken, true);

            statsLabel->setText(QString("Всего: %1 | Корректных: %2 | Битых: %3")
                               .arg(all.size()).arg(valid.size()).arg(broken.size()));
        });
    }

private:
    QLineEdit* fileEdit;
    QTableWidget* table1;
    QTableWidget* table2;
};

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Лабораторная работа №6 — Маркеры");
        resize(800, 600);

        auto* central = new QWidget();
        setCentralWidget(central);
        auto* layout = new QVBoxLayout(central);

        auto* btnLayout = new QHBoxLayout();
        auto* btn1 = new QPushButton("Программа 1 (создание/импорт)");
        auto* btn2 = new QPushButton("Программа 2 (обработка)");
        btnLayout->addWidget(btn1);
        btnLayout->addWidget(btn2);
        layout->addLayout(btnLayout);

        stack = new QStackedWidget();
        prog1 = new Program1Widget();
        prog2 = new Program2Widget();
        stack->addWidget(prog1);
        stack->addWidget(prog2);
        layout->addWidget(stack);

        connect(btn1, &QPushButton::clicked, [this]() { stack->setCurrentIndex(0); });
        connect(btn2, &QPushButton::clicked, [this]() { stack->setCurrentIndex(1); });
    }

private:
    QStackedWidget* stack;
    Program1Widget* prog1;
    Program2Widget* prog2;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
