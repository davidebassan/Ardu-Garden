# Hydroponic Garden System

This project implements a hydroponic garden system controlled via Arduino, managing the pump for irrigation and the light for plants. The system provides a web interface for control and monitoring functionalities.

## Contents

- [Project Description](#project-description)
- [Setup and Usage](#setup-and-usage)
- [Printables](#printables)
- [Connections](#connections)
- [Known Issues](#known-issues)

## Project Description

The system controls an irrigation pump and a series of lights for plants in a hydroponic garden. It features a web interface that allows users to monitor the garden's status and manually control the pump and lights.

The activation time of the pump and the rest time can be modified via the `active` and `sleeping` variables in the code.

## Setup and Usage

1. Upload the source code to your Arduino.
2. Connect the electronic components following the provided electrical schematics.
3. Connect your Arduino to a configured WiFi network.
4. Access your Arduino's IP address via a web browser to access the hydroponic garden control interface.

## Printables
Follow this [link](https://www.printables.com/it/model/720081-modular-hydroponic-tower/files).

## Connections

| Component      | Arduino Pin |
|----------------|-------------|
| Pump           | 12          |
| DHT11 Sensor   | 5           |
| LED Strip      | 8           |
| Built-in LED   | LED_BUILTIN |

## Known Issues

There is a known issue with handling HTTP requests, and it might be better to handle requests using AJAX.
