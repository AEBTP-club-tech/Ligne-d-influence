/**
 * @file isostatique.cpp
 * @brief Implémentation des calculs pour une poutre isostatique
 */

#include "isostatique.hpp"

#include <stdexcept>
#include <cmath>
#include <memory>
#include <vector>

/**
 * @brief Calcule la dérivée première de la rotation pour une section donnée
 * @param alpha Position de la section
 * @param I Moment d'inertie
 * @param L Longueur de la travée
 * @param E Module d'Young
 * @return Valeur de la dérivée première de la rotation
 */
static constexpr double fonct_omega_prime(double alpha, double I, double L, double E) {
    return (-L/(6*E*I))*alpha*(1-alpha/L)*(2-alpha/L);
}

/**
 * @brief Calcule la dérivée seconde de la rotation pour une section donnée
 * @param alpha Position de la section
 * @param I Moment d'inertie
 * @param L Longueur de la travée
 * @param E Module d'Young
 * @return Valeur de la dérivée seconde de la rotation
 */
static constexpr double fonct_omega_secon(double alpha, double I, double L, double E) {
    return (L / (6 * E * I) * alpha * (1 - alpha * alpha / (L*L)));
}
/**
 * @brief Calcule la rotation pour une section donnée
 * @param sigma Position de la section
 * @param L Longueur de la travée
 * @param E Module d'Young
 * @return Valeur de la rotation
 */
static std::vector<double> fonct_OMEGA(double sigma, double L, double E, std::vector<double> alpha,
                                std::vector<double> pos_I_varier, 
                                std::vector<double> I_varer) {
   std::vector<double> OMEGA;
    if(sigma <= L) {
        for(auto i : alpha) {
            double ohm = 0.0;  // Initialize with default value
            if(i <= sigma) {
                for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                    if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                        double I_val = I_varer[j];
                        ohm = ((L-i)*(L+i)-3*pow((L-sigma),2))*i/(6*E*I_val*L);
                        break;
                    }
                }
            }
            else if(i > sigma) {
                for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                    if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                        double I_val = I_varer[j];
                        ohm = -(i*(2*L-i)-3*pow(sigma,2))*(L-i)/(6*E*I_val*L);
                        break;
                    }
                }
            }
            OMEGA.push_back(ohm);
        }
    }
    else {
        OMEGA.resize(alpha.size(), 0);
    }
    return OMEGA;
}

/**
 * @brief Calcule la flèche pour une section donnée
 * @param sigma Position de la section
 * @param L Longueur de la travée
 * @param E Module d'Young
 * @param alpha Position de la section
 * @return Valeur de la flèche
 */
static std::vector<double> fonct_V(double sigma, double L, double E,
            std::vector<double> alpha, 
            std::vector<double> pos_I_varier, 
            std::vector<double> I_varer) {  
    std::vector<double> V;
    if(sigma <= L) {
        for(auto i : alpha) {
            double F = 0.0;  // Initialize with default value
            if(i <= sigma) {
                for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                    if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                        double I_val = I_varer[j];
                        F = -(i*(L-sigma)/(6*E*I_val*L))*(sigma*(2*L-sigma)-pow(i,2));
                        break;
                    }
                }
            }
            else {
                for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                    if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                        double I_val = I_varer[j];
                        F = -(sigma*(L-i)/(6*E*I_val*L))*(i*(2*L-i)-pow(sigma,2));
                        break;
                    }
                }
            }
            V.push_back(F);
        }
    }
    else {
        V.resize(alpha.size(), 0);
    }
    return V;
}

/**
 * @brief Constructeur pour une poutre à inertie constante
 * @param longueur Longueur de la travée
 * @param Young_module Module d'Young du matériau
 * @param Inertia Moment d'inertie constant de la section
 * @param nb_division Nombre de divisions pour le calcul
 */
isostatique::isostatique(float longueur, float Young_module, float Inertia, int nb_division) : 
    E(Young_module),
    I(Inertia),
    L(longueur),
    division(nb_division),
    abscisse(nullptr) 
    {
        abscisse = std::make_unique<std::vector<double>>(alpha());  
    }

isostatique::isostatique(float longueur, float Young_module, std::vector<double> Inertia_vaier,std::vector<double> position_Inertia,int nb_division):
    E(Young_module),
    I_varer(Inertia_vaier),
    pos_I_varier(position_Inertia),
    L(longueur),
    division(nb_division),
    abscisse(nullptr) 
{
    if (I_varer.size()==1) {
        double a = Inertia_vaier[0];
        pos_I_varier.clear();
        I_varer.clear();
        I_varer = {a, a}; 
        pos_I_varier = {0, L}; 
    }

    if (I_varer.size() != pos_I_varier.size()) {
        throw std::invalid_argument("La taille de I_varier doit être égale à la taille de pos_I_varier moins"); 
    }

    if (pos_I_varier.back() != L) {
        throw std::invalid_argument("La dernière position doit être égale à la longueur de la travée");
    }

    abscisse = std::make_unique<std::vector<double>>(alpha());
}

