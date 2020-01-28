(Emulator) Common Source Code Project
=====================================

This is a Git repo containing releases of Toshiya Takeda's [Common
Source Code Project][cscp], an archive of emulators for many different
Japanese microcomputers, mainly from the 1980s.

Sequential commits, from earliest to most recent, of the individual
releases from the [archives] are on the `vendor` branch of this repo.
(The newest version on the archives page is always the latest
version.)


Importing New Versions
----------------------

This is a bit tricky and takes some care, at the moment.

1. Ensure you have a local branch, `vendor`, tracking the `vendor`
   branch from the remote, and that your local branch is up to date
   with the remote. (You may have to do this manually if someone else
   has updated the remote since you last updated your local branch.)

2. Run `./import` repeatedly with the version numbers from the
   [archives] page you wish to import, e.g., `./import 2020-01-23-2`.
   There are no checks that the version you're importing is the one
   immediately following the most-recently imported version, so you
   must make sure you get this right.

3. When done importing new versions, push the vendor branch up to the
   remote: `git push origin vendor` or similar.


<!-------------------------------------------------------------------->
[cscp]: http://takeda-toshiya.my.coocan.jp/common/index.html
[archives]: http://takeda-toshiya.my.coocan.jp/common/history/index.html
