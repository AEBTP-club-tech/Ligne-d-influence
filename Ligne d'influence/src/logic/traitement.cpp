#include "traitement.hpp"
#include "hyperstatique.hpp" 
#include "utile.hpp"
#include "json_handler.hpp"

#include <cmath>
#include <algorithm>
#include <map>
#include <memory> 
#include <cstdlib>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

/**
 * @brief Constructeur de la classe traitement
 * @param tous_longueur_travee Vecteur des longueurs de chaque travée
 * @param tous_young_module Vecteur des modules d'Young pour chaque travée
 * @param tous_Inertie Vecteur des moments d'inertie pour chaque travée
 * @param nb_division Nombre de divisions pour le calcul numérique
 * 
 * Initialise les variables membres et calcule les aires des moments aux appuis.
 * Hérite des propriétés de la classe hyperstatique et initialise les structures de données
 * nécessaires pour le traitement des moments fléchissants.
 */
traitement::traitement(const std::vector<double> &tous_longueur_travee,
    const std::vector<double> &tous_young_module, const std::vector<double> &tous_Inertie, int nb_division) 
    : hyperstatique(tous_longueur_travee, tous_young_module, tous_Inertie, nb_division), 
    nombre_morceau(nb_division), 
    nb_travee(tous_longueur_travee.size()),
    M_appuit(std::make_unique<std::vector<std::vector<double>>>(*Courbe_Moment_appuis)),
    Alpha(std::make_unique<std::vector<std::vector<double>>>(*alpha)),
    coordonne(std::make_unique<std::vector<double>>(*abscisse_total)),
    aire_M_appuis_par_travee(nullptr),
    M_travee_maxe(nullptr),
    plus_grandes_aires_moment(nullptr),
    somme_aires_sections_moment(nullptr),
    somme_aires_sections_fleche(nullptr),
    somme_aires_sections_rotation(nullptr),
    somme_aires_sections_tranchant(nullptr)
{
    initialiser_donnees();
} 

/**
 * @brief Constructeur de la classe traitement
 * @param tous_longueur_travee Vecteur des longueurs de chaque travée
 * @param tous_young_module Vecteur des modules d'Young pour chaque travée
 * @param tous_Inertie Vecteur des moments d'inertie pour chaque travée
 * @param tous_x_coords Vecteur des coordonnées x des moments d'inertie
 * @param nb_division Nombre de divisions pour le calcul numérique
 * 
 * Initialise les variables membres et calcule les aires des moments aux appuis.
 * Hérite des propriétés de la classe hyperstatique et initialise les structures de données
 * nécessaires pour le traitement des moments fléchissants.
 */
traitement::traitement(const std::vector<double> &tous_longueur_travee,
    const std::vector<double> &tous_young_module, const std::vector<std::vector<double>> &tous_Inertie, 
    const std::vector<std::vector<double>> &tous_x_coords, int nb_division) 
    : hyperstatique(tous_longueur_travee, tous_young_module, tous_Inertie, tous_x_coords, nb_division), 
    nombre_morceau(nb_division), 
    nb_travee(tous_longueur_travee.size()),
    M_appuit(std::make_unique<std::vector<std::vector<double>>>(*Courbe_Moment_appuis)),
    Alpha(std::make_unique<std::vector<std::vector<double>>>(*alpha)),
    coordonne(std::make_unique<std::vector<double>>(*abscisse_total)),
    aire_M_appuis_par_travee(nullptr),
    M_travee_maxe(nullptr),
    plus_grandes_aires_moment(nullptr),
    somme_aires_sections_moment(nullptr),
    somme_aires_sections_fleche(nullptr),
    somme_aires_sections_rotation(nullptr),
    somme_aires_sections_tranchant(nullptr)
{
    initialiser_donnees();
}

/**
 * @brief Calcule l'aire sous une courbe en utilisant la méthode des trapèzes
 * @param x Vecteur des abscisses (coordonnées x)
 * @param y Vecteur des ordonnées (valeurs y)
 * @return L'aire calculée sous la courbe
 * 
 * La méthode des trapèzes divise l'aire en petits trapèzes et somme leurs aires.
 * Chaque trapèze est défini par deux points consécutifs (x[i],y[i]) et (x[i+1],y[i+1]).
 * L'aire d'un trapèze est calculée comme (base1 + base2) * hauteur / 2.
 * 
 * @throw std::runtime_error Si les dimensions des vecteurs ne correspondent pas
 * @throw std::runtime_error Si les abscisses ne sont pas triées
 * @throw std::runtime_error Si une hauteur négative est détectée
 * @throw std::runtime_error Si le résultat est NaN ou infini
 */
