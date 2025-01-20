# Particle Simulator

A particle simulator written in C++ using **GLFW**, **GLEW**, and **GLM**.

# Overview

1. [Preview](#preview)
2. [Controls](#controls)
3. [License](#license)

---

## Preview

Screenshot:

![Particle Simulator Screenshot](preview/screenshot.png)

Demo:

![Particle Simulator Video](preview/demo.gif)

---

## Controls

- **Mouse Controls:**

  - `Left mouse click`: Add particles
  - `Right mouse click`: Remove particles
  - `Ctrl + left mouse click`: Add single particle
  - `Mouse wheel down`: Decrease brush size
  - `Mouse wheel up`: Increase brush size

- **Keyboard Controls:**

  - `[` : Decrease brush size
  - `]` : Increase brush size
  - `F` : Pause and frame step forward
  - `R` : Remove all particles
  - `S` : Decrease particle velocity
  - `W` : Increase particle velocity
  - `Space` : Pause/Resume simulation
  - `Comma (,)` : Slow down time
  - `Period (.)` : Speed up time
  - **Particle mass:**
    -  `0-9` : 10<sup>0</sup> to 10<sup>9</sup> Kg
    - `Ctrl + 0-9` : 10<sup>10</sup> to 10<sup>19</sup> Kg
    - `Numpad 0-9` : 10<sup>20</sup> to 10<sup>29</sup> Kg
    - `Ctrl + Numpad 0-9` : 10<sup>30</sup> to 10<sup>39</sup> Kg
 
  - `F1` : Toggle UI
  - `ESC` : Exit program

---

## License

This project is licensed under the MIT License. Feel free to use, modify, and distribute it as needed.
