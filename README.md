# VLarn

_VLarn_ is a free **_roguelike_** dungeon crawl adventure game, similar in
concept to _Hack_, _Rogue_, or _Moria_, but with a different philosophy, feel,
and winning criteria.

## Lineage

_VLarn_ is an _Ultra-Larn_ derivative, descended from Julian Olds' _ULarn
1.6.3a_, which is descended from David Richerby's, Josh Brandt's, and Josh
Bressers' _ULarn 1.5.4_, which is descended from Phil Cordier's _ULarn 2.0_,
which is descended from Noah Morgan's _Larn 12_, with James McNamara's patches.

## Features

_VLarn_ includes both a graphical tiled interface, as well as a classic ASCII
text UI, with optional menu items for most commands.

## Availability

_VLarn_ is currently available for Unix, Windows, and Amiga systems. Ports are
currently planned for Atari ST, OpenVMS, and z/OS TOS/E.

Like _ULarn_ 1.6, _VLarn_ is easily portable to different systems, with all
system dependent code isolated into separate modules.

## History

VLarn is a fork of Julian Olds' _ULarn 1.6_ branch.

Please contact the current maintainer — Jeffrey Johnson
\<[jhj@trnsz.com](mailto:jhj@trnsz.com)\> — with any bug reports or questions.
Please include '_VLarn_' in the Subject line.

To avoid user confusion and version clashes with other _Larn_/_ULarn_
derivatives, the next release of _VLarn_ will be **VLarn 5.0**. It will
incorporate changes and additions from Josh Bressers' _ULarn 1.7_, as well as
Nathan Tenny's '_Variant ULarn_' release 1.5.4.

## New features in VLarn 5

- Adjusted maps to not require diagonal movements.
- Extended time limits
- Tweaked carrying capacity
- Increased checkpoint frequency
- Lemmings less aggressive
- Modified scoreboard system
- Removed Dealer McDope's pad
- Negative score prevention
- Various bits of code clean-up

## New features in ULarn 1.6 Patchlevel 3a

- Windows message using GetMessage instead of PeekMessage
- Fixed an end-of-game winners' scoreboard bug

## New features in ULarn 1.6 Patchlevel 3

- Resurrected curses-based (ASCII text/TTY) interface
- Amiga (AGA) port
- Improved graphic tile set
- Display improvements for Windows systems
- Potion of Heroism and Coke now correctly wear off
- Ring of Strength enchantment level corrections
- Winner's Scoreboard expanded
- Larn Revenue Service back taxes can now be paid
- Enhanced text formatting for paying taxes
- Corrected output of error messages during initialization
- Fixed bugs in Vaporise Rock spell for difficulty level 4+
- Removed unneeded function calls causing extra map redraws
- Fixed calculation of AC and WC when destroying armor or weapons
