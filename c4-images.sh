# c4-images.sh c4ix|bare -- Doom's part of c4's build-images.sh, which
# sources it from c4's root (games/README.md in c4)
GAMEDIR=games/doom
case "$1" in
c4ix)
    # linked against C4IX's libc, with the shareware WAD beside it
    # (c4ix.vfs.txt: /usr/games/doom)
    if ls .doom/*.c4o > /dev/null 2>&1 && [ -f $PB/lib/libc.c4s ]; then
        say "linking Doom for C4IX"
        ./c4rlink32 .doom/*.c4o .doom/plat/dg-c4ix.c4o -l $PB/lib/libc.c4s -o $DISK/c4ix-doom.c4r > /dev/null
        if [ -f $GAMEDIR/data/doom1.wad ]; then cp $GAMEDIR/data/doom1.wad $DISK/doom1.wad; fi
    fi ;;
bare)
    if ls .doom/*.c4o > /dev/null 2>&1; then
        item doom
        ./c4rlink32 .doom/*.c4o .doom/plat/dg-bare.c4o .libc-bare/libc-*.c4o -o $GAMESDISK/doom.c4r > /dev/null
        if [ -f $GAMEDIR/data/doom1.wad ]; then cp $GAMEDIR/data/doom1.wad $GAMESDISK/; fi
        GAMES_ON_DISK="$GAMES_ON_DISK DOOM"
    fi ;;
esac
