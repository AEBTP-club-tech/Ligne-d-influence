#include "json_handler.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>

/**
 * @brief Charge un fichier JSON depuis le disque
 * @param filename Le chemin du fichier à charger
 * @return Un objet json contenant les données du fichier
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
json JsonHandler::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
    
    json data;
    file >> data;
    return data;
}

/**
 * @brief Sauvegarde un objet JSON dans un fichier
 * @param data L'objet json à sauvegarder
 * @param filename Le chemin du fichier de destination
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::saveToFile(const json& data, const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
    
    file << data.dump(4); // 4 est le nombre d'espaces pour l'indentation
}

/**
 * @brief Parse une chaîne de caractères en objet JSON
 * @param jsonString La chaîne de caractères à parser
 * @return Un objet json correspondant à la chaîne d'entrée
 */
json JsonHandler::parseString(const std::string& jsonString) {
    return json::parse(jsonString);
}

/**
 * @brief Convertit un objet JSON en chaîne de caractères formatée
 * @param data L'objet json à convertir
 * @return Une chaîne de caractères représentant l'objet JSON
 */
std::string JsonHandler::toString(const json& data) {
    return data.dump(4);
}

/**
 * @brief Sauvegarde des données structurées dans un fichier JSON
 * @param data Une map contenant des vecteurs de doubles
 * @param filename Le chemin du fichier de destination
 * 
 * Cette fonction est optimisée pour sauvegarder des données numériques
 * organisées en paires clé-valeur, où chaque valeur est un vecteur de doubles.
 */
void JsonHandler::saveData(const std::map<std::string, std::vector<double>>& data, const std::string& filename) {
    json j;
    for (const auto& [key, values] : data) {
        j[key] = values;
    }
    saveToFile(j, filename);
}

/**
 * @brief Charge des données structurées depuis un fichier JSON
 * @param filename Le chemin du fichier à charger
 * @return Une map contenant les données chargées
 * 
 * Cette fonction est l'inverse de saveData, elle reconstruit la structure
 * de données originale à partir du fichier JSON.
 */
std::map<std::string, std::vector<double>> JsonHandler::loadData(const std::string& filename) {
    json j = loadFromFile(filename);
    std::map<std::string, std::vector<double>> result;
    
    for (auto it = j.begin(); it != j.end(); ++it) {
        result[it.key()] = it.value().get<std::vector<double>>();
    }
    
    return result;
}

/**
 * @brief Sauvegarde une matrice de données dans un fichier JSON
 * @param data Un vecteur de vecteurs de doubles (matrice)
 * @param filename Le chemin du fichier de destination
 */
void JsonHandler::saveStructuredData(const std::vector<std::vector<double>>& data, const std::string& filename) {
    json j = data;
    saveToFile(j, filename);
}

/**
 * @brief Charge une matrice de données depuis un fichier JSON
 * @param filename Le chemin du fichier à charger
 * @return Une matrice de doubles
 */
std::vector<std::vector<double>> JsonHandler::loadStructuredData(const std::string& filename) {
    json j = loadFromFile(filename);
    return j.get<std::vector<std::vector<double>>>();
}

/**
 * @brief Sauvegarde un vecteur simple de données dans un fichier JSON
 * @param data Un vecteur de doubles
 * @param filename Le chemin du fichier de destination
 */
void JsonHandler::saveSimpleData(const std::vector<double>& data, const std::string& filename) {
    json j = data;
    saveToFile(j, filename);
}

/**
 * @brief Charge un vecteur simple de données depuis un fichier JSON
 * @param filename Le chemin du fichier à charger
 * @return Un vecteur de doubles
 */
std::vector<double> JsonHandler::loadSimpleData(const std::string& filename) {
    json j = loadFromFile(filename);
    return j.get<std::vector<double>>();
}

/**
 * @brief Ajoute une indentation au flux de sortie
 * @param out Le flux de sortie
 * @param level Le niveau d'indentation (nombre d'espaces)
 */
void JsonHandler::indent(std::ostream& out, int level) {
    for (int i = 0; i < level; ++i) {
        out << "    ";
    }
}

/**
 * @brief Écrit une valeur numérique dans le flux de sortie
 * @param out Le flux de sortie
 * @param value La valeur à écrire
 * 
 * Gère les cas spéciaux comme NaN et Inf en les convertissant en "null"
 */
void JsonHandler::writeValue(std::ostream& out, double value) {
    if (std::isnan(value) || std::isinf(value)) {
        out << "null";
    } else {
        out << std::setprecision(std::numeric_limits<double>::digits10 + 1) << value;
    }
}

/**
 * @brief Écrit une chaîne de caractères dans le flux de sortie
 * @param out Le flux de sortie
 * @param str La chaîne à écrire
 * 
 * Ajoute les guillemets nécessaires pour le format JSON
 */
void JsonHandler::writeString(std::ostream& out, const std::string& str) {
    out << "\"" << str << "\"";
}

/**
 * @brief Sérialise un vecteur de doubles en JSON
 * @param data Le vecteur à sérialiser
 * @param out Le flux de sortie
 */
void JsonHandler::serialize(const std::vector<double>& data, std::ostream& out) {
    out << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        writeValue(out, data[i]);
        if (i < data.size() - 1) {
            out << ",";
        }
    }
    out << "]";
}

/**
 * @brief Sérialise une matrice de doubles en JSON
 * @param data La matrice à sérialiser
 * @param out Le flux de sortie
 */
void JsonHandler::serialize(const std::vector<std::vector<double>>& data, std::ostream& out) {
    out << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        out << "\n    ";
        serialize(data[i], out);
        if (i < data.size() - 1) {
            out << ",";
        }
    }
    out << "\n]";
}

