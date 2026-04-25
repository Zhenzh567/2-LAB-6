#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>
#include <vector>
#include <algorithm>

// ================================ КЛАСС МАРКЕР ================================
class Marker {
public:
    QString name;
    QString description;
    QString color;
    int inkLevel;

    Marker() : inkLevel(-1) {}
    Marker(const QString& n, const QString& d, const QString& c, int ink)
        : name(n), description(d), color(c), inkLevel(ink) {}

    bool isValid() const {
        return !name.isEmpty() && !description.isEmpty() && !color.isEmpty()
               && inkLevel >= 0 && inkLevel <= 100;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["name"] = name;
        obj["description"] = description;
        obj["color"] = color;
        obj["inkLevel"] = inkLevel;
        return obj;
    }

    static Marker fromJson(const QJsonObject& obj) {
        return Marker(obj["name"].toString(),
                      obj["description"].toString(),
                      obj["color"].toString(),
                      obj["inkLevel"].toInt());
    }

    static Marker fromTxtLine(const QString& line, bool& ok) {
        ok = false;
        QStringList parts = line.split('/');
        if (parts.size() != 4) return Marker();
        bool convOk;
        int ink = parts[3].toInt(&convOk);
        if (!convOk) return Marker();
        ok = true;
        return Marker(parts[0], parts[1], parts[2], ink);
    }
};

// ================================ ГЛОБАЛЬНЫЕ ФУНКЦИИ ================================
void saveToJson(const std::vector<Marker>& markers, const QString& filename) {
    QJsonArray arr;
    for (const auto& m : markers)
        arr.append(m.toJson());
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly))
        file.write(QJsonDocument(arr).toJson());
}

std::vector<Marker> loadFromJson(const QString& filename) {
    std::vector<Marker> markers;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) return markers;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isArray()) return markers;
    for (auto val : doc.array())
        markers.push_back(Marker::fromJson(val.toObject()));
    return markers;
}

// ================================ ПРОГРАММА 1 (Создание) ================================
class Program1Dialog : public QDialog {
    Q_OBJECT
public:
    Program1Dialog(std::vector<Marker>& markers, const QString& jsonFile, QWidget* parent = nullptr)
        : QDialog(parent), m_markers(markers), m_jsonFile(jsonFile) {
        setWindowTitle("Программа 1 — Создать маркер");
        resize(400, 300);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        mainLayout->addWidget(new QLabel("Название:"));
        m_nameEdit = new QLineEdit; mainLayout->addWidget(m_nameEdit);

        mainLayout->addWidget(new QLabel("Описание:"));
        m_descEdit = new QLineEdit; mainLayout->addWidget(m_descEdit);

        mainLayout->addWidget(new QLabel("Цвет:"));
        m_colorEdit = new QLineEdit; mainLayout->addWidget(m_colorEdit);

        mainLayout->addWidget(new QLabel("Уровень чернил (0-100):"));
        m_inkSpin = new QSpinBox; m_inkSpin->setRange(0,100); mainLayout->addWidget(m_inkSpin);

        QPushButton* btnOk = new QPushButton("Добавить");
        QPushButton* btnTxt = new QPushButton("Загрузить из .txt");
        QPushButton* btnCancel = new QPushButton("Закрыть");

        QHBoxLayout* btnLayout = new QHBoxLayout;
        btnLayout->addWidget(btnOk);
        btnLayout->addWidget(btnTxt);
        btnLayout->addWidget(btnCancel);
        mainLayout->addLayout(btnLayout);

        connect(btnOk, &QPushButton::clicked, this, &Program1Dialog::addMarker);
        connect(btnTxt, &QPushButton::clicked, this, &Program1Dialog::loadFromTxt);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::accept);
    }

private slots:
    void addMarker() {
        Marker m(m_nameEdit->text(), m_descEdit->text(),
                 m_colorEdit->text(), m_inkSpin->value());
        if (!m.isValid()) {
            QMessageBox::warning(this, "Ошибка", "Заполните все поля и уровень чернил от 0 до 100");
            return;
        }
        m_markers.push_back(m);
        saveToJson(m_markers, m_jsonFile);
        QMessageBox::information(this, "Готово", "Маркер добавлен");
        m_nameEdit->clear();
        m_descEdit->clear();
        m_colorEdit->clear();
        m_inkSpin->setValue(0);
    }

    void loadFromTxt() {
        QString fileName = QFileDialog::getOpenFileName(this, "Выберите .txt файл", "", "*.txt");
        if (fileName.isEmpty()) return;
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
            return;
        }
        QTextStream stream(&file);
        int added = 0;
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.trimmed().isEmpty()) continue;
            bool ok;
            Marker m = Marker::fromTxtLine(line, ok);
            if (ok) {
                m_markers.push_back(m);
                added++;
            }
        }
        saveToJson(m_markers, m_jsonFile);
        QMessageBox::information(this, "Готово", QString("Загружено %1 маркеров").arg(added));
    }

