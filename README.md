
## - XpriT Robotics – WRO 2025 Future Engineers

### Engineering Materials

This repository contains the engineering materials of our autonomous vehicle model developed by **XpriT Robotics**, competing in the **WRO 2025 Future Engineers** category. It includes source code, documentation, design files, photos, schematics, and a driving demonstration video.

### Repository Structure

* **t-photos/** – Two team photos: a formal one and a light-hearted group shot.
* **v-photos/** – Six images of the vehicle: front, back, left, right, top, and bottom.
* **video/** – A `video.md` containing a link to a YouTube video (public or link-accessible) showing at least 30 seconds of autonomous driving—for each challenge type. 
* **schemes/** – Electromechanical schematic diagrams (JPEG, PNG, or PDF) showing how components (electronics, motors, sensors) connect. 
* **src/** – Control software source code for all programmed components.
* **models/** – Files for 3D-printed, laser-cut, or CNC-machined vehicle parts. (If none, the directory can be removed.)
* **other/** – Additional documentation: e.g., guide for connecting to SBC/SBM, hardware specs, communication protocols, datasets. (Can be removed if empty.)

---

### Introduction

*(To be completed by the team)*
Describe the code architecture:

* What modules the code consists of, and how they interface with the electromechanical components.
* The process to build, compile, and upload the code to the vehicle’s controllers.
* Highlight key design decisions and roles of each component.

---

### Engineering Journal

Include a thorough engineering journal documenting:

* Motivation behind design choices for mobility, sensors, navigation, and obstacle handling.
* Progress snapshots, iterations, failed experiments, and learning process.
* Final design rationale and testing methodology.
  This journal is required as part of the documentation and scoring criteria.

---

### Commit History

Ensure your GitHub history reflects active engineering development:

* **First commit**: at least 2 months before the competition, containing at least one-fifth of the final code.
* **Second commit**: at least 1 month before the competition.
* **Third commit**: at least 2 weeks before the competition.
  Additional commits are encouraged.

---

### Vehicle Challenges & Competition Focus

Your README should reflect awareness of the competition’s key aspects:

* The task involves navigating a randomly generated track (variable layout each round), using computer vision and sensor fusion to detect the track and vehicle state. 
* The vehicle must use open-source hardware and support planning & control beyond simple differential drive (e.g., steering mechanisms)
* Strategy, creativity, problem-solving, teamwork, and documentation are judged alongside execution and performance. 

---

### Sample Outline (Markdown)

```markdown
# XpriT Robotics – WRO 2025 Future Engineers

## Engineering Materials
This repository includes all engineering assets for our self-driving vehicle...

### Repository Structure
- **t-photos/**
- **v-photos/**
- **video/**
- **schemes/**
- **src/**
- **models/**
- **other/**

## Introduction
(Describe code modules, building process, component interactions...)

## Engineering Journal
(Design motivation, progress, reflections, final strategy...)

## Commit History
- First commit: YYYY-MM-DD
- Second commit: YYYY-MM-DD
- Third commit: YYYY-MM-DD

## Vehicle Challenges
(Explain sensor fusion, steering mechanics, autonomy, randomness of rounds...)


