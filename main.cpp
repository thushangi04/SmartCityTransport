#include <cctype>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "CitySetup.h"
#include "Graph.h"
#include "Locations.h"
#include "NetworkAnalyzer.h"
#include "Passenger.h"
#include "PassengerQueue.h"
#include "SimulationManager.h"


namespace {


/*
 * =========================================================
 * BASIC UI UTILITIES
 * =========================================================
 */


/*
 * Clear the terminal screen.
 */
void clearScreen() {

#ifdef _WIN32

    std::system("cls");

#else

    std::system("clear");

#endif
}


/*
 * Pause before returning to another screen.
 */
void waitForEnter() {

    std::cout
        << "\n------------------------------------------------------------\n"
        << "Press ENTER to return to the main menu...";


    std::cin.ignore(
        std::numeric_limits<std::streamsize>::max(),
        '\n'
    );


    std::cin.get();
}


/*
 * Display the main system title.
 */
void printHeader() {

    std::cout
        << "============================================================\n"
        << "              NOVA CITY TRANSPORT SYSTEM\n"
        << "        Smart Public Transportation Simulator\n"
        << "============================================================\n";
}


/*
 * Display a section heading.
 */
void printSection(
    const std::string& title) {

    std::cout
        << "\n============================================================\n"
        << "  "
        << title
        << "\n"
        << "============================================================\n";
}


/*
 * Display success message.
 */
void showSuccess(
    const std::string& message) {

    std::cout
        << "\n[ SUCCESS ] "
        << message
        << "\n";
}


/*
 * Display error message.
 */
void showError(
    const std::string& message) {

    std::cout
        << "\n[ ERROR ] "
        << message
        << "\n";
}


/*
 * Display informational message.
 */
void showInfo(
    const std::string& message) {

    std::cout
        << "\n[ INFO ] "
        << message
        << "\n";
}


/*
 * =========================================================
 * INPUT UTILITIES
 * =========================================================
 */


/*
 * Read a valid integer inside a range.
 */
int readInteger(
    const std::string& prompt,
    int minimum,
    int maximum) {

    int value;


    while (true) {

        std::cout
            << prompt;


        if (
            std::cin >> value
            &&
            value >= minimum
            &&
            value <= maximum
        ) {

            return value;
        }


        showError(
            "Please enter a number between "
            + std::to_string(minimum)
            + " and "
            + std::to_string(maximum)
            + "."
        );


        std::cin.clear();


        std::cin.ignore(
            std::numeric_limits<std::streamsize>::max(),
            '\n'
        );
    }
}


/*
 * Validate a time using HH:MM format.
 */
bool isValidTime(
    const std::string& time) {

    if (
        time.size() != 5
        ||
        time[2] != ':'
    ) {

        return false;
    }


    if (
        !std::isdigit(
            static_cast<unsigned char>(time[0])
        )
        ||
        !std::isdigit(
            static_cast<unsigned char>(time[1])
        )
        ||
        !std::isdigit(
            static_cast<unsigned char>(time[3])
        )
        ||
        !std::isdigit(
            static_cast<unsigned char>(time[4])
        )
    ) {

        return false;
    }


    const int hour =

        ((time[0] - '0') * 10)

        +

        (time[1] - '0');


    const int minute =

        ((time[3] - '0') * 10)

        +

        (time[4] - '0');


    return
        hour >= 0
        &&
        hour <= 23
        &&
        minute >= 0
        &&
        minute <= 59;
}


/*
 * Read a valid departure time.
 */
std::string readTime(
    const std::string& prompt) {

    std::string time;


    while (true) {

        std::cout
            << prompt;


        std::cin
            >> time;


        if (
            isValidTime(time)
        ) {

            return time;
        }


        showError(
            "Invalid time. Use HH:MM format, for example 07:30."
        );
    }
}


/*
 * Ask user for confirmation.
 */
bool confirm(
    const std::string& message) {

    char answer;


    while (true) {

        std::cout
            << message
            << " (Y/N): ";


        std::cin
            >> answer;


        answer =
            static_cast<char>(
                std::toupper(
                    static_cast<unsigned char>(answer)
                )
            );


        if (
            answer == 'Y'
        ) {

            return true;
        }


        if (
            answer == 'N'
        ) {

            return false;
        }


        showError(
            "Please enter Y or N."
        );
    }
}


/*
 * =========================================================
 * LOCATION UI
 * =========================================================
 */


/*
 * Display all Nova City locations.
 */
void printLocations() {

    std::cout
        << "\n+----+----------------------------------+\n"
        << "| ID | Location                         |\n"
        << "+----+----------------------------------+\n";


    for (
        int i = 0;
        i < NUM_LOCATIONS;
        ++i
    ) {

        std::cout
            << "| "
            << i;


        if (
            i < 10
        ) {

            std::cout
                << "  ";
        }


        std::cout
            << "| "
            << locationName(i);


        const std::string name =
            locationName(i);


        const int remainingSpaces =
            32
            -
            static_cast<int>(name.size());


        for (
            int space = 0;
            space < remainingSpaces;
            ++space
        ) {

            std::cout
                << ' ';
        }


        std::cout
            << "|\n";
    }


    std::cout
        << "+----+----------------------------------+\n";
}


/*
 * Read a location.
 */
int readLocation(
    const std::string& prompt) {

    return readInteger(
        prompt,
        0,
        NUM_LOCATIONS - 1
    );
}


/*
 * =========================================================
 * SAME LOCATION RESULT
 * =========================================================
 */
void displaySameLocationResult(
    int location) {

    std::cout
        << "\n------------------------------------------------------------\n"
        << "ROUTE RESULT\n"
        << "------------------------------------------------------------\n"

        << "Source             : "
        << locationName(location)
        << '\n'

        << "Destination        : "
        << locationName(location)
        << '\n'

        << "\nThe passenger is already at the destination.\n"

        << "\nDistance           : 0 km\n"

        << "Travel Time        : 0 min\n"

        << "Scheduled Waiting  : 0 min\n"

        << "Interchange Time   : 0 min\n"

        << "Total Journey Time : 0 min\n"

        << "Transfers          : 0\n";
}


/*
 * =========================================================
 * CITY GRAPH
 * =========================================================
 */
void runGraphDisplay(
    const Graph& graph) {

    clearScreen();

    printHeader();

    printSection(
        "CITY TRANSPORT NETWORK"
    );


    std::cout
        << "\nThe network below represents Nova City using a weighted\n"
        << "graph. Locations are vertices and bus/train connections\n"
        << "are weighted edges.\n\n";


    graph.displayGraph();


    waitForEnter();
}


/*
 * =========================================================
 * BFS
 * =========================================================
 */
void runBFSDemo(
    const Graph& graph) {

    clearScreen();

    printHeader();

    printSection(
        "BFS CONNECTIVITY TEST"
    );


    std::cout
        << "\nBreadth-First Search checks whether two locations are\n"
        << "connected in the transportation network.\n";


    printLocations();


    const int source =

        readLocation(
            "\nEnter source location ID      : "
        );


    const int destination =

        readLocation(
            "Enter destination location ID : "
        );


    if (
        source ==
        destination
    ) {

        std::cout
            << "\n------------------------------------------------------------\n"
            << "BFS RESULT\n"
            << "------------------------------------------------------------\n"

            << "Source      : "
            << locationName(source)
            << '\n'

            << "Destination : "
            << locationName(destination)
            << '\n'

            << "Connected   : YES\n"

            << "Traversal   : "
            << locationName(source)
            << '\n';


        showInfo(
            "Source and destination are the same location."
        );


        waitForEnter();

        return;
    }


    std::vector<int> traversal;


    const bool found =

        graph.bfs(
            source,
            destination,
            &traversal
        );


    std::cout
        << "\n------------------------------------------------------------\n"
        << "BFS RESULT\n"
        << "------------------------------------------------------------\n"

        << "Source      : "
        << locationName(source)
        << '\n'

        << "Destination : "
        << locationName(destination)
        << '\n'

        << "Connected   : "
        << (
            found
                ? "YES"
                : "NO"
        )

        << "\n\nTraversal   : ";


    for (
        std::size_t i = 0;
        i < traversal.size();
        ++i
    ) {

        if (
            i > 0
        ) {

            std::cout
                << " -> ";
        }


        std::cout
            << locationName(
                traversal[i]
            );
    }


    std::cout
        << '\n';


    if (
        found
    ) {

        showSuccess(
            "A route exists between the selected locations."
        );
    }

    else {

        showError(
            "No route exists between the selected locations."
        );
    }


    waitForEnter();
}


/*
 * =========================================================
 * DIJKSTRA ROUTE FINDER
 * =========================================================
 */
void runShortestPathDemo(
    const Graph& graph) {

    clearScreen();

    printHeader();

    printSection(
        "SMART ROUTE FINDER"
    );


    std::cout
        << "\nChoose your origin and destination.\n";


    printLocations();


    const int source =

        readLocation(
            "\nEnter source location ID      : "
        );


    const int destination =

        readLocation(
            "Enter destination location ID : "
        );


    if (
        source ==
        destination
    ) {

        displaySameLocationResult(
            source
        );


        waitForEnter();

        return;
    }


    std::cout
        << "\nSelect route optimization method:\n\n"

        << "  [1] Fastest Realistic Route\n"
        << "      Includes timetable waiting and transfers.\n\n"

        << "  [2] Shortest Distance Route\n"
        << "      Minimizes total kilometres travelled.\n\n"

        << "  [3] Minimum In-Vehicle Time Route\n"
        << "      Minimizes only bus/train travelling time.\n\n";


    const int routeChoice =

        readInteger(
            "Select an option [1-3]: ",
            1,
            3
        );


    if (
        routeChoice == 1
    ) {

        const std::string departureTime =

            readTime(
                "\nEnter departure time (HH:MM): "
            );


        std::cout
            << "\nSearching for the fastest timetable-aware route...\n";


        const std::vector<JourneySegment> path =

            graph.getFastestPath(
                source,
                destination,
                departureTime
            );


        std::cout
            << '\n';


        graph.printPath(
            path,
            departureTime
        );
    }


    else if (
        routeChoice == 2
    ) {

        std::cout
            << "\nSearching for the shortest-distance route...\n";


        const std::vector<JourneySegment> path =

            graph.getShortestPath(
                source,
                destination,
                WeightMode::DISTANCE
            );


        std::cout
            << '\n';


        graph.printPath(
            path
        );
    }


    else {

        std::cout
            << "\nSearching for the minimum travel-time route...\n";


        const std::vector<JourneySegment> path =

            graph.getShortestPath(
                source,
                destination,
                WeightMode::TIME
            );


        std::cout
            << '\n';


        graph.printPath(
            path
        );
    }


    waitForEnter();
}


/*
 * =========================================================
 * FIFO QUEUE DEMO
 * =========================================================
 */
void runQueueDemo() {

    clearScreen();

    printHeader();

    printSection(
        "FIFO PASSENGER QUEUE DEMONSTRATION"
    );


    std::cout
        << "\nThis demonstration shows how passengers enter and leave\n"
        << "a First-In-First-Out transport queue.\n\n";


    Passenger passenger1(
        "P-DEMO-1",
        NORTH_RESIDENTIAL,
        UNIVERSITY,
        "07:00"
    );


    Passenger passenger2(
        "P-DEMO-2",
        NORTH_RESIDENTIAL,
        AIRPORT,
        "07:02"
    );


    Passenger passenger3(
        "P-DEMO-3",
        NORTH_RESIDENTIAL,
        GENERAL_HOSPITAL,
        "07:04"
    );


    PassengerQueue queue;


    std::cout
        << "Adding three passengers to the queue...\n\n";


    queue.addPassenger(
        passenger1
    );


    queue.addPassenger(
        passenger2
    );


    queue.addPassenger(
        passenger3
    );


    queue.displayQueue();


    Passenger* boarded =

        queue.removePassenger();


    if (
        boarded != nullptr
    ) {

        showSuccess(
            boarded->getPassengerID()
            +
            " boarded first because the queue follows FIFO."
        );
    }


    std::cout
        << "\nQueue after first passenger boards:\n\n";


    queue.displayQueue();


    waitForEnter();
}


/*
 * =========================================================
 * OPERATING DAY SIMULATION
 * =========================================================
 */
void runOperatingSimulation(
    SimulationManager& simulation,
    bool& simulationHasRun) {

    clearScreen();

    printHeader();

    printSection(
        "OPERATING-DAY PASSENGER SIMULATION"
    );


    std::cout
        << "\nSimulation Configuration\n"
        << "------------------------------------------------------------\n"

        << "Passenger demand : 680 passengers\n"

        << "Demand period    : 05:00 - 24:00\n"

        << "Transport        : Bus + Train\n"

        << "Bus capacity     : 40 passengers\n"

        << "Train capacity   : 120 passengers\n"

        << "Bus headway      : 15 minutes\n"

        << "Train headway    : 20 minutes\n"

        << "Interchange time : 5 minutes\n"

        << "Random seed      : 42\n";


    std::cout
        << "\nThis operation simulates all passenger journeys for the\n"
        << "complete operating day.\n\n";


    if (
        !confirm(
            "Start operating-day simulation?"
        )
    ) {

        showInfo(
            "Simulation cancelled."
        );


        waitForEnter();

        return;
    }


    clearScreen();

    printHeader();


    std::cout
        << "\nStarting simulation...\n"
        << "Please wait while passenger journeys are processed.\n\n";


    simulation
        .runOperatingDaySimulation(
            1
        );


    simulationHasRun =
        true;


    showSuccess(
        "Operating-day simulation completed successfully."
    );


    std::cout
        << "\nYou can now use:\n"

        << "  Option 6 - View journey records\n"

        << "  Option 7 - View profiling report\n"

        << "  Option 8 - Export CSV\n";


    waitForEnter();
}


/*
 * =========================================================
 * DISPLAY RECORDS
 * =========================================================
 */
void showJourneyRecords(
    SimulationManager& simulation,
    bool simulationHasRun) {

    clearScreen();

    printHeader();

    printSection(
        "PASSENGER JOURNEY RECORDS"
    );


    if (
        !simulationHasRun
    ) {

        showError(
            "No simulation data is available."
        );


        showInfo(
            "Run Option 5 first."
        );


        waitForEnter();

        return;
    }


    std::cout
        << "\nDisplaying the first 10 passenger journeys.\n";


    simulation
        .getStatisticsManager()
        .displayRecords(
            10
        );


    waitForEnter();
}


/*
 * =========================================================
 * PROFILING REPORT
 * =========================================================
 */
void showProfilingReport(
    SimulationManager& simulation,
    NetworkAnalyzer& analyzer,
    bool simulationHasRun) {

    clearScreen();

    printHeader();

    printSection(
        "NETWORK PERFORMANCE & PROFILING"
    );


    if (
        !simulationHasRun
    ) {

        showError(
            "No simulation data is available."
        );


        showInfo(
            "Run Option 5 first."
        );


        waitForEnter();

        return;
    }


    AnalysisResult result =

        analyzer.analyze(

            simulation
                .getStatisticsManager()
                .getRecords(),

            simulation
                .getServiceStats(),

            simulation
                .getDemandPeriods()
        );


    analyzer.printReport(

        result,

        simulation
            .getDemandPeriods()
    );


    waitForEnter();
}


/*
 * =========================================================
 * CSV EXPORT
 * =========================================================
 */
void exportCSV(
    SimulationManager& simulation,
    bool simulationHasRun) {

    clearScreen();

    printHeader();

    printSection(
        "EXPORT JOURNEY DATA"
    );


    if (
        !simulationHasRun
    ) {

        showError(
            "No simulation data is available."
        );


        showInfo(
            "Run Option 5 first."
        );


        waitForEnter();

        return;
    }


    std::cout
        << "\nThe following file will be created:\n\n"
        << "  journey_records.csv\n\n"

        << "It contains passenger journey times, waiting times,\n"
        << "interchange times, routes and completion status.\n\n";


    if (
        !confirm(
            "Export journey records?"
        )
    ) {

        showInfo(
            "CSV export cancelled."
        );


        waitForEnter();

        return;
    }


    if (

        simulation
            .getStatisticsManager()
            .saveCSV(
                "journey_records.csv"
            )
    ) {

        showSuccess(
            "journey_records.csv created successfully."
        );


        std::cout
            << "\nThe file is located in your NovaCity project folder.\n";
    }

    else {

        showError(
            "The CSV file could not be created."
        );
    }


    waitForEnter();
}


/*
 * =========================================================
 * MAIN MENU
 * =========================================================
 */
void displayMainMenu(
    bool simulationHasRun) {

    clearScreen();

    printHeader();


    std::cout
        << "\nSystem Status\n"
        << "------------------------------------------------------------\n"

        << "Simulation Data : "
        << (
            simulationHasRun
                ? "AVAILABLE"
                : "NOT GENERATED"
        )

        << "\n\n"

        << "MAIN MENU\n"
        << "------------------------------------------------------------\n"

        << " [1] View City Transport Network\n"
        << "     Display buses, trains, locations and graph edges.\n\n"

        << " [2] Test Network Connectivity (BFS)\n"
        << "     Check whether two locations are connected.\n\n"

        << " [3] Smart Route Finder (Dijkstra)\n"
        << "     Find fastest, shortest or minimum travel-time route.\n\n"

        << " [4] Passenger Queue Demonstration\n"
        << "     Demonstrate FIFO passenger queue behaviour.\n\n"

        << " [5] Run Operating-Day Simulation\n"
        << "     Simulate all 680 passenger journeys.\n\n"

        << " [6] View Passenger Journey Records\n"
        << "     Display sample simulated journeys.\n\n"

        << " [7] View Performance & Profiling Report\n"
        << "     Analyze demand, routes, waiting and utilization.\n\n"

        << " [8] Export Journey Data to CSV\n"
        << "     Save detailed simulation results.\n\n"

        << " [0] Exit Application\n"

        << "------------------------------------------------------------\n";
}


}


