#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QHeaderView>

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

class MainWindow : public QWidget {
public:
    MainWindow() {
        setWindowTitle("Программа 1 - Создание и импорт маркеров");
        resize(700, 500);

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
        inkLayout->addWidget(new QLabel("Уровень чернил (0-100):"));
        inkSpin = new QSpinBox();
        inkSpin->setRange(0, 100);
        inkLayout->addWidget(inkSpin);
        formLayout->addLayout(inkLayout);

        layout->addWidget(formGroup);

        auto* btnLayout = new QHBoxLayout();
        auto* addBtn = new QPushButton("Добавить в JSON");
        auto* importBtn = new QPushButton("Импорт из TXT");
        auto* viewBtn = new QPushButton("Показать все");
        btnLayout->addWidget(addBtn);
        btnLayout->addWidget(importBtn);
        btnLayout->addWidget(viewBtn);
        layout->addLayout(btnLayout);

        table = new QTableWidget();
        table->setColumnCount(4);
        table->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Уровень чернил"});
        table->horizontalHeader()->setStretchLastSection(true);
        layout->addWidget(table);

        connect(addBtn, &QPushButton::clicked, this, &MainWindow::addMarker);
        connect(importBtn, &QPushButton::clicked, this, &MainWindow::importFromTxt);
        connect(viewBtn, &QPushButton::clicked, this, &MainWindow::showAll);

        showAll();
    }

private:
    QLineEdit* nameEdit;
    QLineEdit* descEdit;
    QLineEdit* colorEdit;
    QSpinBox* inkSpin;
    QTableWidget* table;

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
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
