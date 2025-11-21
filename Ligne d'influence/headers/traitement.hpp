/**
 * @file traitement.hpp
 * @brief Définition de la classe traitement pour l'analyse des moments fléchissants
 */

#ifndef __TRAITEMENT__
#define __TRAITEMENT__

#include "hyperstatique.hpp"
#include <vector>
#include <map>
#include <memory>

/**
 * @class traitement
 * @brief Classe spécialisée dans l'analyse des moments fléchissants des structures hyperstatiques
 * 
 * Cette classe hérite de hyperstatique et fournit des méthodes pour :
 * - Calculer les aires sous les courbes de moments fléchissants
 * - Identifier les sections critiques de la structure
 * - Analyser les moments maximaux aux appuis et dans les travées
 * - Exporter les résultats d'analyse dans des fichiers
 */
class traitement : public hyperstatique 
{
    using hyperstatique::hyperstatique; // Héritage des fonctions publiques de la classe parent
    
    private:
        // Variables de configuration
        int nombre_morceau;  // Nombre de divisions pour l'intégration numérique
        int nb_travee;       // Nombre total de travées dans la structure
        
        // Données héritées de la classe hyperstatique
        std::unique_ptr<std::vector<std::vector<double>>> M_appuit;  // Matrice des moments aux appuis [travée][position]
        std::unique_ptr<std::vector<std::vector<double>>> Alpha;     // Matrice des coefficients alpha [travée][position]
        std::unique_ptr<std::vector<double>> coordonne;              // Vecteur des coordonnées des points d'analyse

        /**
         * @brief Initialise les structures de données nécessaires pour l'export
         * @details Alloue la mémoire pour les conteneurs de résultats et initialise leurs valeurs
         */
        void initialiser_donnees();

        /**
         * @brief Recherche un nombre dans un vecteur
         * @param vecteur Le vecteur dans lequel effectuer la recherche
         * @param nombre La valeur à rechercher
         * @return true si le nombre est présent, false sinon
         */
        bool contient(const std::vector<double>& vecteur, double nombre);

        /**
         * @brief Identifie les moments maximaux dans les travées
         * @param vecteur Matrice 3D des moments de travée [travée][section][position]
         * @return Map associant les positions aux valeurs maximales
         */
        std::map<std::string, double> max_1(std::vector<std::vector<std::vector<double>>>& vecteur);

        /**
         * @brief Identifie les moments maximaux aux appuis
         * @param vecteur Matrice 2D des moments d'appui [appui][position]
         * @return Map associant les positions aux valeurs maximales
         */
        std::map<std::string, double> max_2(std::vector<std::vector<double>>& vecteur);

        /**
         * @brief Calcule l'aire sous une courbe par la méthode des trapèzes
         * @param x Vecteur des abscisses (positions)
         * @param y Vecteur des ordonnées (moments)
         * @return Valeur de l'aire calculée
         */
        double trapeze(const std::vector<double>& x, const std::vector<double>& y);
        
        /**
         * @brief Calcule l'aire sous la courbe de moment pour un appui donné
         * @param numero_appuit Indice de l'appui à analyser
         * @return Vecteur des aires calculées pour chaque position
         */
        std::vector<double> aire_M_appuit(int numero_appuit);

        /**
         * @brief Divise un vecteur en sous-listes selon le signe des éléments
         * @param input Vecteur à diviser
         * @return Vecteur de vecteurs contenant les sous-listes positives et négatives
         */
        std::vector<std::vector<double>> split_by_sign(const std::vector<double>& input);
        
        /**
         * @brief Calcule les aires sous les courbes de moment pour tous les appuis
         * @return Map associant chaque appui à son vecteur d'aires
         */
        std::map<std::string, std::vector<double>> aire_M_appuit_jiaby();

        /**
         * @brief Calcule les aires sous les courbes de moment pour toutes les travées
         * @param courbe Pointeur vers la matrice 3D des moments
         * @return Map hiérarchique des aires par travée et section
         */
        std::map<std::string, std::map<std::string, std::vector<double>>> aire_M_travee_jiaby(
            std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe);

        /**
         * @brief Identifie les sections avec les plus grandes aires absolues
         * @param courbe Pointeur vers la matrice 3D des moments
         * @return Map contenant les informations détaillées des plus grandes aires
         */
        std::map<std::string, std::vector<std::map<std::string, double>>> trouver_plus_grandes_aires(
            std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe
        );

