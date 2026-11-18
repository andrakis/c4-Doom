# games/doom — Doom on C4IX, C4DOS and the bare machine

id Software's Doom (1993), as doomgeneric has it, compiled by c4lc and
played on libjs's whole display, with its sound effects and its music on
libjs's sound card. The shareware episode, Knee-Deep in the Dead, ships
with it. It is played from the keyboard.

[docs/ports-feasibility.md](../../docs/ports-feasibility.md) says what the
compilers needed for it, and [docs/c4ix-wolf3d.md](../../docs/c4ix-wolf3d.md)
how Wolfenstein 3D, the port before it, was done; this one follows it.

## Where it runs

| where | how | platform |
|---|---|---|
| C4IX, the whole display | `doom`, or Go > Games > Doom; the desktop gives way until it ends (`include/display.h`) | `dg-gui.c`, `dg-c4ix.c` |
| C4IX, a window | `doom -window` in a Terminal, whose window it plays in; also when a program that does not give way has the display. Several at once, each with `pin` on a CPU of its own (docs/c4ix-smp.md) | `dg-gui.c`, `dg-c4ix.c` |
| C4DOS | `DOOM` at the `A:\>` prompt; the page's `?system=c4dos` | `dg-gui.c`, `dg-bare.c` |
| a grin window, under C4DOS | Program Manager's Games > Doom, or `DOOM.C4R` in File Manager or a DOS Prompt, in grin (`?system=c4dos` boots into it) | `dg-gui.c`, `dg-bare.c`, through grin's table (below) |
| the bare machine | `doom.c4r` as the machine's image; the page's `?system=doom` | `dg-gui.c`, `dg-bare.c` |

On C4IX the game asks for a nice of -5 (docs/c4ix-design.md 4.4): served
sooner and longer than the programs beside it, but not to their
exclusion. The game and its WAD are in `/usr/games/doom`, where it starts,
and where its config and saved games go too. There the WAD is a RAM file,
which seeks; the same name looked up from elsewhere can be the host's
file, which does not. On the bare machine and under C4DOS the
WAD is on the games' disk beside it (`src/c4bb/images/games`), read whole
into memory when it opens (libc's `bare.c`).

Under grin, C4DOS's Windows 3.1-style shell (c4's `docs/grin-plan.md`,
3.6), the display is grin's, and grin publishes a table in C4DOS's slot
25 while it has it. The bare platform layer looks there first: finding
grin, it never takes the display (no `gui_attach`), and plays in a window
of grin's instead -- the window `dg-gui.c` asks for when the display is
not the game's (it says so on the console, which grin keeps), through the table's `fwin_open`, `fwin_frame` and
`fwin_poll` -- the C4IX window's three calls, pointed at grin. Each poll
runs grin's loop once, so grin's applications (a Clock, Notepad) carry on
while the game plays; the window is 1x (320x200) or 2x (640x400), from its
Size menu; keys reach the game only while its window has the focus; and
closing the window ends the game, as quitting it would. Without grin -- at
`A:\>`, full screen from grin, or on the bare machine -- nothing changes.

## Your own IWAD: `doom -home`

`doom -home` plays the person's own IWAD rather than the shareware:
`doom.wad` (Doom, registered or The Ultimate Doom), `doom2.wad`, and
Final Doom's `plutonia.wad` and `tnt.wad` (`doomu.wad` and Freedoom's
`freedoom1.wad`/`freedoom2.wad` too). Add it with the page's **Uploads**
(c4's libjs/README.md), in the folder `/home/doom` (one in `/home` itself
plays too), then start it. `-iwad NAME` picks one of several:

| | C4IX | the page, bare machine |
|---|---|---|
| the first found | `doom -home` | `?system=doom-full` |
| The Ultimate Doom | Go > Games > Doom (your files) > The Ultimate Doom, or `doom -home -iwad doom.wad` | `?system=doom1-full` |
| Doom II | ... > Doom II, or `-iwad doom2.wad` | `?system=doom2-full` |
| Final Doom: TNT | ... > Final Doom: TNT Evilution, or `-iwad tnt.wad` | `?system=tnt-full` |
| Final Doom: Plutonia | ... > Final Doom: The Plutonia Experiment, or `-iwad plutonia.wad` | `?system=plutonia-full` |

