#ifndef __HYPERSTATIQUE_HPP__
#define __HYPERSTATIQUE_HPP__

#include "travee.hpp"
#include "rapport_focau.hpp"
#include <memory>
#include <vector>

/**
 * @class hyperstatique
 * @brief Classe principale pour le calcul des structures hyperstatiques
 * 
 * Cette classe gère le calcul des efforts internes (moments, efforts tranchants, rotations, flèches)
 * pour une structure hyperstatique composée de plusieurs travées. Elle utilise une approche
 * basée sur la méthode des forces et la méthode des trois moments.
 */
class hyperstatique
{
    private:
        // Structure données principales
        std::unique_ptr<std::vector<double>> L_tr;     // Longueurs des travées
        std::unique_ptr<std::vector<double>> E_tr;     // Modules de Young
        std::unique_ptr<std::vector<double>> I_tr;     // Moments d'inertie
        int division; // Nombre de divisions par travée pour le calcul des courbes
        int nombre_travee; // Nombre total de travées
        std::unique_ptr<std::vector<std::vector<double>>> I_var; // Moments d'inertie variables
        std::unique_ptr<std::vector<std::vector<double>>> pos_I_var; // Positions des moments d'inertie variables

        /**
         * @brief Initialise les données nécessaires pour l'export
         */
        void initialiser_donnees();
        
        /**
         * @brief Calcule le produit des éléments d'une liste entre deux indices
         * @param liste Vecteur contenant les valeurs à multiplier
         * @param debut Indice de début (inclus)
         * @param fin Indice de fin (exclus)
         * @return Produit des éléments entre debut et fin
         */
        double prod_list(const std::vector<double>& liste, int debut, int fin) const; 
        
        /**
         * @brief Transforme une matrice 2D en vecteur 1D
         * @param liste Matrice à transformer
         * @return Vecteur 1D contenant tous les éléments de la matrice
         */
        std::vector<double> sous_to_one(const std::vector<std::vector<double>>& liste) const;
        
        // Cache pour les calculs intermédiaires
        mutable std::unique_ptr<std::vector<std::vector<double>>> cached_moment_gauche;
        mutable std::unique_ptr<std::vector<std::vector<double>>> cached_moment_droite;
        mutable std::unique_ptr<rapport_focau> rap_cache;
        mutable std::unique_ptr<std::vector<travee>> iso_cache;
        
        /**
         * @brief Initialise et configure les travées pour les calculs
         * @return Pointeur vers le vecteur de travées configurées
         */
        std::vector<travee>* mise_en_place() const;

        // Méthodes de calculs intermédiaires
        /**
         * @brief Calcule les moments aux appuis pour une travée chargée à gauche
         * @return Matrice des moments aux appuis
         */
        std::vector<std::vector<double>> moment_au_appuit_travee_charger_gauche() const;
        
        /**
         * @brief Calcule les moments aux appuis pour une travée chargée à droite
         * @return Matrice des moments aux appuis
         */
        std::vector<std::vector<double>> moment_au_appuit_travee_charger_droite() const;
        
        /**
         * @brief Calcule les rapports focaux pour chaque travée
         * @return Pointeur vers l'objet rapport_focau contenant les résultats
         */
        rapport_focau* rap() const;
        
        /**
         * @brief Calcule les coefficients omega' pour chaque travée
         * @return Matrice des coefficients omega'
         */
        std::vector<std::vector<double>> omega_prime_tr() const;
        
        /**
         * @brief Calcule les coefficients omega'' pour chaque travée
         * @return Matrice des coefficients omega''
         */
        std::vector<std::vector<double>> omega_second_tr() const;
        
        /**
         * @brief Calcule les moments aux appuis gauches pour toutes les travées
         * @return Tenseur 3D des moments aux appuis gauches
         */
        std::vector<std::vector<std::vector<double>>> tr_m_appuis_gauche() const;
        
        /**
         * @brief Calcule les moments aux appuis droits pour toutes les travées
         * @return Tenseur 3D des moments aux appuis droits
         */
        std::vector<std::vector<std::vector<double>>> tr_m_appuis_droite() const; 
        
