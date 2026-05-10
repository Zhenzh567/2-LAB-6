#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <cctype>

class Marker {
public:
    std::string name;
    std::string description;
    std::string color;
    int inkLevel;

    Marker() : name(""), description(""), color(""), inkLevel(0) {}
    
    std::string toJson() const {
        std::stringstream ss;
        ss << "{\n"
           << "  \"name\": \"" << escapeJson(name) << "\",\n"
           << "  \"description\": \"" << escapeJson(description) << "\",\n"
           << "  \"color\": \"" << escapeJson(color) << "\",\n"
           << "  \"inkLevel\": " << inkLevel << "\n"
           << "}";
        return ss.str();
    }
    
    static Marker fromJson(const std::string& json) {
        Marker m;
        m.name = getJsonValue(json, "name");
        m.description = getJsonValue(json, "description");
        m.color = getJsonValue(json, "color");
        std::string inkStr = getJsonValue(json, "inkLevel");
        try { m.inkLevel = std::stoi(inkStr); }
        catch (...) { m.inkLevel = -1; }
        return m;
    }
    
    bool isValid() const {
        if (name.empty() || description.empty() || color.empty())
            return false;
        if (inkLevel < 0 || inkLevel > 100)
            return false;
        for (char c : color)
            if (!std::isalpha(c) && c != ' ' && c != '-')
                return false;
        return true;
    }
    
    std::string getErrors() const {
        std::string errors;
        if (name.empty()) errors += "пустое название ";
        if (description.empty()) errors += "пустое описание ";
        if (color.empty()) errors += "пустой цвет ";
        else {
            bool valid = true;
            for (char c : color)
                if (!std::isalpha(c) && c != ' ' && c != '-')
                    valid = false;
            if (!valid) errors += "некорректный цвет ";
        }
        if (inkLevel < 0 || inkLevel > 100) errors += "неверный уровень ";
        return errors.empty() ? "OK" : errors;
    }
    
    static std::vector<Marker> loadAll(const std::string& filename) {
        std::vector<Marker> markers;
        std::ifstream file(filename);
        if (!file.is_open()) return markers;
        
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        file.close();
        
        size_t pos = 0;
        while ((pos = content.find("{", pos)) != std::string::npos) {
            size_t end = content.find("}", pos);
            if (end == std::string::npos) break;
            std::string obj = content.substr(pos, end - pos + 1);
            markers.push_back(fromJson(obj));
            pos = end + 1;
        }
        return markers;
    }
    
