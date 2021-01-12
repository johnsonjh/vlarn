# **VLarn**

[![DeepSourceA](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=active+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)
[![DeepSourceR](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=resolved+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)

---------

**VLarn** is a free ***roguelike*** dungeon crawl adventure game, similar
in concept to *Hack*, *Rogue*, or *Moria*, but with a different philosophy,
feel, and winning criteria.

## Lineage

*VLarn* is an *Ultra-Larn* derivative, descended from Julian Olds' *ULarn
1.6.3a*, which is descended from David Richerby's, Josh Brandt's, and
Josh Bressers' *ULarn 1.5.4*, which is descended from Phil Cordier's
*ULarn 2.0*, which is descended from Noah Morgan's *Larn 12*, with James
McNamara's patches.

## Features

*VLarn* includes both a graphical tiled interface, as well as a
classic ASCII text UI, with optional menu items for most commands.

## Availability

**VLarn** is currently available for *Unix*, *Windows*, and *Amiga* systems.
Ports are currently planned for *Multics*, *Atari ST*, *OpenVMS*, and *z/OS TOS/E*.

Like *ULarn* 1.6, **VLarn** is easily portable to different systems,
with all system-dependent code isolated in separate modules.

## History

**VLarn** is a fork of Julian Olds' *ULarn 1.6* branch.

## Distribution

* [Gridfinity Gitlab](https://gitlab.gridfinity.com/jeff/vlarn)
* [SourceHut](https://sr.ht/~trn/vlarn/)
* [GitHub](https://github.com/johnsonjh/vlarn)

## Issue Tracking

* [Gridfinity Gitlab Issues](https://gitlab.gridfinity.com/jeff/vlarn/-/issues)

* Alternately, you may contact the current maintainer — Jeffrey Johnson
\<[jhj@trnsz.com](mailto:jhj@trnsz.com)\> — with any bug reports or
questions. Please include '*VLarn*' in the Subject line.

## Branding

To avoid user confusion or version clashes with other *Larn*/*ULarn*
derivatives, the next release of *VLarn* will be **VLarn 5.0**. 

This release will incorporate changes and additions from Josh Bressers'
*ULarn 1.7*, as well as Nathan Tenny's '*Variant ULarn*' release 1.5.4.

## New features in VLarn 5

* Adjusted maps to not require diagonal movements.
* Extended time limits
* Tweaked carrying capacity
* Increased checkpoint frequency
* Lemmings less aggressive
* Modified scoreboard system
* Removed Dealer McDope's pad
* Negative score prevention
* Various bits of code clean-up

## New features in ULarn 1.6 Patchlevel 3a

* Windows message using GetMessage instead of PeekMessage
* Fixed an end-of-game winners' scoreboard bug

## New features in ULarn 1.6 Patchlevel 3

* Resurrected curses-based (ASCII text/TTY) interface
* Amiga (AGA) port
* Improved graphic tile set
* Display improvements for Windows systems
* Potion of Heroism and Coke now correctly wear off
* Ring of Strength enchantment level corrections
* Winner's Scoreboard expanded
* Larn Revenue Service back taxes can now be paid
* Enhanced text formatting for paying taxes
* Corrected output of error messages during initialization
* Fixed bugs in Vaporise Rock spell for difficulty level 4+
* Removed unneeded function calls causing extra map redraws
* Fixed calculation of AC and WC when destroying armor or weapons
