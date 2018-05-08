
Debian
====================
This directory contains files used to package giantd/giant-qt
for Debian-based Linux systems. If you compile giantd/giant-qt yourself, there are some useful files here.

## giant: URI support ##


giant-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install giant-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your giantqt binary to `/usr/bin`
and the `../../share/pixmaps/giant128.png` to `/usr/share/pixmaps`

giant-qt.protocol (KDE)

