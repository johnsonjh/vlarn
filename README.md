# **VLarn**

[![Codacy Badge](https://app.codacy.com/project/badge/Grade/7300e03603374a828bd5467260353f41)](https://www.codacy.com/gh/johnsonjh/vlarn/dashboard?utm_source=github.com&utm_medium=referral&utm_content=johnsonjh/vlarn&utm_campaign=Badge_Grade)
[![DeepSourceA](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=active+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)
[![DeepSourceR](https://deepsource.io/gh/johnsonjh/vlarn.svg/?label=resolved+issues)](https://deepsource.io/gh/johnsonjh/vlarn/?ref=repository-badge)

---

<!-- toc -->

- [Overview](#overview)
- [Availability](#availability)
  * [Distribution](#distribution)
- [Bug Reports](#bug-reports)
- [History](#history)
  * [New features in VLarn 5-alpha](#new-features-in-vlarn-5-alpha)
  * [New features in ULarn 1.6 Patchlevel 3a](#new-features-in-ularn-16-patchlevel-3a)
  * [New features in ULarn 1.6 Patchlevel 3](#new-features-in-ularn-16-patchlevel-3)
- [Roadmap](#roadmap)
- [License](#license)

<!-- tocstop -->

## Overview

* **VLarn** is a free **_roguelike_** dungeon crawl adventure game, similar
  in concept to _Hack_, _Rogue_, or _Moria_, but with a different philosophy,
  feel, and winning criteria.

* _VLarn_ includes a tiled (*graphical*) interface, as well as a classic
  ASCII text interface, with optional menu items available for most commands.

## Availability

* **VLarn** is currently available for _Unix_, _Windows_, and _Amiga_ systems.
  * Ports are currently in the works for _Multics_, _Atari ST_, and _OpenVMS_.

* Like _ULarn_ 1.6, **VLarn** is easily portable to different systems, with
  all system-dependent code isolated in separate modules.

### Distribution

- [GitHub](https://github.com/johnsonjh/vlarn)
- [GitLab (HQ)](https://gitlab.com/johnsonjh/vlarn)
- [GitLab (trnsz)](https://gitlab.trnsz.com/johnsonjh/vlarn)
- [NotABug](https://notabug.org/trn/vlarn)
- [SourceHut](https://sr.ht/~trn/vlarn)

## Bug Reports

- You may contact the VLarn maintainer, Jeffrey H. Johnson, via e-mail at
  \<[trnsz@pobox.com](mailto:trnsz@pobox.com)\>, with bug reports or questions.
  Please include '_VLarn_' in the Subject line.

## History

* **VLarn** is a direct descendant of Julian Olds' _Ultra Larn_ _1.6_.

* Specifically, _VLarn_ is an _Ultra-Larn_ derivative, based on
  *Julian Olds*' _ULarn 1.6.3a_, which was a rewrite of _Ultra Larn_ _1.5.4_
  by *David Richerby*, *Josh Brandt* and *Josh Bressers*, which was derived
  from *Phil Cordier*'s _ULarn 2.0_, which was derived from *Noah Morgan*'s
  _Larn 12_ with *James McNamara*'s patches.

### New features in VLarn 5-alpha

- Adjusted maps to not require diagonal movements.
- Extended time limits
- Tweaked carrying capacity
- Increased checkpoint frequency
- Lemmings less aggressive
- Modified scoreboard system
- Removed Dealer McDope's pad
- Various bits of code clean-up
- Default to ncurses (legacy curses remains supported)

### New features in ULarn 1.6 Patchlevel 3a

- Windows message using GetMessage instead of PeekMessage
- Fixed an end-of-game winners' scoreboard bug

### New features in ULarn 1.6 Patchlevel 3

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

## Roadmap

* Future releases may incorporate, with explicit permission, changes and
  additions from Josh Bressers' _ULarn 1.7_, as well as Nathan Tenny's
  '_Variant ULarn_' release 1.5.4.

  * To avoid user confusion or version clashes with other _Larn_/_ULarn_
    derivatives, the next release of _VLarn_ will be **VLarn 5.0.0**.

## License

* **VLarn**, *as a whole*, is distributed under the terms of the
  **GNU General Public License*, **version 2**.

* **VLarn** incorporates some components which are, individually, available
  under the terms and conditions of a modified **3-clause BSD-style license**.

* See the [LICENSE](https://github.com/johnsonjh/vlarn/blob/master/LICENSE.md)
  file for complete licensing and redistribution information.