/**
 * @brief Constructeur pour une poutre à inertie constante avec section spécifique
 * @param longueur Longueur de la travée
 * @param Young_module Module d'Young du matériau
 * @param Inertia Moment d'inertie constant de la section
 * @param nb_division Nombre de divisions pour le calcul
 * @param x Position de la section à étudier
 */
isostatique::isostatique(float longueur, float Young_module, float Inertia, int nb_division, float x) :
    E(Young_module),
    I(Inertia),
    L(longueur),
    division(nb_division),
    section(x),
    abscisse(nullptr) 
    {
        abscisse = std::make_unique<std::vector<double>>(alpha());
    }

/**
 * @brief Calcule l'effort tranchant pour toutes les sections
 * @return Vecteur de vecteurs contenant les efforts tranchants pour chaque section
 */
std::vector<std::vector<double>> isostatique::effort_tranchant() {
    std::vector<std::vector<double>> Eff_tranchant;
    Eff_tranchant.reserve(division+1); 

    for(auto i:alpha()) {
        Eff_tranchant.push_back(T(i,false));
    }
    return Eff_tranchant; 
}

/**
 * @brief Calcule les abscisses correspondant aux efforts tranchants
 * @return Vecteur de vecteurs contenant les abscisses pour chaque section
 */
std::vector<std::vector<double>> isostatique::abscisse_effort_tranchant() {
    std::vector<std::vector<double>> coo;
    for(auto i:alpha()) {
        coo.push_back(T(i,true)); 
    }
    return coo;
}

/**
 * @brief Calcule le moment fléchissant pour toutes les sections
 * @return Vecteur de vecteurs contenant les moments fléchissants pour chaque section
 */
std::vector<std::vector<double>> isostatique::moment_flechissant()
{
    std::vector<std::vector<double>> moment_flechissant;
    for(auto i : alpha()) {
        moment_flechissant.push_back(M(i));
    } 
    return moment_flechissant; 
}

/**
 * @brief Calcule la rotation pour toutes les sections
 * @return Vecteur de vecteurs contenant les rotations pour chaque section
 */
std::vector<std::vector<double>> isostatique::rotation()
{
    std::vector<std::vector<double>> Rot;
    for(auto i : alpha()) {
        Rot.push_back(OMEGA(i));
    } 
    return Rot;
}

/**
 * @brief Calcule la flèche pour toutes les sections
 * @return Vecteur de vecteurs contenant les flèches pour chaque section
 */
std::vector<std::vector<double>> isostatique::fleche() {
    std::vector<std::vector<double>> FLE;
    for(auto i : alpha()) {
        FLE.push_back(V(i));
    } 
    return FLE;
}

/**
 * @brief Génère les abscisses de calcul le long de la travée
 * @return Vecteur contenant les abscisses de calcul, la dernière valeur étant exactement égale à L
 */
std::vector<double> isostatique::alpha() {
    std::vector<double> alpha;
    alpha.reserve(division + 1);  // Pré-allocation pour plus d'efficacité
    
    for(auto i{0}; i < division; ++i) {
        double a{(L/division)*i};
        alpha.push_back(a);
    }
    // Ajout de la dernière valeur exactement égale à L
    alpha.push_back(L);
    
    return alpha; 
}

/**
 * @brief Calcule l'effort tranchant pour une section donnée
 * @param sigma Position de la section
 * @param abscisse Booléen indiquant si on retourne les abscisses (true) ou les efforts (false)
 * @return Vecteur contenant soit les abscisses soit les efforts tranchants
 */
std::vector<double> isostatique::T(double sigma,bool abscisse) {
    std::vector<double> T;
    std::vector<double> coordonne;

    if(sigma<=L) {
        for(auto i : alpha()) {
            if(i<sigma) {
                T.push_back(-i/L); coordonne.push_back(i);
            }
            else if(i>sigma) {
                T.push_back(1-i/L); coordonne.push_back(i);
            }
            else if(i==sigma) {
                T.push_back(-i/L); coordonne.push_back(i);
                T.push_back(1-i/L); coordonne.push_back(i);
            }
        }
    }
    else {
        for(auto i{0}; i<division; ++i){T.push_back(0);} 
        coordonne=alpha();
    }
    if (abscisse) { return coordonne;}
    else {return T;}  
}

/**
 * @brief Calcule le moment fléchissant pour une section donnée
 * @param sigma Position de la section
 * @return Vecteur contenant les moments fléchissants
 */
std::vector<double> isostatique::M(double sigma) {
    std::vector<double> Mu;
    if(sigma<=L) {
        for(const auto i: alpha()) {
            if(i<=sigma) { Mu.push_back(i*(1-sigma/L));}
            else if(i>sigma) { Mu.push_back(sigma*(1-i/L));}
        }
    }
    else {
        for(auto i{0}; i<division; ++i) {Mu.push_back(0);}
    }
    return Mu;
}

/**
 * @brief Calcule la dérivée première de la rotation
 * @return Vecteur contenant les dérivées premières de la rotation
 */
