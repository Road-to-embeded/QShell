# QShell
QShell is a cross-platform shell like terminal. 

## New Features and Customization to Implement
- [] Add man pages.
- [x] Add unknown commands error message.
- [x] Add unknown commands output.
- [x] Add different colors for user + hostname, user input, shell ouput. 
- [] Implement file and dir output distintions (maybe different colors for now).
- [x] Customize cursor as a block.
- [x] Implement a clear screen method. 
- [] Create right click popup method.
- [] Add new header with new terminal, search icon, hamburger menu.
    - [] New header displays current working directory. 
    - [] New terminal button creates a new terminal window as a new tab child to the shell.
    - [] Each new tab displays user, hostname, current working DIR, and close tab icon.
        - [] Close tab icon closes respective tab.
    - [] Search icon opens a find menu popup window with search bar with navigation up and down arrows, erase all button, and some search filters.
    - [] Hamburger menu displays zoom functuonality, and other options.
- [] Add command chaining operations.
- [] Implement piping and redirections.
- [] Add output redirections. 
- [] Handle more advance shell operations and interactive commands such as cat, grep, read. 
- [] Customize overall shell style.
- [] Add translucent feel to shell.
- [] Add setttings to change shell styles.

## Known Bugs
- [] Command (**clear**)sends ANSI code "\033[H\033[2J" as output instead of processing command.

