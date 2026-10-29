
#include <iostream>
#include "Traveller.h"
#include "Trip.h"
 
int main() {
    std::cout << "=== Movement (State) demo ===\n";
 
    Traveller* traveller = new Traveller();  // starts in FootMode

    traveller->move();   // "Walking slowly."
    traveller->move();   // "Walking slowly." (still on land)

    // In the real game, terrain comes from the map/Place the traveller is
    // standing on. Here we simulate the traveller stepping onto water.
    traveller->setTerrain("water");

    traveller->move();   // guard fires inside FootMode::move(): terrain == "water",
                          // Traveller transitions itself to BoatMode

    traveller->move();   // now delegates to BoatMode::move() ("Rowing across the water.")
    traveller->move();   // still rowing — distanceTravelled() not yet > 5 on land

    // Simulate reaching the shore again.
    traveller->setTerrain("land");

    traveller->move();   // guard fires inside BoatMode::move(): terrain == "land"
                          // AND distanceTravelled() > 5, so Traveller transitions
                          // itself back to FootMode

    traveller->move();   // back to "Walking slowly."

    delete traveller;    // Traveller destructor deletes whatever state is current
 
    std::cout << "\n=== Route finding (Strategy) demo ===\n";
 
    Trip* trip = new Trip(new ShortestRoute());
    trip->plan();                              // "Plotting the shortest path..."
 
    trip->setStrategy(new FastestRoute());      // client swaps strategy at runtime
    trip->plan();                              // "Plotting the fastest path..."
 
    trip->setStrategy(new ScenicRoute());
    trip->plan();                              // "Plotting the most scenic path..."
 
    delete trip;        // Trip destructor deletes whatever strategy is current
 
    std::cout << "\nDone.\n";
    return 0;
}