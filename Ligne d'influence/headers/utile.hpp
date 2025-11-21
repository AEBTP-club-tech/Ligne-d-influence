#ifndef __UTILE__
#define __UTILE__

#include <string>
#include <vector>
#include <iostream>
#include <any>
#include <map>
#include <sstream>
#include <regex>

#include <sys/stat.h> // Nécessaire pour la création de dossiers

#ifdef _WIN32
#include <direct.h> 
    #define MKDIR(dir) _mkdir(dir) // Macro pour la création de dossier sous Windows
#else
    #define MKDIR(dir) mkdir(dir, 0755) // Macro pour la création de dossier sous Unix/Linux avec permissions 0755 (rwxr-xr-x)
#endif 

/**
 * @brief Fonction d'entrée utilisateur avec un message simple
 * @param message Le message à afficher à l'utilisateur
 * @return La chaîne de caractères saisie par l'utilisateur
 */
std::string input(const std::string& message);

/**
 * @brief Fonction d'entrée utilisateur avec un message complexe (vecteur de valeurs)
 * @param message Vecteur contenant les messages à afficher
 * @return La chaîne de caractères saisie par l'utilisateur
 */
std::string input(const std::vector<std::any>& message);

/**
 * @brief Convertit une valeur numérique en chaîne de caractères
 * @tparam T Type de la valeur à convertir
 * @param number La valeur à convertir
 * @return La chaîne de caractères représentant la valeur
 */
template <typename T>
std::string str(T number);

/**
 * @brief Convertit une valeur en entier
 * @tparam T Type de la valeur à convertir
 * @param number La valeur à convertir
 * @return L'entier converti
 */
template <typename T>
int Int(T number);

/**
 * @brief Convertit une valeur en nombre à virgule flottante
 * @tparam T Type de la valeur à convertir
 * @param number La valeur à convertir
 * @return Le nombre à virgule flottante converti
 */
template <typename T>
float Float(T number);

/**
 * @brief Fonction de formatage de chaîne de caractères style Python f-string
 * @tparam Args Types des arguments variables
 * @param format Le format de la chaîne avec des placeholders {}
 * @param args Les arguments à insérer dans le format
 * @return La chaîne formatée
 */
template<typename... Args>
std::string f(const std::string& format, Args... args);

/**
 * @brief Version sans arguments de la fonction f
 * @param format Le format de la chaîne
 * @return La chaîne non formatée
 */
template<typename... Args>
std::string f(const std::string& format);

/**
 * @brief Version récursive de la fonction f pour traiter les arguments un par un
 * @tparam T Type du premier argument
 * @tparam Args Types des arguments restants
 * @param format Le format de la chaîne
 * @param value La première valeur à insérer
 * @param args Les arguments restants
 * @return La chaîne formatée
 */
template<typename T, typename... Args>
std::string f(const std::string& format, T value, Args... args);

/**
 * @brief Affiche le contenu d'un vecteur de chaînes de caractères
 * @param liste Le vecteur à afficher
 */
void print(const std::vector<std::string>& liste);

/**
 * @brief Affiche le contenu d'un vecteur de nombres à virgule flottante
 * @param liste Le vecteur à afficher
 */
void print(const std::vector<float>& liste);

/**
 * @brief Affiche le contenu d'un vecteur d'entiers
 * @param liste Le vecteur à afficher
 */
void print(const std::vector<int>& liste);

/**
 * @brief Affiche le contenu d'un vecteur de nombres doubles
 * @param liste Le vecteur à afficher
 */
void print(const std::vector<double>& liste);

/**
 * @brief Affiche le contenu d'une matrice de nombres à virgule flottante
 * @param liste La matrice à afficher
 */
void print(const std::vector<std::vector<float>>& liste);

/**
 * @brief Affiche le contenu d'une matrice d'entiers
 * @param liste La matrice à afficher
 */
void print(const std::vector<std::vector<int>>& liste);

/**
 * @brief Affiche le contenu d'une matrice de nombres doubles
 * @param liste La matrice à afficher
 */
void print(const std::vector<std::vector<double>>& liste);  

/**
 * @brief Affiche le contenu d'une matrice de chaînes de caractères
 * @param liste La matrice à afficher
 */
void print(const std::vector<std::vector<std::string>>& liste);

/**
 * @brief Affiche le contenu d'un vecteur de valeurs hétérogènes
 * @param liste Le vecteur à afficher
 */
void print(const std::vector<std::any>& liste);

/**
 * @brief Affiche une chaîne de caractères
 * @param message La chaîne à afficher
 */
void print(const std::string& message);

/**
 * @brief Affiche le contenu d'une map de chaînes vers nombres à virgule flottante
 * @param dictionnaire La map à afficher
 */
void print(const std::map<std::string, float>& dictionnaire);

/**
 * @brief Affiche le contenu d'une map de chaînes vers nombres doubles
 * @param dictionnaire La map à afficher
 */
