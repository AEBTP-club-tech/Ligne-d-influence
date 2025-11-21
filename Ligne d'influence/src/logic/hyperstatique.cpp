#include "hyperstatique.hpp"
#include "travee.hpp"
#include "rapport_focau.hpp"
#include "utile.hpp"
#include "json_handler.hpp"

#include <future>
#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// =====================================================
// Fonctions de calcul de base (constexpr)
// =====================================================

/**
 * @brief Calcule l'interpolation linéaire entre deux valeurs m et n
 * @param m Valeur de départ
 * @param n Valeur d'arrivée
 * @param x Position actuelle
 * @param l Longueur totale
 * @return Valeur interpolée
 */
static constexpr double interpolate(double m, double n, double x, double l) {
    return m * (1 - x / l) + n * x / l;
}

/**
 * @brief Calcule la rotation d'une section de poutre
 * @param m Moment à gauche
 * @param n Moment à droite
 * @param x Position actuelle
 * @param l Longueur de la travée
 * @param E Module de Young
 * @param I Moment d'inertie
 * @return Rotation de la section
 */
static constexpr double calcul_rotation(double m, double n, double x, double l, double E, double I) {
    return -m * (2 * (l*l) - 6 * l * x + 3 * (x*x)) / (6 * E * I * l) 
           - n * ((l*l) - 3 * (x*x)) / (6 * E * I * l);
}

/**
 * @brief Calcule la flèche d'une section de poutre
 * @param m Moment à gauche
 * @param n Moment à droite
 * @param x Position actuelle
 * @param l Longueur de la travée
 * @param E Module de Young
 * @param I Moment d'inertie
 * @return Flèche de la section
 */
static constexpr double calcul_fleche(double m, double n, double x, double l, double E, double I) {
    return -m * x * (l - x) * (2 * l - x) / (6 * E * I * l) 
           - n * x * (l - x) * (l + x) / (6 * E * I * l);
}

/**
 * @brief Calcule l'effort tranchant par interpolation
 * @param m Moment à gauche
 * @param n Moment à droite
 * @param l Longueur de la travée
 * @return Effort tranchant interpolé
 */
static constexpr double interpolate_effort_tranchant(double m, double n, double l) {
    return (-m + n) / l;
}

// =====================================================
// Constructeur et initialisation
// =====================================================

/**
 * @brief Constructeur de la classe hyperstatique
 * @param tous_longueur_travee Vecteur des longueurs des travées
 * @param tous_young_module Vecteur des modules de Young
 * @param tous_Inertie Vecteur des moments d'inertie
 * @param tous_x_coords Vecteur des coordonnées x des moments d'inertie
 * @param nb_division Nombre de divisions par travée
 * 
 * Initialise tous les membres de la classe et effectue les calculs préliminaires
 * nécessaires pour l'analyse hyperstatique de la structure.
 */
hyperstatique::hyperstatique(const std::vector<double>& tous_longueur_travee, 
                           const std::vector<double>& tous_young_module, 
                           const std::vector<std::vector<double>>& tous_Inertie, 
                           const std::vector<std::vector<double>>& tous_x_coords,
                           int nb_division):
    L_tr(std::make_unique<std::vector<double>>(tous_longueur_travee)),  
    E_tr(std::make_unique<std::vector<double>>(tous_young_module)),
    I_tr(std::make_unique<std::vector<double>>()),
    division(nb_division),
    nombre_travee(tous_longueur_travee.size()),
    I_var(std::make_unique<std::vector<std::vector<double>>>(tous_Inertie)),
    pos_I_var(std::make_unique<std::vector<std::vector<double>>>(tous_x_coords)),
    cached_moment_gauche(nullptr),
    cached_moment_droite(nullptr),
    rap_cache(nullptr),
    iso_cache(nullptr),
    phy(nullptr),
    phy_prime(nullptr),
    GAUCHE_DROITE(nullptr),
    Mu_iso_total(nullptr),
    W_iso_total(nullptr),
    V_iso_total(nullptr),
    T_iso_total(nullptr),
    abscisse_T_iso_total(nullptr),
    abscisse_total(nullptr),
    alpha(nullptr),
    Courbe_Moment_appuis(nullptr),
    Courbe_R_appuis(nullptr),
    Courbe_moment_en_travee(nullptr),
    Courbe_rotation_en_travee(nullptr),
    Courbe_fleche_en_travee(nullptr),
    Courbe_effort_tranchant_en_travee(nullptr),
    Abscisse_Courbe_effort_tranchant_en_travee(nullptr)
{
    // Vérification de la cohérence des données
    if (tous_longueur_travee.size() != static_cast<std::vector<double>::size_type>(nombre_travee) || 
        tous_young_module.size() != static_cast<std::vector<double>::size_type>(nombre_travee) || 
        tous_Inertie.size() != static_cast<std::vector<double>::size_type>(nombre_travee) || 
        tous_x_coords.size() != static_cast<std::vector<double>::size_type>(nombre_travee) ) {
        std::cerr << "Erreur: Les vecteurs n'ont pas la même taille" << std::endl;
        std::cerr << "L.size() = " << tous_longueur_travee.size() << std::endl;
        std::cerr << "E_values.size() = " << tous_young_module.size() << std::endl;
        std::cerr << "I_varier.size() = " << tous_Inertie.size() << std::endl;
        std::cerr << "pos_I_values.size() = " << tous_x_coords.size() << std::endl;
        throw std::invalid_argument("Dimensions incohérentes: tous les vecteurs d'entrée doivent avoir la même taille");
    }

    // Vérification des tailles des sous-vecteurs
    for (size_t i = 0; i < tous_Inertie.size(); ++i) {
        if (tous_Inertie[i].size() != tous_x_coords[i].size()) {
            std::cerr << "Erreur: Le nombre de valeurs d'inertie ne correspond pas au nombre de positions pour la travée " << i << std::endl;
            std::cerr << "I_varier[" << i << "].size() = " << tous_Inertie[i].size() << std::endl;
            std::cerr << "pos_I_values[" << i << "].size() = " << tous_x_coords[i].size() << std::endl;
        }
    }

    for (size_t i = 0; i < tous_Inertie.size(); ++i) {
        if (tous_Inertie[i].size() == 1) {
            double a = (*I_var)[i][0];
            (*I_var)[i] = {a,a};
            (*pos_I_var)[i].clear();
            (*pos_I_var)[i]={0, (*L_tr)[i]}; 
        }
    }
    
    if (nombre_travee <= 0 || nb_division <= 0) {
        throw std::invalid_argument("Le nombre de travées et de divisions doit être positif");
    }

    initialiser_donnees();
}

