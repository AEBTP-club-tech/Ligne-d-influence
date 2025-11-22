import json 
import os 

def open_json(fils : str = "total_abscissas", src_fils = "influence_lines" ):
    """
    src_fils: 
        "analysis",
        "boundary_conditions",
        "influence_lines",
        "properties",
        "static_analysis"
    
    analysis_json = 
        "largest_moment_areas",
        "max_span_moments",
        "max_support_moments",
        "section_moment_areas",
        "section_rotation_areas",
        "section_shear_areas",
        "split_span_moment_areas",
        "support_moment_areas"

    boundary_conditions_json = 
        "support_moments"

    influence_lines = 
        "shear_abscissas",
        "span_deflections",
        "span_moments",
        "span_rotations",
        "span_shear_forces",
        "support_moments",
        "support_reactions",
        "total_abscissas"

    propreties_json = 
        "abscissas_of_moment_of_inertia",
        "coefficient_a",
        "coefficient_b",
        "coefficient_c",
        "moment_of_inertia",
        "phi_prime",
        "phi",
        "span_lengths",
        "young_modulus"

    static_analysis_json = 
        "abscissas",
        "bending_moments",
        "deflections",
        "rotations",
        "shear_abscissas",
        "shear_forces"
        """
    try:
        a = "Cas : 1"
        file_path = os.getcwd() + "/data/results/" + src_fils+"/"+ fils
        with open(file_path + ".json", "r", encoding="utf-8") as file:
            return json.load(file)
        
    except FileNotFoundError:
        try:
            a = "Cas : 2"
            file_path =os.getcwd() + "/../../../data/results/" + src_fils+"/"+ fils 
            with open(file_path + ".json", "r", encoding="utf-8") as file:
                return json.load(file)
        except:
            a = "Cas : 3"
            file_path =os.getcwd() + "/../data/results/" + src_fils+"/"+ fils 
            with open(file_path + ".json", "r", encoding="utf-8") as file:
                return json.load(file)
    
second_fils = [
    "analysis",
    "boundary_conditions",
    "influence_lines",
    "properties",
    "static_analysis"
]

analysis_json = [
    "largest_moment_areas",
    "max_span_moments",
    "max_support_moments",
    "section_moment_areas",
    "section_rotation_areas",
    "section_shear_areas",
    "split_span_moment_areas",
    "support_moment_areas"
]

boundary_conditions_json = [
    "support_moments"
]

influence_lines = [
    "shear_abscissas",
    "span_deflections",
    "span_moments",
    "span_rotations",
    "span_shear_forces",
    "support_moments",
    "support_reactions",
    "total_abscissas"
]

propreties_json = [
    "abscissas_of_moment_of_inertia",
    "coefficient_a",
    "coefficient_b",
    "coefficient_c",
    "moment_of_inertia",
    "phi_prime",
    "phi",
    "span_lengths",
    "young_modulus"
]

static_analysis_json = [
    "abscissas",
    "bending_moments",
    "deflections",
    "rotations",
    "shear_abscissas",
    "shear_forces"
]

# Configuration par défaut
DEFAULT_CONFIG = {
    "grid": True,
    "travee": True,
    "noeud": True,
    "vitesse": 10,
    "vitesse_bridge": 0.005,
    "legend": True,
    "axe_y_inverser": True,
    "default_matplotlib_color":True, 
    "default_matplotlib_style":True,
    "style": {
        "line_color": "royalblue",
        "grid_color": "#E0E0E0",
        "minor_grid_color": "#F5F5F5",
        "noeud_color": "#FF4444",
        "noeud_size": 100,
        "line_width": 2.5,
        "line_style": "-",
        "background_color": "#FFFFFF",
        "edge_color": "#2C3E50",
        "shadow_color": "gray",
        "shadow_alpha": 0.3,
        "title": "",
        "xlabel": "Longueur des travées",
        "ylabel": "Valeur",
        "axis_fontsize": 12,
        "font_family": "Times New Roman",
        "legend_position": "best",
        "marker_style": "o",
        "legend_fontsize": 10
    }
}

"""with open("Configuration.json" , "+w", encoding="utf-8") as file:
    json.dump(DEFAULT_CONFIG, file,ensure_ascii=False,indent=4) """

max_M_point = open_json("max_span_moments", "analysis")
max_M_areas = open_json("largest_moment_areas", "analysis")["plus_grande_aire"][0]

max_support_point = open_json("max_support_moments","analysis")
a = []
b = []
for i in open_json("support_moment_areas", "analysis").values():
    b.append(sum(i))
    a.append(abs(sum(i))) 

c = max(a) 
max_support_areas = a.index(c) 
if -max(a) == b[a.index(c)]:
    c = -c 

x_normal = open_json() 
x_forces = open_json("shear_abscissas")

# span_deflections = open_json("span_deflections")
# span_moments = open_json("span_moments")
# span_rotations = open_json("span_rotations")
# span_shear_forces = open_json("span_shear_forces")
# support_moments = open_json("support_moments")
# support_reactions = open_json("support_reactions") 

neouds = open_json("neouds_lengths", "properties")

distances = []
for i in range(len(neouds)):
    distance = round(neouds[i], 5)
    distances.append(f"{distance}")

# Fusionner avec la configuration par défaut
try:
    with open("Configuration.json","r",encoding="utf-8") as file:
        current_config = json.load(file)
except:
    print("Configuration.json missing")
    current_config = DEFAULT_CONFIG.copy()

text_format = {"family":"Times New Roman", "size":20} 