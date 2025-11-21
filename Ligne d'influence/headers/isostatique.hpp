#ifndef __Iso__
#define __Iso__
#include <vector>
#include <memory>

/**
 * @class isostatique
 * @brief Classe représentant une structure isostatique (poutre)
 * 
 * Cette classe permet de calculer les différents paramètres mécaniques d'une poutre isostatique
 * tels que les efforts tranchants, moments fléchissants, rotations et flèches.
 */
class isostatique {
    private:
        float E;        ///< Module d'Young du matériau (en Pa)
        float I;        ///< Moment d'inertie de la section (en m^4)
        std::vector<double> I_varer;
        std::vector<double> pos_I_varier;
        float L;        ///< Longueur de la poutre (en m)
        int division;   ///< Nombre de divisions pour le maillage
        float section;  ///< Section de la poutre (en m^2)

        // intrpolation lineaire pour determiner y de x connue
        double Interpolate(double x_0, double y_0, double x_1, double y_1, double x_i);

        /**
         * @brief Calcule les coefficients alpha pour les calculs de déformation
         * @return Vecteur des coefficients alpha
         */
        std::vector<double> alpha(); 

        /**
         * @brief Calcule les efforts tranchants
         * @param sigma Contrainte appliquée
         * @param abscisse_X Indique si le calcul est fait selon l'abscisse X
         * @return Vecteur des efforts tranchants
         */
        std::vector<double> T(double sigma,bool abscisse_X);

        /**
         * @brief Calcule les moments fléchissants
         * @param sigma Contrainte appliquée
         * @return Vecteur des moments fléchissants
         */
        std::vector<double> M(double sigma); 

        /**
         * @brief Calcule la rotation (omega)
         * @param sigma Contrainte appliquée
         * @return Vecteur des rotations
         */
        std::vector<double> OMEGA(double sigma); 

        /**
         * @brief Calcule les efforts tranchants
         * @param sigma Contrainte appliquée
         * @return Vecteur des efforts tranchants
         */
        std::vector<double> V(double sigma);
        
    public:
        /**
         * @brief Constructeur de la classe isostatique
         * @param longueur Longueur de la poutre
         * @param Young_module Module d'Young du matériau
         * @param Inertia Moment d'inertie de la section
         * @param nb_division Nombre de divisions pour le maillage
         */
        isostatique(float longueur, float Young_module, float Inertia,int nb_division);
        
        isostatique(float longueur, float Young_module, std::vector<double> Inertia_vaier,std::vector<double> position_Inertia,int nb_division);

        /**
         * @brief Constructeur surchargé avec section
         * @param longueur Longueur de la poutre
         * @param Young_module Module d'Young du matériau
         * @param Inertia Moment d'inertie de la section
         * @param nb_division Nombre de divisions pour le maillage
         * @param x Section de la poutre
         */
        isostatique(float longueur, float Young_module, float Inertia,int nb_division, float x); 

        std::unique_ptr<std::vector<double>> abscisse; ///< Pointeur vers le vecteur des abscisses

        /**
         * @brief Calcule les abscisses pour le tracé des efforts tranchants
         * @return Matrice des abscisses pour les efforts tranchants
         */
        std::vector<std::vector<double>> abscisse_effort_tranchant();

        /**
         * @brief Calcule les efforts tranchants sur toute la poutre
         * @return Matrice des efforts tranchants
         */
        std::vector<std::vector<double>> effort_tranchant(); 

        /**
         * @brief Calcule les moments fléchissants sur toute la poutre
         * @return Matrice des moments fléchissants
         */
        std::vector<std::vector<double>> moment_flechissant();

        /**
         * @brief Calcule les rotations sur toute la poutre
         * @return Matrice des rotations
         */
        std::vector<std::vector<double>> rotation();

        /**
         * @brief Calcule les flèches sur toute la poutre
         * @return Matrice des flèches
         */
        std::vector<std::vector<double>> fleche();  
        
        /**
         * @brief Calcule la dérivée première de la rotation
         * @return Vecteur des dérivées premières de la rotation
         */
        std::vector<double> omega_prime();

        /**
         * @brief Calcule la dérivée seconde de la rotation
         * @return Vecteur des dérivées secondes de la rotation
         */
        std::vector<double> omega_second();

        /**
         * @brief Calcule les valeurs d'inertie interpolées le long de la travée
         * @return Vecteur contenant les valeurs d'inertie interpolées pour chaque point de la travée
         */
        std::vector<double> inertie_interpolee();
};

#endif