double traitement::trapeze(const std::vector<double>& x, const std::vector<double>& y) 
{
    if (x.size() != y.size()) {
        throw std::runtime_error("Mismatch between moments and abscisses dimensions: x.size()=" + 
                               std::to_string(x.size()) + ", y.size()=" + std::to_string(y.size())); 
    }
    
    // Vérifier que les abscisses sont triées
    if (!std::is_sorted(x.begin(), x.end())) {
        throw std::runtime_error("Abscisses must be sorted in ascending order");
    }
    
    double aire = 0;
    double erreur_accumulee = 0;  // Pour suivre l'erreur d'arrondi accumulée
    
    for (size_t i = 0; i < x.size() - 1; ++i) {
        double hauteur = x[i+1] - x[i];
        if (hauteur <= 0) {
            throw std::runtime_error("Invalid interval: non-positive height detected");
        }
        
        // Calcul de l'aire du trapèze avec compensation d'erreur
        double base1 = y[i];
        double base2 = y[i+1];
        double aire_trapeze = (base1 + base2) * hauteur / 2;
        
        // Compensation d'erreur
        double temp = aire + aire_trapeze;
        erreur_accumulee += (aire - temp) + aire_trapeze;
        aire = temp;
    }
    
    // Ajouter l'erreur accumulée
    aire += erreur_accumulee;
    
    // Vérifier la validité du résultat
    if (std::isnan(aire) || std::isinf(aire)) {
        throw std::runtime_error("Invalid area calculation: NaN or infinite value detected");
    }
    
    return aire;
}

/**
 * @brief Calcule l'aire des moments pour un appui spécifique
 * @param numero_appuit Numéro de l'appui à analyser (0-indexé)
 * @return Vecteur des aires calculées pour chaque travée adjacente à l'appui
 * 
 * Cette fonction divise les moments aux appuis par travée et calcule l'aire
 * sous la courbe pour chaque travée en utilisant la méthode des trapèzes.
 * Les résultats sont organisés dans un vecteur où chaque élément correspond
 * à l'aire calculée pour une travée spécifique.
 */
std::vector<double> traitement::aire_M_appuit(int numero_appuit) 
{   
    std::vector<std::vector<double>> div_M_appuit_par_travee;
    div_M_appuit_par_travee.reserve(nb_travee); 
    
    std::vector<double> div_1;
    div_1.reserve(nombre_morceau+1);  

    // Division des moments par travée
    int compteur = 0;
    while (compteur < static_cast<int>((*M_appuit)[numero_appuit].size()))
    {
        div_1.push_back((*M_appuit)[numero_appuit][compteur]);
        if (div_1.size() == static_cast<size_t>(nombre_morceau + 1)) {
            div_M_appuit_par_travee.push_back(div_1);
            div_1.clear(); 
        }
        ++compteur;
    }
    
    // Calcul des aires pour chaque travée
    std::vector<double> aire;
    aire.reserve(nb_travee);
    
    compteur = 0;
    for (auto& i : div_M_appuit_par_travee) {
        aire.push_back(trapeze((*Alpha)[compteur], i)); 
        ++compteur;
    } 
    return aire;
}

/**
 * @brief Organise les aires des moments aux appuis dans une map ordonnée
 * @return Map contenant les aires des moments pour chaque appui
 * 
 * Crée une map où chaque clé est de la forme "M_X" où X est le numéro de l'appui.
 * Les valeurs sont les aires calculées pour chaque travée adjacente.
 * La map est triée numériquement par numéro d'appui pour faciliter l'accès aux données.
 */
std::map<std::string, std::vector<double>> traitement::aire_M_appuit_jiaby()
{
    std::map<std::string, std::vector<double>> jiaby;
    // Ajout des éléments dans l'ordre numérique
    for (int i = 0; i <= nb_travee; ++i) {
        std::string key = "M_" + str(i);
        jiaby.emplace(key, aire_M_appuit(i));
    }
    
    // Tri des clés
    std::vector<std::string> keys;
    keys.reserve(jiaby.size());
    for (const auto& pair : jiaby) {
        keys.push_back(pair.first);
    }
    
    std::sort(keys.begin(), keys.end(), [](const std::string& a, const std::string& b) {
        return std::stoi(a.substr(2)) < std::stoi(b.substr(2));
    });
    
    // Création d'une nouvelle map ordonnée
    std::map<std::string, std::vector<double>> sorted_jiaby;
    for (const auto& key : keys) {
        sorted_jiaby[key] = jiaby[key];
    }
    
    return sorted_jiaby;
}

/**
 * @brief Calcule la somme des éléments d'un vecteur avec compensation d'erreur
 * @param vecteur Vecteur de nombres à sommer
 * @return La somme des éléments avec précision améliorée
 * 
 * Implémente l'algorithme de Kahan pour la sommation avec compensation d'erreur.
 * Cette méthode permet de réduire les erreurs d'arrondi lors de la sommation
 * de nombres à virgule flottante, particulièrement utile pour les grands vecteurs
 * ou les nombres de magnitudes très différentes.
 */