        /**
         * @brief Calcule les moments aux appuis gauches pour une travée spécifique
         * @param index_travee Index de la travée à calculer
         * @return Matrice des moments aux appuis gauches
         */
        std::vector<std::vector<double>> m_appuis_gauche(int index_travee) const;
        
        /**
         * @brief Calcule les moments aux appuis droits pour une travée spécifique
         * @param index_travee Index de la travée à calculer
         * @return Matrice des moments aux appuis droits
         */
        std::vector<std::vector<double>> m_appuis_droite(int index_travee) const;
        
        /**
         * @brief Calcule les coefficients G et D pour toutes les travées
         * @return Tenseur 3D des coefficients G et D
         */
        std::vector<std::vector<std::vector<double>>> tr_G_D() const;
        
        // Calculs principaux
        /**
         * @brief Calcule la courbe des moments aux appuis
         * @return Matrice des moments aux appuis
         */
        std::vector<std::vector<double>> courbe_M_appuit() const;
        
        /**
         * @brief Calcule la courbe des réactions aux appuis
         * @return Matrice des réactions aux appuis
         */
        std::vector<std::vector<double>> courbe_R_appuit() const; 
        
        /**
         * @brief Calcule les efforts tranchants hyperstatiques pour un cas spécifique
         * @param number Numéro du cas à calculer
         * @return Tenseur 3D des efforts tranchants
         */
        std::vector<std::vector<std::vector<double>>> Pour_T_hyp(int number);
        
        /**
         * @brief Calcule les efforts tranchants avec option pour obtenir toutes les abscisses
         * @param get_all_abscisse Si true, retourne toutes les abscisses
         * @return Tenseur 3D des efforts tranchants
         */
        std::vector<std::vector<std::vector<double>>> T_effort_tranchant(bool get_all_abscisse); 
        
        /**
         * @brief Calcule les moments fléchissants dans toutes les travées
         * @return Tenseur 3D des moments fléchissants
         */
        std::vector<std::vector<std::vector<double>>> M_flechissant() const;
        
        /**
         * @brief Calcule les rotations dans toutes les travées
         * @return Tenseur 3D des rotations
         */
        std::vector<std::vector<std::vector<double>>> W_rotaion() const;
        
        /**
         * @brief Calcule les flèches dans toutes les travées
         * @return Tenseur 3D des flèches
         */
        std::vector<std::vector<std::vector<double>>> V_fleche() const; 
        
        // Calculs isostatiques
        /**
         * @brief Calcule les moments totaux en configuration isostatique
         * @return Tenseur 3D des moments totaux
         */
        std::vector<std::vector<std::vector<double>>> Mu_total() const;
    
        /**
         * @brief Calcule les rotations totales en configuration isostatique
         * @return Tenseur 3D des rotations totales
         */
        std::vector<std::vector<std::vector<double>>> W_total() const;
        
        /**
         * @brief Calcule les flèches totales en configuration isostatique
         * @return Tenseur 3D des flèches totales
         */
        std::vector<std::vector<std::vector<double>>> V_total() const;
        
        /**
         * @brief Calcule les efforts tranchants totaux en configuration isostatique
         * @return Tenseur 3D des efforts tranchants totaux
         */
        std::vector<std::vector<std::vector<double>>> T_total() const;
        
        /**
         * @brief Calcule les abscisses des points de calcul des efforts tranchants
         * @return Tenseur 3D des abscisses
         */
        std::vector<std::vector<std::vector<double>>> abscisse_T_total() const;
        
        /**
         * @brief Calcule les abscisses des points de calcul
         * @param liste Matrice des points
         * @return Vecteur des abscisses
         */
        std::vector<double> abscisse_des_point(std::vector<std::vector<double>> liste); 
        
        /**
         * @brief Calcule les abscisses pour toutes les travées
         * @return Matrice des abscisses
         */
        std::vector<std::vector<double>> abscisse(); 
      