`D_FindIWAD` (`d_iwad.c`) looks in Doom's folder first, then in `/home`,
for the name `-iwad` gives (as given, in lower case or in upper case);
without one it takes the first of those names it finds, in that order --
Doom II before Doom, as Chocolate Doom prefers -- and names the others
it finds on the terminal. A name that is not there says so, and names
those that are.

| | its folder | then |
|---|---|---|
| C4IX (it has a `/home`) | `/home/doom/` | `/home/` |
| the bare machine, C4DOS | `doom/` at drive 0's root (`A:\DOOM`) | drive 0's root |

On C4IX those are RAM files of the host's, which seek, read a part at a
time; on the bare machine the IWAD is read whole into memory when it
opens, so `doom-full` has 128 MB. With `-home` the zone is 16 MiB
(`i_system.c`), as Chocolate Doom's is, for Doom II's and Final Doom's
bigger maps; the shareware keeps its 6. Without a file, `doom -home` says
what it wants and stops. The game's config is where it runs, shared
with the shareware's; its saved games are in `.savegame/IWAD/` there,
one folder for each IWAD (`m_config.c`), as Chocolate Doom keeps them,
since a save of one is not a save of another's maps. Dehacked patches and Chex Quest's `.deh` are not in this
build, so `chex.wad` and `hacx.wad` are not offered.

## Where it comes from