void print(const std::map<std::string, double>& dictionnaire);

/**
 * @brief Écrit un vecteur de nombres doubles dans un fichier texte
 * @param name_fils Le nom du fichier
 * @param vecteur Le vecteur à écrire
 */
void in_text(std::string name_fils, std::vector<double> vecteur);

/**
 * @brief Écrit un vecteur de nombres doubles dans un fichier texte avec un message
 * @param name_fils Le nom du fichier
 * @param vecteur Le vecteur à écrire
 * @param message Le message à ajouter
 */
void in_text(std::string name_fils, std::vector<double> vecteur, std::string message);

/**
 * @brief Écrit une matrice de nombres doubles dans un fichier texte avec un message
 * @param name_fils Le nom du fichier
 * @param vecteur La matrice à écrire
 * @param message Le message à ajouter
 */
void in_text(std::string name_fils, std::vector<std::vector<double>> vecteur, std::string message);

/**
 * @brief Vérifie l'existence d'un fichier et le crée si nécessaire
 * @param path_fils Le chemin du fichier
 */
void files(std::string path_fils); 

/**
 * @brief Écrit un vecteur de nombres doubles dans un fichier CSV avec un message
 * @param filename Le nom du fichier
 * @param data Le vecteur à écrire
 * @param message Le message à ajouter
 */
void in_csv(const std::string& filename, const std::vector<double>& data, const std::string& message = "");

/**
 * @brief Écrit une matrice de nombres doubles dans un fichier CSV avec un message
 * @param filename Le nom du fichier
 * @param data La matrice à écrire
 * @param message Le message à ajouter
 */
void in_csv(const std::string& filename, const std::vector<std::vector<double>>& data, const std::string& message = "");

/**
 * @brief Écrit un tenseur 3D de nombres doubles dans un fichier CSV avec un message
 * @param filename Le nom du fichier
 * @param data Le tenseur à écrire
 * @param message Le message à ajouter
 */
void in_csv(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data, const std::string& message = "");

/**
 * @brief Écrit une map de vecteurs de nombres doubles dans un fichier CSV
 * @param filename Le nom du fichier
 * @param data La map à écrire
 * @param message Le message à ajouter (optionnel)
 */
void in_csv(const std::string& filename, const std::map<std::string, std::vector<double>>& data, const std::string& message = ""); 

/**
 * @brief Écrit une map de nombres doubles dans un fichier CSV
 * @param filename Le nom du fichier
 * @param data La map à écrire
 * @param message Le message à ajouter (optionnel)
 */
void in_csv(const std::string& filename, const std::map<std::string,double>& data, const std::string& message = ""); 

/**
 * @brief Écrit un vecteur de nombres doubles dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data Le vecteur à écrire
 */
void in_json(const std::string& filename, const std::vector<double>& data);

/**
 * @brief Écrit une matrice de nombres doubles dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data La matrice à écrire
 */
void in_json(const std::string& filename, const std::vector<std::vector<double>>& data);

/**
 * @brief Écrit un tenseur 3D de nombres doubles dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data Le tenseur à écrire
 */
void in_json(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data);

/**
 * @brief Écrit une map de vecteurs de nombres doubles dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data La map à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, std::vector<double>>& data);

/**
 * @brief Écrit une map de nombres doubles dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data La map à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, double>& data);

/**
 * @brief Écrit une map de vecteurs de maps dans un fichier JSON
 * @param filename Le nom du fichier
 * @param data La map à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, std::vector<std::map<std::string, double>>>& data);

/**
 * @brief Effectue une opération d'addition ou de soustraction entre deux vecteurs
 * @param A Premier vecteur
 * @param B Deuxième vecteur
 * @param plus true pour addition, false pour soustraction
 * @return Le vecteur résultant de l'opération
 */
std::vector<double> sum_vect(std::vector<double>& A , std::vector<double>& B, bool plus); 

// Implémentations inline des fonctions de conversion
template <typename T>
inline std::string str(T number) { return std::to_string(number); }

template <typename T>
inline int Int(T number) { return std::stoi(number); }

template <typename T>
inline float Float(T number) { return std::stof(number); }

template <typename T>
inline double Double(T number) {return std::stod(number);} 

/**
 * @brief Vérifie si un dossier existe
 * @param chemin Le chemin du dossier à vérifier
 * @return true si le dossier existe, false sinon
 */
bool dossierExiste(const std::string& chemin);

/**
 * @brief Crée un dossier s'il n'existe pas
 * @param chemin Le chemin du dossier à créer
 */
void creeDossier(const std::string& chemin);

/**
 * @brief Détermine le chemin absolu d'un dossier
 * @param chemin Le chemin relatif du dossier
 * @return Le chemin absolu du dossier
 */
std::string lieuDossier(const std::string& chemin);

/**
 * @brief Obtient le chemin absolu du dossier contenant l'exécutable
 * @return Le chemin absolu du dossier de l'exécutable
 */
std::string getExecutablePath();

#endif
