#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

// Функция для разбора информации о остановке
json parseStop(const std::string& input) {
    std::regex stopRegex(R"(Stop ([^:]+): ([\d.-]+), ([\d.-]+), (.+))");
    std::smatch match;

    if (std::regex_match(input, match, stopRegex)) {
        json stopJson;
        stopJson["type"] = "Stop";
        stopJson["name"] = match[1].str();
        stopJson["latitude"] = std::stof(match[2].str());
        stopJson["longitude"] = std::stof(match[3].str());

        // Обработка расстояний
        json roadDistancesJson;
        std::string roadDistances = match[4].str();
        std::regex distanceRegex(R"((\d+)m to ([\w\s]+))");
        std::smatch distanceMatch;
        std::string::const_iterator searchStart(roadDistances.cbegin());
        while (std::regex_search(searchStart, roadDistances.cend(), distanceMatch, distanceRegex)) {
            distanceMatch[2].str().pop_back();
            roadDistancesJson[distanceMatch[2].str()] = std::stoi(distanceMatch[1].str());
            searchStart = distanceMatch.suffix().first;
        }

        stopJson["road_distances"] = roadDistancesJson;
        return stopJson;
    }

    throw std::invalid_argument("Invalid Stop input format.");
}

// Функция для разбора информации о автобусе
json parseBus(const std::string& input) {
    std::regex busRegex(R"(Bus ([^:]+): (.+))");
    std::smatch match;

    if (std::regex_match(input, match, busRegex)) {
        json busJson;
        busJson["type"] = "Bus";
        busJson["name"] = match[1].str();

        // Разбиение остановок
        std::string stopsString = match[2].str();
        std::vector<std::string> stops;
        std::regex stopRegex(R"([-|>| ]+)");
        std::sregex_token_iterator it(stopsString.begin(), stopsString.end(), stopRegex, -1);
        std::sregex_token_iterator reg_end;

        while (it != reg_end) {
            stops.push_back(it->str());
            ++it;
        }

        // Проверка, является ли автобус кольцевым маршрутом
        bool isRoundtrip = stops.front() == stops.back();

        if (!isRoundtrip) {
            for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it) { // Обратный порядок
                stops.push_back(*it);
            }
        }

        busJson["stops"] = stops;
        busJson["is_roundtrip"] = isRoundtrip;

        return busJson;
    }

    throw std::invalid_argument("Invalid Bus input format.");
}

// Главная функция
int main() {
    std::string input;

    // Пример входных данных для остановок
    std::cout << "Введите остановку или автобус:\n";
    std::getline(std::cin, input);

    try {
        if (input._Starts_with("Stop")) {
            json stopJson = parseStop(input);
            std::cout << stopJson.dump(4) << std::endl; // Форматированный вывод JSON
        } else if (input._Starts_with("Bus")) {
            json busJson = parseBus(input);
            std::cout << busJson.dump(4) << std::endl; // Форматированный вывод JSON
        } else {
            std::cout << "Неподдерживаемый формат." << std::endl;
        }
    } catch (const std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }

    return 0;
}