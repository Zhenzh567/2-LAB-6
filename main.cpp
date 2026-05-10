#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Marker {
public:
    std::string name;
    std::string description;
    std::string color;
    int inkLevel;

    Marker() : name(""), description(""), color(""), inkLevel(0) {}
    
    json toJson() const {
        return json{{"name", name}, {"description", description}, 
                   {"color", color}, {"inkLevel", inkLevel}};
    }
    
    static Marker fromJson(const json& j) {
        Marker m;
        if (j.contains("name") && j["name"].is_string())
            m.name = j["name"].get<std::string>();
        if (j.contains("description") && j["description"].is_string())
            m.description = j["description"].get<std::string>();
        if (j.contains("color") && j["color"].is_string())
            m.color = j["color"].get<std::string>();
        if (j.contains("inkLevel") && j["inkLevel"].is_number_integer())
            m.inkLevel = j["inkLevel"].get<int>();
        else
            m.inkLevel = -1;
        return m;
    }
    
    static std::vector<Marker> loadAll(const std::string& filename) {
        std::vector<Marker> markers;
        std::ifstream file(filename);
        if (!file.is_open()) return markers;
        try {
            json j;
            file >> j;
            if (j.is_array())
                for (const auto& item : j)
                    markers.push_back(fromJson(item));
        } catch (...) {}
        file.close();
        return markers;
    }
    
    static void saveAll(const std::string& filename, const std::vector<Marker>& markers) {
        json j = json::array();
        for (const auto& m : markers)
            j.push_back(m.toJson());
        std::ofstream file(filename);
        if (file.is_open()) {
            file << j.dump(4);
            file.close();
        }
    }
    
    static Marker fromTxtLine(const std::string& line) {
        Marker m;
        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> tokens;
        while (std::getline(ss, token, '/'))
            tokens.push_back(token);
        if (tokens.size() >= 4) {
            m.name = tokens[0];
            m.description = tokens[1];
            m.color = tokens[2];
            try { m.inkLevel = std::stoi(tokens[3]); }
            catch (...) { m.inkLevel = -1; }
        }
        return m;
    }
    
    void print() const {
        std::cout << "Название: " << name << "\n"
                  << "Описание: " << description << "\n"
                  << "Цвет: " << color << "\n"
                  << "Уровень чернил: " << inkLevel << "\n";
    }
};

void clearScreen() {
    system("cls");
}

Marker createMarker() {
    Marker m;
    std::cout << "Название: ";
    std::getline(std::cin, m.name);
    std::cout << "Описание: ";
    std::getline(std::cin, m.description);
    std::cout << "Цвет: ";
    std::getline(std::cin, m.color);
    while (true) {
        std::cout << "Уровень чернил (0-100): ";
        std::string input;
        std::getline(std::cin, input);
        try {
            m.inkLevel = std::stoi(input);
            if (m.inkLevel >= 0 && m.inkLevel <= 100) break;
            std::cout << "Ошибка: значение от 0 до 100\n";
        } catch (...) {
            std::cout << "Ошибка: введите число\n";
        }
    }
    return m;
}

Marker importFromTxt(const std::string& filename) {
    Marker m;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "Ошибка открытия файла\n";
        return m;
    }
    std::string line;
    if (!std::getline(file, line)) {
        std::cout << "Файл пуст\n";
        file.close();
        return m;
    }
    file.close();
    m = Marker::fromTxtLine(line);
    std::cout << "Импортировано:\n";
    m.print();
    std::string input;
    std::cout << "\nНазвание [" << m.name << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) m.name = input;
    std::cout << "Описание [" << m.description << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) m.description = input;
    std::cout << "Цвет [" << m.color << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) m.color = input;
    std::cout << "Уровень чернил [" << m.inkLevel << "]: ";
    std::getline(std::cin, input);
    if (!input.empty()) {
        try {
            int val = std::stoi(input);
            if (val >= 0 && val <= 100) m.inkLevel = val;
        } catch (...) {}
    }
    return m;
}

int main() {
    setlocale(LC_ALL, "Russian");
    const std::string FILENAME = "markers.json";
    
    while (true) {
        clearScreen();
        std::cout << "1. Создать маркер\n"
                  << "2. Импорт из TXT\n"
                  << "3. Показать все\n"
                  << "0. Выход\n"
                  << "Выбор: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                Marker m = createMarker();
                auto markers = Marker::loadAll(FILENAME);
                markers.push_back(m);
                Marker::saveAll(FILENAME, markers);
                std::cout << "Добавлено\n";
                std::cin.get();
                break;
            }
            case 2: {
                clearScreen();
                std::cout << "Имя TXT файла: ";
                std::string txtFile;
                std::getline(std::cin, txtFile);
                Marker m = importFromTxt(txtFile);
                auto markers = Marker::loadAll(FILENAME);
                markers.push_back(m);
                Marker::saveAll(FILENAME, markers);
                std::cout << "Сохранено\n";
                std::cin.get();
                break;
            }
            case 3: {
                clearScreen();
                auto markers = Marker::loadAll(FILENAME);
                for (size_t i = 0; i < markers.size(); ++i) {
                    std::cout << "Маркер " << i+1 << ":\n";
                    markers[i].print();
                    std::cout << "\n";
                }
                std::cin.get();
                break;
            }
        }
    }
    return 0;
}