private:
    std::vector<Marker>& m_markers;
    QString m_jsonFile;
    QLineEdit *m_nameEdit, *m_descEdit, *m_colorEdit;
    QSpinBox *m_inkSpin;
};

// ================================ ПРОГРАММА 2 (Вывод и битые) ================================
class Program2Dialog : public QDialog {
    Q_OBJECT
public:
    Program2Dialog(const QString& jsonFile, QWidget* parent = nullptr)
        : QDialog(parent), m_jsonFile(jsonFile) {
        setWindowTitle("Программа 2 — Корректные / Битые");
        resize(900, 500);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        QHBoxLayout* tablesLayout = new QHBoxLayout;
        m_validTable = new QTableWidget; m_validTable->setColumnCount(4);
        m_validTable->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Чернила"});
        m_brokenTable = new QTableWidget; m_brokenTable->setColumnCount(4);
        m_brokenTable->setHorizontalHeaderLabels({"Название", "Описание", "Цвет", "Чернила"});
        tablesLayout->addWidget(m_validTable);
        tablesLayout->addWidget(m_brokenTable);
        mainLayout->addLayout(tablesLayout);

        QPushButton* btnRefresh = new QPushButton("Обновить");
        QPushButton* btnClose = new QPushButton("Закрыть");
        QHBoxLayout* btnLayout = new QHBoxLayout;
        btnLayout->addWidget(btnRefresh);
        btnLayout->addWidget(btnClose);
        mainLayout->addLayout(btnLayout);

        connect(btnRefresh, &QPushButton::clicked, this, &Program2Dialog::refresh);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

        refresh();
    }

private slots:
    void refresh() {
        auto markers = loadFromJson(m_jsonFile);
        std::vector<Marker> valid, broken;
        for (const auto& m : markers) {
            if (m.isValid()) valid.push_back(m);
            else broken.push_back(m);
        }
        saveToJson(broken, "broken.json");

        m_validTable->setRowCount(valid.size());
        for (size_t i = 0; i < valid.size(); ++i) {
            m_validTable->setItem(i, 0, new QTableWidgetItem(valid[i].name));
            m_validTable->setItem(i, 1, new QTableWidgetItem(valid[i].description));
            m_validTable->setItem(i, 2, new QTableWidgetItem(valid[i].color));
            m_validTable->setItem(i, 3, new QTableWidgetItem(QString::number(valid[i].inkLevel)));
        }
        m_brokenTable->setRowCount(broken.size());
        for (size_t i = 0; i < broken.size(); ++i) {
            m_brokenTable->setItem(i, 0, new QTableWidgetItem(broken[i].name));
            m_brokenTable->setItem(i, 1, new QTableWidgetItem(broken[i].description));
            m_brokenTable->setItem(i, 2, new QTableWidgetItem(broken[i].color));
            m_brokenTable->setItem(i, 3, new QTableWidgetItem(QString::number(broken[i].inkLevel)));
        }
    }

private:
    QString m_jsonFile;
    QTableWidget *m_validTable, *m_brokenTable;
};

// ================================ ГЛАВНОЕ ОКНО ================================
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Лабораторная работа №6 — Вариант 5 (Маркеры)");
        resize(400, 150);

        QWidget* central = new QWidget;
        QVBoxLayout* layout = new QVBoxLayout(central);

        QPushButton* btnProg1 = new QPushButton("Программа 1: Создать маркер + загрузить txt");
        QPushButton* btnProg2 = new QPushButton("Программа 2: Показать корректные / битые");

        layout->addWidget(btnProg1);
        layout->addWidget(btnProg2);

        setCentralWidget(central);

        connect(btnProg1, &QPushButton::clicked, this, &MainWindow::openProgram1);
        connect(btnProg2, &QPushButton::clicked, this, &MainWindow::openProgram2);
    }

private slots:
    void openProgram1() {
        auto markers = loadFromJson("markers.json");
        Program1Dialog dlg(markers, "markers.json", this);
        dlg.exec();
    }

    void openProgram2() {
        Program2Dialog dlg("markers.json", this);
        dlg.exec();
    }
};

// ================================ MAIN ================================
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
