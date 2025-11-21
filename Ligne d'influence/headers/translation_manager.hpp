#ifndef __TRANSLATION_MANAGER_HPP__
#define __TRANSLATION_MANAGER_HPP__

#include <string>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>

class TranslationManager {
private:
    static std::unique_ptr<TranslationManager> instance;
    nlohmann::json translations;
    std::string current_language;

    TranslationManager() : current_language("en") {
        // Charger le fichier de traduction
        std::ifstream file("src/translation/translation.json");
        if (file.is_open()) {
            file >> translations;
        }
    }

public:
    static TranslationManager& getInstance() {
        if (!instance) {
            instance = std::make_unique<TranslationManager>();
        }
        return *instance;
    }

    void setLanguage(const std::string& lang) {
        if (lang == "fr" || lang == "en") {
            current_language = lang;
        }
    }

    std::string getDirectoryName(const std::string& key) const {
        try {
            return translations["directories"][key][current_language].get<std::string>();
        } catch (...) {
            return key; // Retourne la clé si la traduction n'est pas trouvée
        }
    }

    std::string getFileName(const std::string& directory, const std::string& key) const {
        try {
            return translations["files"][directory][key][current_language].get<std::string>();
        } catch (...) {
            return key; // Retourne la clé si la traduction n'est pas trouvée
        }
    }

    std::string getFullPath(const std::string& directory, const std::string& file) const {
        return getDirectoryName(directory) + "/" + getFileName(directory, file);
    }
};

std::unique_ptr<TranslationManager> TranslationManager::instance = nullptr;

#endif // __TRANSLATION_MANAGER_HPP__ 