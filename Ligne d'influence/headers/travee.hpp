#ifndef __TRAVEE__
#define __TRAVEE__
#include <string>
#include <unordered_map> 

#include "isostatique.hpp"
#include <vector>

/**
 * @class travee
 * @brief Classe représentant une travée dans une structure de poutre
 * 
 * Cette classe hérite de la classe isostatique et gère les propriétés mécaniques
 * d'une travée, notamment sa longueur, son module d'Young et son inertie.
 */
class travee : public isostatique
{
using isostatique::isostatique; // Héritage des constructeurs de la classe isostatique

public:
    travee()=delete; // Suppression du constructeur par défaut pour forcer l'utilisation des constructeurs paramétrés
    
    /**
     * @brief Constructeur avec inertie constante
     * @param L Longueur de la travée
     * @param E Module d'Young du matériau
     * @param I Valeur constante de l'inertie
     * @param nb_division Nombre de divisions pour le calcul
     */
    travee(float L, float E, float I, int nb_division);

    /**
     * @brief Constructeur avec inertie variable
     * @param L Longueur de la travée
     * @param E Module d'Young du matériau
     * @param I Vecteur des valeurs d'inertie le long de la travée
     * @param pos_I_varier Vecteur des positions où l'inertie change
     * @param nb_division Nombre de divisions pour le calcul
     */
    travee(float L, float E, std::vector<double> I, std::vector<double> pos_I_varier, int nb_division);
    
    /**
     * @brief Constructeur avec inertie variable
     * @param L Longueur de la travée
     * @param E Module d'Young du matériau
     * @param I Vecteur des valeurs d'inertie le long de la travée
     * @param pos_I_varier Vecteur des positions où l'inertie change
     * @param nb_division Nombre de divisions pour le calcul
     * @param hauteur_condition Cas des section des poutre :: option == ["constante", "parabolique", "encorbellements successifes"] 
     */

    /**
     * @brief Constructeur avec inertie variable
     * @param L Longueur de la travée
     * @param E Module d'Young du matériau
     * @param I Vecteur des valeurs d'inertie le long de la travée
     * @param nb_division Nombre de divisions pour le calcul
     */
    //travee(float L, float E, std::vector<double> I, int nb_division);
    
    /**
     * @brief Calcule le coefficient a pour les équations de la ligne d'influence
     * @return Valeur du coefficient a
     */
    double a();
    
    /**
     * @brief Calcule le coefficient b pour les équations de la ligne d'influence
     * @return Valeur du coefficient b
     */
    double b();
    
    /**
     * @brief Calcule le coefficient c pour les équations de la ligne d'influence
     * @return Valeur du coefficient c
     */
    double c();
    
    /**
     * @brief Modifie la longueur de la travée
     * @param l Nouvelle longueur
     */
    void set_longueur(double l);
    
    /**
     * @brief Modifie le module d'Young du matériau
     * @param E Nouvelle valeur du module d'Young
     */
    void set_young_modul(double E);
    
    /**
     * @brief Modifie la valeur de l'inertie
     * @param I Nouvelle valeur de l'inertie
     */
    void set_Inerti(double I);

    /**
     * @brief Affiche les résultats des calculs de la travée
     */
    void afficheResultats();
    
    /**
     * @brief Affiche les résultats stockés dans une map
     * @param dict Map contenant les résultats à afficher
     */
    void print(const std::unordered_map<std::string, double>& dict);
    
    /**
     * @brief Retourne la longueur de la travée
     * @return Longueur de la travée
     */
    float get_longueur() const;
    
private:
    std::string Id;         // Identifiant unique de la travée
    std::string materiau;   // Type de matériau utilisé
    double Young;          // Module d'Young du matériau
    double longueur;       // Longueur de la travée
    double Inertie;        // Moment d'inertie de la section
    std::vector<double> pos_I_varier; // Vecteur des positions où l'inertie change
    std::vector<double> I_varer; // Vecteur des valeurs d'inertie correspondantes
};

#endif