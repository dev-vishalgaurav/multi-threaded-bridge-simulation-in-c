Assignment: Simulate the Ledyard Bridge Construction Project
Suppose that they need to rebuild the Ledyard Bridge again.

This construction requires closing one lane of the bridge, making it a one-way bridge. Traffic may only cross the bridge in one direction at a time.

This construction also will weaken the bridge, limiting its capacity to at most MAX_CARS vehicles. (E.g., try MAX_CARS = 3.)

Coding

In your system, each vehicle should be represented by a thread, which executes the function OneVehicle(direction) when it arrives at the bridge.

   OneVehicle(direction) {
        ArriveBridge(direction);
        // now the car is on the bridge!

        OnBridge(direction);

        ExitBridge(direction);
        // now the car is off
   }
direction should be TO_NORWICH or TO_HANOVER.
(You may certainly add other arguments, or collapse this all into a general argument structure, as appropriate.)

ArriveBridge must not return until it is safe for the car to get on the bridge.

OnBridge should, as a side-effect, print the state of the bridge and waiting cars, in some nice format, to make it easier to monitor the behavior of the system. (So.... watch out for race conditions here, too!)

 

"Can we have a bridgekeeper thread?"

No. The car threads must synchronize themselves; you may not have an extra thread directing traffic.

Basic Requirements

Safety. Your simulation should always prohibit "bad interleavings" where:

cars going opposite directions crash on the bridge.
the bridge collapses, because too many cars were on it.
Liveness. Your simulation should also ensure that:

if a car gets on the bridge, it will eventually cross and get off
if cars are waiting, and the bridge is empty, a car will get on
Efficiency. Your simulation should also make efficient use of the bridge capacity. That is......

if there are fewer than MAX_CARS on the bridge (say, traveling to Hanover)
and they are traveling sufficiently slowly
and there's a car waiting to go Hanover
then that car will get on the bridge too
(If MAX_CARS > 1 but your solution only allows one car at a time on the bridge, then that's a problem.)

Be sure to test your code to try to produce a good sampling of potential interleavings. However, also be sure to have a principled design---because testing here may show the presence of bugs, but probably cannot assure you of their absence.

Be sure your code does not have dangerous race conditions.
