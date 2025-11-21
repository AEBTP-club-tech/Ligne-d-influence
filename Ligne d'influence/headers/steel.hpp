#ifndef STEEL_H
#define STEEL_H

#include <string>

class Steel {
public:
    // Constructor
    Steel(const std::string& grade = "S235", const std::string& condition = "normal");

    // Getters
    double getFyk() const { return fyk; }
    double getGammaS() const { return gamma_s; }
    double getFyd() const { return fyd; }
    double getEs() const { return Es; }
    double getSigmaSBarre() const { return sigma_s_barre; }
    double getFu() const { return fu; }
    double getG() const { return G; }
    double getNu() const { return nu; }
    double getRho() const { return rho; }
    double getEpsilonU() const { return epsilon_u; }
    std::string getGrade() const { return steel_grade; }
    
    // Methods
    bool isValidForWelding() const;
    double getAllowableStress(const std::string& load_type) const;
    double getShearStrength() const;

private:
    std::string steel_grade;  // Steel grade (e.g., S235, S275, S355)
    double fyk;              // Characteristic yield strength (MPa)
    double gamma_s;          // Partial safety factor
    double fyd;             // Design yield strength (MPa)
    double Es;              // Young's modulus (MPa)
    double sigma_s_barre;    // Maximum stress limit (MPa)
    double fu;              // Ultimate strength (MPa)
    double G;               // Shear modulus (MPa)
    double nu;              // Poisson's ratio
    double rho;             // Density (kg/m³)
    double epsilon_u;       // Ultimate strain
    
    void initializeProperties();  // Helper method to set properties based on grade
};

#endif // STEEL_H 