/**
 * @brief Constructeur de la classe hyperstatique
 * @param tous_longueur_travee Vecteur des longueurs des travées
 * @param tous_young_module Vecteur des modules de Young
 * @param tous_Inertie Vecteur des moments d'inertie
 * @param nb_division Nombre de divisions par travée
 * 
 * Initialise tous les membres de la classe et effectue les calculs préliminaires
 * nécessaires pour l'analyse hyperstatique de la structure.
 */
hyperstatique::hyperstatique(const std::vector<double>& tous_longueur_travee, 
                           const std::vector<double>& tous_young_module, 
                           const std::vector<double>& tous_Inertie,
                           int nb_division)
    : L_tr(std::make_unique<std::vector<double>>(tous_longueur_travee)),  
    E_tr(std::make_unique<std::vector<double>>(tous_young_module)),
    I_tr(std::make_unique<std::vector<double>>(tous_Inertie)),
    division(nb_division),
    nombre_travee(tous_longueur_travee.size()),
    I_var(nullptr),
    pos_I_var(nullptr),
    cached_moment_gauche(nullptr),
    cached_moment_droite(nullptr),
    rap_cache(nullptr),
    iso_cache(nullptr),
    phy(nullptr),
    phy_prime(nullptr),
    GAUCHE_DROITE(nullptr),
    Mu_iso_total(nullptr),
    W_iso_total(nullptr),
    V_iso_total(nullptr),
    T_iso_total(nullptr),
    abscisse_T_iso_total(nullptr),
    abscisse_total(nullptr),
    alpha(nullptr),
    Courbe_Moment_appuis(nullptr),
    Courbe_R_appuis(nullptr),
    Courbe_moment_en_travee(nullptr),
    Courbe_rotation_en_travee(nullptr),
    Courbe_fleche_en_travee(nullptr),
    Courbe_effort_tranchant_en_travee(nullptr),
    Abscisse_Courbe_effort_tranchant_en_travee(nullptr)
{
    // Vérification de la cohérence des données
    if (tous_longueur_travee.size() != static_cast<std::vector<double>::size_type>(nombre_travee) || 
        tous_young_module.size() != static_cast<std::vector<double>::size_type>(nombre_travee) || 
        tous_Inertie.size() != static_cast<std::vector<double>::size_type>(nombre_travee)) {
        throw std::invalid_argument("Dimensions incohérentes: tous les vecteurs d'entrée doivent avoir la même taille");
    }
    
    if (nombre_travee <= 0 || nb_division <= 0) {
        throw std::invalid_argument("Le nombre de travées et de divisions doit être positif");
    }
    
    initialiser_donnees();
}

/**
 * @brief Initialise les données nécessaires pour l'export
 */
void hyperstatique::initialiser_donnees() {
    
    // Initialiser les calculs dont dépendent d'autres méthodes
    rap_cache = std::make_unique<rapport_focau>(a_tr(), b_tr(), c_tr(), nombre_travee);
    phy = std::make_unique<std::vector<double>>(rap_cache->phy());
    phy_prime = std::make_unique<std::vector<double>>(rap_cache->phy_prime());
    alpha = std::make_unique<std::vector<std::vector<double>>>(abscisse());
    abscisse_total = std::make_unique<std::vector<double>>(abscisse_des_point(*alpha));  
    
    // Mettre en cache les résultats pour éviter les recalculs
    GAUCHE_DROITE = std::make_unique<std::vector<std::vector<std::vector<double>>>>(tr_G_D());
    Courbe_Moment_appuis = std::make_unique<std::vector<std::vector<double>>>(courbe_M_appuit());
    Mu_iso_total = std::make_unique<std::vector<std::vector<std::vector<double>>>>(Mu_total());
    W_iso_total = std::make_unique<std::vector<std::vector<std::vector<double>>>>(W_total());
    V_iso_total = std::make_unique<std::vector<std::vector<std::vector<double>>>>(V_total());
    T_iso_total = std::make_unique<std::vector<std::vector<std::vector<double>>>>(T_total());
    abscisse_T_iso_total = std::make_unique<std::vector<std::vector<std::vector<double>>>>(abscisse_T_total()); 
    
    Courbe_moment_en_travee = std::make_unique<std::vector<std::vector<std::vector<double>>>>(M_flechissant());
    Courbe_rotation_en_travee = std::make_unique<std::vector<std::vector<std::vector<double>>>>(W_rotaion());
    Courbe_fleche_en_travee = std::make_unique<std::vector<std::vector<std::vector<double>>>>(V_fleche()); 
    Courbe_effort_tranchant_en_travee = std::make_unique<std::vector<std::vector<std::vector<double>>>>(T_effort_tranchant(false));
    Abscisse_Courbe_effort_tranchant_en_travee = std::make_unique<std::vector<std::vector<std::vector<double>>>>(T_effort_tranchant(true));
    Courbe_R_appuis = std::make_unique<std::vector<std::vector<double>>>(courbe_R_appuit()); 
}

/**
 * @brief Initialise et retourne le vecteur des travées
 * @return Pointeur vers le vecteur des travées
 * 
 * Cette fonction crée et met en cache les objets travée pour chaque section
 * si ce n'est pas déjà fait.
 */
std::vector<travee>* hyperstatique::mise_en_place() const
{
    if (!iso_cache) {
        iso_cache = std::make_unique<std::vector<travee>>();
        iso_cache->reserve(nombre_travee);
        
        for (int i = 0; i < nombre_travee; ++i) {
            if (I_var && pos_I_var) {
                // Cas d'inertie variable
                iso_cache->emplace_back((*L_tr)[i], (*E_tr)[i], (*I_var)[i], (*pos_I_var)[i], division);
            } 
            else {
                // Cas d'inertie constante 
                iso_cache->emplace_back((*L_tr)[i], (*E_tr)[i], (*I_tr)[i], division);
            }
        }
    }
    return iso_cache.get();
}

