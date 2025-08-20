#include <sys/stat.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../include/configuration.h"
#include "../include/types.h"
#include "../include/menus.h"
#include "../include/sims.h"
#include "../include/logger.h"
#include "../include/maze.h"

int checkInput(char *choice_buffer, size_t buf_size, int *choice) {
    if (!fgets(choice_buffer, buf_size, stdin)) {
        printf("Error reading input. Please try again.\n");
        return -1;
    }

    char *endptr;
    *choice = strtol(choice_buffer, &endptr, 10);

    if (endptr == choice_buffer) {
        printf("Invalid input! Please enter a number.\n");
        return -1;
    }

    while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') {
        endptr++;
    }
    if (*endptr != '\0') {
        printf("Invalid input! Please enter only a number.\n");
        return -1;
    }

    if (*choice < 1 || *choice > 4) {
        printf("Invalid choice! Please enter a number between 1 and 4.\n");
        return -1;
    }
    return 0;
}

void mainmenu()
{
    int choice;
    char choice_buffer[100];
    Individual elite;
    int use_elite;
    Simulationcontext context = {
        .start_x = 0, .start_y = 0,
        .goal_x = 0, .goal_y = 0,
        .maze_width = DEFAULT_MAZE_WIDTH,
        .maze_height = DEFAULT_MAZE_HEIGHT,
        .maze = NULL,
        .sensors = {
            {0, 0, 0, 50},
            {0, 0, -M_PI/2, 50},
            {0, 0, M_PI/2, 50},
            {0, 0, -M_PI/4, 30},
            {0, 0, M_PI/4, 30}
        }
    };

    printf("Welcome to this simulation program\n");
    printf("Please select your desired operation:\n");
    
    while(true) 
    {
        printf("\n=== MAIN MENU ===\n");
        printf("1. Run simulation\n");
        printf("2. Analysis submenu\n");
        printf("3. Load maze (TBD)\n");
        printf("4. Quit program\n");
        printf("Enter your choice (1-4):");
        
        if (checkInput(choice_buffer, sizeof(choice_buffer), &choice) != 0) {
        continue;
        }
        
        switch (choice) {
            case 1:
                printf("\nStarting simulation...\n");
                use_elite = load_best_individual_from_file_wrapper(&elite, "robot_log.json");
                
                if (use_elite) {
                    printf("Elite individual found, fitness %.2f\n", elite.fitness);
                } else {
                    printf("No elite individual found, starting from scratch\n");
                }
                
                simulate_population_batch(&context, &elite, use_elite);
                printf("\nEvolution completed!\n");
                break;

                
            
            case 2:
                analysis_submenu();
                break;
            
            case 3:
                printf("\nLoad maze feature not implemented yet.\n");
                break;
            
            case 4:
                printf("\nThank you for using the simulation program. Goodbye!\n");
                
                // Cleanup before exit
                if (context.maze) {
                    free_matrix(context.maze, context.maze_height);
                }
                return;
            
            default:
                printf("Unexpected error. Please try again.\n");
                break;
        }
    }
}

