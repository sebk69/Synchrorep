# Synchrorep

Synchorep is an opensource software to synchronize two directories, that mean reporting all modifications of one to the other and vice versa. At the end of synchronization, both directories will be strictly the same.

This is usefull principaly for nomads who work with a laptop or usb key but may interest also users who want making differencial copy to gain time.

### Compilation on Ubuntu 18.04+

You must install some dependencies :
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

Note : Synchrorep require that you are in a nautilus or KDE environment

##### To launch configuration
type in a terminal :
``` bash
$ synchrorep --config
```
Or launch synchrorep icon in Gnome "Activity" window

##### To start synchronization
(for example for "/home/me" directory) in terminal
``` bash
$ synchrorep
```
Or rigth click on folder in a nautilus window and choose "Synchronize..."