double traitement::sum(std::vector<double> vecteur) {   
    double sum = 0.0;
    double c = 0.0;  // Une compensation pour les bits de bas poids perdus
    for (double i : vecteur) {
        double y = i - c;    // C'est ici que la magie se produit
        double t = sum + y;
        c = (t - sum) - y;   // (t - sum) récupère l'ordre supérieur de y; soustraire y récupère -(ordre inférieur de y)
        sum = t;
    }
    return sum;
}

/**
 * @brief Trouve le maximum absolu dans un vecteur 3D de moments
 * @param vecteur Vecteur 3D des moments [travée][section][point]
 * @return Map contenant la valeur maximale et ses indices
 * 
 * Cette fonction recherche le moment maximal (en valeur absolue) dans une structure
 * de données 3D représentant les moments sur toutes les travées et sections.
 * Elle retourne un map contenant :
 * - "valeur" : la valeur maximale (avec son signe)
 * - "index_travee" : l'indice de la travée contenant le maximum
 * - "index_section" : l'indice de la section contenant le maximum
 */
std::map<std::string, double> traitement::max_1(std::vector<std::vector<std::vector<double>>>& vecteur)
{
    std::vector<double> bb;
    // Recherche du maximum par travée
    for(auto& i : vecteur) {
        std::vector<double> aa;
        for (auto& j : i) {
            std::vector<double> vect;
            for (auto& k : j) {
                vect.push_back(fabs(k));
            }
            double max_val = *std::max_element(vect.begin(), vect.end());
            aa.push_back(max_val); 
        }
        double max_val_2 = *std::max_element(aa.begin(), aa.end());
        bb.push_back(max_val_2);
    }
    double max = *std::max_element(bb.begin(), bb.end());

    // Recherche de l'indice de la travée contenant le maximum
    auto it = std::find(bb.begin(), bb.end(), max);
    int index_travee = std::distance(bb.begin(), it); 
    
    // Recherche de l'indice de la section contenant le maximum
    int index_section = 0;  // Initialize with default value
    int compteur = 0;
    for (auto& i : vecteur[index_travee]) {
        if (contient(i, max) || contient(i, -max) ) {
            index_section = compteur; 
            break;  // Exit loop once found
        }
        ++compteur;
    }

    // Vérification du signe du maximum
    double max_val_3 = *std::max_element(vecteur[index_travee][index_section].begin(), vecteur[index_travee][index_section].end());
    if (max_val_3 != max) {
        max = -max;
    }

    // Calcul plus précis de la position en utilisant l'interpolation linéaire
    double position;
    if (index_section < static_cast<int>((*Alpha)[index_travee].size())) {
        // Si la position est directement disponible dans Alpha
        position = (*Alpha)[index_travee][index_section];
    } else {
        // Interpolation linéaire entre les points disponibles
        int index_inf = std::floor(index_section * ((*Alpha)[index_travee].size() - 1) / nombre_morceau);
        int index_sup = std::ceil(index_section * ((*Alpha)[index_travee].size() - 1) / nombre_morceau);
        
        // S'assurer que les indices sont valides
        index_inf = std::max(0, std::min(index_inf, static_cast<int>((*Alpha)[index_travee].size()) - 1));
        index_sup = std::max(0, std::min(index_sup, static_cast<int>((*Alpha)[index_travee].size()) - 1));
        
        double x1 = static_cast<double>(index_inf) * nombre_morceau / ((*Alpha)[index_travee].size() - 1);
        double x2 = static_cast<double>(index_sup) * nombre_morceau / ((*Alpha)[index_travee].size() - 1);
        double y1 = (*Alpha)[index_travee][index_inf];
        double y2 = (*Alpha)[index_travee][index_sup];
        
        // Interpolation linéaire
        if (x2 != x1) {
            position = y1 + (y2 - y1) * (index_section - x1) / (x2 - x1);
        } else {
            position = y1;  // Si les points sont identiques
        }
    }
    
    return {{"valeur",max}, {"index_travee",index_travee}, {"index_section", index_section}, {"position", position}};
}

/**
 * @brief Trouve le maximum absolu dans un vecteur 2D de moments
 * @param vecteur Vecteur 2D des moments [travée][point]
 * @return Map contenant la valeur maximale et son indice
 * 
 * Version simplifiée de max_1 pour les structures 2D.
 * Retourne un map contenant :
 * - "valeur" : la valeur maximale (avec son signe)
 * - "index_travee" : l'indice de la travée contenant le maximum
 * - "index_section" : toujours 0 (car structure 2D)
 */
