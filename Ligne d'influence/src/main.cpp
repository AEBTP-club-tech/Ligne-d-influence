#include "utile.hpp"
#include "traitement.hpp"
#include "beton.hpp"
#include "steel.hpp"
#include "wood.hpp"
#include "config.hpp"
#include "history_logger.hpp"

#include <vector>
#include <memory> //pour les pointeur intelligent 
#include <chrono>
#include <iostream> 
#include <filesystem> //pour 
#include <stdexcept>
#include <string>
#include <nlohmann/json.hpp>

// Convert string to WoodStrengthClass
static EC5::WoodStrengthClass stringToWoodClass(const std::string& str) {
    if (str == "C24") return EC5::WoodStrengthClass::C24;
    if (str == "C30") return EC5::WoodStrengthClass::C30;
    if (str == "D30") return EC5::WoodStrengthClass::D30;
    throw std::invalid_argument("Invalid wood class: " + str);
}

int main() {
    // Initialiser le logger d'historique
    HistoryLogger logger;
    nlohmann::json DATA; 

    // Obtention du chemin absolu du dossier principal du projet
    std::string exePath = getExecutablePath();
    std::string inputPath;
    std::string dataPath = exePath;
    
    inputPath = dataPath;
    dataPath = dataPath + "/data";

    // Chargement de la configuration
    Configuration config;
    config.loadFromFile(inputPath);

    // Enregistrer la configuration dans l'historique
    nlohmann::json config_data;
    config_data["choix"] = config.choix;
    config_data["spans"] = config.spans;
    config_data["division"] = config.division;
    config_data["inertieVariable"] = config.inertieVariable;
    DATA["configuration_loaded"]= config_data; 
    //logger.addEntry(config_data, "configuration_loaded");

    std::vector<double> Young;
    if (config.choix == "Concrete") {
    // Création d'un objet beton
    Beton BETON = Beton(config.betonClass, config.Preference, config.prise, config.temps, config.condition);
        Young = std::vector<double>(config.spans.size(), BETON.Ecm);
    }
    else if (config.choix == "Steel") {
        // Création d'un objet steel
        Steel acier = Steel(config.acierClass, config.acierCodition); 
        Young = std::vector<double>(config.spans.size(), acier.getEs());
    }
    else if (config.choix == "Wood") {
        // Création d'un objet wood
        EC5::WoodCalculator wood;
        Young = std::vector<double>(config.spans.size(), wood.getWoodProperties(stringToWoodClass(config.woodClass)).E_0_mean);
    }

    // Début du calcul principal
    print("\n---------------------DEBUT DE CALCULE---------------------\n");
    auto start = std::chrono::high_resolution_clock::now();
    auto start_programme = start;

    // Create hyperstatic structure with the provided spans
    std::unique_ptr<traitement> hyp;
    if (config.inertieVariable) {
        hyp = std::make_unique<traitement>(config.spans, Young, config.Inertie_varier, config.pos_Inertie, config.division);
    } else {
        hyp = std::make_unique<traitement>(config.spans, Young, config.Inertie, config.division);
    }

    // Calculer et enregistrer le temps de calcul
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    nlohmann::json calculation_data;
    calculation_data["duration_ms"] = duration;
    calculation_data["material"] = config.choix;
    DATA["calculation_completed"] = calculation_data; 
    //logger.addEntry(calculation_data, "calculation_completed");

    // Affichage du temps de calcul
    std::cout << "Time taken : " << duration << " milli_secondes" << std::endl;
    print("---------------------FIN DE CALCULE---------------------\n");

    // Début de l'exportation des résultats
    print("\n---------------------DEBUT D'EXPORTATION DONNEE EN json---------------------\n");
    start = std::chrono::high_resolution_clock::now();

    // Création du dossier data s'il n'existe pas
    std::filesystem::create_directories(dataPath);

    // Exportation des résultats au format JSON
    std::cout << lieuDossier(dataPath) << std::endl;
    hyp->export_donnee(dataPath + "/results");

    // Calculer et enregistrer le temps d'exportation
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    nlohmann::json export_data;
    export_data["duration_ms"] = duration;
    export_data["export_path"] = dataPath + "/results";
    DATA["data_exported"] = export_data; 
    //logger.addEntry(export_data, "data_exported");

    // Affichage du temps d'exportation
    std::cout << "Time taken : " << duration << " milli_secondes" << std::endl;
    print("---------------------FIN D'EXPORTATION DONNEE EN json---------------------\n");

    // Calculer et enregistrer le temps total d'exécution
    auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_programme).count();
    
    nlohmann::json total_data;
    total_data["total_duration_ms"] = total_duration;
    DATA["total_duration_ms"] = total_duration; 
    //logger.addEntry(total_data, "execution_completed");

    logger.addEntry(DATA, "");

    std::cout << "Time taken total: " << total_duration << " milli_secondes" << std::endl;

    return 0;
}
