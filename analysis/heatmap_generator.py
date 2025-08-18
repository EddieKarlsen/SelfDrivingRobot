#!/usr/bin/env python3
"""
Förbättrad Heatmap Generator för Robot Labyrinth Simulation
Förbättrad version med bättre visualisering som matchar referensbilden
"""

import json
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.colors import LinearSegmentedColormap, ListedColormap
import seaborn as sns
import argparse
import sys
from pathlib import Path

class RobotHeatmapGenerator:
    def __init__(self, json_file="robot_log.json", maze_file="maze_log.txt"):
        """Initialisera heatmap generator"""
        self.json_file = json_file
        self.maze_file = maze_file
        self.data = None
        self.maze_data = None
        
        # Definiera färgscheman som matchar referensbilden
        self.setup_colormaps()
    
    def setup_colormaps(self):
        """Skapa anpassade färgscheman"""
        # Heatmap färger (grön-gul-röd)
        self.heatmap_colors = ['#000000', '#FF0000', '#FFFF00', '#00FF00']
        self.heatmap_cmap = LinearSegmentedColormap.from_list(
            'robot_heat', self.heatmap_colors, N=256
        )
        
        # Maze färger
        maze_colors = ['#000000', '#808080', '#FFFFFF', '#0000FF', '#FF0000']  # svart, grå, vit, blå, röd
        self.maze_cmap = ListedColormap(maze_colors)

    def load_data(self):
        """Ladda JSON data från loggfil"""
        try:
            with open(self.json_file, 'r', encoding='utf-8') as f:
                self.data = json.load(f)
            print(f"Laddade data från {self.json_file}")
            return True
        except FileNotFoundError:
            print(f"Kunde inte hitta filen {self.json_file}")
            return False
        except json.JSONDecodeError as e:
            print(f"JSON-fel: {e}")
            return False

    def load_maze_from_txt(self, maze_file):
        """Ladda maze från separat textfil"""
        try:
            with open(maze_file, 'r', encoding='utf-8') as f:
                maze_data = json.load(f)
            return maze_data
        except Exception as e:
            print(f"Kunde inte ladda maze från {maze_file}: {e}")
            return None

    def parse_maze_grid(self, grid_strings):
        """Konvertera string-grid till numerisk array för visualisering"""
        maze_array = []
        for row_str in grid_strings:
            row = []
            for char in row_str:
                if char == '#':      # Vägg
                    row.append(1)
                elif char == 'B':    # Border/Kant
                    row.append(1)
                elif char == 'O':    # Open/Öppen
                    row.append(0)
                elif char == 'S':    # Start
                    row.append(2)
                elif char == 'G':    # Goal/Mål
                    row.append(3)
                else:
                    row.append(0)    # Default till öppen
            maze_array.append(row)
        return np.array(maze_array)

    def extract_movement_data(self, generation=None, best_only=False):
        """Extrahera rörelsedata från JSON"""
        movements = []
        maze_info = None
        
        if not self.data or 'generations' not in self.data:
            print("Ingen giltig data hittades")
            return movements, maze_info
        
        for gen_data in self.data['generations']:
            current_gen = gen_data['generation']
            if generation is not None and current_gen != generation:
                continue
            
            if maze_info is None and 'maze_info' in gen_data:
                maze_info = gen_data['maze_info']
            
            for individual in gen_data['individuals']:
                if best_only and not individual.get('is_best', False):
                    continue
                
                if 'movements' in individual:
                    for move in individual['movements']:
                        movements.append({
                            'x': move['position'][0],
                            'y': move['position'][1],
                            'generation': current_gen,
                            'individual_id': individual['id'],
                            'fitness': individual['fitness'],
                            'step': move['step'],
                            'action': move['action'],
                            'reached_goal': individual.get('reached_goal', False)
                        })
        
        print(f"Extraherade {len(movements)} rörelser")
        return movements, maze_info

    def create_improved_heatmap(self, movements, maze_info, title="Robot Movement Heatmap", 
                               output_file="improved_heatmap.png"):
        """Skapa förbättrad heatmap som matchar referensbilden"""
        if not movements or not maze_info:
            print("Ingen rörelsedata eller maze-info att visualisera")
            return False

        # Skapa maze array
        maze_array = self.parse_maze_grid(maze_info['layout'])
        rows, cols = maze_array.shape
        
        # Skapa heatmap grid baserat på rörelsedata
        heatmap = np.zeros((rows, cols), dtype=float)
        
        # Fyll heatmap med besöksfrekvens
        for move in movements:
            # Konvertera till grid-koordinater
            grid_x = int(np.clip(round(move['x']), 0, cols-1))
            grid_y = int(np.clip(round(move['y']), 0, rows-1))
            heatmap[grid_y, grid_x] += 1
        
        # Normalisera heatmap
        if heatmap.max() > 0:
            heatmap = heatmap / heatmap.max()
        
        # Skapa figure med flera subplots
        fig, axes = plt.subplots(2, 2, figsize=(16, 12))
        fig.suptitle(title, fontsize=16, fontweight='bold')
        
        # 1. Ren heatmap (som referensbilden)
        ax1 = axes[0, 0]
        self.create_clean_heatmap(ax1, heatmap, maze_array, "Alla Rörelser", maze_info)
        
        # 2. Endast lyckade individer
        ax2 = axes[0, 1] 
        successful_movements = [m for m in movements if m['reached_goal']]
        success_heatmap = np.zeros((rows, cols), dtype=float)
        
        for move in successful_movements:
            grid_x = int(np.clip(round(move['x']), 0, cols-1))
            grid_y = int(np.clip(round(move['y']), 0, rows-1))
            success_heatmap[grid_y, grid_x] += 1
            
        if success_heatmap.max() > 0:
            success_heatmap = success_heatmap / success_heatmap.max()
            
        self.create_clean_heatmap(ax2, success_heatmap, maze_array, 
                                f"Lyckade Rutter ({len(set(m['individual_id'] for m in successful_movements))} individer)",
                                maze_info)
        
        # 3. Generationsanalys
        ax3 = axes[1, 0]
        self.create_generation_analysis(ax3, movements, maze_array)
        
        # 4. Statistik
        ax4 = axes[1, 1]
        self.create_stats_visualization(ax4, movements)
        
        plt.tight_layout()
        plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"💾 Förbättrad heatmap sparad som {output_file}")
        return True

    def create_clean_heatmap(self, ax, heatmap, maze_array, title, maze_info=None):
        """Skapa ren heatmap som matchar referensbilden"""
        # Skapa composite bild
        display_array = np.zeros_like(maze_array, dtype=float)
        
        # Sätt väggar till -1 för svart färg
        wall_mask = (maze_array == 1)
        display_array[wall_mask] = -1
        
        # Sätt heatmap värden för öppna områden
        open_mask = (maze_array == 0) | (maze_array == 2) | (maze_array == 3)
        display_array[open_mask] = heatmap[open_mask]
        
        # Visa med anpassad colormap
        im = ax.imshow(display_array, cmap=self.heatmap_cmap, 
                      vmin=-1, vmax=1, interpolation='nearest', aspect='equal')
        
        # Hitta start och mål från maze_array om maze_info inte finns
        if maze_info:
            start_pos = maze_info.get('start', None)
            goal_pos = maze_info.get('goal', None)
        else:
            start_pos = None
            goal_pos = None
        
        # Hitta start och mål från grid om inte i maze_info
        if start_pos is None or goal_pos is None:
            start_positions = np.where(maze_array == 2)
            goal_positions = np.where(maze_array == 3)
            
            if len(start_positions[0]) > 0:
                start_pos = [start_positions[1][0], start_positions[0][0]]  # [x, y]
            if len(goal_positions[0]) > 0:
                goal_pos = [goal_positions[1][0], goal_positions[0][0]]  # [x, y]
        
        # Markera start och mål om de hittades
        if start_pos:
            ax.text(start_pos[0], start_pos[1], 'Start', ha='center', va='center', 
                   color='white', fontweight='bold', fontsize=8,
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='blue', alpha=0.7))
        if goal_pos:
            ax.text(goal_pos[0], goal_pos[1], 'Goal', ha='center', va='center', 
                   color='white', fontweight='bold', fontsize=8,
                   bbox=dict(boxstyle='round,pad=0.3', facecolor='red', alpha=0.7))
        
        ax.set_title(title, fontsize=12, fontweight='bold')
        ax.set_xticks([])
        ax.set_yticks([])
        
        # Lägg till colorbar
        cbar = plt.colorbar(im, ax=ax, shrink=0.8)
        cbar.set_label('Besöksfrekvens', rotation=270, labelpad=15)
        
        return im

    def create_generation_analysis(self, ax, movements, maze_array):
        """Visa utveckling över generationer"""
        generations = sorted(set(m['generation'] for m in movements))
        
        if len(generations) > 1:
            # Skapa heatmap för senaste generation
            latest_gen = max(generations)
            latest_movements = [m for m in movements if m['generation'] == latest_gen]
            
            rows, cols = maze_array.shape
            gen_heatmap = np.zeros((rows, cols), dtype=float)
            
            for move in latest_movements:
                grid_x = int(np.clip(round(move['x']), 0, cols-1))
                grid_y = int(np.clip(round(move['y']), 0, rows-1))
                gen_heatmap[grid_y, grid_x] += 1
            
            if gen_heatmap.max() > 0:
                gen_heatmap = gen_heatmap / gen_heatmap.max()
            
            self.create_clean_heatmap(ax, gen_heatmap, maze_array, 
                                    f"Generation {latest_gen}")
        else:
            ax.text(0.5, 0.5, 'Endast en generation\ntillgänglig', 
                   ha='center', va='center', transform=ax.transAxes)
            ax.set_title('Generationsanalys')

    def create_stats_visualization(self, ax, movements):
        """Skapa statistikvisualisering"""
        ax.axis('off')
        
        if not movements:
            ax.text(0.5, 0.5, "Ingen data tillgänglig", ha='center', va='center',
                   transform=ax.transAxes)
            return
        
        # Beräkna statistik
        generations = set(m['generation'] for m in movements)
        individuals = set(m['individual_id'] for m in movements)
        successful_individuals = set(m['individual_id'] for m in movements if m['reached_goal'])
        fitness_values = [m['fitness'] for m in movements]
        
        # Skapa statistiktext
        stats = [
            "📊 STATISTIK",
            "",
            f"Generationer: {min(generations)} - {max(generations)}",
            f"Antal individer: {len(individuals)}",
            f"Lyckade individer: {len(successful_individuals)}",
            f"Framgångsgrad: {len(successful_individuals)/len(individuals)*100:.1f}%",
            "",
            f"Totala rörelser: {len(movements):,}",
            f"Genomsnitt steg/individ: {len(movements)/len(individuals):.1f}",
            "",
            f"Fitness min: {min(fitness_values):.1f}",
            f"Fitness max: {max(fitness_values):.1f}",
            f"Fitness medel: {np.mean(fitness_values):.1f}",
        ]
        
        stats_text = '\n'.join(stats)
        ax.text(0.05, 0.95, stats_text, transform=ax.transAxes, 
               verticalalignment='top', fontsize=10, fontfamily='monospace',
               bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))

    def create_comparison_heatmaps(self, movements, maze_info, output_file="comparison_heatmaps.png"):
        """Skapa jämförelse mellan olika typer av heatmaps"""
        if not movements or not maze_info:
            return False
            
        maze_array = self.parse_maze_grid(maze_info['layout'])
        rows, cols = maze_array.shape
        
        # Skapa olika heatmaps
        all_heatmap = np.zeros((rows, cols), dtype=float)
        success_heatmap = np.zeros((rows, cols), dtype=float)
        fail_heatmap = np.zeros((rows, cols), dtype=float)
        
        successful_movements = [m for m in movements if m['reached_goal']]
        failed_movements = [m for m in movements if not m['reached_goal']]
        
        # Fyll heatmaps
        for move in movements:
            grid_x = int(np.clip(round(move['x']), 0, cols-1))
            grid_y = int(np.clip(round(move['y']), 0, rows-1))
            all_heatmap[grid_y, grid_x] += 1
            
        for move in successful_movements:
            grid_x = int(np.clip(round(move['x']), 0, cols-1))
            grid_y = int(np.clip(round(move['y']), 0, rows-1))
            success_heatmap[grid_y, grid_x] += 1
            
        for move in failed_movements:
            grid_x = int(np.clip(round(move['x']), 0, cols-1))
            grid_y = int(np.clip(round(move['y']), 0, rows-1))
            fail_heatmap[grid_y, grid_x] += 1
        
        # Normalisera
        for hmap in [all_heatmap, success_heatmap, fail_heatmap]:
            if hmap.max() > 0:
                hmap /= hmap.max()
        
        # Skapa figure
        fig, axes = plt.subplots(1, 3, figsize=(18, 6))
        fig.suptitle('Rörelseanalys Jämförelse', fontsize=16, fontweight='bold')
        
        heatmaps = [all_heatmap, success_heatmap, fail_heatmap]
        titles = ['(A) Alla Rörelser', '(B) Lyckade Rutter', '(C) Misslyckade Rutter']
        
        for i, (ax, hmap, title) in enumerate(zip(axes, heatmaps, titles)):
            self.create_clean_heatmap(ax, hmap, maze_array, title, maze_info)
        
        plt.tight_layout()
        plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"🔍 Jämförelse heatmaps sparad som {output_file}")
        return True

    def create_generation_evolution(self, movements, maze_info, output_file="generation_evolution.png"):
        """Skapa animation/jämförelse av generationsutveckling"""
        if not movements or not maze_info:
            return False
            
        generations = sorted(set(m['generation'] for m in movements))
        if len(generations) < 2:
            print("Behöver minst 2 generationer för evolution-analys")
            return False
        
        maze_array = self.parse_maze_grid(maze_info['layout'])
        rows, cols = maze_array.shape
        
        # Välj 4 jämnt fördelade generationer
        step = max(1, len(generations) // 4)
        selected_gens = generations[::step][:4]
        
        fig, axes = plt.subplots(2, 2, figsize=(16, 12))
        fig.suptitle('Evolution av Rörelsemönster', fontsize=16, fontweight='bold')
        
        axes_flat = axes.flatten()
        
        for i, gen in enumerate(selected_gens):
            if i >= 4:
                break
                
            gen_movements = [m for m in movements if m['generation'] == gen]
            gen_heatmap = np.zeros((rows, cols), dtype=float)
            
            for move in gen_movements:
                grid_x = int(np.clip(round(move['x']), 0, cols-1))
                grid_y = int(np.clip(round(move['y']), 0, rows-1))
                gen_heatmap[grid_y, grid_x] += 1
            
            if gen_heatmap.max() > 0:
                gen_heatmap = gen_heatmap / gen_heatmap.max()
            
            self.create_clean_heatmap(axes_flat[i], gen_heatmap, maze_array, 
                                    f"Generation {gen}", maze_info)
        
        # Dölj tomma subplots
        for i in range(len(selected_gens), 4):
            axes_flat[i].set_visible(False)
        
        plt.tight_layout()
        plt.savefig(output_file, dpi=300, bbox_inches='tight', facecolor='white')
        print(f"🧬 Generationsevolution sparad som {output_file}")
        return True

    def create_heatmap(self, movements, maze_info, title="Robot Movement Heatmap", 
                      output_file="heatmap.png", bins=50):
        """Förbättrad huvudfunktion för heatmap-skapande"""
        return self.create_improved_heatmap(movements, maze_info, title, output_file)

    def create_path_analysis(self, output_file="path_analysis.png"):
        """Skapa detaljerad analys av ruttval"""
        movements, maze_info = self.extract_movement_data()
        if not movements or not maze_info:
            print("Ingen data för ruttanalys")
            return False

        fig, axes = plt.subplots(2, 2, figsize=(16, 12))
        fig.suptitle('Detaljerad Ruttanalys', fontsize=16, fontweight='bold')

        # Gruppera rörelser per individ
        individual_paths = {}
        for move in movements:
            ind_id = move['individual_id']
            if ind_id not in individual_paths:
                individual_paths[ind_id] = []
            individual_paths[ind_id].append(move)

        # Sortera rörelser per individ efter steg
        for ind_id in individual_paths:
            individual_paths[ind_id].sort(key=lambda x: x['step'])

        maze_array = self.parse_maze_grid(maze_info['layout'])

        # 1. Alla rutter
        ax = axes[0, 0]
        ax.imshow(maze_array, cmap='gray', alpha=0.3)
        for ind_id, path in individual_paths.items():
            x_path = [p['x'] for p in path]
            y_path = [p['y'] for p in path]
            reached_goal = path[0]['reached_goal'] if path else False
            color = 'green' if reached_goal else 'red'
            alpha = 0.8 if reached_goal else 0.3
            ax.plot(x_path, y_path, color=color, alpha=alpha, linewidth=1)
        ax.set_title('Alla Individuella Rutter')
        
        # Lägg till start/mål markörer
        start_pos = maze_info.get('start', [1, 1])
        goal_pos = maze_info.get('goal', [23, 23])
        ax.plot(start_pos[0], start_pos[1], 'bo', markersize=8, label='Start')
        ax.plot(goal_pos[0], goal_pos[1], 'rs', markersize=8, label='Mål')
        ax.legend()

        # 2. Endast lyckade rutter
        ax = axes[0, 1]
        ax.imshow(maze_array, cmap='gray', alpha=0.3)
        successful_paths = {k: v for k, v in individual_paths.items() 
                          if v and v[0]['reached_goal']}
        for ind_id, path in successful_paths.items():
            x_path = [p['x'] for p in path]
            y_path = [p['y'] for p in path]
            ax.plot(x_path, y_path, 'g-', alpha=0.7, linewidth=2)
        ax.set_title(f'Lyckade Rutter ({len(successful_paths)} st)')
        ax.plot(start_pos[0], start_pos[1], 'bo', markersize=8)
        ax.plot(goal_pos[0], goal_pos[1], 'rs', markersize=8)

        # 3. Heatmap av startpositioner
        ax = axes[1, 0]
        start_positions = [(path[0]['x'], path[0]['y']) for path in individual_paths.values() if path]
        if start_positions:
            x_starts, y_starts = zip(*start_positions)
            ax.hexbin(x_starts, y_starts, gridsize=20, cmap='Blues')
            ax.set_title('Startpositioner Distribution')

        # 4. Heatmap av slutpositioner
        ax = axes[1, 1]
        end_positions = [(path[-1]['x'], path[-1]['y']) for path in individual_paths.values() if path]
        if end_positions:
            x_ends, y_ends = zip(*end_positions)
            ax.hexbin(x_ends, y_ends, gridsize=20, cmap='Reds')
            ax.set_title('Slutpositioner Distribution')

        plt.tight_layout()
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"🛤️ Ruttanalys sparad som {output_file}")
        return True

    def get_movement_stats_text(self, movements):
        """Skapa statistiktext för visualisering"""
        if not movements:
            return "Ingen data tillgänglig"
        
        generations = set(m['generation'] for m in movements)
        individuals = set(m['individual_id'] for m in movements)
        successful_individuals = set(m['individual_id'] for m in movements if m['reached_goal'])
        fitness_values = [m['fitness'] for m in movements]
        
        stats = [
            "📊 MOVEMENT STATISTIK:",
            f" Generationer: {min(generations)} - {max(generations)}",
            f" Antal individer: {len(individuals)}",
            f" Lyckade individer: {len(successful_individuals)} ({len(successful_individuals)/len(individuals)*100:.1f}%)",
            f" Totala rörelser: {len(movements)}",
            f" Genomsnitt steg/individ: {len(movements)/len(individuals):.1f}",
            f" Fitness range: {min(fitness_values):.1f} - {max(fitness_values):.1f}",
            "",
            "🎯 FRAMGÅNGSANALYS:",
            f" Framgångsgrad: {len(successful_individuals)}/{len(individuals)} individer"
        ]
        
        return '\n'.join(stats)