        /**
         * @brief Calcule la somme des aires pour chaque section
         * @param courbe Pointeur vers la matrice 3D des moments
         * @return Map des sommes d'aires par section
         */
        std::map<std::string, std::vector<std::map<std::string, double>>> somme_aires_par_section(
            std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe);
        
        /**
         * @brief Calcule l'aire sous la courbe de moment pour une section spécifique
         * @param travee Indice de la travée
         * @param section Indice de la section
         * @param courbe Pointeur vers la matrice 3D des moments
         * @return Vecteur des aires calculées
         */
        std::vector<double> aire_M_travee_section(int travee, int section, 
            std::unique_ptr<std::vector<std::vector<std::vector<double>>>>& courbe);

        /**
         * @brief Calcule la somme des éléments d'un vecteur
         * @param vecteur Vecteur à sommer
         * @return Somme des éléments du vecteur
         */
        double sum(std::vector<double> vecteur);
    
    public:
        /**
         * @brief Constructeur de la classe traitement
         * @param tous_longueur_travee Vecteur des longueurs des travées
         * @param tous_young_module Vecteur des modules d'Young des matériaux
         * @param tous_Inertie Vecteur des moments d'inertie des sections
         * @param nb_division Nombre de divisions pour l'intégration numérique
         */
        traitement(const std::vector<double>& tous_longueur_travee, 
            const std::vector<double>& tous_young_module,
            const std::vector<double>& tous_Inertie, 
            int nb_division); 
        
        /**
         * @brief Constructeur de la classe traitement
         * @param tous_longueur_travee Vecteur des longueurs des travées
         * @param tous_young_module Vecteur des modules d'Young des matériaux
         * @param tous_Inertie Vecteur des moments d'inertie des sections
         * @param tous_x_coords Vecteur des coordonnées x des moments d'inertie
         * @param nb_division Nombre de divisions pour l'intégration numérique
         */
        traitement(const std::vector<double>& tous_longueur_travee, 
            const std::vector<double>& tous_young_module,
            const std::vector<std::vector<double>>& tous_Inertie, 
            const std::vector<std::vector<double>>& tous_x_coords,
            int nb_division);

        ~traitement() = default;
        
        // Désactivation de la copie pour éviter les problèmes de gestion de mémoire
        traitement(const traitement&) = delete;
        traitement& operator=(const traitement&) = delete;
        
        // Autorisation du déplacement pour optimiser les performances
        traitement(traitement&&) = default;
        traitement& operator=(traitement&&) = default;
        
        // Membres pour l'aire des moments en travée
        std::unique_ptr<std::map<std::string, std::map<std::string, std::vector<double>>>> aires_travee;

        // Membres pour l'export des données
        std::unique_ptr<std::map<std::string, std::vector<double>>> aire_M_appuis_par_travee;  // Aires des moments aux appuis par travée
        std::unique_ptr<std::map<std::string, double>> M_travee_maxe;                          // Moments maximaux enregistrés par travée
        std::unique_ptr<std::map<std::string, double>> M_deflections_travee_maxe;                          // Deflections maximaux enregistrés par travée
        std::unique_ptr<std::map<std::string, double>> M_rotations_travee_maxe;                          // Rotations maximaux enregistrés par travée
        std::unique_ptr<std::map<std::string, double>> M_tranchants_travee_maxe;                          // Tranchants maximaux enregistrés par travée
        std::unique_ptr<std::map<std::string, std::vector<std::map<std::string, double>>>> plus_grandes_aires_moment;  // Sections avec les plus grandes aires
        std::unique_ptr<std::map<std::string, std::vector<std::map<std::string, double>>>> somme_aires_sections_moment;  // Somme des aires par section moment
        std::unique_ptr<std::map<std::string, std::vector<std::map<std::string, double>>>> somme_aires_sections_fleche;  // Somme des aires par section fleche  
        std::unique_ptr<std::map<std::string, std::vector<std::map<std::string, double>>>> somme_aires_sections_rotation;  // Somme des aires par section rotation
        std::unique_ptr<std::map<std::string, std::vector<std::map<std::string, double>>>> somme_aires_sections_tranchant;  // Somme des aires par section tranchant

        /**
         * @brief Exporte les résultats d'analyse dans un dossier
         * @param dossier Chemin du dossier de destination pour l'export
         * @details Exporte les aires, moments maximaux et autres résultats dans des fichiers
         */
        void export_donnee(const std::string& dossier);
};

#endif