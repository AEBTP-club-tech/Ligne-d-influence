#include "utile.hpp" 
#include <string>
#include <iostream>
#include <vector>
#include <iostream>
#include <any>
#include <map>
#include <iomanip> // Pour std::setprecision
#include <regex>
#include <sstream>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace {
    /**
     * @brief Fonction template pour afficher un vecteur d'éléments
     * @tparam T Type des éléments du vecteur
     * @param liste Vecteur à afficher
     * 
     * Cette fonction affiche les éléments d'un vecteur dans le format [elem1, elem2, ...]
     * Gère le cas d'un vecteur vide en affichant simplement []
     */
    template<typename T>
    void print_vector(const std::vector<T>& liste) {
        if (liste.empty()) {
            std::cout << "[]";
            return;
        }
        
        std::cout << "[" << liste[0];
        for (size_t i = 1; i < liste.size(); ++i) {
            std::cout << " , " << liste[i];
        }
        std::cout << "]";
    }

    /**
     * @brief Fonction template pour afficher une matrice 2D
     * @tparam T Type des éléments de la matrice
     * @param liste Matrice à afficher
     * 
     * Cette fonction affiche une matrice dans le format:
     * {Debut:
     * [row1]
     * [row2]
     * ...
     * :Fin}
     */
    template<typename T>
    void print_vector_2d(const std::vector<std::vector<T>>& liste) {
        if (liste.empty()) {
            std::cout << "\n{Vide}\n";
            return;
        }
        
        std::cout << "\n{Debut:\n";
        for (const auto& row : liste) {
            print_vector(row);
            std::cout << '\n';
        }
        std::cout << ":Fin}\n";
    }

    /**
     * @brief Remplace le premier placeholder dans une chaîne de format
     * @param format Chaîne contenant des placeholders { }
     * @param value Valeur à insérer
     * @return Chaîne avec le premier placeholder remplacé
     * 
     * Utilise une expression régulière pour trouver et remplacer le premier placeholder
     */
    std::string replace_first_placeholder(const std::string& format, const std::string& value) {
        static const std::regex placeholder_pattern("\\{[^}]*\\}");
        return std::regex_replace(format, placeholder_pattern, value, std::regex_constants::format_first_only);
    }

    /**
     * @brief Convertit une valeur en chaîne avec une précision appropriée
     * @tparam T Type de la valeur à convertir
     * @param value Valeur à convertir
     * @return Chaîne représentant la valeur
     * 
     * Pour les nombres à virgule flottante, utilise une précision de 6 décimales
     */
    template<typename T>
    std::string to_string_with_precision(const T& value) {
        std::ostringstream ss;
        if constexpr (std::is_floating_point_v<T>) {
            ss << std::fixed << std::setprecision(6);
        }
        ss << value;
        return ss.str();
    }
}

/**
 * @brief Demande une entrée utilisateur avec un message
 * @param message Message à afficher
 * @return Chaîne saisie par l'utilisateur
 */
std::string input(const std::string& message) {
    std::cout << message;
    std::string input;
    std::getline(std::cin, input);
    return input;
}

/**
 * @brief Demande une entrée utilisateur avec plusieurs messages
 * @param message Vecteur de messages à afficher
 * @return Chaîne saisie par l'utilisateur
 * 
 * Affiche chaque élément du vecteur de messages, en gérant différents types (int, float, double, const char*)
 */
std::string input(const std::vector<std::any>& message) {
    for (const auto& element : message) {
        try {
            if (element.type() == typeid(int)) 
                std::cout << std::any_cast<int>(element) << ' ';
            else if (element.type() == typeid(const char*)) 
                std::cout << std::any_cast<const char*>(element) << ' ';
            else if (element.type() == typeid(float)) 
                std::cout << std::any_cast<float>(element) << ' ';
            else if (element.type() == typeid(double)) 
                std::cout << std::any_cast<double>(element) << ' ';
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Erreur de conversion: " << e.what() << '\n';
        }
    }
    
    std::string variable;
    std::getline(std::cin, variable);
    return variable;
}

