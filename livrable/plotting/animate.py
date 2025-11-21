from utils import *
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

Curve = "span_rotations"

# Load data and configuration
if Curve == "span_shear_forces":
    shear_abscissas = np.array(open_json("shear_abscissas"))
    curvature = open_json(Curve)
    frames = []
    x_coords = []
    # Flatten the nested structure while keeping track of corresponding abscissas
    for span_idx, span in enumerate(curvature):
        for section_idx, section in enumerate(span):
            frames.append(section)
            x_coords.append(shear_abscissas[span_idx][section_idx])
    x_normal = x_coords  # Store all x coordinates
else:
    x_normal = np.array(open_json())  # Default is total_abscissas
    curvature = open_json(Curve)
    frames = []
    for span in curvature:
        frames.extend(span)

# Ensure x_normal starts from 0 and has no negative values
if Curve == "span_shear_forces":
    # For shear forces, each frame has its own x_normal values
    x_normal = [x - min(x) for x in x_normal]
else:
    x_normal = x_normal - min(x_normal)  # Shift all values so minimum is 0

curvature = open_json(Curve)
max_moment_data = open_json("max_" + Curve, "analysis")
config = current_config  # From utils.py
neouds = open_json("neouds_lengths", "properties")  # Load node positions
# Adjust neouds to match x_normal shift
neouds = np.array(neouds) 

# Get distances for x-axis labels
distances = []
for i in range(len(neouds)):
    distance = round(neouds[i], 5)
    distances.append(f"{distance}")

# Calculate the global min and max for y-axis
y_min = min(min(frame) for frame in frames)
y_max = max(max(frame) for frame in frames)

# Ensure the maximum moment from max_span_moments.json is included in the range
absolute_max = max_moment_data["valeur"]
y_max =  absolute_max * 1.1
y_min = -absolute_max * 1.1

def set_xlabel_properties(ax):
    # Set x-axis ticks and labels like in XLABEL function
    ax.set_xticks(neouds)
    ax.set_xticklabels(distances)
    plt.setp(ax.get_xticklabels(), rotation=45)

def draw_structural_elements(ax, progress=1.0):
    # Draw bridge spans (travées) if enabled
    if config["travee"]:
        if Curve == "span_shear_forces":
            # For shear forces, use the full range of x coordinates
            all_x = np.concatenate(x_normal)
            max_x = max(all_x)
            min_x = min(all_x)
            x_range = np.linspace(min_x, max_x, 100)
            visible_x = x_range[x_range <= max_x * progress]
        else:
            max_x = max(x_normal)
            cutoff = max_x * progress
            visible_x = x_normal[x_normal <= cutoff]
            
        if len(visible_x) > 0:  # Only plot if we have points
            visible_zeros = np.zeros(len(visible_x))
            if config["default_matplotlib_style"]:
                ax.plot(visible_x, visible_zeros,
                        linestyle='--',
                        alpha=0.5,
                        label='Travées')
            else:
                ax.plot(visible_x, visible_zeros,
                        color=config["style"]["edge_color"],
                        linestyle='--',
                        linewidth=config["style"]["line_width"]/2,
                        alpha=0.5,
                        label='Travées')
    
    # Draw nodes (noeuds) if enabled
    if config["noeud"]:
        if Curve == "span_shear_forces":
            all_x = np.concatenate(x_normal)
            max_x = max(all_x)
            visible_nodes = neouds[neouds <= max_x * progress]
        else:
            max_x = max(x_normal)
            visible_nodes = neouds[neouds <= max_x * progress]
            
        if len(visible_nodes) > 0:
            if config["default_matplotlib_style"]:
                ax.scatter(visible_nodes, [0]*len(visible_nodes),
                          label='Noeuds')
            else:
                ax.scatter(visible_nodes, [0]*len(visible_nodes),
                          color=config["style"]["noeud_color"],
                          s=config["style"]["noeud_size"],
                          zorder=5,
                          label='Noeuds')

def init():
    print("Initializing animation...")  # Debug print
    ax.clear()
    ax.grid(True)
    set_xlabel_properties(ax)
    
    # Set x limits based on the data type
    if Curve == "span_shear_forces":
        all_x = np.concatenate(x_normal)
        ax.set_xlim(min(all_x), max(all_x))
    else:
        ax.set_xlim(0, max(x_normal))
        
    ax.set_ylim(y_min, y_max)
    if config["axe_y_inverser"]:
        ax.invert_yaxis()
    return ax.plot([], [])[0],

def animate(frame_index):
    ax.clear()
    ax.grid(True)
    set_xlabel_properties(ax)
    
    # Calculate progress (0 to 1)
    progress = frame_index / len(frames)
    
    # Draw structural elements with full visibility
    draw_structural_elements(ax, progress)
    
    # Draw horizontal lines
    ax.axhline(y=absolute_max, color='r', linestyle='--', alpha=0.5, label=f'Max moment: {absolute_max:.2f}')
    ax.axhline(y=-absolute_max, color='r', linestyle='--', alpha=0.5)
    
    # Plot the moment diagram up to current frame
    if frame_index < len(frames):
        if Curve == "span_shear_forces":
            current_x = x_normal[frame_index]
            current_frame = frames[frame_index]
        else:
            current_x = x_normal
            current_frame = frames[frame_index]
            
        if config["default_matplotlib_style"]:
            ax.plot(current_x, current_frame,
                   label='Moment')
        else:
            ax.plot(current_x, current_frame,
                   color=config["style"]["line_color"],
                   linewidth=config["style"]["line_width"],
                   label='Moment')
    
    if config["default_matplotlib_style"]:
        ax.set_xlabel("Longueur des travées")
        ax.set_ylabel("Moment")
    else:
        ax.set_xlabel(config["style"]["xlabel"])
        ax.set_ylabel(config["style"]["ylabel"])
    
    # Set x limits based on the current frame's data for shear forces
    if Curve == "span_shear_forces":
        all_x = np.concatenate(x_normal)
        ax.set_xlim(min(all_x), max(all_x))
    else:
        ax.set_xlim(0, max(x_normal))
    
    ax.set_ylim(y_min, y_max)
    if config["axe_y_inverser"]:
        ax.invert_yaxis()
    if config["legend"]:
        ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    
    # Add some padding to the right side of the plot to accommodate the legend
    plt.subplots_adjust(right=0.85)
    
    return ax.plot([], [])[0],

# Create figure and axis
plt.close('all')  # Close any existing figures
fig, ax = plt.subplots(figsize=(10, 6))
if config["default_matplotlib_style"]:
    fig.patch.set_facecolor(config["style"]["background_color"])
    ax.set_facecolor(config["style"]["background_color"])

# Create animation
anim = FuncAnimation(
    fig,
    animate,
    frames=len(frames),
    init_func=init,
    interval=config["vitesse_bridge"],  # Animation speed
    blit=False,
    repeat=True
)

def save_animation():
    global anim 
    anim.save("FIG.gif") 
    
plt.show()