std::vector<double> isostatique::omega_prime() {
    std::vector<double> omega;
    
    if (I_varer.empty()) {
        // Cas d'une inertie constante
        for(auto i : alpha()) {
            omega.push_back(-i*(L-i)*(2*L-i)/(6*E*I*L));
        }
    } else {
        // Cas d'une inertie variable
        for (auto i : alpha()) {
            for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                    double I_val = I_varer[j];
                    omega.push_back(fonct_omega_prime(i, I_val, L, E));
                    break;
                }
            }
        }
    }
    return omega;
}

/**
 * @brief Calcule la dérivée seconde de la rotation
 * @return Vecteur contenant les dérivées secondes de la rotation
 */
std::vector<double> isostatique::omega_second() {
    std::vector<double> omega;
    
    if (I_varer.empty()) {
        // Cas d'une inertie constante
        for(auto i : alpha()) { 
            omega.push_back(i*(L-i)*(L+i)/(6*E*I*L));
        }
    } else {
        // Cas d'une inertie variable
        for (auto i : alpha()) {
            for (size_t j = 0; j < pos_I_varier.size()-1; ++j) {
                if (i >= pos_I_varier[j] && i <= pos_I_varier[j+1]) {
                    double I_val = I_varer[j];
                    omega.push_back(fonct_omega_secon(i, I_val, L, E));
                    break;
                }
            }
        }
    }
    return omega;
}

/**
 * @brief Calcule la rotation pour une section donnée
 * @param sigma Position de la section
 * @return Vecteur contenant les rotations
 */
std::vector<double> isostatique::OMEGA(double sigma)
{
    if (I_varer.empty()) {
        // Cas d'une inertie constante
        std::vector<double> OMEGA;
        if(sigma <= L) {
            for(auto i : alpha()) {
                double ohm;
                if(i <= sigma) {
                    ohm = ((L-i)*(L+i)-3*pow((L-sigma),2))*i/(6*E*I*L);
                }
                else if(i > sigma) {
                    ohm = -(i*(2*L-i)-3*pow(sigma,2))*(L-i)/(6*E*I*L);
                }
                else {
                    ohm = 0;
                }
                OMEGA.push_back(ohm);
            }
        }
        else {
            OMEGA.resize(alpha().size(), 0);
        }
        return OMEGA;
    } else {
        // Cas d'une inertie variable
        return fonct_OMEGA(sigma, L, E, alpha(), pos_I_varier, I_varer);
    }
}

/**
 * @brief Calcule la flèche pour une section donnée
 * @param sigma Position de la section
 * @return Vecteur contenant les flèches
 */
std::vector<double> isostatique::V(double sigma)
{
    if (I_varer.empty()) {
        // Cas d'une inertie constante
        std::vector<double> fleche;
        if(sigma <= L) {
            for(auto i : alpha()) {
                double F;
                if(i <= sigma) {
                    F = -(i*(L-sigma)/(6*E*I*L))*(sigma*(2*L-sigma)-pow(i,2));
                }
                else {
                    F = -(sigma*(L-i)/(6*E*I*L))*(i*(2*L-i)-pow(sigma,2));
                }
                fleche.push_back(F);
            }
        }
        else {
            fleche.resize(alpha().size(), 0);
        }
        return fleche;
    } else {
        // Cas d'une inertie variable
        return fonct_V(sigma, L, E, alpha(), pos_I_varier, I_varer);
    }
}

/**
 * @brief Effectue une interpolation linéaire entre deux points
 * @param x_0 Abscisse du premier point
 * @param y_0 Ordonnée du premier point
 * @param x_1 Abscisse du deuxième point
 * @param y_1 Ordonnée du deuxième point
 * @param x_i Abscisse du point à interpoler
 * @return Valeur interpolée
 */
double isostatique::Interpolate(double x_0, double y_0, double x_1, double y_1, double x_i) {
    double firste = (x_0 - x_1)/(y_0 - y_1); 
    return y_0 - (x_0 - x_i)/firste;  
}

/**
 * @brief Calcule les valeurs d'inertie interpolées le long de la travée
 * @return Vecteur contenant les valeurs d'inertie interpolées pour chaque point de la travée
 */
std::vector<double> isostatique::inertie_interpolee() {
    std::vector<double> inerties;
    std::vector<double> points = alpha();
    
    if (I_varer.empty()) {
        // Cas d'une inertie constante
        inerties.resize(points.size(), I);
        return inerties;
    }
    
    // Cas d'une inertie variable
    for (double x : points) {
        // Trouver l'intervalle approprié
        for (size_t i = 0; i < pos_I_varier.size() - 1; ++i) {
            if (x >= pos_I_varier[i] && x <= pos_I_varier[i + 1]) {
                // Interpolation linéaire entre les points
                double x0 = pos_I_varier[i];
                double x1 = pos_I_varier[i + 1];
                double y0 = I_varer[i];
                double y1 = I_varer[i + 1];
                
                double inertie = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
                inerties.push_back(inertie);
                break;
            }
        }
    }
    
    return inerties;
}