std::map<std::string, double> traitement::max_2(std::vector<std::vector<double>>& vecteur)
{
    std::vector<double> bb;
    for (auto& i : vecteur){
        std::vector<double> vect;
        for (auto& j : i) {
            vect.push_back(fabs(j));
        }
        double max_val = *std::max_element(vect.begin(), vect.end());
        bb.push_back(max_val);
    }
    double max = *std::max_element(bb.begin(), bb.end());

    auto it = std::find(bb.begin(), bb.end(), max);
    int index_moment_au_appuis = std::distance(bb.begin(), it);
    
    double max_val_3 = *std::max_element(vecteur[index_moment_au_appuis].begin(), vecteur[index_moment_au_appuis].end());
    if (max_val_3 != max) {
        max = -max;
    }

    auto& donne = vecteur[index_moment_au_appuis];
    int position_index = 0;  // Initialize with default value

    int compteur = 0;
    for (auto i : donne) {
        if (i == max) {
            position_index = compteur;
            break;  // Exit loop once found
        }
        ++compteur;
    }

    // Calcul de la travée et de la position relative
    int travee = position_index / (nombre_morceau + 1);
    int position_dans_travee = position_index % (nombre_morceau + 1);
    
    // Calcul plus précis de la position en utilisant l'interpolation linéaire
    double position;
    if (position_dans_travee < static_cast<int>((*Alpha)[travee].size())) {
        // Si la position est directement disponible dans Alpha
        position = (*Alpha)[travee][position_dans_travee];
    } else {
        // Interpolation linéaire entre les points disponibles
        int index_inf = std::floor(position_dans_travee * ((*Alpha)[travee].size() - 1) / nombre_morceau);
        int index_sup = std::ceil(position_dans_travee * ((*Alpha)[travee].size() - 1) / nombre_morceau);
        
        // S'assurer que les indices sont valides
        index_inf = std::max(0, std::min(index_inf, static_cast<int>((*Alpha)[travee].size()) - 1));
        index_sup = std::max(0, std::min(index_sup, static_cast<int>((*Alpha)[travee].size()) - 1));
        
        double x1 = static_cast<double>(index_inf) * nombre_morceau / ((*Alpha)[travee].size() - 1);
        double x2 = static_cast<double>(index_sup) * nombre_morceau / ((*Alpha)[travee].size() - 1);
        double y1 = (*Alpha)[travee][index_inf];
        double y2 = (*Alpha)[travee][index_sup];
        
        // Interpolation linéaire
        if (x2 != x1) {
            position = y1 + (y2 - y1) * (position_dans_travee - x1) / (x2 - x1);
        } else {
            position = y1;  // Si les points sont identiques
        }
    }

    return {{"valeur", max}, {"appuis", index_moment_au_appuis}, {"index", position_index}, 
            {"position", position}, {"travee", static_cast<double>(travee)}};
}

/**
 * @brief Vérifie si un vecteur contient une valeur donnée
 * @param vecteur Vecteur à analyser
 * @param nombre Valeur à rechercher
 * @return true si la valeur est trouvée, false sinon
 * 
 * Utilise une tolérance epsilon pour la comparaison des nombres à virgule flottante.
 * La comparaison est effectuée avec une précision de 1e-10 pour tenir compte
 * des erreurs d'arrondi potentielles.
 */