// Surcharges de la fonction print pour différents types de vecteurs
void print(const std::vector<float>& liste) { print_vector(liste); }
void print(const std::vector<int>& liste) { print_vector(liste); }
void print(const std::vector<double>& liste) { print_vector(liste); }
void print(const std::vector<std::string>& liste) { print_vector(liste); }

// Surcharges de la fonction print pour différents types de matrices
void print(const std::vector<std::vector<float>>& liste) { print_vector_2d(liste); }
void print(const std::vector<std::vector<int>>& liste) { print_vector_2d(liste); }
void print(const std::vector<std::vector<double>>& liste) { print_vector_2d(liste); }
void print(const std::vector<std::vector<std::string>>& liste) { print_vector_2d(liste); }

/**
 * @brief Affiche un vecteur d'éléments de type any
 * @param liste Vecteur à afficher
 * 
 * Gère différents types d'éléments dans le vecteur (int, float, double, const char*)
 */
void print(const std::vector<std::any>& liste) {
    for (const auto& element : liste) {
        try {
            if (element.type() == typeid(int)) 
                std::cout << std::any_cast<int>(element) << ' ';
            else if (element.type() == typeid(const char*)) 
                std::cout << std::any_cast<const char*>(element) << ' ';
            else if (element.type() == typeid(float)) 
                std::cout << std::any_cast<float>(element) << ' ';
            else if (element.type() == typeid(double)) 
                std::cout << std::any_cast<double>(element) << ' ';
        } catch (const std::bad_any_cast& e) {
            std::cerr << "Erreur de conversion: " << e.what() << '\n';
        }
    }
}

void print(const std::string& message) {
    std::cout << message;
}

/**
 * @brief Affiche un dictionnaire (map) de valeurs
 * @tparam T Type des valeurs dans le dictionnaire
 * @param dictionnaire Map à afficher
 * @param nom Nom optionnel du dictionnaire
 * 
 * Affiche le dictionnaire dans le format:
 * {Debut:nom
 * "key1":value1,
 * "key2":value2,
 * ...
 * Fin:nom}
 */
template<typename T>
void print_map(const std::map<std::string, T>& dictionnaire, const std::string& nom = "") {
    if (dictionnaire.empty()) {
        std::cout << "\n{Dictionnaire vide}\n";
        return;
    }
    
    std::cout << "\n{Debut" << (nom.empty() ? "" : ":" + nom) << "\n";
    for (const auto& [key, value] : dictionnaire) {
        std::cout << std::fixed << std::setprecision(6)
                  << '"' << key << '"' << ":" << value << ",\n";
    }
    std::cout << "Fin" << (nom.empty() ? "" : ":" + nom) << "}\n";
}

// Surcharges de print pour différents types de dictionnaires
void print(const std::map<std::string, float>& dictionnaire) {
    print_map(dictionnaire);
}

void print(const std::map<std::string, double>& dictionnaire) {
    print_map(dictionnaire, "dictionnaire");
}

/**
 * @brief Fonction de formatage de chaîne (cas de base)
 * @tparam Args Types des arguments restants
 * @param format Chaîne de format
 * @return Chaîne formatée
 */
template<typename... Args>
std::string f(const std::string& format) {
    return format;
}

/**
 * @brief Fonction de formatage de chaîne (cas récursif)
 * @tparam T Type de la valeur courante
 * @tparam Args Types des arguments restants
 * @param format Chaîne de format
 * @param value Valeur courante à insérer
 * @param args Arguments restants
 * @return Chaîne formatée
 * 
 * Remplace les placeholders { } dans la chaîne de format par les valeurs fournies
 */
template<typename T, typename... Args>
std::string f(const std::string& format, T value, Args... args) {
    std::string str_value;
    
    if constexpr (std::is_same_v<T, std::string>) {
        str_value = value;
    } else {
        str_value = to_string_with_precision(value);
    }
    
    std::string new_format = replace_first_placeholder(format, str_value);
    return f(new_format, args...);
}

/**
 * @brief Écrit un vecteur dans un fichier texte
 * @param name_fils Nom du fichier
 * @param vecteur Vecteur à écrire
 * 
 * Écrit le vecteur dans le format [elem1, elem2, ...]
 */
