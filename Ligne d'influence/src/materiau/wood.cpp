#include "wood.hpp"
#include <stdexcept>

namespace EC5 {

WoodProperties WoodCalculator::getWoodProperties(WoodStrengthClass strength_class) {
    WoodProperties props{};
    
    // Initialize properties based on strength class according to EN 338
    switch (strength_class) {
        case WoodStrengthClass::C24:  // Common structural softwood
            props.fm_k = 24.0;     // Bending strength
            props.ft_0_k = 14.0;   // Tension parallel
            props.ft_90_k = 0.4;   // Tension perpendicular
            props.fc_0_k = 21.0;   // Compression parallel
            props.fc_90_k = 2.5;   // Compression perpendicular
            props.fv_k = 4.0;      // Shear
            props.E_0_mean = 11.0; // Mean MOE parallel
            props.E_0_05 = 7.4;    // 5% MOE parallel
            props.E_90_mean = 0.37; // Mean MOE perpendicular
            props.G_mean = 0.69;   // Mean shear modulus
            props.rho_k = 350.0;   // Characteristic density
            props.rho_mean = 420.0; // Mean density
            break;
            
        case WoodStrengthClass::C30:
            props.fm_k = 30.0;
            props.ft_0_k = 18.0;
            props.ft_90_k = 0.4;
            props.fc_0_k = 23.0;
            props.fc_90_k = 2.7;
            props.fv_k = 4.0;
            props.E_0_mean = 12.0;
            props.E_0_05 = 8.0;
            props.E_90_mean = 0.40;
            props.G_mean = 0.75;
            props.rho_k = 380.0;
            props.rho_mean = 460.0;
            break;

        case WoodStrengthClass::D30:  // Hardwood example
            props.fm_k = 30.0;
            props.ft_0_k = 18.0;
            props.ft_90_k = 0.6;
            props.fc_0_k = 23.0;
            props.fc_90_k = 8.0;
            props.fv_k = 4.0;
            props.E_0_mean = 11.0;
            props.E_0_05 = 9.2;
            props.E_90_mean = 0.73;
            props.G_mean = 0.69;
            props.rho_k = 530.0;
            props.rho_mean = 640.0;
            break;

        // Add more strength classes as needed
        
        default:
            throw std::invalid_argument("Unsupported wood strength class");
    }
    
    return props;
}

ModificationFactors WoodCalculator::getModificationFactors(
    ServiceClass service_class,
    LoadDurationClass load_duration,
    [[maybe_unused]] WoodStrengthClass strength_class
) {
    ModificationFactors factors;
    
    // k_mod values according to EC5 Table 3.1
    switch (service_class) {
        case ServiceClass::CLASS_1:
            switch (load_duration) {
                case LoadDurationClass::PERMANENT:      factors.k_mod = 0.60; break;
                case LoadDurationClass::LONG_TERM:     factors.k_mod = 0.70; break;
                case LoadDurationClass::MEDIUM_TERM:   factors.k_mod = 0.80; break;
                case LoadDurationClass::SHORT_TERM:    factors.k_mod = 0.90; break;
                case LoadDurationClass::INSTANTANEOUS: factors.k_mod = 1.10; break;
            }
            break;
            
        case ServiceClass::CLASS_2:
            switch (load_duration) {
                case LoadDurationClass::PERMANENT:      factors.k_mod = 0.60; break;
                case LoadDurationClass::LONG_TERM:     factors.k_mod = 0.70; break;
                case LoadDurationClass::MEDIUM_TERM:   factors.k_mod = 0.80; break;
                case LoadDurationClass::SHORT_TERM:    factors.k_mod = 0.90; break;
                case LoadDurationClass::INSTANTANEOUS: factors.k_mod = 1.10; break;
            }
            break;
            
        case ServiceClass::CLASS_3:
            switch (load_duration) {
                case LoadDurationClass::PERMANENT:      factors.k_mod = 0.50; break;
                case LoadDurationClass::LONG_TERM:     factors.k_mod = 0.55; break;
                case LoadDurationClass::MEDIUM_TERM:   factors.k_mod = 0.65; break;
                case LoadDurationClass::SHORT_TERM:    factors.k_mod = 0.70; break;
                case LoadDurationClass::INSTANTANEOUS: factors.k_mod = 0.90; break;
            }
            break;
    }
    
    // k_def values according to EC5 Table 3.2
    switch (service_class) {
        case ServiceClass::CLASS_1: factors.k_def = 0.60; break;
        case ServiceClass::CLASS_2: factors.k_def = 0.80; break;
        case ServiceClass::CLASS_3: factors.k_def = 2.00; break;
    }
    
    // k_h calculation (simplified version)
    factors.k_h = 1.0;  // Default value, should be calculated based on member height
    
    // γM (partial factor for material properties) according to EC5
    factors.gamma_M = 1.3;  // Standard value for solid timber
    
    return factors;
}

} // namespace EC5