bool traitement::contient(const std::vector<double>& vecteur, double nombre)
{
    const double epsilon = 1e-10;  // Tolérance pour la comparaison de nombres à virgule flottante
    for (double element : vecteur) {
        if (std::abs(element - nombre) < epsilon) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Initialise toutes les données nécessaires pour l'export
 * 
 * Cette fonction initialise les structures de données suivantes :
 * - aire_M_appuis_par_travee : aires des moments aux appuis par travée
 * - M_travee_maxe : moments maximaux en travée
 * - plus_grandes_aires_moment : sections avec les plus grandes aires
 * - somme_aires_sections : somme des aires par section
 * 
 * Ces données sont utilisées pour l'export et l'analyse des résultats.
 */
void traitement::initialiser_donnees() {
    
    // Initialiser les aires des moments en travée
    aires_travee = std::make_unique<std::map<std::string, std::map<std::string, std::vector<double>>>>(aire_M_travee_jiaby(Courbe_moment_en_travee)); 
    
    // Initialiser les aires des moments aux appuis par travée
    aire_M_appuis_par_travee = std::make_unique<std::map<std::string, std::vector<double>>>(aire_M_appuit_jiaby());
    
    // Initialiser les moments maximaux en travée pour moment au appui par pique
    M_travee_maxe = std::make_unique<std::map<std::string, double>>(max_1(*Courbe_moment_en_travee));

    // Initialiser les deflections maximaux en travée
    M_deflections_travee_maxe = std::make_unique<std::map<std::string, double>>(max_1(*Courbe_fleche_en_travee));

    // Initialiser les rotations maximales en travée
    M_rotations_travee_maxe = std::make_unique<std::map<std::string, double>>(max_1(*Courbe_rotation_en_travee));

    // Initialiser les tranchants maximaux en travée
    M_tranchants_travee_maxe = std::make_unique<std::map<std::string, double>>(max_1(*Courbe_effort_tranchant_en_travee));
    
    // Initialiser les plus grandes aires identifiées
    plus_grandes_aires_moment = std::make_unique<std::map<std::string, std::vector<std::map<std::string, double>>>>(trouver_plus_grandes_aires(Courbe_moment_en_travee));
    
    // Initialiser la somme des aires par section moment
    somme_aires_sections_moment = std::make_unique<std::map<std::string, std::vector<std::map<std::string, double>>>>(somme_aires_par_section(Courbe_moment_en_travee));

    // Initialiser la somme des aires par section fleche
    somme_aires_sections_fleche = std::make_unique<std::map<std::string, std::vector<std::map<std::string, double>>>>(somme_aires_par_section(Courbe_moment_en_travee));

    // Initialiser la somme des aires par section rotation
    somme_aires_sections_rotation = std::make_unique<std::map<std::string, std::vector<std::map<std::string, double>>>>(somme_aires_par_section(Courbe_moment_en_travee));

    // Initialiser la somme des aires par section tranchant
    somme_aires_sections_tranchant = std::make_unique<std::map<std::string, std::vector<std::map<std::string, double>>>>(somme_aires_par_section(Courbe_moment_en_travee));
}

/**
 * @brief Calcule l'aire sous la courbe de moment pour une travée et section spécifiques
 * @param travee Numéro de la travée (0-indexé)
 * @param section Numéro de la section (0-indexé)
 * @param courbe Pointeur vers le vecteur 3D des moments [travée][section][point]
 * @return Vecteur contenant les aires calculées pour chaque partie de la courbe
 * 
 * Cette fonction calcule l'aire sous la courbe de moment pour une section spécifique
 * d'une travée donnée. La courbe est d'abord divisée en parties positives et négatives
 * en utilisant split_by_sign, puis l'aire est calculée pour chaque partie avec la méthode
 * des trapèzes.
 * 
 * @throw std::runtime_error Si les indices sont invalides
 * @throw std::runtime_error Si les dimensions des vecteurs ne correspondent pas
 * @throw std::runtime_error Si le calcul d'aire produit une valeur invalide
 */
std::vector<double> traitement::aire_M_travee_section(int travee, int section, 
    std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe) {
    // Vérification des indices
    if (travee < 0 || travee >= nb_travee || section < 0 || section > nombre_morceau) {
        std::string error_msg = "Indices invalides : travee=" + std::to_string(travee) + 
                              " (max=" + std::to_string(nb_travee-1) + 
                              "), section=" + std::to_string(section) + 
                              " (max=" + std::to_string(nombre_morceau) + ")";
        throw std::runtime_error(error_msg);
    }

    // Récupération des abscisses pour la travée
    std::vector<double> abscisses = (*Alpha)[travee];
    
    // Vérification de la taille de la courbe
    if (travee >= static_cast<int>(courbe->size()) || section >= static_cast<int>((*courbe)[travee].size())) {
        throw std::runtime_error("Indices hors limites de la courbe");
    }
    
    // Récupération des moments pour la section spécifique
    std::vector<double> moments = (*courbe)[travee][section];
    
    // Si le nombre de moments est supérieur au nombre d'abscisses, prendre un sous-ensemble
    if (moments.size() > abscisses.size()) {
        std::vector<double> moments_reduits;
        moments_reduits.reserve(abscisses.size());
        
        // Prendre un point sur trois (car 303/101 ≈ 3)
        for (size_t i = 0; i < abscisses.size(); ++i) {
            size_t index = i * (moments.size() / abscisses.size());
            if (index >= moments.size()) {
                index = moments.size() - 1;
            }
            moments_reduits.push_back(moments[index]);
        }
        moments = std::move(moments_reduits);
    }
    
    // Vérification finale de la taille des vecteurs
    if (abscisses.size() != moments.size()) {
        std::string error_msg = "Mismatch entre les dimensions des abscisses (" + 
                              std::to_string(abscisses.size()) + 
                              ") et des moments (" + 
                              std::to_string(moments.size()) + 
                              ") pour travee=" + 
                              std::to_string(travee) + 
                              ", section=" + 
                              std::to_string(section);
        throw std::runtime_error(error_msg);
    }
    
    // Division de la courbe en parties positives et négatives
    std::vector<std::vector<double>> parties = split_by_sign(moments);
    
    // Vecteur pour stocker les aires calculées
    std::vector<double> aires;
    aires.reserve(parties.size());
    
    // Calcul de l'aire pour chaque partie
    for (const auto& partie : parties) {
        if (partie.empty()) {
            continue; // Ignorer les parties vides
        }
        
        // Création des vecteurs d'abscisses et de moments pour cette partie
        std::vector<double> abscisses_partie;
        std::vector<double> moments_partie;
        
        // Pour chaque moment dans la partie, trouver l'abscisse correspondante
        for (size_t i = 0; i < partie.size(); ++i) {
            size_t index = i % abscisses.size();
            abscisses_partie.push_back(abscisses[index]);
            moments_partie.push_back(partie[i]);
        }
        
        // Trier les abscisses et les moments correspondants
        std::vector<size_t> indices(abscisses_partie.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), 
            [&abscisses_partie](size_t i1, size_t i2) { 
                return abscisses_partie[i1] < abscisses_partie[i2]; 
            });
        
        // Créer les vecteurs triés
        std::vector<double> abscisses_triees;
        std::vector<double> moments_tries;
        abscisses_triees.reserve(abscisses_partie.size());
        moments_tries.reserve(moments_partie.size());
        
        for (size_t i : indices) {
            abscisses_triees.push_back(abscisses_partie[i]);
            moments_tries.push_back(moments_partie[i]);
        }
        
        // Supprimer les points en double
        std::vector<double> abscisses_uniques;
        std::vector<double> moments_uniques;
        abscisses_uniques.reserve(abscisses_triees.size());
        moments_uniques.reserve(moments_tries.size());
        
        // Ajouter le premier point
        abscisses_uniques.push_back(abscisses_triees[0]);
        moments_uniques.push_back(moments_tries[0]);
        
        // Parcourir les points et supprimer les doublons
        for (size_t i = 1; i < abscisses_triees.size(); ++i) {
            if (abscisses_triees[i] != abscisses_triees[i-1]) {
                abscisses_uniques.push_back(abscisses_triees[i]);
                moments_uniques.push_back(moments_tries[i]);
            }
        }
        
        // Vérifier que nous avons au moins deux points
        if (abscisses_uniques.size() < 2) {
            aires.push_back(0.0);
            continue;
        }
        
        // Calculer l'aire de cette partie
        double aire_partie = trapeze(abscisses_uniques, moments_uniques);
        
        // Vérifier la validité de l'aire calculée
        if (std::isnan(aire_partie) || std::isinf(aire_partie)) {
            throw std::runtime_error("Calcul d'aire invalide : valeur NaN ou infinie détectée");
        }
        
        // Ajouter l'aire au vecteur
        aires.push_back(aire_partie);
    }
    
    // Si aucune aire n'a été calculée, ajouter 0
    if (aires.empty()) {
        aires.push_back(0.0);
    }
    
    return aires;
}

/**
 * @brief Exporte les données calculées vers des fichiers JSON
 * @param dossier Chemin du dossier de destination
 * 
 * Cette fonction exporte les données suivantes dans le dossier spécifié :
 * - Données de base (héritées de hyperstatique)
 * - Aires des moments aux appuis par travée
 * - Moments maximaux aux appuis
 * - Moments maximaux en travée
 * - Aires des moments en travée (calculées avec split_by_sign)
 * - Plus grandes aires identifiées
 * - Somme des aires par section moment
 * - Somme des aires par section fleche
 * - Somme des aires par section rotation
 * - Somme des aires par section tranchant
 * 
 * Les fichiers sont organisés dans une structure de dossiers :
 * - /analysis/ : contient les fichiers de données utilisées
 * - /analysis/support_moment_areas.json
 * - /analysis/max_support_moments.json
 * - /analysis/max_span_moments.json
 * - /analysis/split_span_moment_areas.json
 * - /analysis/largest_moment_areas.json
 * - /analysis/section_moment_areas.json
 * - /analysis/section_deflection_areas.json
 * - /analysis/section_rotation_areas.json
 * - /analysis/section_shear_areas.json
 */
void traitement::export_donnee(const std::string& dossier) {
    // Initialiser toutes les données nécessaires
    initialiser_donnees();
    
    // Exporter les données de base
    exporter_donnees_json(dossier);
    
    // Créer les dossiers pour les données utilisées
    creeDossier(dossier + "/analysis");
    
    // Exporter les aires des moments aux appuis par travée
    JsonHandler::writeToFile(dossier + "/analysis/support_moment_areas.json", *aire_M_appuis_par_travee);
    
    // Exporter les moments maximaux aux appuis
    JsonHandler::writeToFile(dossier + "/analysis/max_support_moments.json", max_2(*Courbe_Moment_appuis));
    
    // Exporter les moments maximaux en travée
    JsonHandler::writeToFile(dossier + "/analysis/max_span_moments.json", *M_travee_maxe);

    // Exporter les deflections maximaux en travée
    JsonHandler::writeToFile(dossier + "/analysis/max_span_deflections.json", *M_deflections_travee_maxe);

    // Exporter les rotations maximales en travée
    JsonHandler::writeToFile(dossier + "/analysis/max_span_rotations.json", *M_rotations_travee_maxe);

    // Exporter les tranchants maximaux en travée
    JsonHandler::writeToFile(dossier + "/analysis/max_span_shear_forces.json", *M_tranchants_travee_maxe);
    
    // Exporter les aires des moments en travée avec split_by_sign
    JsonHandler::writeToFile(dossier + "/analysis/split_span_moment_areas.json", *aires_travee);
    
    // Exporter les plus grandes aires identifiées
    JsonHandler::writeToFile(dossier + "/analysis/largest_moment_areas.json", *plus_grandes_aires_moment);
    
    // Exporter la somme des aires par section
    JsonHandler::writeToFile(dossier + "/analysis/section_moment_areas.json", *somme_aires_sections_moment);

    // Exporter la somme des aires par section
    JsonHandler::writeToFile(dossier + "/analysis/section_deflection_areas.json", *somme_aires_sections_fleche);

    // Exporter la somme des aires par section
    JsonHandler::writeToFile(dossier + "/analysis/section_rotation_areas.json", *somme_aires_sections_rotation);

    // Exporter la somme des aires par section
    JsonHandler::writeToFile(dossier + "/analysis/section_shear_areas.json", *somme_aires_sections_tranchant);
}

/**
 * @brief Divise un vecteur en sous-listes basées sur le signe des éléments
 * @param input Vecteur à diviser
 * @return Vecteur de vecteurs contenant les sous-listes
 * 
 * Cette fonction divise le vecteur d'entrée en sous-listes où chaque sous-liste
 * contient des éléments consécutifs du même signe (positif ou négatif).
 * Les zéros sont considérés comme positifs.
 * 
 * Par exemple, pour le vecteur [1, 2, -3, -4, 5, 0, -1], le résultat sera :
 * [[1, 2], [-3, -4], [5, 0], [-1]]
 */
std::vector<std::vector<double>> traitement::split_by_sign(const std::vector<double>& input) {
    std::vector<std::vector<double>> result;
    
    if (input.empty()) {
        return result;
    }
    
    // Initialiser la première sous-liste avec le premier élément
    std::vector<double> current_sublist;
    current_sublist.push_back(input[0]);
    
    // Déterminer le signe du premier élément
    bool current_sign = input[0] >= 0;
    
    // Parcourir le reste du vecteur
    for (size_t i = 1; i < input.size(); ++i) {
        bool element_sign = input[i] >= 0;
        
        // Si le signe change, ajouter la sous-liste actuelle au résultat
        // et commencer une nouvelle sous-liste
        if (element_sign != current_sign) {
            result.push_back(current_sublist);
            current_sublist.clear();
            current_sign = element_sign;
        }
        
        current_sublist.push_back(input[i]);
    }
    
    // Ajouter la dernière sous-liste au résultat
    if (!current_sublist.empty()) {
        result.push_back(current_sublist);
    }
    
    return result;
}

/**
 * @brief Calcule les aires sous les courbes de moment pour toutes les travées
 * @param courbe Pointeur vers le vecteur 3D des moments [travée][section][point]
 * @return Map contenant les aires pour chaque travée et section
 * 
 * Cette fonction calcule les aires sous les courbes de moment pour chaque travée
 * et chaque section en utilisant la méthode des trapèzes. Les résultats sont organisés
 * dans une structure de données imbriquée avec les clés :
 * - "T_X" pour les travées (X = numéro de travée)
 * - "S_Y" pour les sections (Y = numéro de section)
 * 
 * Les aires sont calculées séparément pour les parties positives et négatives de la courbe
 * en utilisant la fonction split_by_sign.
 */
std::map<std::string, std::map<std::string, std::vector<double>>> traitement::aire_M_travee_jiaby(
    std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe ) {
    std::map<std::string, std::map<std::string, std::vector<double>>> result;
    
    // Parcourir chaque travée
    for (int travee = 0; travee < nb_travee; ++travee) {
        std::string travee_key = "T_" + std::to_string(travee);
        std::map<std::string, std::vector<double>> travee_data;
        
        // Parcourir chaque section de la travée
        for (int section = 0; section <= nombre_morceau; ++section) {
            std::string section_key = "S_" + std::to_string(section);
            
            // Calculer les aires pour cette section en utilisant aire_M_travee_section
            std::vector<double> aires = aire_M_travee_section(travee, section, courbe);
            
            // Ajouter les aires à la section
            travee_data[section_key] = aires;
        }
        
        // Ajouter les données de la travée au résultat
        result[travee_key] = travee_data;
    }
    
    return result;
}

/**
 * @brief Trouve les sections avec les plus grandes aires absolues
 * @param courbe Pointeur vers le vecteur 3D des moments [travée][section][point]
 * @return Map contenant les informations sur les plus grandes aires
 * 
 * Cette fonction identifie les sections présentant les plus grandes aires absolues
 * sous les courbes de moment. Le résultat est organisé en deux catégories :
 * - "plus_grande_aire" : contient la section avec l'aire absolue la plus grande
 * - "top_10_aires" : contient les 10 sections avec les plus grandes aires
 * 
 * Pour chaque section, les informations suivantes sont stockées :
 * - "aire" : valeur de l'aire absolue
 * - "travee" : numéro de la travée
 * - "section" : numéro de la section
 * - "index_aire" : index de l'aire dans le classement
 */
std::map<std::string, std::vector<std::map<std::string, double>>> traitement::trouver_plus_grandes_aires(
    std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe) {
        
    std::map<std::string, std::vector<std::map<std::string, double>>> result;
    std::vector<std::map<std::string, double>> plus_grande_aire;
    std::vector<std::map<std::string, double>> top_10_aires;

    // Calculer les aires pour toutes les travées et sections
    auto aires_travee = aire_M_travee_jiaby(courbe);
    
    // Structure pour stocker toutes les aires avec leurs informations
    std::vector<std::tuple<double, int, int>> toutes_aires;
    
    // Parcourir toutes les travées et sections pour collecter les aires
    for (const auto& travee_pair : aires_travee) {
        int travee = std::stoi(travee_pair.first.substr(2)); // Extraire le numéro de travée de "T_X"
        
        for (const auto& section_pair : travee_pair.second) {
            int section = std::stoi(section_pair.first.substr(2)); // Extraire le numéro de section de "S_Y"
            double aire = section_pair.second[0]; // L'aire est stockée dans le premier élément du vecteur
            
            toutes_aires.emplace_back(std::abs(aire), travee, section);
        }
    }
    
    // Trier les aires par ordre décroissant
    std::sort(toutes_aires.begin(), toutes_aires.end(), 
        [](const auto& a, const auto& b) { return std::get<0>(a) > std::get<0>(b); });
    
    // Extraire la plus grande aire
    if (!toutes_aires.empty()) {
        const auto& plus_grande = toutes_aires[0];
        plus_grande_aire.push_back({
            {"aire", std::get<0>(plus_grande)},
            {"travee", static_cast<double>(std::get<1>(plus_grande))},
            {"section", static_cast<double>(std::get<2>(plus_grande))},
            {"index_aire", 0.0}
        });
    }
    
    // Extraire les 10 plus grandes aires
    for (size_t i = 0; i < std::min(toutes_aires.size(), static_cast<size_t>(10)); ++i) {
        const auto& aire = toutes_aires[i];
        top_10_aires.push_back({
            {"aire", std::get<0>(aire)},
            {"travee", static_cast<double>(std::get<1>(aire))},
            {"section", static_cast<double>(std::get<2>(aire))},
            {"index_aire", 0.0}
        });
    }
    
    // Construire le résultat final
    result["plus_grande_aire"] = plus_grande_aire;
    result["top_10_aires"] = top_10_aires;
    
    return result;
}

/**
 * @brief Calcule la somme des aires pour chaque section
 * @param courbe Pointeur vers le vecteur 3D des moments [travée][section][point]
 * @return Map contenant les sommes des aires par section
 * 
 * Cette fonction calcule la somme des aires pour chaque section en considérant
 * toutes les travées. Le résultat est organisé par travée, avec pour chaque section :
 * - "S_X" : numéro de la section (X = numéro de section)
 * - "aire" : somme des aires pour cette section
 * 
 * Cette information est utile pour identifier les sections critiques où les moments
 * sont les plus importants sur l'ensemble de la structure.
 */
std::map<std::string, std::vector<std::map<std::string, double>>> traitement::somme_aires_par_section(
    std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe) {
    std::map<std::string, std::vector<std::map<std::string, double>>> result;
    
    // Get the areas for each span
    auto areas = aire_M_travee_jiaby(courbe);
    
    // For each span
    for (const auto& [span_name, span_areas] : areas) {
        std::vector<std::map<std::string, double>> section_sums;
        
        // For each section in the span
        for (const auto& [section_name, section_data] : span_areas) {
            std::map<std::string, double> section_sum;
            
            // Sum up the areas for this section across all spans
            for (const auto& [other_span_name, other_span_areas] : areas) {
                
                // Find the corresponding section in other spans
                auto it = other_span_areas.find(section_name);
                if (it != other_span_areas.end()) {
                    int section_num = std::stoi(section_name.substr(2));
                    section_sum["S_" + std::to_string(section_num)] = section_num;
                    // Add the area value to the sum
                    for (const auto& value : it->second) {
                        section_sum["aire"] += value;
                    }
                }
            }
            
            section_sums.push_back(section_sum);
        }
        
        result[span_name] = section_sums;
    }
    
    return result;
}