#ifndef __BETON_HPP__
#define __BETON_HPP__

#include <string>

class Beton {
public:
    Beton(const std::string& fck_str = "C35/45", 
          const std::string& preference = "cylindrique",
          const std::string& prise = "normale",
          int temps = 28,
          const std::string& condition = "general");
    
    ~Beton() = default;

    // Propriétés mécaniques de base
    double fck_cil;    // Cylindrical characteristic strength
    double fck;        // Characteristic compressive strength
    double fcm;        // Mean compressive strength
    double fctm;       // Mean tensile strengthZ
    double fctk_005;   // Lower characteristic tensile strength (5% fractile)
    double fctk_095;   // Upper characteristic tensile strength (95% fractile)
    double fcd;        // Design compressive strength
    double fctd;       // Design tensile strength
    double Ecm;        // Mean elastic modulus
    double gamma_c;    // Partial safety factor for concrete
    
    // Propriétés de déformation
    double epsilon_c1; // Strain at peak stress
    double epsilon_cu1;// Ultimate strain
    double epsilon_c2; // Strain at peak stress for parabola-rectangle diagram
    double epsilon_cu2;// Ultimate strain for parabola-rectangle diagram
    double epsilon_c3; // Strain at peak stress for bi-linear diagram
    double epsilon_cu3;// Ultimate strain for bi-linear diagram
    
    // Propriétés physiques
    double nu;         // Coefficient de Poisson (default: 0.2)
    double G;          // Module de cisaillement
    double alpha_t;    // Coefficient de dilatation thermique (default: 10e-6 /°C)
    double rho;        // Masse volumique en kg/m³
    
    // Propriétés de fluage et retrait
    double phi_0;      // Coefficient de fluage de base
    double beta_h;     // Coefficient dépendant de l'humidité relative
    double beta_fcm;   // Coefficient dépendant de la résistance du béton
    double beta_t0;    // Coefficient dépendant de l'âge du béton au chargement
    double epsilon_cd0;// Retrait de dessiccation de base
    double epsilon_ca; // Retrait endogène
    
    // Paramètres temporels
    int t;            // Age of concrete in days
    std::string prise;// Setting type (normale, rapide, lent)
    double bcc;       // Strength development coefficient

    // Méthodes de calcul supplémentaires
    double calculate_creep_coefficient(double t, double t0, double h0) const;
    double calculate_shrinkage_strain(double t, double ts, double h0) const;
    double calculate_thermal_strain(double delta_T) const;
    double calculate_shear_modulus() const;

private:
    // Helper methods
    double Fcm() const;
    double Bcc() const;
    double Fctm() const;
    double Fck() const;
    double Fctm_2() const;
    
    // Nouvelles méthodes privées
    void initialize_deformation_properties();
    void initialize_physical_properties();
    void initialize_creep_properties();
};

#endif 