void in_text(std::string name_fils, std::vector<double> vecteur)
{
    std::ofstream file{name_fils};
    if (file.is_open()) {
        if (vecteur.empty()) {
            file << "[] : tableau vide";
            return;
        }

        file << "[" << vecteur[0];
        for (size_t i = 1; i < vecteur.size(); ++i) {
            file << " , " << vecteur[i];
        }
        file << "]";
        file.close();
    }   
}

/**
 * @brief Écrit un vecteur avec un message dans un fichier texte
 * @param name_fils Nom du fichier
 * @param vecteur Vecteur à écrire
 * @param message Message à écrire avant le vecteur
 */
void in_text(std::string name_fils, std::vector<double> vecteur, std::string message)
{
    std::ofstream file{name_fils};
    if (file.is_open()) {
        file<<message<<"\n"; 
        if (vecteur.empty()) {
            file << "[] : tableau vide";
            return;
        }

        file << "[" << vecteur[0];
        for (size_t i = 1; i < vecteur.size(); ++i) {
            file << " , " << vecteur[i];
        }
        file << "]";
        file.close();
    } 
}

/**
 * @brief Écrit une matrice avec un message dans un fichier texte
 * @param name_fils Nom du fichier
 * @param vecteur Matrice à écrire
 * @param message Message à écrire avant la matrice
 */
void in_text(std::string name_fils, std::vector<std::vector<double>> vecteur, std::string message) {
    std::ofstream file{name_fils};
    if (file.is_open()) { 
        file << message <<"\n"; 
        if (vecteur.empty()) {
            file << "\n{Vide}\n";
            return;
        }
        
        file << "\n{Debut:\n";
        for (const auto& row : vecteur) {
            if (row.empty()) {
                file << "[] : tableau vide";
                return;
            }
    
            file << "[" << row[0];
            for (size_t i = 1; i < row.size(); ++i) {
                file << " , " << row[i];
            }
            file << "]";
            file << '\n';
        }
        file << ":Fin}\n";
    }
}

/**
 * @brief Vérifie l'existence d'un fichier et le crée s'il n'existe pas
 * @param path_fils Chemin du fichier
 */
void files(std::string path_fils)
{
    std::ifstream fileCheck(path_fils);

    if (!fileCheck) {
        //le fichier n'exite pas
        std::ofstream newFile(path_fils);

        if (newFile.is_open()) {
            newFile.close();
        }
        else {
            std::cout<<"Erreur :: Impossible de cree le fichier"<<std::endl;
        }
    }
    else {
        std::cout<<"Le fichier"<<path_fils<<"existe deja."<<std::endl;
        fileCheck.close();
    }
}

/**
 * @brief Écrit une matrice dans un fichier CSV
 * @param filename Nom du fichier
 * @param data Matrice à écrire
 * @param message Message optionnel à écrire en en-tête
 */
