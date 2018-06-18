# photon_coffeecounter

This is sketch is designed to read out the cup counters from Jura coffeemakers by a particle photon.

Some Jura coffeemakers offers a serial interface for service purpose.

Attention, some coffeemakers have a standard sub d port, which could be seen as a rs-232 compatible connection - it is not!
The pin configuration can be found in the net.

The interface itself uses a simple "encryption" with a symmetric shift of 4 byte packets.

The api offers some basic operations to access the user space of the memory.

Dependend of the coffee maker model, the machines have counters for:

* total cups
* available coffee receipes (e.g. Cappucino)

In both cases you have to reverse engineer the memory adress, to get the correct counter location.

Example:
Our machine has counters for each individual coffee receipe. In the service menu of the machine, you can check how much cups of Espresso have been drunk so far (e.g. 299).

Next step is to read out the memory adresses and search for a value of 299.
In the best case, you will end up with one memory adress (e.g. 0x281).
In the worst case, you will end up with some memory adresses.

In the second case, get one Espresso and check the memory content again. The Espresso counter should now be 300.

By doing so for every relevant coffee receipe, you get all memory adresses of the counters and by adding them up, you get your cup count for your coffee maker.

Hint: Adresses 0x280 0x281 0x282 0x283 ... 0x288 often contains some of the default counters.
