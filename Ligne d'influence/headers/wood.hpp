#ifndef WOOD_HPP
#define WOOD_HPP

namespace EC5 {

// Wood strength classes according to EN 338
enum class WoodStrengthClass {
    C14, C16, C18, C20, C22, C24, C27, C30, C35, C40, C45, C50,  // Softwood
    D18, D24, D30, D35, D40, D50, D60, D70                        // Hardwood
};

// Service classes according to EC5
enum class ServiceClass {
    CLASS_1,  // Indoor, heated - moisture content < 12%
    CLASS_2,  // Covered, unheated - moisture content < 20%
    CLASS_3   // Exposed to weather - moisture content > 20%
};

// Load duration classes
enum class LoadDurationClass {
    PERMANENT,      // > 10 years
    LONG_TERM,     // 6 months - 10 years
    MEDIUM_TERM,   // 1 week - 6 months
    SHORT_TERM,    // < 1 week
    INSTANTANEOUS  // Wind, accidental loads
};

struct WoodProperties {
    // Characteristic values in N/mm²
    double fm_k;    // Bending strength
    double ft_0_k;  // Tensile strength parallel to grain
    double ft_90_k; // Tensile strength perpendicular to grain
    double fc_0_k;  // Compression strength parallel to grain
    double fc_90_k; // Compression strength perpendicular to grain
    double fv_k;    // Shear strength
    
    // Stiffness properties in kN/mm²
    double E_0_mean;   // Mean modulus of elasticity parallel to grain
    double E_0_05;    // 5% modulus of elasticity parallel to grain
    double E_90_mean;  // Mean modulus of elasticity perpendicular to grain
    double G_mean;     // Mean shear modulus
    
    // Density in kg/m³
    double rho_k;      // Characteristic density
    double rho_mean;   // Mean density
};

// Modification factors
struct ModificationFactors {
    double k_mod;     // Modification factor for duration of load and moisture
    double k_def;     // Deformation factor for creep
    double k_h;       // Size effect factor for height
    double gamma_M;   // Partial factor for material properties
};

class WoodCalculator {
public:
    static ModificationFactors getModificationFactors(
        ServiceClass service_class,
        LoadDurationClass load_duration,
        WoodStrengthClass strength_class
    );
    
    static WoodProperties getWoodProperties(WoodStrengthClass strength_class);
    
    // Design strength calculation
    static double getDesignStrength(
        double characteristic_strength,
        double k_mod,
        double gamma_M
    ) {
        return k_mod * characteristic_strength / gamma_M;
    }
};

} // namespace EC5

#endif // WOOD_HPP