void in_csv(const std::string& filename, const std::vector<std::vector<double>>& data, const std::string& message) {
    files(filename); 
    std::ofstream file{filename};
    if (file.is_open()) {
        if (!message.empty()) {
            file << message << "\n";
        }
        for (const auto& row : data) {
            for (size_t i = 0; i < row.size(); ++i) {
                file << row[i];
                if (i < row.size() - 1) {
                    file << ",";
                }
            }
            file << "\n";
        }
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un tenseur 3D dans un fichier CSV
 * @param filename Nom du fichier
 * @param data Tenseur à écrire
 * @param message Message optionnel à écrire en en-tête
 */
void in_csv(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data, const std::string& message) {
    files(filename); 
    std::ofstream file{filename};
    if (file.is_open()) {
        if (!message.empty()) {
            file << message << "\n";
        }
        for (const auto& matrix : data) {
            for (const auto& row : matrix) {
                for (size_t i = 0; i < row.size(); ++i) {
                    file << row[i];
                    if (i < row.size() - 1) {
                        file << ",";
                    }
                }
                file << "\n";
            }
            file << "\n"; // Separate matrices with a blank line
        }
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un vecteur dans un fichier CSV
 * @param filename Nom du fichier
 * @param data Vecteur à écrire
 * @param message Message optionnel à écrire en en-tête
 */
void in_csv(const std::string& filename, const std::vector<double>& data, const std::string& message) {
    files(filename); 
    std::ofstream file{filename};
    if (file.is_open()) {
        if (!message.empty()) {
            file << message << "\n";
        }
        for (size_t i = 0; i < data.size(); ++i) {
            file << data[i];
            if (i < data.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un dictionnaire de vecteurs dans un fichier CSV
 * @param filename Nom du fichier
 * @param data Dictionnaire à écrire
 * @param message Message optionnel à écrire en en-tête
 */
void in_csv(const std::string &filename, const std::map<std::string, std::vector<double>>& data, const std::string& message)
{
    files(filename); 
    std::ofstream file{filename};
    if (file.is_open()) {
        if (!message.empty()) {
            file << message << "\n";
        }
        for (const auto& [key, values] : data) {
            file << key << ","; 
            for (size_t i = 0; i < values.size(); ++i) {
                file << values[i];
                if (i < values.size() - 1) {
                    file << ",";
                }
            }
            file<<"\n"; 
        }
        file.close();
    }  
    else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }   
}

/**
 * @brief Écrit un dictionnaire de valeurs simples dans un fichier CSV
 * @param filename Nom du fichier
 * @param data Dictionnaire à écrire
 * @param message Message optionnel à écrire en en-tête
 */
void in_csv(const std::string& filename, const std::map<std::string,double>& data, const std::string& message) {
    files(filename); 
    std::ofstream file{filename};
    if (file.is_open()) {
        if (!message.empty()) {
            file << message << "\n";
        }
        for (const auto& [key, values] : data) {
            file << key << "," << values << std::endl;
        }
        file.close();
    }  
    else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
} 

// Surcharges de in_csv sans message
void in_csv(const std::string& filename, const std::vector<double>& data) {
    in_csv(filename, data, "");
}

void in_csv(const std::string& filename, const std::vector<std::vector<double>>& data) {
    in_csv(filename, data, "");
}

void in_csv(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data) {
    in_csv(filename, data, "");
}

/**
 * @brief Vérifie si un dossier existe
 * @param chemin Chemin du dossier
 * @return true si le dossier existe, false sinon
 */
bool dossierExiste(const std::string &chemin)
{
    struct stat info;
    if (stat(chemin.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0; 
};

/**
 * @brief Crée un dossier s'il n'existe pas
 * @param chemin Chemin du dossier à créer
 */
void creeDossier(const std::string& chemin)
{
    if (!dossierExiste(chemin)) {
        int status = MKDIR(chemin.c_str());
        if (status == 0) {
            std::cout<<"Le dossier \""<<chemin<<"\" a ete cree avec succes."<<std::endl;
        }
        else {
            std::cerr<< "Erreur lors de la creation du dossier \"" << chemin << "\"."<<std::endl;
        }
    }
    else { 
        std::cout<< "Le dossier \"" << chemin << "\" existe deja." << std::endl;
    }
};

/**
 * @brief Vérifie l'accessibilité d'un dossier
 * @param chemin Chemin du dossier
 * @return Message indiquant si le dossier existe et est accessible
 */
std::string lieuDossier(const std::string& chemin) {
    if (dossierExiste(chemin)) {
        return "Le dossier \"" + chemin + "\" existe et est accessible.";
    } else {
        return "Le dossier \"" + chemin + "\" n'existe pas ou n'est pas accessible.";
    }
}

// Instanciations explicites des templates
template std::string f<std::string, std::string>(const std::string&, std::string, std::string);
template std::string f<int, int>(const std::string&, int, int);
template std::string f<double, double>(const std::string&, double, double);
template std::string f<const char*, const char*>(const std::string&, const char*, const char*);

/**
 * @brief Écrit un vecteur dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Vecteur à écrire
 */
void in_json(const std::string& filename, const std::vector<double>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            file << std::fixed << std::setprecision(6) << data[i];
            if (i < data.size() - 1) {
                file << ",";
            }
        }
        file << "]";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit une matrice dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Matrice à écrire
 */
void in_json(const std::string& filename, const std::vector<std::vector<double>>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            file << "[";
            for (size_t j = 0; j < data[i].size(); ++j) {
                file << std::fixed << std::setprecision(6) << data[i][j];
                if (j < data[i].size() - 1) {
                    file << ",";
                }
            }
            file << "]";
            if (i < data.size() - 1) {
                file << ",";
            }
        }
        file << "]";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un tenseur 3D dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Tenseur à écrire
 */
void in_json(const std::string& filename, const std::vector<std::vector<std::vector<double>>>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "[";
        for (size_t i = 0; i < data.size(); ++i) {
            file << "[";
            for (size_t j = 0; j < data[i].size(); ++j) {
                file << "[";
                for (size_t k = 0; k < data[i][j].size(); ++k) {
                    file << std::fixed << std::setprecision(6) << data[i][j][k];
                    if (k < data[i][j].size() - 1) {
                        file << ",";
                    }
                }
                file << "]";
                if (j < data[i].size() - 1) {
                    file << ",";
                }
            }
            file << "]";
            if (i < data.size() - 1) {
                file << ",";
            }
        }
        file << "]";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un dictionnaire de vecteurs dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Dictionnaire à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, std::vector<double>>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "{";
        size_t count = 0;
        for (const auto& [key, values] : data) {
            file << "\"" << key << "\":[";
            for (size_t i = 0; i < values.size(); ++i) {
                file << std::fixed << std::setprecision(6) << values[i];
                if (i < values.size() - 1) {
                    file << ",";
                }
            }
            file << "]";
            if (++count < data.size()) {
                file << ",";
            }
        }
        file << "}";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un dictionnaire de valeurs simples dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Dictionnaire à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, double>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "{";
        size_t count = 0;
        for (const auto& [key, value] : data) {
            file << "\"" << key << "\":" << std::fixed << std::setprecision(6) << value;
            if (++count < data.size()) {
                file << ",";
            }
        }
        file << "}";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Écrit un dictionnaire de vecteurs de maps dans un fichier JSON
 * @param filename Nom du fichier
 * @param data Dictionnaire à écrire
 */
void in_json(const std::string& filename, const std::map<std::string, std::vector<std::map<std::string, double>>>& data) {
    files(filename);
    std::ofstream file(filename);
    if (file.is_open()) {
        file << "{";
        size_t count = 0;
        for (const auto& [key, vector_maps] : data) {
            file << "\"" << key << "\":[";
            for (size_t i = 0; i < vector_maps.size(); ++i) {
                file << "{";
                size_t map_count = 0;
                for (const auto& [map_key, value] : vector_maps[i]) {
                    file << "\"" << map_key << "\":" << std::fixed << std::setprecision(6) << value;
                    if (++map_count < vector_maps[i].size()) {
                        file << ",";
                    }
                }
                file << "}";
                if (i < vector_maps.size() - 1) {
                    file << ",";
                }
            }
            file << "]";
            if (++count < data.size()) {
                file << ",";
            }
        }
        file << "}";
        file.close();
    } else {
        std::cerr << "Erreur d'ouverture du fichier: " << filename << '\n';
    }
}

/**
 * @brief Effectue une opération vectorielle (addition ou soustraction)
 * @param A Premier vecteur
 * @param B Deuxième vecteur
 * @param plus true pour addition, false pour soustraction
 * @return Vecteur résultant
 */
std::vector<double> sum_vect(std::vector<double>& A , std::vector<double>& B, bool plus = true) { 
    std::vector<double> sum; 

    if (plus) {
        for (size_t i = 0; i < A.size(); ++i) {
            sum.push_back(A[i]+B[i]);    
        }
    }
    else {
        for (size_t i = 0; i < A.size(); ++i) {
            sum.push_back(A[i]-B[i]);     
        }
    }
    return sum; 
}

/**
 * @brief Obtient le chemin du répertoire de l'exécutable
 * @return Chemin du répertoire de l'exécutable
 * 
 * Fonction compatible Windows et Unix
 */
std::string getExecutablePath() {
    char path[MAX_PATH];
#ifdef _WIN32
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string executablePath(path);
    return executablePath.substr(0, executablePath.find_last_of("\\/"));
#else
    ssize_t count = readlink("/proc/self/exe", path, MAX_PATH);
    if (count != -1) {
        std::string executablePath(path, count);
        return executablePath.substr(0, executablePath.find_last_of("/"));
    }
    return "";
#endif
}