Dennis Karpenko(dk1111), Suk Jin Hong(sh1949)


Concurrency(?): Handles multiple games, fails to check for duplicate names. more on this below.

Design Choices: 
    
    readNames():
        This reads the initial input of the form ("P|name||"), which is manually input with rc, and automatically with gc. 
        We elected to (as for all other polls) to use an infinite loop while we wait for the name(s). Other than that, the code
        is rather simple, it is a "tokenizer" of sorts, in that we split the input into a prefix ("P|" ?) and suffix ("name||" ?),
        and then proceed to check the validity and process both (really just printing out the names to the opponents, if valid).
    processMoves():
        This is our move processor, which has the same "structure" as readNames, in that we also poll for inputs, using a for(;;) loop,
        tokenizing and then doing if statements on move combinations if the inputs are valid. We could have done switch cases, but there's
        only six combinations of moves, so it don't really matter either way. We (break;) out of the for(;;) loop once we are done processsing.
    didTheyLeave():
        Again, this is a tokenizing for(;;) loop, exceopt we're not even tokenizing really, just looking at the first byte of the input,
        if the input is nonzero. We only do a if (input = 'C' and input2 = 'C') check, since it doesn't really matter what it the inputs are in
        any other case. If they both are invalid (not C or Q), either gc would say so, or we would just kick the clients out if they're using rc.
        If either of the clients put ('Q'), ie., they quit, we also just throw them out. So we only need the 'C' check for both.
    addNames(): 
        This a remnant from us trying to do the name-validity check for multiple concurrent game instances. It is a simple 2d global char names[]array
        parser, which iterates through the array (checking for matching existing names), and if none are found, appends both "anticipating/waiting" players
        to the end of the arrary, and (ideally) we would continue to start a new game in main(). Unfortunately we could not figure this out :(

    main:
        We create a waiting room which is a 2 int array, to hold accepted player sockets. We open listener, check if a port-name was provided
        , adding one ourselves if not, and listen. We have another for(;;) loop, which polls() until we have two players.
        Once those two players are found, we fork a new game instance in which we readNames() -- this outputs begin/wait/try-again messages, 
        run processMoves() once, then while(didTheyLeave == 1) -- meaning the two players did not leave after the first round
        , we processMoves() the players inputs again, and so on. Meanwhile, the parent polls for two new players yet again.

        This is where we tried to add an addNames() check to the for loop (before forking a child), but ran into issues stemming from
        the fact that we did a single game instance before we tried doing concurrent ones, which is fine except for the fact that we had ourselves
        valid-names? check(s) after we forked, which is pointless, and meant we would have to refactor our code. Once we did get this to work,
        there were issues with children inheriting the global names[] array. TBH we could probably fix this with one or two more days, but not sure.
    
    overall (project structure):
        There's nothing too fancy, a lot of what this project needs is polling() which requires a good amount of while/for(;;) loops.
        We did at first try to just make a big block in main, but this became unmanageable rather fast, so we made (essentially) all of what is our
        nice and modular helper functions which kind of all do the same thing in almost the same ways. The project is rather linear in terms of 
        what you need to do for it, so there's no crazy big framework of how our code works. It's as simple as:

              poll() -> processNames() -> poll() -> processMoves() -> (didTheyLeave() and if not processMoves(), polling() some more).

Test Plans: honestly just play an insane amount of games. with the same names, or bad inputs, checking if we can fork as much as we want, etc.
            Also just spending a lifetime in gdb, we had to figure out gc did not like '\0' which took us a little too long lmao. 
        
    