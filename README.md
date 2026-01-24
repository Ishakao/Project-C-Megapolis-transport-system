## Megalopolis Transport System  

![alt text](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)


![alt text](https://img.shields.io/badge/Raylib-4.5-red?style=for-the-badge)



    A sophisticated transport network simulation and pathfinding application built with C++ and a custom Raylib-based GUI.


## 📖 About The Project  
### Based on <a href="https://github.com/Ishakao/simpleUI">SimpleUI</a>

The Megalopolis Transport System is a graph-based application designed to model and analyze a complex city transit network consisting of three distinct transport modes: Metro, Bus, and Train.

This project solves real-world navigation problems by modeling the city as an undirected multigraph. Unlike simple navigation apps, this system accounts for complex travel factors, including:

    Congestion Coefficients: Real-time line loading affects travel speed.

    Transfer Penalties: Switching transport modes incurs time costs based on a specific inter-modal matrix.

    User Preferences: Routes are calculated and sorted based on a user-defined balance between total time and the number of transfers.

The application visualizes these networks and results using a custom-written Graphical User Interface (GUI) powered by Raylib.
## ✨ Key Features
1. Network Connectivity Analysis

Using Depth-First Search (DFS), the system analyzes the transport graph to:

    Identify isolated zones within the city.

    Analyze connectivity for specific sub-graphs (e.g., Metro-only or Bus-only networks).

    Determine the largest connected component in the megalopolis.

2. Intelligent Pathfinding

Implements Dijkstra’s Algorithm with a custom Priority Queue to find the optimal route.

    State-based Search: Nodes are represented as (Station, Last_Transport_Mode) to correctly calculate transfer penalties.

    Dynamic Weighting: Edge weights are calculated dynamically: BaseTime * (1 + LoadFactor * Sensitivity).

3. Smart Route Sorting

Search results are not just listed; they are ranked using a Custom Quicksort Implementation.

    Metric: Routes are scored by Time + (K * Transfers), where K is a user-defined coefficient.

    Tie-Breaking: If scores are equal, routes are sorted by Time → Transfer Count → Station ID.

4. Custom GUI Framework

Built entirely in C++ using Raylib, the interface features:

    Interactive map visualization.

    Custom UI components (Buttons, Lists, Inputs) built from scratch.

## 🛠️ Tech Stack & Algorithms
    Language: C++ (Standard 17/20 recommended)

    Graphics Engine: <a href="https://github.com/Ishakao/simpleUI">SimpleUI</a> (based on Raylib)

    GUI Library: SimpleUI, Raylib

    Data Structures:

        Adjacency Lists (for Graph storage)

        Priority Queue (for Dijkstra)

        Vectors & Structs

    Core Algorithms:

        Dijkstra (Shortest Path)

        DFS (Connectivity Components)

        Quicksort (Result Ordering - Custom implementation)

## 🧩 Usage Example

    Load Data: The app initializes with N stations.

    Analyze: Click "Check Connectivity" to see isolated zones (displayed in descending order of size).

    Find Route:

        Select Start Station.

        Select Target Station(s).

        Set Transfer Comfort Coefficient (Higher = fewer transfers preferred).

        Click Calculate.

    View Results: The list will display the best routes, detailing:

        Total Time

        Number of Transfers

        Step-by-step path (e.g., "Metro -> Transfer -> Bus").

👤 Author

[SOSY BAKA]

    GitHub: @Ishakao


📄 License

NONE
