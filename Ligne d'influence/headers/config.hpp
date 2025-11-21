#pragma once
#include <vector>
#include <string>

class Configuration {
public:
    std::vector<double> spans;
    int division;
    std::string betonClass;
    std::string Preference;
    std::string prise;
    std::string choix;
    int temps;
    std::string condition;
    std::string acierClass;
    std::string acierCodition;
    std::string woodClass;
    std::string woodCodition;
    bool inertieVariable;
    double I;
    std::vector<double> Inertie;
    std::vector<std::vector<double>> pos_Inertie;
    std::vector<std::vector<double>> Inertie_varier;

    Configuration() : 
        division(0), 
        temps(0), 
        inertieVariable(false), 
        I(0.0) {}

    void loadFromFile(const std::string& inputPath);
}; 