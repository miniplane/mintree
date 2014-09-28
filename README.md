mintree
=======

A simple B+Tree implementation I wrote for the Algorythm & Data Structures course at universtiy.

Container.h and simpletest.C were provided with the assignment.


    new ............................... create new Container
    delete ............................ delete Container
    add <key> [...] ................... add <key>(s) with Container::add( int )
    remove <key> [...] ................ remove <key>(s) with Container::remove( int )
    member <key> ...................... call Container::member( <key> )
    size .............................. call Container::size()
    empty ............................. call Container::empty()
    min ............................... call Container::min()
    max ............................... call Container::max()
    print ............................. print container with operator<<()
    apply [asc|desc|dontcare [<n>]] ... traverse container with PrintN functor
    trace ............................. toggle tracing on/off
    fadd <filename> ................... add values read from file <filename>
    fremove <filename> ................ remove values read from file <filename>
    radd [<n> [<seed>]] ............... add <n> random values, optionally reset generator to <seed>
    rremove [<n> [<seed>]] ............ remove <n> random values, optionally reset generator to <seed>
    quit .............................. quit program
