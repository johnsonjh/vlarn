This is Ularn 1.6, patchlevel 3a, as of 3 May 2004.
---------------------------------------------------

This software as a whole is distributed under the Gnu General Public License.
A copy of this license should be included in this archive.

NB: Not all of the source is GPL. See copyright.txt for details.

NEWS
----
The Amiga version is now complete. 
The unix TTY and X11 versions are now complete. 

NOTE: The Amiga version requires an AGA amiga (Amiga 1200 or 4000).

Compiling Info
-------------- 
The windows executable is built with the Borland C++ 5.5 command line tools.
The Amiga version is built with SAS C/C++ 6.51.

While I do not produce executable distributions for the unix versions it 
should not be too difficult to build from the source distribution. 
See install_x11.txt for more details.

The unix port has been tested on:

Pentium4 running Red Hat linux 7.1
Amiga 4000/040 running NetBSD 1.5.2
Sun UltraSparc running Solaris 2.5.1

Changes from 1.6.3 to 1.6.3a
----------------------------
This is a Windows Graphical interface version update only.
It contains a single fix that should dramatically reduce the amount of CPU time 
requried by changing from getting windows messages using PeekMessage in a busy loop
to using GetMessage which is the proper OS friendly way of doing it.
There is no change to the gameplay and no perceptible change in response times.

All other variants are unchanged.


Changes from 1.6.2 to 1.6.3
---------------------------

BUG FIXES

Fixed potion of heroism and Coked so the boost to charisma is lowered correctly 
when the effect wears off.

Fixed effect of changing the enchantment level on a ring of strength.

Fixed Winners Scoreboard so that more than 1 winner can be displayed.

Fixed LRS so tax can now be paid.

Fixed text formatting in LRS when paying tax.

Fixed a bug in output of error messages during initialisation that caused data 
corruption.

Fixed Vaporise Rock spell for dificulty level 4+ to redraw statues correctly 
after the spell animation is displayed if the statue isn't destroyed.

Removed function calls causing extra repainting of the map window.

Fixed recalculation of AC and WC when destroying armour/weapons due to over 
enchantment.


NEW FEATURES

Added the Amiga version. Only supports AGA Amigas (1200/4000).
This has been tested on my Amiga 4000/040 and WinUAE.

