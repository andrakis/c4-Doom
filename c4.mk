# c4.mk -- this game's part of c4's Makefile. c4 includes it from
# games/doom/c4.mk, its submodule, and runs it from c4's root, with c4's
# tools and variables (games/README.md in c4). Without it, c4 builds
# everything else, and test-doom says it was skipped.

# Doom (games/doom/README.md): doomgeneric's units with c4lc, the same
# objects for C4IX, the bare machine and C4DOS, but for the two that say
# whether the display has to be asked for (dg-c4ix.c, dg-bare.c, in
# .doom/plat). build-images.sh links them: against C4IX's libc, and whole
# with the bare libc of wolf3d-bare-objs.
DOOM       := games/doom
DOOM_PLAT  := dg-host dg-c4ix dg-bare
DOOM_UNITS := $(filter-out $(DOOM_PLAT),$(patsubst $(DOOM)/%.c,%,$(wildcard $(DOOM)/*.c)))
DOOM_HDRS  := $(wildcard $(DOOM)/*.h $(DOOM)/c4inc/*.h $(DOOM)/c4/*.h) \
              $(wildcard src/c4ix/libc/include/*.h src/c4ix/libc/include/sys/*.h) \
              src/c4ix/include/display.h libjs/guest/gui.h libjs/guest/gui.c \
              libjs/guest/snd.h libjs/guest/snd.c libjs/guest/accel.h
DOOM_CC    := $(call lc32,-c 400000) -O $(IMG_G) -mfuse -fno-builtin -c -conforming \
              -D C4DOOM -D FEATURE_SOUND -I $(DOOM)/c4inc -I src/c4ix/libc/include \
              -I src/c4ix/include -I libjs/guest -I $(DOOM)
doom-objs: $(DOOM_UNITS:%=.doom/%.c4o) .doom/plat/dg-c4ix.c4o .doom/plat/dg-bare.c4o
.doom/%.c4o: $(DOOM)/%.c $(DOOM_HDRS) $(C4LC_LISP) .img_g | $(LC32_BIN)
	@mkdir -p .doom/plat
	$(DOOM_CC) $< $@ > /dev/null
.doom/plat/%.c4o: $(DOOM)/%.c $(DOOM_HDRS) $(C4LC_LISP) .img_g | $(LC32_BIN)
	@mkdir -p .doom/plat
	$(DOOM_CC) $< $@ > /dev/null
PHONY += doom-objs

# ... and by gcc for the host, with no display and no sound (dg-host.c):
# the reference test-doom holds the c4lc build to. 32-bit, as the c4lc
# build is; the 64-bit build plays demo1 differently.
DOOM_HOST_SRCS := $(filter-out $(DOOM)/dg-gui.c $(DOOM)/dg-snd.c $(DOOM)/dg-c4ix.c $(DOOM)/dg-bare.c, \
                  $(wildcard $(DOOM)/*.c))
doom-host: $(DOOM_HOST_SRCS) $(wildcard $(DOOM)/*.h $(DOOM)/c4/*.h)
	gcc -m32 -O2 -std=gnu99 -w -DC4DOOM -DNORMALUNIX -DLINUX -D_DEFAULT_SOURCE -I$(DOOM) -o $@ $(DOOM_HOST_SRCS) -lm

# The data, which is not in the repository: data/doom1.wad, the shareware
# package as it was distributed, checked by its SHA-256. c4's
# games/fetch.sh takes it from $DOOM_WAD when that names a copy, or
# else from the mirrors in data/MIRRORS (data/README.md).
doom-data: $(DOOM)/data/doom1.wad
$(DOOM)/data/doom1.wad:
	@sh games/fetch.sh $@ 1d7d43be501e67d927e415e0b8f3e29c3bf33075e859721816f652a526cac771 DOOM_WAD $$(cat $(DOOM)/data/MIRRORS 2>/dev/null)
PHONY += doom-data

# c4's port: Doom's accelerators (c4/accel.mjs) against their loops, on the
# bare machine, with libjs's own (cli.js loads them, docs/accel.md). In a
# directory of its own: c4-images.sh links .doom/*.c4o into the game. The
# timedemo runs below have them, so their frames are the accelerators'; the
# gcc build has none. The General MIDI run goes without (--no-accel): the
# music's time is the machine's, which the accelerators shorten.
.doom/test/acceltest.c4o: $(DOOM)/c4/acceltest.c $(DOOM_HDRS) $(C4LC_LISP) .img_g | $(LC32_BIN)
	@mkdir -p .doom/test
	$(DOOM_CC) -I $(DOOM)/c4 $< $@ > /dev/null
.doom/test/acceltest.c4r: .doom/test/acceltest.c4o libc-bare-objs
	./c4rlink32 $< .libc-bare/libc-*.c4o -o $@ > /dev/null

# test-doom: demo3 in Doom's -timedemo, which plays a tic a frame and here
# on a virtual clock (dg-sum.c), built by gcc and by c4lc -- on the bare
# machine, under C4DOS and inside C4IX -- must show the same 2173 frames.
# The second and third together, as they share nothing. demo1 and demo2
# match too (games/doom/README.md); demo3 is the shortest. About five
# minutes, nearly all of it on c4m. And on the bare machine once more,
# with its music on a sound card with a General MIDI synth (cli.js --midi):
# the same frames, and the music as MIDI, drums and all (midi-notes.mjs).
test-doom: doom-data doom-host c4bb-images test-libc-bare .doom/test/acceltest.c4r
	@rm -rf .doom_t && mkdir -p .doom_t
	@timeout 600 node libjs/cli.js -m 64 .doom/test/acceltest.c4r > .doom_t/accel.txt 2>&1; cat .doom_t/accel.txt
	@grep -q "^accel: Doom's, [0-9]* cases, each as its loop has it" .doom_t/accel.txt \
	   || { echo "test-doom: the accelerators do not draw as their loops"; exit 1; }
	@cd .doom_t && cp ../games/doom/data/doom1.wad . && ../doom-host -iwad doom1.wad -timedemo demo3 -nosound 2>/dev/null \
	   | grep '^timedemo' > host.txt
	@cat .doom_t/host.txt
	@cd src/c4bb/images/games && { timeout 1500 node ../../../../libjs/cli.js -m 64 -d . doom.c4r -timedemo demo3 -nosound \
	     > ../../../../.doom_t/bare.out 2>/dev/null & \
	   printf 'doom -timedemo demo3 -nosound\nver\n' | timeout 1500 node ../../../../libjs/cli.js -m 64 -d . ../c4dos32.c4r \
	     > ../../../../.doom_t/dos.out 2>/dev/null; wait; }
	@{ cd src/c4bb/images/games && timeout 1500 node ../../../../libjs/cli.js -m 64 -d . --no-accel --midi ../../../../.doom_t/gm.txt doom.c4r \
	     -timedemo demo3 -nosfx > ../../../../.doom_t/gm.out 2>/dev/null; } & \
	 printf 'doom -timedemo demo3 -nosound\nexit\n' | timeout 1500 node libjs/cli.js -m 64 -d src/c4bb/images/disk src/c4bb/images/c4ix32.c4r \
	   2>/dev/null > .doom_t/c4ix.out; wait
	@for w in bare dos c4ix; do grep '^timedemo' .doom_t/$$w.out | cmp -s .doom_t/host.txt - \
	   || { echo "test-doom: $$w's frames differ:"; tail -5 .doom_t/$$w.out; exit 1; }; done
	@grep '^timedemo' .doom_t/gm.out | cmp -s .doom_t/host.txt - \
	   || { echo "test-doom: with General MIDI music, the frames differ:"; tail -5 .doom_t/gm.out; exit 1; }
	@m=$$(node libjs/tests/src/midi-notes.mjs .doom_t/gm.txt 200 4) && echo "test-doom: General MIDI: $$m" \
	   || { echo "test-doom: the General MIDI music did not play: $$m"; exit 1; }
	@grep -A1 '^timedemo' .doom_t/dos.out | grep -Eq '^A(:\\)?>C4DOS version' \
	   || { echo "test-doom: C4DOS did not come back to its prompt:"; tail -5 .doom_t/dos.out; exit 1; }
	@rm -rf .doom_t
	@echo "test-doom: OK"
PHONY += test-doom

GAME_OBJS  += doom-objs
GAME_TESTS += test-doom
