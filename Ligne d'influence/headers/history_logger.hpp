#ifndef HISTORY_LOGGER_HPP
#define HISTORY_LOGGER_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>


class HistoryLogger {
public:
    explicit HistoryLogger(const std::string& logFile = "data/history.json");

    // Ajouter une entrée dans l'historique
    void addEntry(const nlohmann::json& data, const std::string& entryType = "calculation");

    // Obtenir l'historique complet
    nlohmann::json getHistory() const;

    // Obtenir l'historique filtré par type
    nlohmann::json getHistoryByType(const std::string& entryType) const;

    // Effacer l'historique
    void clearHistory();

private:
    std::string logFile;
    std::vector<nlohmann::json> history;

    // Charger l'historique depuis le fichier
    void loadHistory();

    // Sauvegarder l'historique dans le fichier
    void saveHistory() const;

    // Obtenir l'horodatage actuel au format ISO
    static std::string getCurrentTimestamp();
};

#endif // HISTORY_LOGGER_HPP 