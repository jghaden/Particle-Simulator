# Particle Simulator

A particle simulator written in C++ using **GLFW**, **GLEW**, and **GLM**. The simulation is capable of handling **10-20 thousand** particles in realtime.

# Overview

1. [Preview](#preview)
2. [Controls](#controls)
3. [License](#license)

---

## Preview

https://github.com/user-attachments/assets/a8ae763a-8b8a-4134-98f5-9457c6fdb71e

---

## Controls

- **Mouse Controls:**

  - `Left mouse click`: Add particles
  - `Right mouse click`: Remove particles
  - `Ctrl + left mouse click`: Add single particle
  - `Mouse wheel down`: Decrease brush size
  - `Mouse wheel up`: Increase brush size

- **Keyboard Controls:**
  - **Simulation:**
    - `Space` : Pause & resume simulation
    - `Comma (,)` : Slow down time
    - `Period (.)` : Speed up time
    - `F` : Pause and step forward one frame
    - `R` : Remove all particles
  - **Particle brush:**
    - `[` : Decrease brush size
    - `]` : Increase brush size
    - `Ctrl + [` : Decrease brush size (increments of 10)
    - `Ctrl + [` : Increase brush size (increments of 10)
  - **Particle velocity:**
    - `W` : Increase vertical speed
    - `A` : Decrease horizontal speed
    - `S` : Decrease vertical speed
    - `D` : Increase horizontal speed
  - **Particle mass:**
    - `0-9` : 10<sup>0</sup> to 10<sup>9</sup> Kg
    - `Ctrl + 0-9` : 10<sup>10</sup> to 10<sup>19</sup> Kg
    - `Numpad 0-9` : 10<sup>20</sup> to 10<sup>29</sup> Kg
    - `Ctrl + Numpad 0-9` : 10<sup>30</sup> to 10<sup>39</sup> Kg
  - **Miscellaneous:**
    - `F1` : Toggle UI
    - `ESC` : Exit program

---

## License

This project is licensed under the MIT License. Feel free to use, modify, and distribute it as needed.
