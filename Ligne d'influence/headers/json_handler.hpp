#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @class JsonHandler
 * @brief Classe utilitaire pour la manipulation de données JSON
 * 
 * Cette classe fournit des méthodes statiques pour charger, sauvegarder et manipuler
 * des données au format JSON. Elle utilise la bibliothèque nlohmann/json pour le parsing
 * et la sérialisation des données.
 */
class JsonHandler {
public:
    /**
     * @brief Charge un fichier JSON depuis le disque
     * @param filename Le chemin du fichier à charger
     * @return Un objet json contenant les données du fichier
     * @throws std::runtime_error si le fichier ne peut pas être lu
     */
    static json loadFromFile(const std::string& filename);
    
    /**
     * @brief Sauvegarde des données JSON dans un fichier
     * @param data Les données JSON à sauvegarder
     * @param filename Le chemin du fichier de destination
     * @throws std::runtime_error si le fichier ne peut pas être écrit
     */
    static void saveToFile(const json& data, const std::string& filename);
    
    /**
     * @brief Parse une chaîne de caractères en objet JSON
     * @param jsonString La chaîne JSON à parser
     * @return Un objet json contenant les données parsées
     * @throws nlohmann::json::parse_error si la chaîne n'est pas un JSON valide
     */
    static json parseString(const std::string& jsonString);
    
    /**
     * @brief Convertit un objet JSON en chaîne de caractères
     * @param data L'objet JSON à convertir
     * @return Une chaîne de caractères représentant le JSON
     */
    static std::string toString(const json& data);

    /**
     * @brief Sauvegarde des données sous forme de map de vecteurs dans un fichier JSON
     * @param data Les données à sauvegarder (clé -> vecteur de valeurs)
     * @param filename Le chemin du fichier de destination
     * 
     * Cette méthode est optimisée pour les structures de données complexes
     * où chaque clé est associée à un vecteur de valeurs numériques.
     */
    static void saveData(const std::map<std::string, std::vector<double>>& data, const std::string& filename);
    
    /**
     * @brief Charge des données depuis un fichier JSON dans une map de vecteurs
     * @param filename Le chemin du fichier à charger
     * @return Une map contenant les données chargées
     */
    static std::map<std::string, std::vector<double>> loadData(const std::string& filename);
    
    /**
     * @brief Sauvegarde des données structurées (matrice 2D) dans un fichier JSON
     * @param data La matrice de données à sauvegarder
     * @param filename Le chemin du fichier de destination
     * 
     * Cette méthode est optimisée pour les matrices de données numériques
     * où chaque ligne est représentée par un vecteur de valeurs.
     */
    static void saveStructuredData(const std::vector<std::vector<double>>& data, const std::string& filename);
    
    /**
     * @brief Charge des données structurées depuis un fichier JSON
     * @param filename Le chemin du fichier à charger
     * @return Une matrice contenant les données chargées
     */
    static std::vector<std::vector<double>> loadStructuredData(const std::string& filename);
    
    /**
     * @brief Sauvegarde un vecteur simple de données dans un fichier JSON
     * @param data Le vecteur de données à sauvegarder
     * @param filename Le chemin du fichier de destination
     */
    static void saveSimpleData(const std::vector<double>& data, const std::string& filename);
    
    /**
     * @brief Charge un vecteur simple de données depuis un fichier JSON
     * @param filename Le chemin du fichier à charger
     * @return Un vecteur contenant les données chargées
     */
    static std::vector<double> loadSimpleData(const std::string& filename);

    /**
     * @brief Ajoute une indentation au flux de sortie
     * @param out Le flux de sortie à indenter
     * @param level Le niveau d'indentation souhaité
     * 
     * Utilisé pour formater le JSON de manière lisible
     */
    static void indent(std::ostream& out, int level);
    
    /**
     * @brief Écrit une valeur numérique dans le flux de sortie
     * @param out Le flux de sortie
     * @param value La valeur à écrire
     * 
     * Gère le formatage des nombres à virgule flottante
     */
    static void writeValue(std::ostream& out, double value);
    
    /**
     * @brief Écrit une chaîne de caractères dans le flux de sortie
     * @param out Le flux de sortie
     * @param str La chaîne à écrire
     * 
     * Gère l'échappement des caractères spéciaux
     */
    static void writeString(std::ostream& out, const std::string& str);

    /**
     * @brief Sérialise un vecteur de données dans un flux de sortie
     * @param data Les données à sérialiser
     * @param out Le flux de sortie
     */
    static void serialize(const std::vector<double>& data, std::ostream& out);
    
    /**
     * @brief Sérialise une matrice de données dans un flux de sortie
     * @param data Les données à sérialiser
     * @param out Le flux de sortie
     */
    static void serialize(const std::vector<std::vector<double>>& data, std::ostream& out);
    
    /**
     * @brief Sérialise un tenseur 3D de données dans un flux de sortie
     * @param data Les données à sérialiser
     * @param out Le flux de sortie
     */
    static void serialize(const std::vector<std::vector<std::vector<double>>>& data, std::ostream& out);
    
    /**
     * @brief Sérialise une map de vecteurs dans un flux de sortie
     * @param data Les données à sérialiser
     * @param out Le flux de sortie
     */
    static void serialize(const std::map<std::string, std::vector<double>>& data, std::ostream& out);
    
    /**
     * @brief Sérialise une map de valeurs simples dans un flux de sortie
     * @param data Les données à sérialiser
     * @param out Le flux de sortie
     */
    static void serialize(const std::map<std::string, double>& data, std::ostream& out);

    /**
     * @brief Écrit un vecteur de données dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     */
    static void writeToFile(const std::string& filename, const std::vector<double>& data);
    
    /**
     * @brief Écrit une matrice de données dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     */
    static void writeToFile(const std::string& filename, const std::vector<std::vector<double>>& data);
    
    /**
     * @brief Écrit un tenseur 3D de données dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     */
    static void writeToFile(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data);
    
    /**
     * @brief Écrit une map de vecteurs dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     */
    static void writeToFile(const std::string& filename, const std::map<std::string, std::vector<double>>& data);
    
    /**
     * @brief Écrit une map de valeurs simples dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     */
    static void writeToFile(const std::string& filename, const std::map<std::string, double>& data);
    
    /**
     * @brief Écrit une map de maps de vecteurs dans un fichier
     * @param filename Le chemin du fichier de destination
     * @param data Les données à écrire
     * 
     * Cette méthode gère les structures de données complexes imbriquées
     * où chaque clé de premier niveau pointe vers une map de vecteurs.
     */
    static void writeToFile(const std::string& filename, const std::map<std::string, std::map<std::string, std::vector<double>>>& data);

    static void writeToFile(const std::string& filename, 
        const std::map<std::string, std::vector<std::map<std::string, double>>>& data);

    /**
     * @brief Affiche un objet JSON de manière formatée
     * @param data L'objet JSON à afficher
     * @param indent_size Taille de l'indentation
     * @param precision Précision pour les nombres à virgule flottante
     */
    static void prettyPrint(const json& data, int indent_size = 4, int precision = 6);

    /**
     * @brief Sauvegarde un objet JSON dans un fichier avec un formatage personnalisé
     * @param filename Le chemin du fichier de destination
     * @param data L'objet JSON à sauvegarder
     * @param indent_size Taille de l'indentation
     * @param precision Précision pour les nombres à virgule flottante
     */
    static void prettyPrintToFile(const std::string& filename, const json& data, int indent_size = 4, int precision = 6);
}; 