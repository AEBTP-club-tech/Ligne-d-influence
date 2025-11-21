#include "steel.hpp"
#include <string>
#include <stdexcept>
#include <cmath>

Steel::Steel(const std::string& grade, const std::string& condition) {
    steel_grade = grade;
    
    // Set gamma_s based on condition
    if (condition == "accidentel") {
        gamma_s = 1.0;
    } else if (condition == "normal") {
        gamma_s = 1.15;
    } else if (condition == "sismique") {
        gamma_s = 1.1;
    } else {
        throw std::invalid_argument("Condition invalide. Utilisez 'normal', 'accidentel' ou 'sismique'");
    }

    initializeProperties();
}

void Steel::initializeProperties() {
    // Standard properties for all steel grades
    Es = 210000.0;    // MPa
    nu = 0.3;         // Poisson's ratio
    G = Es / (2 * (1 + nu));  // Shear modulus
    rho = 7850.0;     // kg/m³

    // Grade-specific properties
    if (steel_grade == "S235") {
        fyk = 235.0;
        fu = 360.0;
        epsilon_u = 0.26;
    } else if (steel_grade == "S275") {
        fyk = 275.0;
        fu = 430.0;
        epsilon_u = 0.24;
    } else if (steel_grade == "S355") {
        fyk = 355.0;
        fu = 510.0;
        epsilon_u = 0.22;
    } else if (steel_grade == "S450") {
        fyk = 450.0;
        fu = 550.0;
        epsilon_u = 0.20;
    } else {
        throw std::invalid_argument("Grade d'acier non supporté");
    }

    // Calculate derived properties
    fyd = fyk / gamma_s;
    sigma_s_barre = 0.8 * fyk;
}

bool Steel::isValidForWelding() const {
    // Vérification de la soudabilité basée sur le grade
    return (steel_grade == "S235" || steel_grade == "S275" || steel_grade == "S355");
}

double Steel::getAllowableStress(const std::string& load_type) const {
    if (load_type == "statique") {
        return fyd;
    } else if (load_type == "fatigue") {
        return 0.5 * fyd;
    } else if (load_type == "dynamique") {
        return 0.7 * fyd;
    }
    throw std::invalid_argument("Type de charge invalide");
}

double Steel::getShearStrength() const {
    // Résistance au cisaillement (selon Eurocode 3)
    return fyd / sqrt(3.0);
}
