#include "config.hpp"
#include "parser.hpp"
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>


void Configuration::loadFromFile(const std::string& inputPath) {
    std::ifstream inputFile(inputPath + "/input.txt");
    if (!inputFile.is_open()) {
        throw std::runtime_error("Erreur: Impossible d'ouvrir le fichier input.txt");
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        if (line.empty() || line[0] == '#') continue;

        if (line.find("Longueur:") != std::string::npos) {
            spans = Parser::parseVector(line);
        }
        else if (line.find("Precision:") != std::string::npos) {
            division = std::stoi(Parser::getValue(line));
        }
        else if (line.find("Beton:") != std::string::npos) {
            betonClass = Parser::getValue(line);
        }
        else if (line.find("preference:") != std::string::npos) {
            Preference = Parser::getValue(line);
        }
        else if (line.find("prise:") != std::string::npos) {
            prise = Parser::getValue(line);
        }
        else if (line.find("condition:") != std::string::npos) {
            condition = Parser::getValue(line);
        }
        else if (line.find("temps:") != std::string::npos) {
            temps = std::stoi(Parser::getValue(line));
        }
        else if (line.find("Inertie_variable:") != std::string::npos) {
            std::string value = Parser::getValue(line);
            inertieVariable = (value.find("y") != std::string::npos || value.find("Y") != std::string::npos);
        }
        else if (inertieVariable) {
            for (size_t n = 0; n < spans.size(); ++n) {
                if (line.find("Iv_" + std::to_string(n) + ':') != std::string::npos) {
                    Inertie_varier.push_back(Parser::parseVector(line));
                }
                if (line.find("Xv_" + std::to_string(n) + ':') != std::string::npos) {
                    pos_Inertie.push_back(Parser::parseVector(line));
                }
            }
        }
        else if (line.find("I:") != std::string::npos  && !inertieVariable) {
            I = std::stod(Parser::getValue(line));
            Inertie = std::vector<double>(spans.size(), I);
        }
        else if (line.find("Steel:") != std::string::npos) {
            acierClass = Parser::getValue(line);
        }
        else if (line.find("SteelCondition:") != std::string::npos) {
            acierCodition = Parser::getValue(line);
        }
        else if (line.find("Wood:") != std::string::npos) {
            woodClass = Parser::getValue(line);
        }
        else if (line.find("WoodCondition:") != std::string::npos) {
            woodCodition = Parser::getValue(line);
        }
        else if (line.find("CHOICHE:") != std::string::npos) {
            choix = Parser::getValue(line);
        }
    }
    inputFile.close();

    if (spans.empty()) {
        throw std::invalid_argument("Error: No spans provided in input file!");
    }
} 