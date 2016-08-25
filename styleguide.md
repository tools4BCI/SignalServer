## Coding conventions

### The main message:
* KISS
* Orientate yourself on the files, already included in the respective projects
* Write simple code
* Write intuitive code
* Write coherent code

**and never forget:**      _Don't write ugly code ;-)_

### General rules
* Code "language" is english  <br />
  (e.g., no "datei", "fichier", or "fichero" variable names)
* No needless comments <br />
  (e.g., //and this loop incements the counter)
*   No global variables
* Comment complex code and also code-segments  <br />
  (In case of complex algorithms, describe the algorithm and don't make the code unreadable, commenting every line)
* Use descriptive names for variables, functions, etc.
* Avoid variable names like "x,y,z,i,..."  <br />
  (except e.g., for a loop counter, if the loop merely spans a few lines)
* Highlight bad hacks with "//FIXXXME" and add a description <br />
  (the number of "X" is related to the severity of the hack)
* Add todos into the sourcecode, in case of missing features or similar
* Add "seperators" between functions <br />
  (e.g., 60-70 times "-" resulting in this: //------------------- )
* Member variables must be private
* No silent catches
* Exceptions must not be used for flow-control purposes
* Documentation, documentation, documentation  <br />
  (please prefer doxygen, if possible)
* Indent size is 2 spaces
* Don't use 'tabs' for indents

### Naming and Code structure
* Methods and Functions:**<br />
  firstWordLowercaseRestCapitalizedWithoutUnderlines
* Constants:<br />
  CAPITALIZED_WITH_UNDERLINES
* Local variables:<br />
  lowercase_with_underlines
* Member variables:<br />
  lowercase_with_underlines_and_a_trailing_underline_
* Global variables:<br />
  similar_to_members_but_ending_with_global_
* Classes, Structs, Unions:<br />
  AllWordsCapitalizedWithoutUnderlines

* Exceptions:<br />
  ClassNameEndsWithException
* Methods, providing write access should be named: "setXYZ" <br />
  (e.g., setUpdatecoefficient)
* Methods, providing read access should be named "getXYZ" <br />
  (e.g., getCoffee)
* Boolean read methods should be named "isXYZ", "hasXYZ", "allowsXYZ,... <br />
  (e.g., isWithCaffeine, hasEnoughCoffeeForMe,...)

###Some C++ stuff:

*    No C-style casts
*    Every class must have a virtual destructor
*    All headers must be protected with #define statements against multiple inclusion
*    Favour a split in header and source file
*    One class per file

###Commit rules

*    Do not commit code or oter files, protected by intellectual property rights
*    Every commit must have a meaningful description (no lazy commits)
*    Newly commited code must not make the project uncompilable
*    For experiments and co, use branches
*    Test commited code also on other platforms before comitting
