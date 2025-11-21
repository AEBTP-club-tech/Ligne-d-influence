#include "beton.hpp"
#include <sstream>
#include <cmath>
#include <stdexcept>

namespace {
    constexpr double GAMMA_C_GENERAL = 1.5;
    constexpr double GAMMA_C_SPECIAL = 1.2;
    constexpr double FCTK_005_FACTOR = 0.7;
    constexpr double FCTK_095_FACTOR = 1.3;
    constexpr double ECM_BASE = 22000.0;
    constexpr double FCM_OFFSET = 8.0;
    
    // Nouvelles constantes
    constexpr double DEFAULT_POISSON = 0.2;
    constexpr double DEFAULT_ALPHA_T = 10e-6;  // /°C
    constexpr double DEFAULT_RHO = 2400.0;     // kg/m³
}

Beton::Beton(const std::string& classe,
             const std::string& preference,
             const std::string& prise,
             int temps,
             const std::string& condition) 
    : t(temps), prise(prise) {
    
    if (t <= 0) {
        throw std::invalid_argument("Le temps doit être positif");
    }

    // Parse fck string (format: "C35/45")
    try {
        std::stringstream ss(classe);
        std::string token;
        if (!std::getline(ss, token, '/')) {
            throw std::invalid_argument("Format de classe invalide");
        }
        if (token.empty() || token[0] != 'C') {
            throw std::invalid_argument("La classe doit commencer par 'C'");
        }
        fck_cil = std::stoi(token.substr(1));
        
        if (!std::getline(ss, token)) {
            throw std::invalid_argument("Format de classe invalide");
        }
        int fckCub = std::stoi(token);

        fck = (preference == "cylindrique") ? fck_cil : fckCub;
    } catch (const std::exception& e) {
        throw std::invalid_argument("Format de classe invalide: " + std::string(e.what()));
    }

    gamma_c = (condition == "general") ? GAMMA_C_GENERAL : GAMMA_C_SPECIAL;

    // Calculate basic properties
    fcm = Fcm();
    bcc = Bcc();
    fctm = Fctm();
    fctk_005 = FCTK_005_FACTOR * fctm;
    fctk_095 = FCTK_095_FACTOR * fctm;
    fctd = fctk_005 / gamma_c;
    Ecm = ECM_BASE * std::pow(fcm/10.0, 0.3);
    fcd = fck / gamma_c;

    // Initialize additional properties
    initialize_deformation_properties();
    initialize_physical_properties();
    initialize_creep_properties();
}

void Beton::initialize_deformation_properties() {
    // Selon l'Eurocode 2 Table 3.1
    if (fck <= 50) {
        epsilon_c1 = 0.002;
        epsilon_cu1 = 0.0035;
        epsilon_c2 = 0.002;
        epsilon_cu2 = 0.0035;
        epsilon_c3 = 0.00175;
        epsilon_cu3 = 0.0035;
    } else {
        // Pour les bétons haute performance
        epsilon_c1 = 0.002 + 0.000085 * std::pow(fck - 50.0, 0.5);
        epsilon_cu1 = 0.0035 - (fck - 50.0)/200.0;
        epsilon_c2 = 0.002 + 0.000085 * std::pow(fck - 50.0, 0.5);
        epsilon_cu2 = 0.0035 - (fck - 50.0)/200.0;
        epsilon_c3 = epsilon_c2;
        epsilon_cu3 = epsilon_cu2;
    }
}

void Beton::initialize_physical_properties() {
    nu = DEFAULT_POISSON;
    alpha_t = DEFAULT_ALPHA_T;
    rho = DEFAULT_RHO;
    G = Ecm / (2.0 * (1.0 + nu));
}

void Beton::initialize_creep_properties() {
    // Coefficients de base pour le fluage selon l'Eurocode 2
    phi_0 = 2.0;  // Valeur de base à ajuster selon l'humidité relative
    beta_h = 1.5 * (1.0 + (0.012 * std::pow(50.0, 0.5))) * 100.0;
    beta_fcm = 16.8 / std::sqrt(fcm);
    beta_t0 = 1.0 / (0.1 + std::pow(t, 0.20));
    
    // Coefficients pour le retrait
    epsilon_cd0 = 0.85 * ((220.0 + 110.0 * alpha_t) * std::exp(-alpha_t * fcm)) * 1e-6;
    epsilon_ca = -2.5 * (fck - 10.0) * 1e-6;
}

double Beton::calculate_creep_coefficient(double t, double t0, double h0) const {
    if (t <= t0) {
        throw std::invalid_argument("Le temps t doit être supérieur à t0");
    }
    
    double beta_h_t = std::min(1.5 * (1.0 + std::pow(0.012 * h0, 0.5)) * 100.0, 1500.0);
    double beta_c_t_t0 = std::pow((t - t0) / (beta_h_t + t - t0), 0.3);
    
    return phi_0 * beta_c_t_t0;
}

double Beton::calculate_shrinkage_strain(double t, double ts, double h0) const {
    if (t <= ts) {
        throw std::invalid_argument("Le temps t doit être supérieur à ts");
    }
    
    double beta_ds_t_ts = (t - ts) / ((t - ts) + 0.04 * std::pow(h0, 1.5));
    double epsilon_cd_t = beta_ds_t_ts * epsilon_cd0;
    
    double beta_as_t = 1.0 - std::exp(-0.2 * std::sqrt(t));
    double epsilon_ca_t = beta_as_t * epsilon_ca;
    
    return epsilon_cd_t + epsilon_ca_t;
}

double Beton::calculate_thermal_strain(double delta_T) const {
    return alpha_t * delta_T;
}

double Beton::calculate_shear_modulus() const {
    return G;
}

double Beton::Fcm() const {
    if (t >= 28) {
        return fck + FCM_OFFSET;
    }
    return Bcc() * (fck + FCM_OFFSET);
}

double Beton::Bcc() const {
    double s;
    if (prise == "normale") {
        s = 0.2;
    } else if (prise == "rapide") {
        s = 0.25;
    } else if (prise == "lent") {
        s = 0.38;
    } else {
        throw std::invalid_argument("Type de prise invalide");
    }
    return std::exp(s * (1.0 - std::sqrt(28.0/t)));
}

double Beton::Fctm() const {
    if (fck <= 50) {
        return 0.3 * std::pow(fck, 2.0/3.0);
    }
    return 2.12 * std::log1p(fcm/10.0);
}

double Beton::Fck() const {
    return (t < 28) ? fcm - FCM_OFFSET : fck;
}

double Beton::Fctm_2() const {
    const double s = (t < 28) ? 1.0 : 2.0/3.0;
    return Fctm() * std::pow(bcc, s);
} 