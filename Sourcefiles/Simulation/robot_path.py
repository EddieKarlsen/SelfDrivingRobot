import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import imageio
import os

# Läs in loggdata
filename = "robot_gen_049.csv"
df = pd.read_csv(filename)

# Hitta bästa individen
best = df[df["is_best"] == 1].iloc[0]

# Simulera rörelse 
np.random.seed(42)
x, y = [1.0], [1.0]
for _ in range(best["steps_taken"]):
    dx, dy = np.random.uniform(-1, 1, 2)
    x.append(np.clip(x[-1] + dx, 0, 50))
    y.append(np.clip(y[-1] + dy, 0, 50))

# Gör GIF
fig, ax = plt.subplots(figsize=(6, 6))
frames = []
for i in range(len(x)):
    ax.clear()
    ax.set_xlim(0, 50)
    ax.set_ylim(0, 50)
    ax.set_title(f"Generation {best['generation']}, Step {i}")
    ax.plot(x[:i], y[:i], color='blue')
    ax.plot(x[i], y[i], 'ro')
    
    # Spara varje bild
    filename = f"frame_{i:03d}.png"
    fig.savefig(filename)
    frames.append(imageio.imread(filename))

# Spara till GIF
imageio.mimsave("robot_path.gif", frames, fps=10)

# Rensa tillfälliga filer
for f in frames:
    os.remove(f.filename)
print("GIF sparad som robot_path.gif")
