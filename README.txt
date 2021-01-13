(OLD) NEWS
----------

The Amiga version is complete. 
The Unix TTY and X11 versions are complete. 

NOTE: The Amiga version requires an AGA Amiga (Amiga 1200 or 4000).


Compiling Info
--------------

The Windows executable is built with the Borland C++ 5.5 command line tools.
The Amiga version is built with SAS C/C++ 6.51.

While I do not produce executable distributions for the Unix versions it
should not be too difficult to build from the source distribution.

The Unix port has been tested on:
- Red Hat Linux 7.1 on i686
- Amiga 4000/040 running NetBSD 1.5.2
- Sun UltraSparc running Solaris 2.5.1


Changes from 1.6.3 to 1.6.3a
----------------------------

- This is a Windows Graphical interface version update only.
* It contains a single fix that should dramatically reduce the amount
  of CPU time required by changing from using PeekMessage in a busy loop
  to using GetMessage (which is the proper way of doing it.)
  There is no change to the game-play and no perceptible change in
  response times.


Changes from 1.6.2 to 1.6.3
---------------------------

BUG FIXES:

- Fixed potion of heroism and Coked so the boost to charisma is lowered
  correctly when the effect wears off.
- Fixed effect of changing the enchantment level on a ring of strength.
- Fixed Winners Scoreboard so that more than 1 winner can be displayed.
- Fixed LRS so tax can now be paid.
- Fixed text formatting in LRS when paying tax.
- Fixed a bug in output of error messages during initialization that
  caused data corruption.
- Fixed Vaporize Rock spell for difficulty level 4+ to redraw statues
  correctly after the spell animation if the statue isn't destroyed.
- Removed function calls causing extra repainting of the map window.
- Fixed recalculation of AC and WC when destroying armor/weapons due
  to over enchantment.

NEW FEATURES:

- Added the Amiga version. Only supports AGA Amiga (1200/4000).
  * This has been tested on my Amiga 4000/040 and WinUAE.
- Added a Unix TTY version using the curses library.
- Improved some of the graphic tiles. 
  * Thanks to Craig Spencer for providing the new tile images.
- Windows version displays area separators using the current system
  colors for 3D objects (COLOR_3DFACE, COLOR_3DHILIGHT, COLOR_3DSHADOW).


Changes from 1.6.1 to 1.6.2
---------------------------

NOTE: Save files from earlier versions are likely not compatible with 1.6.2.
While the save file format hasn't changed, the interpretation of some of the
values has. Unexpected behavior may result if using an old save game. The
scoreboard from 1.6.1 should be compatible.

BUG FIXES:

- Had another go at fixing Dealer McDope's.
- Fixed a subtle initialization bug in the message window.
  * While only seen on NetBSD/Amiga, it may have affected other versions.
- Fixed spell damage for when wielding Slayer or Vorpal Blade.
- Fixed a problem in the map that could move the cursor at the players
  location outside the displayable map area, causing screen corruption.
- Fixed setting of the game difficulty level.
- Corrected the help file to have the correct options format for specifying
  the difficulty level.
- Fixed alter reality spell for wizard.
- Fixed storage of window position in ularn.ini file for Windows version.

NEW FEATURES:

- Added Unix/X11 support.
- New menu item to enable/disable beeps during the game.
- A beep is now played when the player gains a level.
- Learning multiple copies of Genocide, Alter reality and Permanance spells
  allows multiple castings of these spells.
- Walls enhanced to use shaped walls based on wall connectivity.
- Windows version now remembers the last state of the window.
- Some graphics changed in the tile set.


Changes from 1.6.0 to 1.6.1
---------------------------

NOTE: Scoreboard and saved games are incompatible between 1.6.0 and 1.6.1.

BUG FIXES:

- Fixed a bug in the rusting of armor that caused the pluses to be
  set to a dud value when the armor was very rusted.
- Fixed disenchanting/rusting of items to have appropriate effects on stats.
- Fixed a bug that caused slow monsters to either move every turn not move
  at all when the player is hasted.
- Fixed going up/down stairs when there is a monster on the staircase at
  the other end.
- Fixed a error in the smart-move routine that could cause demon princes
  and the king of hell to get "stuck" in the wall at the edge of the map.
- Fixed array indexing problems for calculating the enchantment level of
  new items.
- Fixed elevator up/down for operation in the volcano.
- Player positioning for dungeon level changes has been made a bit smarter.
  * It now selects the closest empty position to the desired location
    rather than performing a map scan from the current position.
- Fixed Dealer McDope's Pad.

NEW FEATURES:

- Added 'enhanced_interface' option to ularn.opt, as some people found
  the current method of opening doors disorienting:
  *  The option enhanced_interface/noenhanced_interface can now be specified
  in the ularn.opt file. The default, is noenhanced_interface. If the
  enhanced_interface option is set then the following changes to the\
  interface are used:
    . The player can no longer move onto closed doors to try and open them. 
      Instead the 'o' (Open Door) commands is provided.
      When Open Door is selected the player is prompted for the direction 
      of the door to be opened. 
    . The Close Door command is changed to operate similarly.
    . The 'O' (Open Chest) command is added to open a chest at the player's
      current location. This is a bit more convenient than moving off then
      back onto a chest for each attempt to open it.
- Added a new item: 'amulet of life preservation'. 
  * Prevents all level drain effects.
- Implemented infra-vision ability for monsters.
  * The ability was marked in the spell effect table but was never used.

OTHER CHANGES:

- Completed a code inspection/walk through of every module.
  * This resulted in a general tidy up of the code. Half of the bug fixes
    above were identified as a result of the analysis.
- ULarn now compiles with MinGW on Windows.
  * A MinGW makefile is now included with the source distribution. This is
    another step towards getting the UNIX X11 version working.


ULarn 1.6, patchlevel 0, as of 11 Aug 2003.
-------------------------------------------

NEW FEATURES:

- ULarn 1.6 is currently only available for 32-bit Windows sysems.
- UNIX TTY and X11 versions will appear in due course
- 'configure' scripts are gone in this version. 
- The next version will support AGA Amiga systems.
- All operating system dependent code is isolated in separate modules.
- Introduction of graphic tile display.
- The ability to rename monsters was removed.
- Isolation of all user input and display functions to a single module.
- Removal of GOTO's from the code base.