// =====================================================
// Calculs des abscisses et coordonnées
// =====================================================

/**
 * @brief Calcule les abscisses de tous les points de la structure
 * @return Vecteur des abscisses par travée
 * 
 * Cette fonction calcule les positions de tous les points de calcul
 * le long de la structure, organisées par travée.
 */
std::vector<std::vector<double>> hyperstatique::abscisse() 
{
    std::vector<std::vector<double>> abscisse;
    abscisse.reserve(nombre_travee);

    auto *iso = mise_en_place();
    for (auto& i: *iso) { 
        abscisse.push_back(std::move(*i.abscisse));  
    }
    return abscisse;
}

/**
 * @brief Calcule les abscisses absolues de tous les points
 * @param liste Vecteur des abscisses relatives par travée
 * @return Vecteur des abscisses absolues
 * 
 * Convertit les abscisses relatives de chaque travée en abscisses absolues
 * en tenant compte de la position de chaque travée dans la structure.
 */
std::vector<double> hyperstatique::abscisse_des_point(std::vector<std::vector<double>> liste)
{
    std::vector<double> x;
    for(auto i{0}; i < static_cast<int>(liste.size()); i++) {
        int conteur = 1; double a = 0;
        while (conteur <= i)
        {
            a += (*L_tr)[conteur-1];
            conteur+=1; 
        }
        for (auto alpha : liste[i]){
            double b = alpha + a;
            x.push_back(b);  
        }
    }
    return x; 
}

/**
 * @brief Calcule les abscisses pour l'effort tranchant
 * @param number Indice de la travée concernée
 * @return Vecteur des abscisses pour l'effort tranchant
 * 
 * Cette fonction calcule les positions spécifiques pour le calcul
 * de l'effort tranchant dans une travée donnée.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::Pour_T_hyp(int number)
{
    // abscisse de chaque courbe de effort tranchant de chaque section
    std::vector<std::vector<std::vector<double>>> abscisse_hyp_effort_tranchant_total;
    abscisse_hyp_effort_tranchant_total.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) { // Par travee
        
        int conteur_de_section = 0; //afahana maka indice de section 
        
        //ilaigna @ coordonnee
        auto& abscisse_T = (*abscisse_T_iso_total)[index_travee]; //pour abscisse T sur le travee concerner
        auto coo = *alpha; // alpha
        
        std::vector<std::vector<double>> x_1;
        x_1.reserve(division);
        
        for (auto k = 0; k <= division; ++k) {  //Par section
            // Ajoute coordonne
            if (index_travee == number) {
                coo[index_travee] = abscisse_T[conteur_de_section];
            }
            x_1.push_back(abscisse_des_point(coo));
            ++conteur_de_section;
        }
        abscisse_hyp_effort_tranchant_total.push_back(x_1);
    }
    return abscisse_hyp_effort_tranchant_total; 
}

// =====================================================
// Calculs des efforts et déformations
// =====================================================

/**
 * @brief Calcule les moments fléchissants totaux
 * @return Vecteur des moments fléchissants par travée et section
 * 
 * Cette fonction calcule les moments fléchissants en chaque point
 * de la structure, en tenant compte des effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::Mu_total() const
{
    std::vector<std::vector<std::vector<double>>> Mu;
    Mu.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        Mu.push_back(i.moment_flechissant());
    }
    return Mu;
}

/**
 * @brief Calcule les rotations totales
 * @return Vecteur des rotations par travée et section
 * 
 * Calcule les rotations en chaque point de la structure,
 * en considérant les effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::W_total() const
{
    std::vector<std::vector<std::vector<double>>> rot;
    rot.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        rot.push_back(i.rotation());
    }
    return rot;
}

/**
 * @brief Calcule les flèches totales
 * @return Vecteur des flèches par travée et section
 * 
 * Calcule les déplacements verticaux (flèches) en chaque point
 * de la structure, en tenant compte des effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::V_total() const
{
    std::vector<std::vector<std::vector<double>>> rot;
    rot.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        rot.push_back(i.fleche()); 
    }
    return rot;
}

/**
 * @brief Calcule les efforts tranchants totaux
 * @return Vecteur des efforts tranchants par travée et section
 * 
 * Calcule les efforts tranchants en chaque point de la structure,
 * en considérant les effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::T_total() const
{
    std::vector<std::vector<std::vector<double>>> rot;
    rot.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        rot.push_back(i.effort_tranchant());  
    }
    return rot;
}

/**
 * @brief Calcule les abscisses pour les efforts tranchants
 * @return Vecteur des abscisses pour les efforts tranchants
 * 
 * Détermine les positions spécifiques pour le calcul des efforts
 * tranchants dans chaque travée.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::abscisse_T_total() const
{
    std::vector<std::vector<std::vector<double>>> rot;
    rot.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        rot.push_back(i.abscisse_effort_tranchant());  
    }
    return rot;
}

// =====================================================
// Calculs des coefficients de rigidité
// =====================================================

/**
 * @brief Calcule le produit des éléments d'une liste
 * @param liste Vecteur des valeurs
 * @param debut Indice de début
 * @param fin Indice de fin
 * @return Produit des éléments
 * 
 * Calcule le produit des éléments d'une liste entre les indices
 * début et fin inclus.
 */
double hyperstatique::prod_list(const std::vector<double>& liste, int debut, int fin) const
{
    double resultat = 1.0;
    for (int a = debut; a <= fin; ++a) {
        resultat *= liste[a];
    }
    return resultat;
}

/**
 * @brief Calcule les coefficients a pour chaque travée
 * @return Vecteur des coefficients a
 * 
 * Calcule les coefficients de rigidité a pour chaque travée,
 * utilisés dans les calculs hyperstatiques.
 */
std::vector<double> hyperstatique::a_tr() const
{
    std::vector<double> a;
    a.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        a.push_back(i.a());
    }
    return a;
}

