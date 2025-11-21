#ifndef __RappFoc__
#define __RappFoc__
    #include <vector>
    
    /**
     * @class rapport_focau
     * @brief Classe permettant de calculer les rapports de focaux pour une structure
     * 
     * Cette classe est utilisée pour calculer les coefficients de focaux (φ et φ') 
     * qui sont essentiels dans l'analyse des structures continues, notamment pour
     * le calcul des moments de flexion.
     */
    class rapport_focau
    {
    private:
        std::vector<double> a;  ///< Vecteur des coefficients a pour chaque travée
        std::vector<double> b;  ///< Vecteur des coefficients b pour chaque travée
        std::vector<double> c;  ///< Vecteur des coefficients c pour chaque travée
        int nb_travee;         ///< Nombre total de travées dans la structure

    public:
        /**
         * @brief Constructeur par défaut
         * 
         * Initialise un objet rapport_focau avec des vecteurs vides et nb_travee = 0
         */
        rapport_focau()=default;

        /**
         * @brief Constructeur paramétré
         * @param liste_a Vecteur des coefficients a pour chaque travée
         * @param liste_b Vecteur des coefficients b pour chaque travée
         * @param liste_c Vecteur des coefficients c pour chaque travée
         * @param nombre_travee Nombre total de travées
         * 
         * Initialise l'objet avec les coefficients fournis pour chaque travée.
         * Les vecteurs a, b et c doivent avoir la même taille que nombre_travee.
         */
        rapport_focau(std::vector<double> liste_a,std::vector<double> liste_b,std::vector<double> liste_c, int nombre_travee);

        /**
         * @brief Calcule les coefficients φ de gauche à droite
         * @return Vecteur des coefficients φ [φ₁, φ₂, ..., φₙ]
         * 
         * Cette méthode calcule les coefficients φ en partant de la travée 1
         * jusqu'à la travée n. Ces coefficients sont utilisés pour le calcul
         * des moments de flexion en partant de la gauche de la structure.
         */
        std::vector<double> phy();

        /**
         * @brief Calcule les coefficients φ' de droite à gauche
         * @return Vecteur des coefficients φ' [φ'ₙ, φ'ₙ₋₁, ..., φ'₁]
         * 
         * Cette méthode calcule les coefficients φ' en partant de la travée n
         * jusqu'à la travée 1. Ces coefficients sont utilisés pour le calcul
         * des moments de flexion en partant de la droite de la structure.
         */
        std::vector<double> phy_prime();
    };
    

#endif


