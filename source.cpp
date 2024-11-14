#include <iostream>
#include <string>
#include <fstream>
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

void replaceBusNames(const std::string &inputFilename, const std::string &outputFilename)
{
    std::ifstream inputFile(inputFilename);
    std::ofstream outputFile(outputFilename);

    if (!inputFile.is_open() || !outputFile.is_open())
    {
        std::cerr << "Ошибка открытия файла." << std::endl;
        return;
    }

    std::string line;
    std::string fullfile;
    std::regex busPattern(R"(Bus(\s*[A-Za-z0-9])*:)"); // regex для поиска названия автобуса
    // Читаем файл построчно
    while (std::getline(inputFile, line))
    {
        fullfile += line;
        fullfile += "\n";
    }

    std::istringstream is(fullfile);
    while (std::getline(is, line))
    {
        if (line._Starts_with("Bus"))
        {
            for (std::sregex_iterator it = std::sregex_iterator(line.begin(), line.end(), busPattern);
                 it != std::sregex_iterator(); it++)
            {
                std::smatch match;
                match = *it;
                std::string tmp = match.str(0).substr(4, match.length(0)-5);
                std::string old_tmp = match.str(0).substr(4, match.length(0)-5);
                std::replace(tmp.begin(), tmp.end(), ' ', '_');
                size_t pos = 0;
                while ((pos = fullfile.find(old_tmp, pos)) != std::string::npos)
                {
                    fullfile.replace(pos, old_tmp.size(), tmp);
                    pos += tmp.length();
                }
            }
        }
    }
    outputFile << fullfile;
    inputFile.close();
    outputFile.close();
}

// Главная функция
int main(int argc, char **argv)
{
    if (argc >= 2)
    {
        std::string inputFilename = "out.txt";    // Имя входного файла
        std::string outputFilename = "_out_.txt"; // Имя выходного файла
        replaceBusNames(inputFilename, outputFilename);
        std::cout << "Обработка завершена. Результат сохранен в " << outputFilename << std::endl;
        return 0;
    }
    std::string input;
    std::istream &in(std::cin);
    std::getline(in, input);
    if (input._Starts_with("1"))
    {
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
                    std::cout << "-----------------------Error------------------\n";
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
        int Id = 1;
        std::vector<json> jsonArray;
        while (std::getline(in, input))
        {
            if (input.find("Bus") == 0)
            {
                auto name = input.substr(4); // Извлекаем название (после "Bus ")
                json busJson;
                busJson["id"] = Id++;
                busJson["type"] = "Bus";
                busJson["name"] = name;
                jsonArray.push_back(busJson);
            }
            else if (input.find("Stop") == 0)
            {
                auto name = input.substr(5); // Извлекаем название (после "Stop ")
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
    }
    else
    {
        std::string line;
        int request_id = 1;               // Идентификатор запроса, который будет увеличиваться
        json output_json = json::array(); // массив для хранения всех записей

        while (std::getline(std::cin, line))
        {
            json output;

            if (line.find("not found") != std::string::npos)
            {
                // Обработка ошибок "not found"
                output["request_id"] = request_id++;
                output["error_message"] = "not found";
            }
            else if (line.find("bus") != std::string::npos)
            {
                // Обработка строк с автобусами
                std::vector<std::string> buses;
                std::istringstream iss(line);
                std::string word;

                while (iss >> word)
                {
                    if (word == "bus")
                    {
                        std::string bus;
                        iss >> bus; // Получаем ID автобуса
                        buses.push_back(bus);
                    }
                }
                output["buses"] = buses;
                output["request_id"] = request_id++;
            }
            else if (line.find("stop") != std::string::npos)
            {
                // Обработка строк с остановками
                std::regex pattern("([0-9]+) stops on route.*?([0-9]+) unique stops.*?([0-9.]+) route length.*?([0-9.]+) curvature");
                std::smatch matches;

                if (std::regex_search(line, matches, pattern))
                {
                    int stop_count = std::stoi(matches[1]);
                    int unique_stop_count = std::stoi(matches[2]);
                    float route_length = std::stof(matches[3]);
                    float curvature = std::stof(matches[4]);

                    output["request_id"] = request_id++;
                    output["stops"] = stop_count;
                    output["unique_stops"] = unique_stop_count;
                    output["route_length"] = route_length;
                    output["curvature"] = curvature;
                }
            }

            if (!output.empty())
            {
                output_json.push_back(output); // Добавляем результат в общий массив
            }
        }

        // Печатаем общий JSON-результат
        std::cout << output_json.dump(4); // Дамп с отступами для удобства чтения
    }
    return 0;
}