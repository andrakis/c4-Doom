# data — Doom's shareware WAD

`doom1.wad` is id Software's shareware Doom v1.9 data, unmodified:
4,196,020 bytes, MD5 `f0cefca49926d00903cf57551d901abe`. It came from
Debian's `doom-wad-shareware` package, 1.9.fixed-5, which took it from
`ftp.cdrom.com:/pub/idgames`.

id's licence for the shareware release reserves distribution to id. On
23 October 1999 John Carmack answered Debian's question about it by
email: "The DOOM shareware wad is freely distributable." Debian's
copyright file for the package records that message, and it is why the
WAD may be fetched and put on c4's disks, as Debian ships it.

## Getting it

`doom1.wad` is not in this repository. In c4, `make doom-data` puts it here
with c4's `games/fetch.sh`:
- from `$DOOM_WAD`, when that names your copy;
- or else from the first URL in `MIRRORS` that gives exactly this file.

Either way it is refused unless its SHA-256 is
`1d7d43be501e67d927e415e0b8f3e29c3bf33075e859721816f652a526cac771`.
Every URL in `MIRRORS` was checked to give exactly that file
(2026-10-25).