def main():
    """Huvudfunktion med utökade kommandoradsargument"""
    parser = argparse.ArgumentParser(description='Förbättrad Robot Heatmap Generator')
    parser.add_argument('--input', '-i', default='robot_log.json', 
                       help='Input JSON fil (default: robot_log.json)')
    parser.add_argument('--maze', '-m', help='Separat maze textfil')
    parser.add_argument('--generation', '-g', type=int, 
                       help='Specifik generation att analysera')
    parser.add_argument('--best-only', '-b', action='store_true', 
                       help='Endast bästa individer')
    parser.add_argument('--paths', '-p', action='store_true', 
                       help='Skapa ruttanalys')
    parser.add_argument('--comparison', '-c', action='store_true',
                       help='Skapa jämförelse mellan olika heatmaps')
    parser.add_argument('--evolution', '-e', action='store_true',
                       help='Skapa generationsevolution')
    parser.add_argument('--output', '-o', default='heatmap.png', 
                       help='Output filnamn')
    
    args = parser.parse_args()
    
    generator = RobotHeatmapGenerator(args.input, args.maze)
    
    if not generator.load_data():
        sys.exit(1)
    
    movements, maze_info = generator.extract_movement_data(
        generation=args.generation, 
        best_only=args.best_only
    )
    
    title = "Robot Movement Heatmap"
    if args.generation is not None:
        title += f" - Generation {args.generation}"
    if args.best_only:
        title += " (Bästa individer)"
    
    # Kör olika typer av analyser
    if args.paths:
        generator.create_path_analysis(args.output.replace('.png', '_paths.png'))
    elif args.comparison:
        generator.create_comparison_heatmaps(movements, maze_info, 
                                           args.output.replace('.png', '_comparison.png'))
    elif args.evolution:
        generator.create_generation_evolution(movements, maze_info,
                                            args.output.replace('.png', '_evolution.png'))
    else:
        generator.create_heatmap(movements, maze_info, title, args.output)

if __name__ == "__main__":
    main()