- **The game:** [doomgeneric](https://github.com/ozkl/doomgeneric) by
  ozkl, commit `e257a09`: Chocolate Doom's code with its platform reduced
  to six calls. It is used under the GNU GPL v2 (`LICENSE`).
  `README.doomgeneric.md` is its own README. Only the portable units and
  their headers are here; its platforms (SDL, X11 and the rest) are not.
- **The music player:** Chocolate Doom's `i_oplmusic.c` by Simon Howard
  (GPL v2), as Raptor's `i_oplmusic.cpp` carries it, made C again in
  `dg-snd.c`. Its General MIDI follows Chocolate Doom's `mus2mid.c`.

## Music: the AdLib, or General MIDI

The music is on the card's OPL by default, as DOSBox set to AdLib plays
it: the GENMIDI lump's FM instruments, D_INTROA for the title. When the
card has a General MIDI synth on its MIDI port (the page's Configuration,
Music: General MIDI, with a SoundFont from the Uploads or the computer's
own synth; c4's `libjs/README.md`), `dg-snd.c` plays the score there
instead, as Doom played it on a Sound Canvas, and the title is D_INTRO.
The MUS reader is the same either way, and so is its timing: each event
goes to the OPL player or becomes a MIDI message, stamped with the
card's stream. MUS to MIDI is `mus2mid.c`'s: channel 15 is percussion
(MIDI's 10th), 9-14 move up one, the controllers and system events are
MIDI's, and Doom's music volume scales each channel's volume by
sqrt(volume / 120), as Chocolate's Windows MIDI player does. `-opl`
keeps the AdLib whatever the card has.
- **The data:** `data/doom1.wad` is id's shareware v1.9 WAD, 4,196,020
  bytes, MD5 `f0cefca49926d00903cf57551d901abe`, unmodified. It came from
  Debian's `doom-wad-shareware` 1.9.fixed-5 package. `data/README.md`
  quotes the permission to pass it on.

## What is here

| file | what |
|---|---|
| doomgeneric's units and headers | the game, with the changes below |
| `dg-sum.c` | what every platform shares: each frame to the platform, 8-bit with its palette; a `-timedemo`'s frames checksummed, on a virtual clock |
| `dg-gui.c` | the platform on libjs's display: `GUI_BLIT8` scaled by the host to 640x480, keys as Doom's key codes, the machine's clock |
| `dg-snd.c` | the sound card: effects on eight of its sixteen sample channels, Doom's eight, and music, MUS played by Chocolate Doom's OPL player on its OPL, or as General MIDI on its MIDI port |
| `dg-c4ix.c`, `dg-bare.c` | whether the display has to be asked for: on C4IX the desktop has it, under grin grin does; and the window instead (C4IX's, or grin's) |
| `dg-host.c` | the platform on a host, built with gcc: no display and no sound, the reference for `make test-doom` |
| `c4inc/` | `stdint.h`, `stdbool.h` and `inttypes.h`, which C4IX's libc lacks |
| `c4/accel.mjs`, `c4/c4accel.h` | the browser's accelerators: the drawing loops in the game's own JavaScript, run by c4's libjs (docs/accel.md in c4) |
| `c4/acceltest.c` | each accelerator against the loop it stands for; `make test-doom` runs it first |

## The changes to doomgeneric

Each is marked `c4doom:` in the source, under `#ifdef C4DOOM`.
Everything else is doomgeneric's own code.

- **8-bit frames.** `I_FinishUpdate` hands the frame and its palette to
  `dg-sum.c` instead of making it 32-bit (`i_video.c`, `doomgeneric.c`).
- **No bitfields** in `i_video.h`'s `struct color`: four bytes are the
  same layout. c4lc has no bitfields.
- **The same frames everywhere.** Doom draws a few things from memory it
  does not own, and those differ from machine to machine:
  - A sprite column whose first row falls a fraction above its post, or
    whose last falls past its end, drew the byte before or after it. That
    is often the next zone block's header, whose pointers are addresses.
    Those rows are dropped (`r_things.c`).
  - The zone is zeroed, with 64 KB of slack past its end (`i_system.c`).
  - Chocolate Doom's emulation of the intercepts overrun wrote a pointer
    into game state. It writes the line's number instead (`p_maputl.c`).
- **A bug of doomgeneric's own:** `D_DoomMain` returns, where the
  original's never did, but `G_TimeDemo` had kept a pointer to its local
  `demolumpname`, read at the first tic. The bare machine happened to
  leave those bytes alone, while C4IX's traps wrote over them. It is
  `static` now (`d_main.c`).
- **No zenity** to show an error in a dialog (`i_system.c`), and no
  `SDL_mixer.h` (`i_sound.c`).
- **Speed.** `FixedMul` and `FixedDiv` in 32-bit pieces (`m_fixed.c`),
  because c4lc does `int64_t` at 32 bits in helper calls. Both give exactly
  what the 64-bit arithmetic does, checked against it over ten million
  pairs. demo3's timedemo on the bare machine went from 68 s to 37 s, of
  which the checksum itself is about a quarter. What is left is mostly
  `R_DrawColumn`'s inner loop.
- **The browser's accelerators** (`c4/accel.mjs`, docs/accel.md in c4).
  Where the machine has them (c4's libjs: the page, `cli.js`), the
  columns, the shadow, the floors' rows, the patches, the timedemo's
  checksum, `FixedMul` and `FixedDiv` are each one instruction, run by the
  host with exactly the C's results: a timedemo's frame is a fifth of the
  instructions it was, with the same checksums. Options has ACCELERATED to
  turn them off (`c4_accelerated` in the config), a ninth line in the
  small font; the menu sits 8 pixels higher for it (`m_menu.c`).
  Everywhere else, and in gcc's build, the C draws as before.

## How fast

In the browser the page's clock keeps the wall's time (`?realtime`,
libjs/web/app.js), so when the machine cannot keep up Doom drops frames
rather than playing slowly. On the machine this was measured on (libjs at
about 60M instructions a second in a background tab), E1M1 played at
about 13 frames a second on the bare machine, and slower inside C4IX with
the desktop and top behind it. Doom on the original hardware ran at up to
35.

## Building and testing

- `make doom-objs` compiles each unit with c4lc into `.doom/`, and the
  two platform choices into `.doom/plat/`. `build-images.sh` links them
  against C4IX's libc as `/usr/games/doom/doom.c4r`, and whole with the
  bare libc (`make wolf3d-bare-objs`) as `images/games/doom.c4r`.
- `make doom-host` is the gcc build, 32-bit as the c4lc build is.
- `make test-doom` plays demo3 with `-timedemo -nosound` four ways: the
  gcc build, then c4lc's on the bare machine, under C4DOS and inside C4IX.
  All must print the same frames:

  ```
  timedemo: demo1: 5065 frames, checksum 2be7597c
  timedemo: demo2: 3875 frames, checksum 1abaeabe
  timedemo: demo3: 2173 frames, checksum 01914fe7
  ```

  A timedemo plays one tic a frame whatever the clock says. The screen
  wipe between levels runs by the clock, though, so while timing the clock
  is virtual (`dg-sum.c`). `-nosound` because the gcc build has none, so
  both run the same code.

  To compare how fast the game runs between machines, add `-show`: the
  frames go to the display (or to the window, with `-window`) as well, so the
  time counts the drawing. Every timedemo prints a speed line before its
  checksum, by the machine's real clock from the first frame:

  ```
  speed: demo3: 2173 frames in 14.205 s, 152.9 fps
  ```

  In the page that clock is the wall's. Under `cli.js`, unpaced, it is
  cycles over the MHz asked for, so there time the run from outside.