/**
 * @brief Calcule les coefficients b pour chaque travée
 * @return Vecteur des coefficients b
 * 
 * Calcule les coefficients de rigidité b pour chaque travée,
 * utilisés dans les calculs hyperstatiques.
 */
std::vector<double> hyperstatique::b_tr() const
{
    std::vector<double> b;
    b.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        b.push_back(i.b());
    }
    return b;
}

/**
 * @brief Calcule les coefficients c pour chaque travée
 * @return Vecteur des coefficients c
 * 
 * Calcule les coefficients de rigidité c pour chaque travée,
 * utilisés dans les calculs hyperstatiques.
 */
std::vector<double> hyperstatique::c_tr() const
{
    std::vector<double> c;
    c.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        c.push_back(i.c());
    }
    return c;
}

/**
 * @brief Calcule les coefficients omega_prime pour chaque travée
 * @return Vecteur des coefficients omega_prime
 * 
 * Calcule les coefficients de déformation omega_prime pour chaque travée,
 * utilisés dans les calculs de rotation.
 */
std::vector<std::vector<double>> hyperstatique::omega_prime_tr() const
{
    std::vector<std::vector<double>> omega_prime;
    omega_prime.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        omega_prime.push_back(i.omega_prime());
    }
    return omega_prime;
}

/**
 * @brief Calcule les coefficients omega_second pour chaque travée
 * @return Vecteur des coefficients omega_second
 * 
 * Calcule les coefficients de déformation omega_second pour chaque travée,
 * utilisés dans les calculs de rotation.
 */
std::vector<std::vector<double>> hyperstatique::omega_second_tr() const
{
    std::vector<std::vector<double>> omega_second;
    omega_second.reserve(nombre_travee);
    
    auto iso = mise_en_place();
    for (auto& i : *iso) {
        omega_second.push_back(i.omega_second());
    }
    return omega_second;
}

/**
 * @brief Retourne le rapport focau
 * @return Pointeur vers l'objet rapport_focau
 * 
 * Retourne l'objet rapport_focau qui contient les coefficients
 * de rigidité calculés pour la structure.
 */
rapport_focau* hyperstatique::rap() const
{
    if (!rap_cache) {
        rap_cache = std::make_unique<rapport_focau>(a_tr(), b_tr(), c_tr(), nombre_travee);
    }
    return rap_cache.get();
}

// =====================================================
// Calculs des moments aux appuis
// =====================================================

/**
 * @brief Calcule les moments aux appuis pour une charge à gauche
 * @return Vecteur des moments aux appuis
 * 
 * Calcule les moments aux appuis en considérant une charge
 * appliquée du côté gauche de la structure.
 */
std::vector<std::vector<double>> hyperstatique::moment_au_appuit_travee_charger_gauche() const
{
    if (cached_moment_gauche) {
        return *cached_moment_gauche;
    }

    std::vector<std::vector<double>> M_appuit;
    M_appuit.reserve(nombre_travee);
    
    auto omega_primes = omega_prime_tr();
    auto omega_seconds = omega_second_tr();
    auto b_vals = b_tr();
    
    for (int i = 0; i < nombre_travee; ++i) {
        std::vector<double> G;
        G.reserve(division + 1);
        
        for (int j = 0; j <= division; ++j) {
            double Om_prime = omega_primes[i][j];
            double Om_second = omega_seconds[i][j]; 
            double phyVal = (*phy)[i];
            double phyPrimeVal = (*phy_prime)[i];
            double gauche = (phyVal/b_vals[i]) * 
                           ((Om_prime + Om_second * phyPrimeVal) / 
                           (1 - phyVal * phyPrimeVal));
            G.push_back(gauche);
        }
        M_appuit.push_back(std::move(G));
    }
    
    cached_moment_gauche = std::make_unique<std::vector<std::vector<double>>>(M_appuit);
    return M_appuit;
}

/**
 * @brief Calcule les moments aux appuis pour une charge à droite
 * @return Vecteur des moments aux appuis
 * 
 * Calcule les moments aux appuis en considérant une charge
 * appliquée du côté droit de la structure.
 */
std::vector<std::vector<double>> hyperstatique::moment_au_appuit_travee_charger_droite() const
{
    if (cached_moment_droite) {
        return *cached_moment_droite;
    }

    std::vector<std::vector<double>> M_appuit;
    M_appuit.reserve(nombre_travee);
    
    auto omega_primes = omega_prime_tr();
    auto omega_seconds = omega_second_tr();
    auto b_vals = b_tr();
    
    for (int i = 0; i < nombre_travee; ++i) {
        std::vector<double> G;
        G.reserve(division + 1);
        
        for (int j = 0; j <= division; ++j) {
            double Om_prime = omega_primes[i][j]; 
            double Om_second = omega_seconds[i][j]; 
            double phyVal = (*phy)[i];
            double phyPrimeVal = (*phy_prime)[i];
            double droite = -(phyPrimeVal/b_vals[i]) * 
                           ((Om_prime * phyVal + Om_second) / 
                           (1 - phyVal * phyPrimeVal));
            G.push_back(droite);
        }
        M_appuit.push_back(std::move(G));
    }
    
    cached_moment_droite = std::make_unique<std::vector<std::vector<double>>>(M_appuit);
    return M_appuit;
}

/**
 * @brief Calcule les moments aux appuis gauches
 * @param index_travee Indice de la travée
 * @return Vecteur des moments aux appuis gauches
 * 
 * Calcule les moments aux appuis gauches pour une travée donnée,
 * en tenant compte des effets hyperstatiques.
 */
std::vector<std::vector<double>> hyperstatique::m_appuis_gauche(int index_travee) const
{
    std::vector<std::vector<double>> gauche;
    gauche.reserve(index_travee + 1);
    
    auto moment_gauche = moment_au_appuit_travee_charger_gauche();
    
    for (int i = 0; i <= index_travee; ++i) {
        double p = prod_list(*phy, i, index_travee - 1);
        std::vector<double> q;
        q.reserve(moment_gauche[index_travee].size());
        
        for (const auto& j : moment_gauche[index_travee]) {
            q.push_back(pow(-1, index_travee - i) * p * j);
        }
        gauche.push_back(std::move(q));
    }
    return gauche;
}