/**
 * @brief Sérialise un tenseur 3D de doubles en JSON
 * @param data Le tenseur à sérialiser
 * @param out Le flux de sortie
 */
void JsonHandler::serialize(const std::vector<std::vector<std::vector<double>>>& data, std::ostream& out) {
    out << "[";
    for (size_t i = 0; i < data.size(); ++i) {
        out << "\n    ";
        serialize(data[i], out);
        if (i < data.size() - 1) {
            out << ",";
        }
    }
    out << "\n]";
}

/**
 * @brief Sérialise une map de vecteurs de doubles en JSON
 * @param data La map à sérialiser
 * @param out Le flux de sortie
 */
void JsonHandler::serialize(const std::map<std::string, std::vector<double>>& data, std::ostream& out) {
    out << "{";
    size_t i = 0;
    for (const auto& [key, values] : data) {
        out << "\n    ";
        writeString(out, key);
        out << ": ";
        serialize(values, out);
        if (++i < data.size()) {
            out << ",";
        }
    }
    out << "\n}";
}

/**
 * @brief Sérialise une map de doubles en JSON
 * @param data La map à sérialiser
 * @param out Le flux de sortie
 */
void JsonHandler::serialize(const std::map<std::string, double>& data, std::ostream& out) {
    out << "{";
    size_t i = 0;
    for (const auto& [key, value] : data) {
        out << "\n    ";
        writeString(out, key);
        out << ": ";
        writeValue(out, value);
        if (++i < data.size()) {
            out << ",";
        }
    }
    out << "\n}";
}

/**
 * @brief Écrit un vecteur de doubles dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data Le vecteur à écrire
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::writeToFile(const std::string& filename, const std::vector<double>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        serialize(data, file);
        file.close();
    } else {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
}

/**
 * @brief Écrit une matrice de doubles dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data La matrice à écrire
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::writeToFile(const std::string& filename, const std::vector<std::vector<double>>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        serialize(data, file);
        file.close();
    } else {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
}

/**
 * @brief Écrit un tenseur 3D de doubles dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data Le tenseur à écrire
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::writeToFile(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        serialize(data, file);
        file.close();
    } else {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
}

/**
 * @brief Écrit une map de vecteurs de doubles dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data La map à écrire
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::writeToFile(const std::string& filename, const std::map<std::string, std::vector<double>>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        serialize(data, file);
        file.close();
    } else {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
}

/**
 * @brief Écrit une map de doubles dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data La map à écrire
 * @throws std::runtime_error Si le fichier ne peut pas être ouvert
 */
void JsonHandler::writeToFile(const std::string& filename, const std::map<std::string, double>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
        serialize(data, file);
        file.close();
    } else {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
}

void JsonHandler::writeToFile(const std::string& filename, 
    const std::map<std::string, std::map<std::string, std::vector<double>>>& data) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    file << "{\n";
    size_t count = 0;
    for (const auto& [outer_key, inner_map] : data) {
        file << "    \"" << outer_key << "\": {\n";
        size_t inner_count = 0;
        for (const auto& [inner_key, values] : inner_map) {
            file << "        \"" << inner_key << "\": [";
            for (size_t i = 0; i < values.size(); ++i) {
                file << values[i];
                if (i < values.size() - 1) {
                    file << ", ";
                }
            }
            file << "]";
            if (++inner_count < inner_map.size()) {
                file << ",\n";
            } else {
                file << "\n";
            }
        }
        file << "    }";
        if (++count < data.size()) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }
    file << "}\n";
}

void JsonHandler::writeToFile(const std::string& filename, 
    const std::map<std::string, std::vector<std::map<std::string, double>>>& data) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filename);
    }

    file << "{\n";
    size_t count = 0;
    for (const auto& [outer_key, vector_of_maps] : data) {
        file << "    \"" << outer_key << "\": [\n";
        size_t vector_count = 0;
        for (const auto& map_data : vector_of_maps) {
            file << "        {";
            size_t map_count = 0;
            for (const auto& [key, value] : map_data) {
                file << "\"" << key << "\": " << value;
                if (++map_count < map_data.size()) {
                    file << ", ";
                }
            }
            file << "}";
            if (++vector_count < vector_of_maps.size()) {
                file << ",\n";
            } else {
                file << "\n";
            }
        }
        file << "    ]";
        if (++count < data.size()) {
            file << ",\n";
        } else {
            file << "\n";
        }
    }
    file << "}\n";
}

/**
 * @brief Formate et affiche les données JSON de manière lisible
 * @param data Les données JSON à afficher
 * @param indent_size La taille de l'indentation (par défaut 2)
 * @param precision La précision des nombres à virgule flottante (par défaut 6)
 * 
 * Cette fonction formate les données JSON de manière à les rendre plus lisibles
 * en ajoutant des sauts de ligne et des indentations appropriés.
 */
void JsonHandler::prettyPrint(const json& data, int indent_size, int precision) {
    std::cout << std::setprecision(precision);
    std::cout << data.dump(indent_size) << std::endl;
}

/**
 * @brief Formate et affiche les données JSON de manière lisible dans un fichier
 * @param filename Le chemin du fichier de destination
 * @param data Les données JSON à afficher
 * @param indent_size La taille de l'indentation (par défaut 2)
 * @param precision La précision des nombres à virgule flottante (par défaut 6)
 * 
 * Cette fonction écrit les données JSON formatées dans un fichier.
 */
void JsonHandler::prettyPrintToFile(const std::string& filename, const json& data, int indent_size, int precision) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier: " + filename);
    }
    
    file << std::setprecision(precision);
    file << data.dump(indent_size) << std::endl;
} 