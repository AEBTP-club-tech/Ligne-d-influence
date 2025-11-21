#include "rapport_focau.hpp"

/**
 * @brief Constructeur de la classe rapport_focau
 * @param liste_a Vecteur des coefficients a (termes diagonaux)
 * @param liste_b Vecteur des coefficients b (termes sous-diagonaux)
 * @param liste_c Vecteur des coefficients c (termes sur-diagonaux)
 * @param nombre_travee Nombre total de travées
 */
rapport_focau::rapport_focau(std::vector<double> liste_a, 
                            std::vector<double> liste_b, 
                            std::vector<double> liste_c,
                            int nombre_travee):
    a(liste_a), b(liste_b), c(liste_c), nb_travee(nombre_travee) {} 

/**
 * @brief Calcule les coefficients φ (phi) pour la résolution du système d'équations
 * @return Vecteur des coefficients φ calculés
 * 
 * Cette fonction implémente la méthode de Thomas pour la résolution de systèmes tridiagonaux.
 * Les coefficients φ sont calculés de gauche à droite (de la première à la dernière travée).
 * Pour la première travée, φ = 0.
 * Pour les travées suivantes, φ est calculé selon la formule :
 * φ[i] = b[i] / (a[i] + c[i-1] - b[i-1] * φ[i-1])
 */
std::vector<double> rapport_focau::phy() {
    std::vector<double> liste_Phy;
    double phy_premier = 0;  // Initialisation du premier coefficient φ
    
    for(auto i{0}; i < nb_travee; ++i) {   
        if(i == 0) {
            // Pour la première travée, φ reste à 0
        }
        else {
            // Calcul du coefficient φ pour la travée courante
            // en utilisant le coefficient φ de la travée précédente
            double phy = b[i]/(a[i] + c[i-1] - b[i-1]*phy_premier);
            phy_premier = phy; 
        }
        liste_Phy.push_back(phy_premier);
    }
    return liste_Phy; 
}

/**
 * @brief Calcule les coefficients φ' (phi prime) pour la résolution du système d'équations
 * @return Vecteur des coefficients φ' calculés
 * 
 * Cette fonction calcule les coefficients φ' de droite à gauche (de la dernière à la première travée).
 * Pour la dernière travée, φ' = 0.
 * Pour les travées précédentes, φ' est calculé selon la formule :
 * φ'[i] = b[i-1] / (c[i-1] + a[i] - b[i] * φ'[i+1])
 * 
 * Le résultat est ensuite inversé pour correspondre à l'ordre des travées.
 */
std::vector<double> rapport_focau::phy_prime() {
    std::vector<double> liste_phy_prime;
    std::vector<double> liste_phy_prime_inverser;
    double phy_prime_premier;
   
    // Calcul des coefficients φ' de droite à gauche
    for(auto i{nb_travee} ; i>0 ; --i) {
        if(i==nb_travee) {
            // Pour la dernière travée, φ' = 0
            phy_prime_premier = 0;
        }
        else {
            // Calcul du coefficient φ' pour la travée courante
            // en utilisant le coefficient φ' de la travée suivante
            double phy_prime{b[i-1]/(c[i-1]+a[i]-b[i]*phy_prime_premier)};
            phy_prime_premier = phy_prime;
        }
        liste_phy_prime.push_back(phy_prime_premier);
    }
     
    // Inversion de l'ordre des coefficients pour correspondre à l'ordre des travées
    for(auto i{nb_travee} ; i>0 ; --i) {
        liste_phy_prime_inverser.push_back(liste_phy_prime[i-1]); 
    }
    return liste_phy_prime_inverser;   
}