/**
 * @brief Calcule les moments aux appuis droits
 * @param index_travee Indice de la travée
 * @return Vecteur des moments aux appuis droits
 * 
 * Calcule les moments aux appuis droits pour une travée donnée,
 * en tenant compte des effets hyperstatiques.
 */
std::vector<std::vector<double>> hyperstatique::m_appuis_droite(int index_travee) const
{
    std::vector<std::vector<double>> droite;
    droite.reserve(nombre_travee - index_travee);
    
    auto moment_droite = moment_au_appuit_travee_charger_droite();
    
    for (int i = index_travee, g = index_travee; i < nombre_travee; ++i, ++g) {
        double p = prod_list(*phy_prime, index_travee + 1, g);
        std::vector<double> q;
        q.reserve(moment_droite[index_travee].size());
        
        for (const auto& j : moment_droite[index_travee]) {
            double hh = pow(-1, index_travee - g) * p * j;
            q.push_back(hh);
        }
        droite.push_back(std::move(q));
    }
    return droite;
}

// =====================================================
// Fonctions utilitaires
// =====================================================

/**
 * @brief Convertit une liste de vecteurs en un seul vecteur
 * @param liste Liste de vecteurs à concaténer
 * @return Vecteur unique contenant tous les éléments
 * 
 * Cette fonction permet de concaténer plusieurs vecteurs en un seul,
 * en conservant l'ordre des éléments.
 */
std::vector<double> hyperstatique::sous_to_one(const std::vector<std::vector<double>>& liste) const
{
    // Calculer la taille totale nécessaire
    size_t total_size = 0;
    for (const auto& vec : liste) {
        total_size += vec.size();
    }
    
    std::vector<double> vita;
    vita.reserve(total_size);
    
    for (const auto& vec : liste) {
        vita.insert(vita.end(), vec.begin(), vec.end());
    }
    return vita;
}

// =====================================================
// Calculs des courbes de moments et réactions
// =====================================================

/**
 * @brief Calcule les moments aux appuis gauches pour toutes les travées
 * @return Vecteur des moments aux appuis gauches
 * 
 * Calcule les moments aux appuis gauches pour toutes les travées
 * de la structure.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::tr_m_appuis_gauche() const
{
    std::vector<std::vector<std::vector<double>>> tr_m_appuis_gauche;
    tr_m_appuis_gauche.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) {
        tr_m_appuis_gauche.push_back(m_appuis_gauche(index_travee));
    }
    return tr_m_appuis_gauche;
}

/**
 * @brief Calcule les moments aux appuis droits pour toutes les travées
 * @return Vecteur des moments aux appuis droits
 * 
 * Calcule les moments aux appuis droits pour toutes les travées
 * de la structure.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::tr_m_appuis_droite() const
{
    std::vector<std::vector<std::vector<double>>> tr_m_appuis_droite;
    tr_m_appuis_droite.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) {
        tr_m_appuis_droite.push_back(m_appuis_droite(index_travee));
    }
    return tr_m_appuis_droite;
}

/**
 * @brief Calcule les moments aux appuis gauche et droit
 * @return Vecteur des moments aux appuis
 * 
 * Combine les calculs des moments aux appuis gauches et droits
 * pour toutes les travées.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::tr_G_D() const
{
    std::vector<std::vector<std::vector<double>>> G_D;
    G_D.reserve(nombre_travee);
    
    auto gauche = tr_m_appuis_gauche();
    auto droite = tr_m_appuis_droite();
    
    for (int i = 0; i < nombre_travee; ++i) {
        std::vector<std::vector<double>> mini_g_D;
        mini_g_D.reserve(gauche[i].size() + droite[i].size());
        
        // Utiliser insert pour ajouter les éléments de manière efficace
        mini_g_D.insert(mini_g_D.end(), gauche[i].begin(), gauche[i].end());
        mini_g_D.insert(mini_g_D.end(), droite[i].begin(), droite[i].end());
        
        G_D.push_back(std::move(mini_g_D));
    }
    return G_D;
}

/**
 * @brief Calcule la courbe des moments aux appuis
 * @return Vecteur des moments aux appuis
 * 
 * Calcule la distribution des moments aux appuis le long
 * de la structure.
 */
std::vector<std::vector<double>> hyperstatique::courbe_M_appuit() const
{
    std::vector<std::vector<double>> courbe;
    courbe.reserve(nombre_travee + 1);
    
    for (int j = 0; j <= nombre_travee; ++j) {
        std::vector<std::vector<double>> appuit_courbe;
        appuit_courbe.reserve(nombre_travee);
        
        for (int i = 0; i < nombre_travee; ++i) {
            appuit_courbe.push_back((*GAUCHE_DROITE)[i][j]);
        }
        courbe.push_back(sous_to_one(std::move(appuit_courbe)));
    }
    return courbe;
}

/**
 * @brief Calcule la courbe des réactions aux appuis
 * @return Vecteur des réactions aux appuis
 * 
 * Calcule la distribution des réactions aux appuis le long
 * de la structure.
 */
std::vector<std::vector<double>> hyperstatique::courbe_R_appuit() const
{
    std::vector<std::vector<double>> courbe;
    courbe.reserve(nombre_travee+1);

    std::vector<std::vector<std::vector<double>>>& V = *Courbe_effort_tranchant_en_travee; 

    for (int i = 0; i < nombre_travee; ++i) { 
        auto val = sum_vect(V[i].back(), V[i].front(), false);
        courbe.push_back(val);
    }

    return courbe; 
}

// =====================================================
// Calculs hyperstatiques
// =====================================================

