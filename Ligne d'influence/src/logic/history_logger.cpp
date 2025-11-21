#include "history_logger.hpp"
#include <iomanip>
#include <sstream>
#include <fstream>

HistoryLogger::HistoryLogger(const std::string& logFile) : logFile(logFile) {
    // Créer le dossier data s'il n'existe pas
    std::filesystem::create_directories(std::filesystem::path(logFile).parent_path());
    loadHistory();
}

void HistoryLogger::addEntry(const nlohmann::json& data, const std::string& entryType) {
    nlohmann::json entry;
    entry["timestamp"] = getCurrentTimestamp();
    entry["type"] = entryType;
    entry["data"] = data;

    history.push_back(entry);
    saveHistory();
}

nlohmann::json HistoryLogger::getHistory() const {
    return nlohmann::json(history);
}

nlohmann::json HistoryLogger::getHistoryByType(const std::string& entryType) const {
    std::vector<nlohmann::json> filtered;
    std::copy_if(history.begin(), history.end(), std::back_inserter(filtered),
                 [&entryType](const nlohmann::json& entry) {
                     return entry["type"] == entryType;
                 });
    return nlohmann::json(filtered);
}

void HistoryLogger::clearHistory() {
    history.clear();
    saveHistory();
}

void HistoryLogger::loadHistory() {
    try {
        if (std::filesystem::exists(logFile)) {
            std::ifstream file(logFile);
            nlohmann::json j = nlohmann::json::parse(file);
            history = j.get<std::vector<nlohmann::json>>();
        }
    } catch (const std::exception& e) {
        // En cas d'erreur, on commence avec un historique vide
        history.clear();
    }
}

void HistoryLogger::saveHistory() const {
    std::ofstream file(logFile);
    nlohmann::json j(history);
    file << std::setw(4) << j << std::endl;
}

std::string HistoryLogger::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm timeinfo;
    localtime_s(&timeinfo, &now_c);

    std::stringstream ss;
    ss << std::put_time(&timeinfo, "%Y-%m-%dT%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << now_ms.count();

    return ss.str();
} 