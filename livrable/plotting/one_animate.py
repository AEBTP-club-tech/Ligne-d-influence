from utils import *
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

def animate_curvature(Curve, span, section): 
    # Create the figure and axis
    fig, ax = plt.subplots()
    line, = ax.plot([], [], 'b-', lw=2)

    if Curve == "span_shear_forces":
        x_normal = open_json("shear_abscissas")[span][section]
    else:
        x_normal = open_json()
    
    curve_data = open_json(Curve)[span][section]
    
    # Add margins to better visualize the curve
    x_margin = (max(x_normal) - min(x_normal)) * 0.1
    y_margin = (max(curve_data) - min(curve_data)) * 0.1
    
    # Set the plot limits with margins
    ax.set_xlim(min(x_normal) - x_margin, max(x_normal) + x_margin)
    ax.set_ylim(min(curve_data) - y_margin, max(curve_data) + y_margin)
    ax.set_xlabel('Position')
    ax.set_ylabel('Moment' if Curve == "span_moments" else 'Shear Force')
    ax.set_title('Moment Animation' if Curve == "span_moments" else 'Shear Force Animation')
    ax.grid(True)  # Add grid for better visualization
    
    # Initialize function for the animation
    def init():
        line.set_data([], [])
        return line,
    
    # Animation function
    def animate(frame):
        # Animate up to the current frame
        x = x_normal[:frame]
        y = curve_data[:frame]
        line.set_data(x, y)
        return line,
    
    # Create the animation with repeat enabled
    anim = FuncAnimation(fig, animate, init_func=init,
                        frames=len(x_normal), interval=50,
                        blit=True, repeat=True)  # Set repeat to True
    
    plt.show()
    return anim

# Call the animation function
if __name__ == "__main__":
    animation = animate_curvature("span_moments", 0, 15) 