/**
 * @brief Calcule les moments fléchissants hyperstatiques
 * @return Vecteur des moments fléchissants
 * 
 * Calcule la distribution des moments fléchissants en tenant
 * compte des effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::M_flechissant() const
{
    std::vector<std::vector<std::vector<double>>> hyp_moment_total;
    hyp_moment_total.reserve(nombre_travee);
    
    std::vector<std::future<std::vector<std::vector<double>>>> futures;
    futures.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) {
        futures.push_back(std::async(std::launch::async, [this, index_travee]() {
            std::vector<std::vector<double>> Mu_travee;
            Mu_travee.reserve(division + 1);
            
            int conteur_de_section = 0;
            double travee_length = (*L_tr)[index_travee];
            
            for (const auto& section : (*alpha)[index_travee]) {   
                std::vector<double> hyp_mu;
                hyp_mu.reserve(division + 1);
                
                auto& Mut = (*Mu_iso_total)[index_travee][conteur_de_section++];
                
                for (int i = 0; i < nombre_travee; ++i) {
                    for (int j = 0; j <= division; ++j) {
                        double moment_interpolation = interpolate(
                            (*GAUCHE_DROITE)[i][index_travee][j],
                            (*GAUCHE_DROITE)[i][index_travee + 1][j],
                            section,
                            travee_length
                        );
                        if (index_travee == i) {
                            hyp_mu.push_back(Mut[j] + moment_interpolation);
                        } else {
                            hyp_mu.push_back(moment_interpolation);
                        }
                    }
                }
                Mu_travee.push_back(std::move(hyp_mu));
            }
            return Mu_travee;
        }));
    }
    
    for (auto& future : futures) {
        hyp_moment_total.push_back(future.get());
    }
    
    return hyp_moment_total;
}

/**
 * @brief Calcule les rotations hyperstatiques
 * @return Vecteur des rotations
 * 
 * Calcule la distribution des rotations en tenant compte
 * des effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::W_rotaion() const
{
    std::vector<std::vector<std::vector<double>>> hyp_rotation_total;
    hyp_rotation_total.reserve(nombre_travee);
    
    std::vector<std::future<std::vector<std::vector<double>>>> futures;
    futures.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) {
        futures.push_back(std::async(std::launch::async, [this, index_travee]() {
            std::vector<std::vector<double>> rot_travee;
            rot_travee.reserve(division + 1);
            
            int conteur_de_section = 0;
            double travee_length = (*L_tr)[index_travee];
            double travee_E = (*E_tr)[index_travee];
            double travee_I = 0.0; // Initialize with default value

            if (I_tr->empty()) {
                // Will be set in the loop below
            }
            else {
                travee_I = (*I_tr)[index_travee];
            }
            
            for (int section = 0; section <= division; ++section) {  
                std::vector<double> hyp_rot;
                hyp_rot.reserve(division + 1);
                
                auto& Rot = (*W_iso_total)[index_travee][conteur_de_section];
                conteur_de_section++;
                
                for (int i = 0; i < nombre_travee; ++i) {
                    if (index_travee == i) {
                        for (int j = 0; j <= division; ++j) {
                            if (I_tr->empty()) {
                                for (size_t k = 0; k < (*pos_I_var)[index_travee].size()-1;++k) {
                                    if ((*pos_I_var)[index_travee][k] <= (*alpha)[index_travee][j] && (*alpha)[index_travee][j] <= (*pos_I_var)[index_travee][k+1]) {
                                        travee_I = (*I_var)[index_travee][k];
                                    }
                                }
                            }

                            double rotation_val = calcul_rotation(
                                (*GAUCHE_DROITE)[i][index_travee][j],
                                (*GAUCHE_DROITE)[i][index_travee + 1][j],
                                (*alpha)[index_travee][j], 
                                travee_length,
                                travee_E,
                                travee_I
                            );
                            hyp_rot.push_back(Rot[j] + rotation_val);
                        }
                    } else {
                        for (int j = 0; j <= division; ++j) {
                            double rotation_val = calcul_rotation(
                                (*GAUCHE_DROITE)[i][index_travee][j],
                                (*GAUCHE_DROITE)[i][index_travee + 1][j],
                             (*alpha)[index_travee][j], 
                                travee_length,
                                travee_E,
                                travee_I
                            );
                            hyp_rot.push_back(rotation_val);
                        }
                    }
                }
                rot_travee.push_back(std::move(hyp_rot));
            }
            return rot_travee;
        }));
    }
    
    for (auto& future : futures) {
        hyp_rotation_total.push_back(future.get());
    }
    
    return hyp_rotation_total;
}

/**
 * @brief Calcule les flèches hyperstatiques
 * @return Vecteur des flèches
 * 
 * Calcule la distribution des flèches en tenant compte
 * des effets hyperstatiques.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::V_fleche() const
{
    std::vector<std::vector<std::vector<double>>> hyp_fleche_total;
    hyp_fleche_total.reserve(nombre_travee);
    
    std::vector<std::future<std::vector<std::vector<double>>>> futures;
    futures.reserve(nombre_travee);
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) {
        futures.push_back(std::async(std::launch::async, [this, index_travee]() {
            std::vector<std::vector<double>> rot_travee;
            rot_travee.reserve(division + 1);
            
            int conteur_de_section = 0;
            double travee_length = (*L_tr)[index_travee];
            double travee_E = (*E_tr)[index_travee];
            double travee_I = 0.0; // Initialize with default value

            if (I_tr->empty()) {
                // Will be set in the loop below
            }
            else {
                travee_I = (*I_tr)[index_travee];
            }
            
            for (int section = 0; section <= division; ++section) {  
                std::vector<double> hyp_rot;
                hyp_rot.reserve(division + 1);
                
                auto& Rot = (*V_iso_total)[index_travee][conteur_de_section];
                
                for (int i = 0; i < nombre_travee; ++i) {
                    for (int j = 0; j <= division; ++j) {

                        if (I_tr->empty()) {
                            for (size_t k = 0; k < (*pos_I_var)[index_travee].size()-1;++k) {
                                if ((*pos_I_var)[index_travee][k] <= (*alpha)[index_travee][j] && (*alpha)[index_travee][j] <= (*pos_I_var)[index_travee][k+1]) {
                                    travee_I = (*I_var)[index_travee][k];
                                }
                            }
                        }
                        double fleche_val = calcul_fleche(
                            (*GAUCHE_DROITE)[i][index_travee][j],
                            (*GAUCHE_DROITE)[i][index_travee + 1][j],
                            (*alpha)[index_travee][j], 
                            travee_length,
                            travee_E,
                            travee_I
                        );
                        if (index_travee == i) { 
                            hyp_rot.push_back(Rot[j] + fleche_val);
                        } else {
                            hyp_rot.push_back(fleche_val);
                        }
                    }
                }
                rot_travee.push_back(std::move(hyp_rot));
                conteur_de_section++; 
            }
            return rot_travee;
        }));
    }
    
    for (auto& future : futures) {
        hyp_fleche_total.push_back(future.get());
    }
    
    return hyp_fleche_total;
}

/**
 * @brief Calcule les efforts tranchants hyperstatiques
 * @param get_all_abscisse Indique si on veut récupérer les abscisses
 * @return Vecteur des efforts tranchants ou des abscisses
 * 
 * Calcule la distribution des efforts tranchants en tenant compte
 * des effets hyperstatiques, ou retourne les abscisses associées.
 */
