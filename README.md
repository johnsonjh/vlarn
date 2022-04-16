# **VLarn**

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7300e03603374a828bd5467260353f41)](https://www.codacy.com/gh/johnsonjh/vlarn/dashboard?utm_source=github.com&utm_medium=referral&utm_content=johnsonjh/vlarn&utm_campaign=Badge_Grade)
[![DeepSourceA](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=active+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)
[![DeepSourceR](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=resolved+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)

<!-- toc -->

## Table of Contents

- [Overview](#overview)
- [Availability](#availability)
- [Distribution](#distribution)
- [Bug Reports](#bug-reports)
- [License](#license)
- [History](#history)

<!-- tocstop -->

## Overview

- **VLarn** is a free **_roguelike_** dungeon crawl adventure game, similar
  in concept to _Hack_, _Rogue_, or _Moria_, but with a different philosophy,
  feel, and winning criteria.

- _VLarn_ includes a tiled (_graphical_) interface, as well as a classic
  ASCII text interface, with optional menu items available for most commands.

## Availability

- **VLarn** is currently available for **UNIX**-like systems
  (in both [_TTY_](INSTALL.TTY.txt) and [_X11_](INSTALL.X11.txt) variants),
  **Microsoft Windows**, and (*classic*) [**AmigaOS**](Amiga-README.txt).

- Ports are currently in-progress for **PC DOS**, **Multics** (_MR12.6e+_),
  **Atari ST**, and **OpenVMS**.

- Additional ports are planned for **BeOS** / **Haiku** and _16-bit_ **CP/M**
  (_e.g._ **CP/M-68K**, **CP/M-Z8K**, **CP/M-86**).

- **VLarn**, like _ULarn_ 1.6, is easily portable to different systems, with
  all system-dependent code isolated in separate modules.

## Distribution

- **VLarn** is available from:
  - [GitHub](https://github.com/johnsonjh/vlarn)
  - [GitLab](https://gitlab.com/johnsonjh/vlarn)
  - [NotABug](https://notabug.org/trn/vlarn)
  - [SourceHut](https://sr.ht/~trn/vlarn)

## Bug Reports

- You may contact the VLarn maintainer, Jeffrey H. Johnson, via e-mail at
  \<[trnsz@pobox.com](mailto:trnsz@pobox.com)\>, with bug reports or questions.
  *Please include `[VLarn]` in the subject line*.

## License

- **VLarn**, _as a whole_, is distributed under the terms of the
  **GNU General Public License**, **version 2**.

- **VLarn** incorporates some components which are, individually, available
  under the terms and conditions of a modified **3-clause BSD-style license**.

- See the [LICENSE file](LICENSE.md) for complete licensing and redistribution
  information.

## History

- **VLarn** is a direct descendant of Julian Olds' _Ultra Larn_ _1.6_.

- Specifically, _VLarn_ is an _Ultra-Larn_ derivative, based on
  _Julian Olds_' _ULarn 1.6.3a_, which was a rewrite of _Ultra Larn_ _1.5.4_
  by _David Richerby_, _Josh Brandt_ and _Josh Bressers_, which was derived
  from _Phil Cordier_'s _ULarn 2.0_, which was derived from _Noah Morgan_'s
  _Larn 12_ with _James McNamara_'s patches.

### New in VLarn 5-alpha

- Adjusted maps to not require diagonal movements.
- Extended time limits
- Tweaked carrying capacity
- Increased checkpoint frequency
- Lemmings less aggressive
- Modified scoreboard system
- Removed Dealer McDope's pad
- Various bits of code clean-up
- Default to ncurses (legacy curses remains supported)

### New in ULarn 1.6 Patchlevel 3a

- Windows message using GetMessage instead of PeekMessage
- Fixed an end-of-game winners' scoreboard bug

### New in ULarn 1.6 Patchlevel 3

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

### Older History

- See the [OLD-NEWS file](OLD-NEWS.txt) for additional information.
