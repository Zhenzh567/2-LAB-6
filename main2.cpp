#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <iomanip>
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
};

void printTable(const std::vector<Marker>& markers, const std::string& title) {
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
}

int main() {
    setlocale(LC_ALL, "Russian");
    
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
    
    printTable(validMarkers, "ТАБЛИЦА 1 (корректные)");
    
    if (!brokenMarkers.empty())
        printTable(brokenMarkers, "ТАБЛИЦА 2 (битые)");
    
    std::cout << "Всего: " << allMarkers.size() 
              << " | Корректных: " << validMarkers.size()
              << " | Битых: " << brokenMarkers.size() << "\n";
    
    std::cin.get();
    return 0;
}