    public:
        /**
         * @brief Constructeur de la classe hyperstatique
         * @param tous_longueur_travee Vecteur des longueurs des travées
         * @param tous_young_module Vecteur des modules de Young
         * @param tous_Inertie Vecteur des moments d'inertie
         * @param nb_division Nombre de divisions par travée
         */
        hyperstatique(const std::vector<double>& tous_longueur_travee, 
                    const std::vector<double>& tous_young_module,
                    const std::vector<double>& tous_Inertie, 
                    int nb_division);
        
        /**
         * @brief Constructeur de la classe hyperstatique
         * @param tous_longueur_travee Vecteur des longueurs des travées
         * @param tous_young_module Vecteur des modules de Young
         * @param tous_Inertie Vecteur des moments d'inertie
         * @param tous_x_coords Vecteur des coordonnées x des moments d'inertie
         * @param nb_division Nombre de divisions par travée
         */
        hyperstatique(const std::vector<double>& tous_longueur_travee, 
                    const std::vector<double>& tous_young_module,
                    const std::vector<std::vector<double>>& tous_Inertie, 
                    const std::vector<std::vector<double>>& tous_x_coords,
                    int nb_division);

        ~hyperstatique() = default;
        
        // Désactiver la copie
        hyperstatique(const hyperstatique&) = delete;
        hyperstatique& operator=(const hyperstatique&) = delete;
        
        // Permettre le déplacement
        hyperstatique(hyperstatique&&) = default;
        hyperstatique& operator=(hyperstatique&&) = default;
        
        // Coefficients et données de base
        /**
         * @brief Calcule les coefficients a pour chaque travée
         * @return Vecteur des coefficients a
         */
        std::vector<double> a_tr() const;
        
        /**
         * @brief Calcule les coefficients b pour chaque travée
         * @return Vecteur des coefficients b
         */
        std::vector<double> b_tr() const;
        
        /**
         * @brief Calcule les coefficients c pour chaque travée
         * @return Vecteur des coefficients c
         */
        std::vector<double> c_tr() const;
        
        // Données de résultats
        std::unique_ptr<std::vector<double>> phy;              // Coefficients phi
        std::unique_ptr<std::vector<double>> phy_prime;        // Coefficients phi'
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> GAUCHE_DROITE;  // Coefficients G et D
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Mu_iso_total;   // Moments isostatiques
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> W_iso_total;    // Rotations isostatiques
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> V_iso_total;    // Flèches isostatiques
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> T_iso_total;    // Efforts tranchants isostatiques
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> abscisse_T_iso_total;  // Abscisses des efforts tranchants
        std::unique_ptr<std::vector<double>> abscisse_total;   // Abscisses totales
        std::unique_ptr<std::vector<std::vector<double>>> alpha;  // Coefficients alpha
        
        // Courbes hyperstatiques
        std::unique_ptr<std::vector<std::vector<double>>> Courbe_Moment_appuis;        // Courbe des moments aux appuis
        std::unique_ptr<std::vector<std::vector<double>>> Courbe_R_appuis;             // Courbe des réactions aux appuis
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Courbe_moment_en_travee;  // Courbe des moments en travée
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Courbe_rotation_en_travee; // Courbe des rotations en travée
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Courbe_fleche_en_travee;  // Courbe des flèches en travée
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Courbe_effort_tranchant_en_travee;  // Courbe des efforts tranchants
        std::unique_ptr<std::vector<std::vector<std::vector<double>>>> Abscisse_Courbe_effort_tranchant_en_travee;  // Abscisses des efforts tranchants
        
        /**
         * @brief Exporte les résultats au format CSV
         * @param dossier Chemin du dossier de destination
         */
        void exporter_donnees_csv(const std::string& dossier) const;
        
        /**
         * @brief Exporte les résultats au format JSON
         * @param dossier Chemin du dossier de destination
         */
        void exporter_donnees_json(const std::string& dossier) const;
        
};

#endif // __HYPERSTATIQUE_HPP__