void analysis_submenu() {
    char choice_buffer[100];
    int choice;
    
    while(true) {
        printf("\n=== HEATMAPS & VISUALIZATION ===\n");
        printf("1. Basic Heatmap (all movements)\n");
        printf("2. Heatmap for specific generation\n");
        printf("3. Heatmap for best individuals\n");
        printf("4. Compare first and last generations\n");
        printf("5. Create GIF animation\n");
        printf("6. Return to main menu\n");
        printf("Choose option (1-6): ");
        
        if (!fgets(choice_buffer, sizeof(choice_buffer), stdin)) {
            printf("Error reading input. Please try again.\n");
            continue;
        }
        
        char *endptr;
        choice = strtol(choice_buffer, &endptr, 10);
        
        // Validate input
        while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') {
            endptr++;
        }
        if (endptr == choice_buffer || *endptr != '\0' || choice < 1 || choice > 6) {
            printf("Invalid choice! Enter a number between 1-6.\n");
            continue;
        }
        
        switch (choice) 
        {
            case 1: {
                printf("\nCreating basic heatmap...\n");
                
                // Windows-kommandon för att skapa mappar
                system("if not exist \"data\\visualizations\\heatmaps\" mkdir \"data\\visualizations\\heatmaps\"");
                
                // Windows-kommando för Python
                char command[512];
                snprintf(command, sizeof(command), 
                    "cd analysis && python heatmap_generator.py --input ..\\robot_log.json --maze ..\\maze_log.txt --output ..\\data\\visualizations\\heatmaps\\basic_heatmap.png");
                int result = system(command);
                if (result == 0) {
                    printf("Heatmap created: data\\visualizations\\heatmaps\\basic_heatmap.png\n");
                } else {
                    printf("Error creating heatmap. Check that:\n");
                    printf("- Python is installed and in PATH\n");
                    printf("- Required Python packages are installed\n");
                    printf("- robot_log.json exists and is valid\n");
                }
                break;
            }
            
            case 2: {
                printf("Enter generation to analyze: ");
                char gen_buffer[50];
                if (fgets(gen_buffer, sizeof(gen_buffer), stdin)) 
                {
                    int generation = atoi(gen_buffer);
                    if (generation >= 0) {
                        char command[512];
                        system("if not exist \"data\\visualizations\\heatmaps\" mkdir \"data\\visualizations\\heatmaps\"");
                        snprintf(command, sizeof(command), 
                                "cd analysis && python heatmap_generator.py --input ..\\robot_log.json --generation %d --output ..\\data\\visualizations\\heatmaps\\gen_%d_heatmap.png", 
                                generation, generation);
                        printf("\nCreating heatmap for generation %d...\n", generation);
                        
                        int result = system(command);
                        if (result == 0) {
                            printf("Heatmap created: data\\visualizations\\heatmaps\\gen_%d_heatmap.png\n", generation);
                        } else {
                            printf("Error creating heatmap (check that the generation exists)\n");
                        }
                    } else {
                        printf("Invalid generation number!\n");
                    }
                }


                break;
            }
            
            case 3: {
                printf("\nCreating heatmap for best individuals...\n");
                system("if not exist \"data\\visualizations\\heatmaps\" mkdir \"data\\visualizations\\heatmaps\"");
                
                char command[512];
                snprintf(command, sizeof(command),
                    "cd analysis && python heatmap_generator.py --input ..\\robot_log.json --best-only --output ..\\data\\visualizations\\heatmaps\\best_heatmap.png");
                
                int result = system(command);
                if (result == 0) {
                    printf("Heatmap created: data\\visualizations\\heatmaps\\best_heatmap.png\n");
                } else {
                    printf("Error creating heatmap\n");
                }
                break;
            }
            
            case 4: {
                printf("\nCreating generation comparison...\n");
                system("if not exist \"data\\visualizations\\reports\" mkdir \"data\\visualizations\\reports\"");
                
                char command[512];
                snprintf(command, sizeof(command),
                    "cd analysis && python heatmap_generator.py --input ..\\robot_log.json --comparison --output ..\\data\\visualizations\\reports\\generation_comparison.png");
                
                int result = system(command);
                if (result == 0) {
                    printf("Comparison created: data\\visualizations\\reports\\generation_comparison.png\n");
                } else {
                    printf("Error creating comparison\n");
                }
                break;
            }
            
            case 5: {
                printf("\n which generation?");
                if (!fgets(choice_buffer, sizeof(choice_buffer), stdin)) {
                    printf("Error reading input. Please try again.\n");
                    continue;
                }
                
                char *endptr;
                int desired_generation = strtol(choice_buffer, &endptr, 10);
                
            
                while (*endptr == ' ' || *endptr == '\t' || *endptr == '\n') {
                    endptr++;
                }
                if (endptr == choice_buffer || *endptr != '\0' || desired_generation < 0 || desired_generation > NUM_GENERATIONS) {
                    printf("Invalid choice! Enter a number between 0- %d.\n" NUM_GENERATIONS);
                    continue;
                }

                int generation =(desired_generation /  PHASES_PER_GENERATION);
                int start = generation * PHASES_PER_GENERATION;
                int end = start + PHASES_PER_GENERATION -1;

                char command[512];
                snprintf(command, sizeof(command),
                    "cd analysis && python heatmap_generator.py --start %d --end %d --input ..\\robot_log.json --maze ..\\maze_log.txt --output ..\\data\\visualizations\\heatmaps\\multigenerational_heatmap.png",
                    start, end);
                int result = system(command);
                if (result == 0) {
                    printf("Heatmap created: data\\visualizations\\heatmaps\\multigenerational_heatmap.png\n");
                } else {
                    printf("Error creating heatmap. Check that:\n");
                    printf("- Python is installed and in PATH\n");
                    printf("- Required Python packages are installed\n");
                    printf("- robot_log.json exists and is valid\n");
                }
                
                break;
            }
            
            case 6:
                printf("Returning to main menu...\n");
                return;
        }
        
        printf("\nPress Enter to continue...");
        getchar();
    }
}