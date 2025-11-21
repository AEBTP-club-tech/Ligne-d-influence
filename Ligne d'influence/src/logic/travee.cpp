/**
 * @file travee.cpp
 * @brief Implémentation de la classe travee pour le calcul des caractéristiques d'une travée
 * @details Cette classe permet de calculer les coefficients de rigidité d'une travée
 *          avec ou sans variation d'inertie le long de sa longueur
 */

#include "travee.hpp"
#include "isostatique.hpp" 

#include <cmath>
#include <iostream>
#include <unordered_map> 
#include <vector>
#include <stdexcept>

/**
 * @brief Constructeur de la classe travee pour une travée à inertie constante
 * @param L Longueur de la travée en mètres
 * @param E Module d'Young du matériau en Pa
 * @param I Moment d'inertie de la section en m^4
 * @param nb_division Nombre de divisions pour le calcul des efforts
 * @details Initialise les propriétés de base de la travée et hérite de la classe isostatique.
 *          L'inertie est considérée comme constante sur toute la longueur de la travée.
 */
travee::travee(float L, float E, float I,int nb_division) : 
    isostatique(L,E,I,nb_division), Id(""), materiau(""), Young(E), longueur(L), Inertie(I) {}

/**
 * @brief Constructeur de la classe travee pour une travée à inertie variable
 * @param L Longueur de la travée en mètres
 * @param E Module d'Young du matériau en Pa
 * @param I Vecteur des valeurs d'inertie le long de la travée en m^4
 * @param pos_I_varier Vecteur des positions où l'inertie change en mètres
 * @param nb_division Nombre de divisions pour le calcul des efforts
 * @throws std::invalid_argument Si les positions d'inertie sont invalides
 * @details Initialise une travée avec une inertie variable. Vérifie la validité des positions
 *         et des valeurs d'inertie fournies.
 */
travee::travee(float L, float E, std::vector<double> I, std::vector<double> pos_I_varier, int nb_division) : 
    isostatique(L,E,I,pos_I_varier,nb_division), Id(""), materiau(""), Young(E), longueur(L), pos_I_varier(pos_I_varier), I_varer(I) {
        // Vérification des positions d'inertie variable
        for(size_t i = 0; i < pos_I_varier.size(); i++) {
            if(pos_I_varier[i] > L) {
                throw std::invalid_argument("La position d'inertie variable est supérieure à la longueur de la travée");
            }
            if(pos_I_varier[i] < 0) {
                throw std::invalid_argument("La position d'inertie variable est négative");
            }
        }

        if(pos_I_varier.size() != I.size()) {
            throw std::invalid_argument("Le nombre de positions d'inertie variable doit être égal au nombre de valeurs d'inertie");
        }

        // Vérification que les positions sont distinctes et ordonnées
        for(size_t i = 0; i < pos_I_varier.size()-1 ; i++) {
            if(pos_I_varier[i] >= pos_I_varier[i+1]) {
                throw std::invalid_argument("Les positions d'inertie variable doivent être strictement croissantes");
            }
        }
    }


/**
 * @brief Calcule le coefficient a de la matrice de rigidité
 * @return Le coefficient a de la matrice de rigidité en m/(N.m^2)
 * @details Pour une travée à inertie constante : a = L/(3*E*I)
 *          Pour une travée à inertie variable : calcul intégral tenant compte des variations d'inertie
 *          Ce coefficient représente la flexibilité de la travée sous l'effet d'un moment unitaire
 */
double travee::a() { 
    if(I_varer.empty()) {
        return longueur / (3 * Young * Inertie);
    }
    else {
        double b = (-longueur / (3 * Young ));  
        double a = 0;
        for(size_t i = 0; i < pos_I_varier.size()-1 ; i++) {
            a += (1/I_varer[i])*(pow((1- pos_I_varier[i+1]/longueur),3)-pow((1- pos_I_varier[i]/longueur),3));
        }
        return a*b;
    }
}

/**
 * @brief Calcule le coefficient b de la matrice de rigidité
 * @return Le coefficient b de la matrice de rigidité en m/(N.m^2)
 * @details Pour une travée à inertie constante : b = L/(6*E*I)
 *          Pour une travée à inertie variable : calcul intégral tenant compte des variations d'inertie
 *          Ce coefficient représente l'interaction entre les moments aux extrémités
 */
static double B(double x, double l){return pow(x,2)/(2*l)-pow(x,3)/(3*pow(l,2));};
double travee::b() { 
    if(I_varer.empty()) {
        return longueur / (6 * Young * Inertie);
    }
    else {
        double b = 0;
        double a = 1/(Young);
        for(size_t i = 0; i < pos_I_varier.size()-1 ; i++) {
            b += (1/I_varer[i])*(B(pos_I_varier[i+1], longueur) - B(pos_I_varier[i], longueur)); 
        }
        return b*a;
    }
}

/**
 * @brief Calcule le coefficient c de la matrice de rigidité
 * @return Le coefficient c de la matrice de rigidité en m/(N.m^2)
 * @details Pour une travée à inertie constante : c = L/(3*E*I)
 *          Pour une travée à inertie variable : calcul intégral tenant compte des variations d'inertie
 *          Ce coefficient est identique à a() et représente la flexibilité de l'autre extrémité
 */
double travee::c() { 
    if(I_varer.empty()) {
        return longueur / (3 * Young * Inertie);
    }
    else {
        double c = 0;
        double a = 1/(Young*pow(longueur,2));
        for(size_t i = 0; i < pos_I_varier.size()-1 ; i++) {
            c += (1/I_varer[i])*(pow(pos_I_varier[i+1], 3) - pow(pos_I_varier[i], 3));
        }
        return c*a;
    }
}

/**
 * @brief Modifie la longueur de la travée
 * @param l Nouvelle longueur en mètres
 * @details Met à jour la longueur de la travée et recalcule les coefficients de rigidité
 */
void travee::set_longueur(double l) { this->longueur = l; }

/**
 * @brief Modifie le module d'Young du matériau
 * @param E Nouveau module d'Young en Pa
 * @details Met à jour le module d'Young et recalcule les coefficients de rigidité
 */
void travee::set_young_modul(double E) { this->Young = E; }

/**
 * @brief Modifie le moment d'inertie de la section
 * @param I Nouveau moment d'inertie en m^4
 * @details Met à jour le moment d'inertie et recalcule les coefficients de rigidité
 */
void travee::set_Inerti(double I) { this->Inertie = I; }

/**
 * @brief Affiche les résultats des calculs de rigidité
 * @details Calcule et affiche les coefficients a, b et c de la matrice de rigidité
 *          dans un format lisible à l'écran
 */
void travee::afficheResultats() {
    std::unordered_map<std::string, double> dict{{"a", a()}, {"b", b()}, {"c", c()}};
    print(dict);
}

/**
 * @brief Affiche le contenu d'un dictionnaire de résultats
 * @param dict Dictionnaire contenant les paires clé-valeur à afficher
 * @details Parcourt le dictionnaire et affiche chaque paire clé-valeur
 *          avec leur unité respective
 */
void travee::print(const std::unordered_map<std::string, double>& dict) {
    for (const auto& pair : dict) {
        std::cout << pair.first << ": " << pair.second << " m/(N.m^2)\n";
    }
}

/**
 * @brief Retourne la longueur de la travée
 * @return La longueur de la travée en mètres
 */
float travee::get_longueur() const
{return this->longueur;
};
