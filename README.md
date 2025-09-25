![Logo](t-photos/XpriT_Robotics_Logo.jpg)
# - XpriT Robotics – WRO 2025 Future Engineers
## Introduction

We are **XpriT Robotics**, a team dedicated to creating robotics projects for participation in various competitions. Our main focus is on education and knowledge sharing, using challenges as opportunities to grow and inspire others. 
Through our projects, we explore robotics and engineering challenges to expand our skills, creativity, and experience.
For this competition, we aim to design a robot capable of avoiding obstacles, completing a circuit, and parking in a designated area.

This repository contains the engineering materials of our autonomous vehicle model developed by **XpriT Robotics**, competing in the **WRO 2025 Future Engineers** category. It includes source code, documentation, design files, photos, schematics, and a driving demonstration video.

## Repository Structure

* [**Bill Of Materials**](#bill-of-materials) - Bill Of all the Materials we used in this project covering mechanical pieces, 3D printed pieces, components and electronics.
* [**Step by Step Asemble Guide**](#step-by-step-asemble-guide) – This section provides a step-by-step guide, covering everything from assembling the parts to uploading the code and configuring some of the components.
* [**Software and Code Explanation**](#software-and-code-explanation) – This section provides a step-by-step guide, covering everything from assembling the parts to uploading the code and configuring some of the components.
* [**Engineering Journal**](#engineering-journal) – A journal where describes the decisions we made along the project.

## Folders Content

* [**T-photos**](T-photos/) – Two team photos: a formal one and a funny group shot.
* [**R-photos**](R-photos/) – Six images of the vehicle: front, back, left, right, top, and bottom.
* [**Video**](Videos/) – A Video of the robot driving.
* [**Schemes**](Schemes/) – Electromechanical schematic diagrams showing how components connect. 
* [**3Dmodels**](3Dmodels/) – Files for 3D-printed pieces of the robot
* [**Code**](Code/) – Robot code for Arduino Nano

## Bill Of Materials

### Electronics:
* 1x Arduino Nano. (Old Bootloader)
* 1x TB6612FNG
* 1x Diode
* 1x LN7805
* 1x Capacitor 100uf
* 1x Push Button
* 1x Push Interruptor
* 1x Active Buzzer

### Components and Sensors:
* 3x Ultrasonic Sensor HC-SR04
* 1x BNO055 gyroscope
* 1x MG90S servomotor
* 1x DC motor N20 with Encoder

### Mechanical and 3D printed Pieces:

* 6x Bearings (4mm interior and 10mm exterior)
* 8x Nuts
* 8x Washers
* 1x Threaded rod
* Every 3D model in the [**3Dmodels**](3Dmodels/) folder

## Project Overview

### Mechanical Design

The robot’s mechanical system was developed with a focus on **compactness, stability, and maneuverability**. It consists of two main subsystems: the **Ackermann steering mechanism** and the **traction system**.

#### Ackermann Steering System
- The steering is based on a simplified **Ackermann geometry**, implemented with a 3D-printed mechanism.  
- A **servo motor** (MG90S) controls the main steering linkage.  
- The linkage transmits motion to two pivoting axles, allowing the front wheels to turn at different angles during curves.  
- This ensures that the inner wheel follows a tighter turning radius than the outer wheel, improving maneuverability and reducing tire slip.  

**Advantages of this design:**
- Simple to manufacture with minimal mechanical parts.  
- Easy to assemble and maintain.  
- Lightweight, compact, and optimized for small turns within the competition field.  


#### Traction System
- The rear wheels are powered by **N20 DC motors with encoders**, providing both propulsion and feedback for precise distance control.  
- The motors transmit motion through a **1:1 gear ratio system**, using custom **resin-printed gears** mounted on a metal axle.  
- The axle drives both rear wheels simultaneously, ensuring consistent forward movement.  

**Advantages of this design:**
- High precision thanks to the encoders.  
- Strong and reliable traction for both straight paths and turns.  
- Custom gears in resin are resistant to wear compared to standard PLA 3D prints.  


#### Chassis
- The chassis is a **compact 3D-printed frame** with integrated mounts for electronics, sensors, and the steering system.  
- The design evolved from early modular prototypes to a single-piece chassis, reducing assembly complexity and improving robustness.  
- Includes a **dedicated adjustable mount for the HuskyLens camera**, allowing flexible positioning during testing and competition.  

### Electronics Design

The electronic system of the robot integrates all the sensors, actuators, and controllers required for autonomous navigation.  
A complete circuit schematic is included in the repository under the folder: [**Schematics**](Schematics/).

#### Microcontroller
- **Arduino Nano** serves as the main controller.  
- It processes sensor inputs, executes navigation algorithms, and controls actuators.  
- Connected via USB for programming and debugging.

#### Motor Driver
- **TB6612FNG dual motor driver**.  
- Used to drive the N20 DC motors with PWM speed control.  
- Provides reliable performance with low heat dissipation.

#### Motors and Actuators
- **2x N20 DC motors with encoders** (rear traction).  
- **1x MG90S servo motor** for the Ackermann steering mechanism.  
- Encoders provide feedback for distance and turning accuracy.

#### Sensors and Vision
- **2x HC-SR04 ultrasonic sensors** (left and right sides).  
  - Used for wall detection and corridor navigation.  
- **BNO055 IMU sensor**.  
  - Provides heading information for orientation correction and PID stabilization.  
- **HuskyLens AI Camera**.  
  - Detects obstacles (red and green cubes) and provides position data for avoidance maneuvers.  

#### Power Supply
- Powered by a **3S Li-Po battery (11.1V)**.  
- Voltage is regulated to supply 5V for the Arduino and sensors, and motor driver receives direct battery voltage.  
- Common ground is shared across all components.

#### Additional Components
- **Buzzer** connected to the Arduino, used as a feedback indicator for distance readings

## Step by Step Asemble Guide



## Software and Code Explanation

The robot’s code was designed to combine stability, reliability, and adaptability during the challenge. Its behavior can be summarized in four main stages:
Straight Navigation with Gyroscope Correction

 The robot advances forward while continuously correcting its heading using the BNO055 gyroscope. If it drifts from the desired orientation, the servo adjusts to bring it back on course, ensuring consistent straight-line movement.


### Corridor Turns with Ultrasonic Sensors

 When one of the ultrasonic sensors detects open space to the left or right, the robot executes a precise 90° turn in that direction. The encoder ensures that the turn is completed fully by measuring wheel pulses, preventing incomplete or unstable turns.


### Object Avoidance with Vision (HuskyLens)

 If the HuskyLens camera detects a red or green cube, the robot immediately prioritizes avoidance. It steers away from the object’s position and moves forward until the obstacle is no longer detected. Once clear, it recenters the servo and resumes its planned trajectory.


### Completion of the Route

 After a predefined number of turns, the robot drives forward for an additional encoder-based distance and then stops completely. This guarantees that the robot finishes the run in a controlled and repeatable manner.



## Engineering Journal

Here's a link to our Engineering Journal: 

https://docs.google.com/document/d/1Jj0EOl-bdzsbxWS2QZuM9kGpr0p5ontHu1RqVm_BO6E/edit?usp=sharing