std::vector<std::vector<std::vector<double>>> hyperstatique::T_effort_tranchant(bool get_all_abscisse) 
{
    std::vector<std::vector<std::vector<double>>> hyp_effort_tranchant_total;
    hyp_effort_tranchant_total.reserve(nombre_travee);

    // abscisse de chaque courbe de effort tranchant de chaque section
    std::vector<std::vector<std::vector<double>>> abscisse_hyp_effort_tranchant_total;
    abscisse_hyp_effort_tranchant_total.reserve(nombre_travee);
    
    // Corrdonnee 
    int tr = 0;
    while (tr < nombre_travee)
    {   
        abscisse_hyp_effort_tranchant_total.push_back(Pour_T_hyp(tr)[tr]);      
        tr++;   
    }
    
    for (int index_travee = 0; index_travee < nombre_travee; ++index_travee) { // Par travee
        std::vector<std::vector<double>> T_travee;
        T_travee.reserve(division + 1);
        
        double travee_length = (*L_tr)[index_travee];
        auto& alpha_second = *alpha;    
        
        int conteur_de_section = 0; //afahana maka indice de section 
        
        for (const auto& section : alpha_second[index_travee]) {  //Par section
            std::vector<double> hyp_T;
            hyp_T.reserve(division + 1);
            
            auto& T = (*T_iso_total)[index_travee][conteur_de_section];
            ++conteur_de_section;
            //Calcul de l'effort tranchant
            for (int i = 0; i < nombre_travee; ++i) {
                // Ajouter le moment isostatique si on est dans la même travée
                if (index_travee == i) {
                    int compteur = 0;
                    for (int j = 0; j <= division; ++j){
                        double moment_interpolation = interpolate_effort_tranchant(
                            (*GAUCHE_DROITE)[i][index_travee][j],
                            (*GAUCHE_DROITE)[i][index_travee + 1][j],
                            travee_length);
                        if (section == alpha_second[index_travee][j]){
                            hyp_T.push_back(T[compteur] + moment_interpolation);
                            compteur+=1;
                            hyp_T.push_back(T[compteur] + moment_interpolation);
                            compteur+=1;
                        }
                        else {
                            hyp_T.push_back(T[compteur] + moment_interpolation);
                            compteur+=1;
                        }
                    }
                }
                else {
                    for (int j = 0; j <= division; ++j) {
                        double moment_interpolation = interpolate_effort_tranchant(
                            (*GAUCHE_DROITE)[i][index_travee][j],
                            (*GAUCHE_DROITE)[i][index_travee + 1][j],
                            travee_length);
                        hyp_T.push_back(moment_interpolation);
                    } 
                }
            }
            T_travee.push_back(std::move(hyp_T));
        }
        hyp_effort_tranchant_total.push_back(std::move(T_travee));
    }
    if (get_all_abscisse==false){return hyp_effort_tranchant_total;}
    else {return abscisse_hyp_effort_tranchant_total;} 
}

// =====================================================
// Export des données
// =====================================================

/**
 * @brief Exporte les données au format CSV
 * @param dossier Chemin du dossier de destination
 * 
 * Exporte toutes les données calculées dans des fichiers CSV
 * organisés par catégorie (constante, forfaitaire, isostatique, hyperstatique).
 */
void hyperstatique::exporter_donnees_csv(const std::string& dossier) const {
    creeDossier(dossier); // Dossier de destination
    creeDossier(dossier + "/properties"); // Dossier des propriétés
    creeDossier(dossier + "/boundary_conditions"); // Dossier des conditions aux limites
    creeDossier(dossier + "/static_analysis"); // Dossier des analyses statiques
    creeDossier(dossier + "/influence_lines"); // Dossier des lignes d'influence

    std::vector<std::future<void>> futures;
    futures.reserve(20); // Nombre total de fichiers à écrire

    // Properties
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/span_lengths.csv", *L_tr); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/young_modulus.csv", *E_tr); }));
    if (I_tr->empty()){
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/moment_of_inertia.csv", *I_var); }));
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/abscissas_of_moment_of_inertia.csv", *pos_I_var); }));
    } else {
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/moment_of_inertia.csv", *I_tr); }));   
    }
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/coefficient_a.csv", a_tr()); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/coefficient_b.csv", b_tr()); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/coefficient_c.csv", c_tr()); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/phi.csv", *phy); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/properties/phi_prime.csv", *phy_prime); }));

    // Boundary Conditions
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/boundary_conditions/support_moments.csv", *GAUCHE_DROITE); }));

    // Static Analysis
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/bending_moments.csv", *Mu_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/rotations.csv", *W_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/deflections.csv", *V_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/shear_forces.csv", *T_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/shear_abscissas.csv", *abscisse_T_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/static_analysis/abscissas.csv", *alpha); }));

    // Influence Lines
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/support_moments.csv", *Courbe_Moment_appuis); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/support_reactions.csv", courbe_R_appuit()); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/span_moments.csv", *Courbe_moment_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/span_rotations.csv", *Courbe_rotation_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/span_deflections.csv", *Courbe_fleche_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/span_shear_forces.csv", *Courbe_effort_tranchant_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/shear_abscissas.csv", *Abscisse_Courbe_effort_tranchant_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { in_csv(dossier + "/influence_lines/total_abscissas.csv", *abscisse_total); }));

    // Attendre que toutes les opérations d'écriture soient terminées
    for (auto& future : futures) {
        future.get();
    }
}