Added a Unix TTY version using the curses library.
Note to those who are thinking of using ncurses under Linux:
I had no end of problems getting the ncurses library to work with my code until I 
realised that the ncurses library is not compatible with the -fpack_struct option
of gcc.
Compiling a program that uses ncurses with -fpack_struct causes really odd behaviour.
As far as I know, this limitation is not documented anywhere!
This has required that the unix version be compiled without packing the 
data structures, and hence the scoreboard and save files are now incompatible with
the windows version :-(

Improved some of the graphic tiles. (Thanks to Craig Spencer for providing the 
new tile images).

Windows version display area separators now use the current system colours for 
3D objects (COLOR_3DFACE, COLOR_3DHILIGHT, COLOR_3DSHADOW).


Changes from 1.6.1 to 1.6.2
---------------------------

NOTE: Save files from earlier versions are probably not compatible with 1.6.2.
While the save file format hasn't changed, the interpretation of some of the values
has. Unexpected behaviour may result if using an old save game.
The scoreboard from 1.6.1 should be compatible.

BUG FIXES

Had another go at fixing Dealer McDope's. I think I got it right this time :-).

Fixed a subtle initialisation bug in the message window. This bug was only observed on 
the NetBSD Amiga version, but may have potentially affect other versions.

Fixed spell damage for when wielding Slayer or Vorpal Blade.

Fixed a problem in redrawing the map that could allow the cursor at the player 
location to be drawn outside the displayable map area.

Fixed setting of the game difficulty level.

Corrected the help file to have the correct options format for specifying the
difficulty level.

Fixed alter reality spell for wizard.

Fixed storage of window position in ularn.ini file for Windows version.


NEW FEATURES

Added Unix/X11 support.

New menu item to enable/disable beeps during the game.

A beep is now played when the player gains a level.

Learning multiple copies of Genocide, Alter reality and Permanance spells
allows multiple castings of these spells.

Walls enhanced to use shaped walls based on wall connectivity.

Windows version now remembers maximize status of window.

Some graphics changed in the tile set.


Changes from 1.6.0 to 1.6.1
---------------------------

NOTE: Save files and the scoreboard are incompatible between 
ULarn 1.6.0 and 1.6.1.

BUG FIXES

Fixed a bug in the rusting of armour that caused the plusses to be 
set to a dud value when the armour was very rusted.

Fixed disenchanting/rusting of items to have appropriate effects on
stats.

Fixed a bug that caused slow monsters to either move every turn not move 
at all when the player is hasted.

Fixed going up/down stairs when there is a monster on the staircase at 
the other end.

Fixed a error in the smartmove routine that could cause demon princes and
the king of hell to get "stuck" in the wall at the edge of the map.

Fixed array indexing problems for calculating the enchantment level of new items.

Fixed elevator up/down for operation in the volcano.

Player positioning for dungeon level changes has been made a bit smarter. It now
selects the closest empty position to the desired location rather than performing
a map scan from the current position.

Fixed Dealer McDope's Pad.


NEW FEATURES

1. Added 'enhanced_interface' option to ularn.opt. 
Some people found the current method of opening doors a bit disorienting.
The option enhanced_interface/noenhanced_interface can now be specified
in the ularn.opt file. The default, is noenhanced_interface.
If the enhanced_interface option is set then the following changes to the 
interface are used:
  . The player can no longer move onto closed dooors to try and open them. 
    Instead the 'o' (Open Door) commands is provided.
    When Open Door is selected the player is prompted for the direction 
    of the door to be opened. 
  . The Close Door command is changed to operate in a similar manner to the 
    Open Door command.
  . The 'O' (Open Chest) command is added to open a chest at the player's
    current location. This is a bit more convenient than moving off then
    back onto a chest for each attempt to open it.

2. Added a new item: 'amulet of life preservation'. 
Prevents all level drain effects.

3. Implemented infravision ability for monsters. The ability was marked in
the spell effect table but was never used.


OTHER CHANGES

Completed a code inspection/walk through of every module. This resulted in a
general tidy up of the code. Half of the bug fixes above were identified as
a result of the analysis.

Ularn now compiles with MinGW. A MinGW makefile is now included with the
source distribution. This is another step towards getting the X windows
version working.
NOTE: Save files and high score tables may not be compatible between
      the Borland C and the MinGW executables.


Ularn 1.6, patchlevel 0, as of 11 Aug 2003.
-------------------------------------------
ULarn 1.6 is currently only available for Windows32 based operating systems.

An X11 version will appear in due course (most likely as soon as I learn how to 
program X11).
Note: The configure scripts are gone in this version. I'm hoping they are no
      longer required as this code should be posix compliant.
      It is probably now easier to just edit config.h for the few system
      specific configuration options remaining than to use the configure
      scripts.
      I'll probably find out when I produce the linux and NetBSD versions.

The next version will probably be for the Amiga as it is the OS I am next most
familiar with. It will definitely require an AGA amiga, and will probably be
better with a third party graphics card (should look nice on my Retina Z3).

This is not just a direct conversion of ularn from the unix version to windows, 
but rather a complete redesign of most of the code.
Calling it ularn 1.6 is probably fairly misleading. For the amount of change
in the code between ularn 1.5 patch level 4 to this version, it should probably
be something like ularn 3.0.
One of the main aims of this conversion was to separate the game play from the
user interface so that different user interfaces could be created for different
systems.
Almost all operating system dependant code has been isolated to a single module
(ularn_win).
It should not be very difficult to re-create the unix text mode version from
this code.

While no significant game play changes have been made, there have been lots of
minor tweaks and bug fixes.
The biggest change is in going from a text based display to a graphic tile
display.
The tiles used came from several different versions of nethack and angband (some
of which have been rather highly edited).
There are also quite a few I drew myself for items I couldn't find in any other
game.
Most of the magic spell tiles and animations are drawn specifically for this
game.

Some features did fall by the wayside, most significantly the posting of mail
messages upon winning the game.
The ability to rename monsters is also gone. It just doesn't work with the
graphics.

--------------------------------------------------------------------------------

Changes:

A detailed change list is rather meaningless for this version as pretty much
everything has been touched and the code been repackaged into different modules.
The focus of the changes has been maintainability. It should be pretty easy to
add new features into the ularn 1.6 code as a lot of the tight coupling between
modules has been broken.
I've tried to eliminate all magic numbers in the code, and use symbolic names
instead. (I've probably missed a few, but I'll fix them as I find them).

Those who are familiar with the ularn 1.5 code will not find things where they
expect.
I've tried to keep the grouping of functions fairly logical so it shouldn't be
to hard to work out where the functions and variables have gone.

The major changes were:

Isolation of all user input and display functions to a single module.
Changing from ASCII text to a tiled display.

Gone is the globally included extern.h. Variables are now exported from the
appropriate modules.

The static keyword has been used rather liberally to isolate the scope of local
variables to the packages concerned.

The centralised save function has been spilt so that each module contains
functions to save and load its own data.

The known array's purpose hase changed from being a single flag, indicating if
the player knows a certain location, to being an array of the item the player
beleives is at that location.

The stealth array has been changed to a set of flags indicating if the monster
has been seen and whether it is awake.

Most of the #defined constant lists have been replaced by enumerations.

A bucket load of comments have been added. Standard comment blocks have been
added to every file and every function.

You will not find the goto statement anywhere in the code anymore.

More bugs than I can count on a rainy Sunday afternoon have been eliminated.

Code has been generally tidied up and made more data driven. This has allowed
the removal of lots of special case code.

--------------------------------------------------------------------------------

Bugs:

Probably still lots of bugs in this, but it seems a lot more stable than 1.5pl4.

If you find a bug, let me know and I'll try and fix it for the next version.
Don't ask me about the stairs up from level 15. Those who have reached level 15
will known what I mean.

Report any bugs found to jolds@bigpond.com with the subject "ularn bug report".

Please include as much detail about what you were doing at the time the bug
occurred.

Try not to report about unfair gameplay features unless it is really broken as
ularn does tend to be a bit unfair at times. Many of these features are
intended.

Don't bother inundate me with suggestions for improvements. If you have thought
of it then there is a good change I have too. I already have a list about 5
pages long of potential additions and don't need any more just yet.

--------------------------------------------------------------------------------

Future improvements

The next major release will be version 2.
I have plans to add more features including new shops, ranged weapons, smarter
monsters, more monster types, maybe some optional sub-quests.

If this goes well I'll work on more enhancements.