/*
 * =========================================================
 * MAIN PROGRAM
 * =========================================================
 */
int main() {

    /*
     * Build Nova City graph.
     */
    Graph graph;


    buildNovaCityGraph(
        graph
    );


    /*
     * Fixed seed = reproducible simulation.
     */
    SimulationManager simulation(
        graph,
        42
    );


    NetworkAnalyzer analyzer;


    bool simulationHasRun =
        false;


    bool running =
        true;


    /*
     * Welcome screen.
     */
    clearScreen();

    printHeader();


    std::cout
        << "\nWelcome to the Nova City Smart Transportation Simulator.\n\n"

        << "This application demonstrates how Graph Theory and data\n"
        << "structures can be used to model a future public-transport\n"
        << "only smart city.\n\n";


    std::cout
        << "Press ENTER to continue...";


    std::cin.get();


    /*
     * Main application loop.
     */
    while (running) {

        displayMainMenu(
            simulationHasRun
        );


        const int choice =

            readInteger(
                "\nSelect an option [0-8]: ",
                0,
                8
            );


        switch (choice) {


            case 1:

                runGraphDisplay(
                    graph
                );

                break;


            case 2:

                runBFSDemo(
                    graph
                );

                break;


            case 3:

                runShortestPathDemo(
                    graph
                );

                break;


            case 4:

                runQueueDemo();

                break;


            case 5:

                runOperatingSimulation(
                    simulation,
                    simulationHasRun
                );

                break;


            case 6:

                showJourneyRecords(
                    simulation,
                    simulationHasRun
                );

                break;


            case 7:

                showProfilingReport(
                    simulation,
                    analyzer,
                    simulationHasRun
                );

                break;


            case 8:

                exportCSV(
                    simulation,
                    simulationHasRun
                );

                break;


            case 0:

                clearScreen();

                printHeader();


                std::cout
                    << "\nThank you for using the Nova City Transport System.\n"
                    << "Simulation session ended successfully.\n\n";


                running =
                    false;


                break;
        }
    }


    return 0;
}