/**
 * @brief Exporte les données au format JSON
 * @param dossier Chemin du dossier de destination
 * 
 * Exporte toutes les données calculées dans des fichiers JSON
 * organisés par catégorie (constante, forfaitaire, isostatique, hyperstatique).
 */
void hyperstatique::exporter_donnees_json(const std::string& dossier) const {
    creeDossier(dossier);  
    creeDossier(dossier + "/properties"); // Dossier des propriétés
    creeDossier(dossier + "/boundary_conditions"); // Dossier des conditions aux limites
    creeDossier(dossier + "/static_analysis"); // Dossier des analyses statiques
    creeDossier(dossier + "/influence_lines"); // Dossier des lignes d'influence

    std::vector<std::future<void>> futures;
    futures.reserve(20); // Nombre total de fichiers à écrire

    // Properties
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        for (size_t i = 0; i < L_tr->size(); ++i) {
            data["L_" + std::to_string(i+1)] = (*L_tr)[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/span_lengths.json"); 
    }));

    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        std::vector<double> n = *L_tr;
        n.insert(n.begin(), 0);
        for (size_t i = 1; i < n.size(); ++i) {
            n[i] = n[i] + n[i-1]; 
        }
        json data = n; 
        JsonHandler::saveToFile(data, dossier + "/properties/neouds_lengths.json"); 
    }));

    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        for (size_t i = 0; i < E_tr->size(); ++i) {
            data["E_" + std::to_string(i+1)] = (*E_tr)[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/young_modulus.json"); 
    }));
    if (I_tr->empty()){
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
            json data;
            for (size_t i = 0; i < I_var->size(); ++i) {
                for (size_t j = 0; j < (*I_var)[i].size(); ++j) {
                    data["I_" + std::to_string(i+1) + "_" + std::to_string(j+1)] = (*I_var)[i][j];
                }
            }
            JsonHandler::saveToFile(data, dossier + "/properties/moment_of_inertia.json"); 
        }));    
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
            json data;
            json kale;

            for (size_t i = 0; i < pos_I_var->size(); ++i) {
                for (size_t j = 0; j < (*pos_I_var)[i].size(); ++j) {
                    kale["S_" + std::to_string(j+1)] = (*pos_I_var)[i][j];
                }
                data["x_"+ std::to_string(i)] = kale; 
                kale.clear(); 
            }
            JsonHandler::saveToFile(data, dossier + "/properties/abscissas_of_moment_of_inertia.json"); 

            json DAA;
            json KAL; 
            for (size_t i = 0; i < pos_I_var->size(); ++i) {
                for (size_t j = 0; j < (*pos_I_var)[i].size(); ++j) {
                    KAL["S_" + std::to_string(j+1)] = (*I_var)[i][j];
                }
                data["x_"+ std::to_string(i)] = kale; 
                kale.clear(); 

                DAA["I_"+ std::to_string(i)] = KAL; 
                KAL.clear(); 
            }
            JsonHandler::saveToFile(DAA, dossier + "/properties/moment_of_inertia.json"); 


        }));
    } else {
        futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
            json data;
            for (size_t i = 0; i < I_tr->size(); ++i) {
                data["I_" + std::to_string(i+1)] = (*I_tr)[i];
            }
            JsonHandler::saveToFile(data, dossier + "/properties/moment_of_inertia.json"); 
        }));
    }
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        auto a_values = a_tr();
        for (size_t i = 0; i < a_values.size(); ++i) {
            data["a_" + std::to_string(i+1)] = a_values[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/coefficient_a.json"); 
    }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        auto b_values = b_tr();
        for (size_t i = 0; i < b_values.size(); ++i) {
            data["b_" + std::to_string(i+1)] = b_values[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/coefficient_b.json"); 
    }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        auto c_values = c_tr();
        for (size_t i = 0; i < c_values.size(); ++i) {
            data["c_" + std::to_string(i+1)] = c_values[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/coefficient_c.json"); 
    }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        for (size_t i = 0; i < phy->size(); ++i) {
            data["phi_" + std::to_string(i+1)] = (*phy)[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/phi.json"); 
    }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { 
        json data;
        for (size_t i = 0; i < phy_prime->size(); ++i) {
            data["phi_prime_" + std::to_string(i+1)] = (*phy_prime)[i];
        }
        JsonHandler::saveToFile(data, dossier + "/properties/phi_prime.json"); 
    }));

    // Boundary Conditions
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/boundary_conditions/support_moments.json", *GAUCHE_DROITE); }));

    // Static Analysis
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/bending_moments.json", *Mu_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/rotations.json", *W_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/deflections.json", *V_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/shear_forces.json", *T_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/shear_abscissas.json", *abscisse_T_iso_total); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/static_analysis/abscissas.json", *alpha); }));

    // Influence Lines
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/support_moments.json", *Courbe_Moment_appuis); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/support_reactions.json", *Courbe_R_appuis); })); 
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/span_moments.json", *Courbe_moment_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/span_rotations.json", *Courbe_rotation_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/span_deflections.json", *Courbe_fleche_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/span_shear_forces.json", *Courbe_effort_tranchant_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/shear_abscissas.json", *Abscisse_Courbe_effort_tranchant_en_travee); }));
    futures.push_back(std::async(std::launch::async, [this, &dossier]() { JsonHandler::writeToFile(dossier + "/influence_lines/total_abscissas.json", *abscisse_total); }));

    // Attendre que toutes les opérations d'écriture soient terminées
    for (auto& future : futures) {
        future.get();
    }
}

/**
 * @brief Affiche les informations d'une travée avec inertie constante
 * @param longueur Longueur de la travée
 * @param module_young Module de Young
 * @param inertie Moment d'inertie
 * @param nb_division Nombre de divisions
 */
static void print(double longueur, double module_young, double inertie, int nb_division) {
    std::cout << "Travée avec inertie constante:" << std::endl;
    std::cout << "  Longueur: " << longueur << " m" << std::endl;
    std::cout << "  Module de Young: " << module_young << " Pa" << std::endl;
    std::cout << "  Moment d'inertie: " << inertie << " m^4" << std::endl;
    std::cout << "  Nombre de divisions: " << nb_division << std::endl;
    std::cout << std::endl;
}