    static void saveAll(const std::string& filename, const std::vector<Marker>& markers) {
        std::ofstream file(filename);
        if (!file.is_open()) return;
        file << "[\n";
        for (size_t i = 0; i < markers.size(); ++i) {
            file << markers[i].toJson();
            if (i < markers.size() - 1) file << ",";
            file << "\n";
        }
        file << "]\n";
        file.close();
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

private:
    static std::string escapeJson(const std::string& s) {
        std::string result;
        for (char c : s) {
            if (c == '\"') result += "\\\"";
            else if (c == '\\') result += "\\\\";
            else result += c;
        }
        return result;
    }
    
    static std::string getJsonValue(const std::string& json, const std::string& key) {
        std::string search = "\"" + key + "\":";
        size_t pos = json.find(search);
        if (pos == std::string::npos) return "";
        
        pos += search.length();
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t'))
            pos++;
        
        if (pos >= json.length()) return "";
        
        if (json[pos] == '\"') {
            pos++;
            std::string value;
            while (pos < json.length() && json[pos] != '\"') {
                if (json[pos] == '\\' && pos + 1 < json.length())
                    pos++;
                value += json[pos];
                pos++;
            }
            return value;
        } else {
            std::string value;
            while (pos < json.length() && json[pos] != ',' && json[pos] != '\n' && json[pos] != '}' && json[pos] != ' ') {
                value += json[pos];
                pos++;
            }
            return value;
        }
    }
};

void clearScreen() {
    system("cls");
}

void program1() {
    const std::string FILENAME = "markers.json";
    
    while (true) {
        clearScreen();
        std::cout << "1. Создать маркер\n"
                  << "2. Импорт из TXT\n"
                  << "3. Показать все\n"
                  << "0. Назад\n"
                  << "Выбор: ";
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        if (choice == 0) break;
        
        if (choice == 1) {
            clearScreen();
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
            auto markers = Marker::loadAll(FILENAME);
            markers.push_back(m);
            Marker::saveAll(FILENAME, markers);
            std::cout << "Добавлено\n";
            std::cin.get();
        }
        else if (choice == 2) {
            clearScreen();
            std::cout << "Имя TXT файла: ";
            std::string txtFile;
            std::getline(std::cin, txtFile);
            
            Marker m;
            std::ifstream file(txtFile);
            if (!file.is_open()) {
                std::cout << "Ошибка открытия файла\n";
                std::cin.get();
                continue;
            }
            std::string line;
            if (!std::getline(file, line)) {
                std::cout << "Файл пуст\n";
                file.close();
                std::cin.get();
                continue;
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
            
            auto markers = Marker::loadAll(FILENAME);
            markers.push_back(m);
            Marker::saveAll(FILENAME, markers);
            std::cout << "Сохранено\n";
            std::cin.get();
        }
        else if (choice == 3) {
            clearScreen();
            auto markers = Marker::loadAll(FILENAME);
            for (size_t i = 0; i < markers.size(); ++i) {
                std::cout << "Маркер " << i+1 << ":\n";
                markers[i].print();
                std::cout << "\n";
            }
            std::cin.get();
        }
    }
}

void program2() {
    std::cout << "Путь к JSON файлу: ";
    std::string filename;
    std::getline(std::cin, filename);
    if (filename.empty()) filename = "markers.json";
    
    auto allMarkers = Marker::loadAll(filename);
    
    std::vector<Marker> validMarkers;
    std::vector<Marker> brokenMarkers;
    
    for (const auto& m : allMarkers) {
        if (m.isValid())
            validMarkers.push_back(m);
        else
            brokenMarkers.push_back(m);
    }
    
    std::sort(validMarkers.begin(), validMarkers.end(),
              [](const Marker& a, const Marker& b) {
                  return a.name > b.name;
              });
    
    if (!brokenMarkers.empty())
        Marker::saveAll("broken.json", brokenMarkers);
    
    auto printTable = [](const std::vector<Marker>& markers, const std::string& title) {
        std::cout << title << "\n";
        std::cout << std::string(100, '-') << "\n";
        std::cout << std::left
                  << std::setw(20) << "Название"
                  << std::setw(30) << "Описание"
                  << std::setw(15) << "Цвет"
                  << std::setw(12) << "Ур.чернил"
                  << std::setw(20) << "Статус" << "\n";
        std::cout << std::string(100, '-') << "\n";
        for (const auto& m : markers) {
            std::cout << std::left
                      << std::setw(20) << m.name.substr(0, 19)
                      << std::setw(30) << m.description.substr(0, 29)
                      << std::setw(15) << m.color.substr(0, 14)
                      << std::setw(12) << m.inkLevel
                      << std::setw(20) << m.getErrors() << "\n";
        }
        std::cout << std::string(100, '-') << "\n";
    };
    
    printTable(validMarkers, "ТАБЛИЦА 1 (корректные)");
    
    if (!brokenMarkers.empty())
        printTable(brokenMarkers, "ТАБЛИЦА 2 (битые)");
    
    std::cout << "Всего: " << allMarkers.size() 
              << " | Корректных: " << validMarkers.size()
              << " | Битых: " << brokenMarkers.size() << "\n";
    
    std::cout << "\nНажмите Enter для возврата...";
    std::cin.get();
}

int main() {
    setlocale(LC_ALL, "Russian");
    
    while (true) {
        clearScreen();
        std::cout << "ЛАБОРАТОРНАЯ РАБОТА 6 - МАРКЕРЫ\n\n";
        std::cout << "1. Программа 1 (создание/импорт)\n";
        std::cout << "2. Программа 2 (обработка)\n";
        std::cout << "0. Выход\n";
        std::cout << "Выбор: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        if (choice == 0) break;
        if (choice == 1) program1();
        if (choice == 2) program2();
    }
    
    return 0;
}
