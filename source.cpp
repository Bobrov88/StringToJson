#include <iostream>
#include <string>
#include <sstream>
#include <nlohmann/json.hpp>
#include <regex>
#include <vector>
#include <unordered_map>

using json = nlohmann::json;

std::string &trim(std::string &str)
{
    const auto b = str.find_first_not_of(" ");
    const auto c = str.find_last_not_of(" ");
    str = str.substr(b, c - b + 1);
    return str;
}

// Функция для разбора информации о остановке
json parseStop(const std::string &input)
{
    std::regex stopRegex(R"(Stop ([^:]+): ([\d.-]+), ([\d.-]+), (.+))");
    std::smatch match;

    if (std::regex_match(input, match, stopRegex))
    {
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
        while (std::regex_search(searchStart, roadDistances.cend(), distanceMatch, distanceRegex))
        {
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
json parseBus(const std::string &input)
{
    std::regex busRegex(R"(Bus ([^:]+): (.+))");
    std::smatch match;

    if (std::regex_match(input, match, busRegex))
    {
        json busJson;
        busJson["type"] = "Bus";
        busJson["name"] = match[1].str();

        std::string stopsString = match[2].str();
        std::vector<std::string> stops;
        std::regex stopRegex(R"([-|>]+)");
        std::sregex_token_iterator it(stopsString.begin(), stopsString.end(), stopRegex, -1);
        std::sregex_token_iterator reg_end;
        while (it != reg_end)
        {
            stops.push_back(trim(it->str()));
            ++it;
        }
        bool isRoundtrip = stops.front() == stops.back();

        if (!isRoundtrip)
        {
            std::vector<std::string> tmp;
            tmp.reserve(stops.size());
            for (auto it = stops.rbegin() + 1; it != stops.rend(); ++it)
            { // Обратный порядок
                tmp.push_back(*it);
            }
            stops.insert(stops.end(), tmp.begin(), tmp.end());
        }
        busJson["stops"] = stops;
        busJson["is_roundtrip"] = isRoundtrip;

        return busJson;
    }

    throw std::invalid_argument("Invalid Bus input format.");
}

// Главная функция
int main()
{
    std::string input;
    std::istream &in(std::cin);
    std::cout << "{ \"base_requests\":[";
    bool is_first = true;
    while (std::getline(in, input))
    {
        try
        {
            if (input._Starts_with("---"))
            {
                break;
            }
            if (!is_first)
                std::cout << ",";
            if (input._Starts_with("Stop"))
            {
                json stopJson = parseStop(input);
                std::cout << stopJson.dump(4) << std::endl; // Форматированный вывод JSON
            }
            else if (input._Starts_with("Bus"))
            {
                json busJson = parseBus(input);
                std::cout << busJson.dump(4) << std::endl; // Форматированный вывод JSON
            }
            else
            {
                std::cout<<"-----------------------Error------------------\n";
            }
        }
        catch (const std::exception &e)
        {
            std::cout << e.what() << std::endl;
        }
        is_first = false;
    }
    std::cout << "],\n";
    std::cout << "\"stat_requests\": ";
    std::string line;
    int Id = 1;
    std::vector<json> jsonArray;
    while (std::getline(in, line))
    {
        if (line.find("Bus") == 0)
        {
            auto name = line.substr(4); // Извлекаем название (после "Bus ")
            json busJson;
            busJson["id"] = Id++;
            busJson["type"] = "Bus";
            busJson["name"] = name;
            jsonArray.push_back(busJson);
        }
        else if (line.find("Stop") == 0)
        {
            auto name = line.substr(5); // Извлекаем название (после "Stop ")
            json stopJson;
            stopJson["id"] = Id++;
            stopJson["type"] = "Stop";
            stopJson["name"] = name;
            jsonArray.push_back(stopJson);
        }
    }
    json finalJson = jsonArray;
    std::cout << finalJson.dump(4) << std::endl;
    std::cout << "]";
    return 0;
}