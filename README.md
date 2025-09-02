# Maze Solver using Genetic Algorithm

This project is an attempt to create a **Genetic Algorithm** to solve mazes. The program evolves a population of solutions to find the most efficient path through a maze.

## Table of Contents
- [License](#license)
- [Requirements](#requirements)
- [Installation](#installation)
- [Usage](#usage)
- [How It Works](#how-it-works)
- [Planned improvements](#Planned-improvements)

## License
This project is licensed under the MIT License and provided "as is" whithout garanties

## Requirements
To run this program, you will need:

- **Python 3.8** or higher
- **Python-bibliotek:** `numpy`, `matplotlib`, `Seaborn` 
- **A C++ compiler** if you plan to build compiled components
- **CMake** (for building the project)
## Installation
1. Clone this repository:
```bash
git clone <repository-url>
```

## Usage
In order to run this program:
1. Configure the settings in Configuration.h
Note: Setting POP_SIZE too high can crash the program (tested up to 50).
2. Create a build dir
``` bash
   mkdir build
   cd build
```
4. Build the programm
``` bash
cmake --build build
```
3. Run the program
``` bash
.\build\SelfDrivingRobot.exe
```

## How it works 
the program uses a Genetic Algorithm to solve maze by:
1. Initalizeing a population of solutions that has their uniqe attributes called chromosomes
2. Evaluatint their performance in the mazes note that a new maze is created after 25 generations this step i ussaly refered to ass a fitness critera
3. selecting the best candidates to next generation in this programm im using elitism selection and
   these chosen individuals are used as perants for the other individuals in the next generation following whats called crossover and mutation.
4. Repeating the process over multiple generations until the maximum generations has been meet

## Planned improvements
- change the selection method to an tournament based one as the current Elitism has a high probability for survivor bias
- increase the readability
- implement a better method for tracking energy usage
- make the sensors more rialistic as the current variant dosnt take into account such as noise
- change the engine logic to a pwm based system 
