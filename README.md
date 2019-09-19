# Synchrorep

Synchorep is an opensource software to synchronize two directories, that means reporting all modifications of one to the other and *vice versa*. At the end of synchronization, both directories will be strictly the same.

This is useful mainly for nomads who work with a laptop or usb key but may interest also users who want to gain time with the use of differential copy.

### Compilation on Ubuntu 18.04+

You must install some dependencies:
``` bash
$ sudo apt-get install libgtk2.0-dev
$ sudo apt-get install libsqlite3-dev
$ sudo apt-get install libnautilus-extension-dev
```

And make :
``` bash
$ make all
$ sudo make install
```

### Usage

Note : Synchrorep requires a nautilus or KDE desktop environment

#### To launch configuration
type in a terminal :
``` bash
$ synchrorep --config
```
Or launch synchrorep icon in Gnome "Activity" window

#### To start synchronization
(for example for "/home/me" directory) in terminal
``` bash
$ synchrorep --execute /home/me
```
Or rigth click on folder in a nautilus window and choose "Synchronize..."
