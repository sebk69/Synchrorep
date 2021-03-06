lib_config = -fPIC -D_REENTRANT `pkg-config --cflags --libs gtk+-2.0 gdk-x11-2.0 sqlite3 gthread-2.0 gio-2.0`
include_config = -I/usr/include

all: synchrorep libnautilus-synchrorep.so

synchrorep: db_access.o ac_common.o ac_behavior.o ac_config.o ac_preferences.o tc_file_with_infos.o tc_files_binomial.o tc_find_files.o tc_misc.o dialog_info.o dialog_delete.o dialog_error.o dialog_file_conflict.o widget_behavior.o window_card.o window_config_main.o window_default_behavior.o new_synchronization.o execute.o logging.o application.o md5sum.o ac_logs.o widget_synchronization_log.o window_log.o window_ask_at_the_end.o widget_ask_at_the_end.o dialog_choose_mode.o tc_mount.o
	g++ -fPIC -D_REENTRANT -L/usr/lib -o synchrorep db_access.o ac_common.o ac_behavior.o ac_config.o ac_preferences.o tc_file_with_infos.o tc_files_binomial.o tc_find_files.o tc_misc.o dialog_info.o dialog_delete.o dialog_error.o dialog_file_conflict.o widget_behavior.o window_card.o window_config_main.o window_default_behavior.o new_synchronization.o execute.o logging.o application.o md5sum.o ac_logs.o widget_synchronization_log.o window_log.o window_ask_at_the_end.o widget_ask_at_the_end.o dialog_choose_mode.o tc_mount.o `pkg-config --cflags --libs gtk+-2.0 gdk-x11-2.0 sqlite3 gthread-2.0 gio-2.0` -lgtk-x11-2.0

libnautilus-synchrorep.so: plugin.o
	g++ -fPIC -shared -o libnautilus-synchrorep.so  plugin.o `pkg-config --cflags --libs gtk+-2.0 gdk-x11-2.0`

db_access.o:
	gcc $(lib_config) $(include_config) -o db_access.o -c synchrorep-core/accessors/db_access.cpp
ac_common.o:
	gcc $(lib_config) $(include_config) -o ac_common.o -c synchrorep-core/accessors/ac_common.cpp
ac_behavior.o:
	gcc $(lib_config) $(include_config) -o ac_behavior.o -c synchrorep-core/accessors/ac_behavior.cpp
ac_config.o:
	gcc $(lib_config) $(include_config) -o ac_config.o -c synchrorep-core/accessors/ac_config.cpp
ac_preferences.o:
	gcc $(lib_config) $(include_config) -o ac_preferences.o -c synchrorep-core/accessors/ac_preferences.cpp

tc_file_with_infos.o:
	gcc $(lib_config) $(include_config) -o tc_file_with_infos.o -c synchrorep-core/technical/tc_file_with_infos.cpp
tc_files_binomial.o:
	gcc $(lib_config) $(include_config) -o tc_files_binomial.o -c synchrorep-core/technical/tc_files_binomial.cpp
tc_find_files.o:
	gcc $(lib_config) $(include_config) -o tc_find_files.o -c synchrorep-core/technical/tc_find_files.cpp
tc_misc.o:
	gcc $(lib_config) $(include_config) -o tc_misc.o -c synchrorep-core/technical/tc_misc.cpp

dialog_info.o:
	gcc $(lib_config) $(include_config) -o dialog_info.o -c synchrorep-core/interface/dialog_info.cpp
dialog_delete.o:
	gcc $(lib_config) $(include_config) -o dialog_delete.o -c synchrorep-core/interface/dialog_delete.cpp
dialog_error.o:
	gcc $(lib_config) $(include_config) -o dialog_error.o -c synchrorep-core/interface/dialog_error.cpp
dialog_file_conflict.o:
	gcc $(lib_config) $(include_config) -o dialog_file_conflict.o -c synchrorep-core/interface/dialog_file_conflict.cpp
dialog_choose_mode.o:
	gcc $(lib_config) $(include_config) -o dialog_choose_mode.o -c synchrorep-core/interface/dialog_choose_mode.cpp
widget_behavior.o:
	gcc $(lib_config) $(include_config) -o widget_behavior.o -c synchrorep-core/interface/widget_behavior.cpp
widget_ask_at_the_end.o:
	gcc $(lib_config) $(include_config) -o widget_ask_at_the_end.o -c synchrorep-core/interface/widget_ask_at_the_end.cpp
window_card.o:
	gcc $(lib_config) $(include_config) -o window_card.o -c synchrorep-core/interface/window_card.cpp
window_config_main.o:
	gcc $(lib_config) $(include_config) -o window_config_main.o -c synchrorep-core/interface/window_config_main.cpp
window_default_behavior.o:
	gcc $(lib_config) $(include_config) -o window_default_behavior.o -c synchrorep-core/interface/window_default_behavior.cpp
window_ask_at_the_end.o:
	gcc $(lib_config) $(include_config) -o window_ask_at_the_end.o -c synchrorep-core/interface/window_ask_at_the_end.cpp

new_synchronization.o:
	gcc $(lib_config) $(include_config) -o new_synchronization.o -c synchrorep-core/applicative/new_synchronization.cpp
execute.o:
	gcc $(lib_config) $(include_config) -o execute.o -c synchrorep-core/applicative/execute.cpp
logging.o:
	gcc $(lib_config) $(include_config) -o logging.o -c synchrorep-core/applicative/logging.cpp
application.o:
	gcc $(lib_config) $(include_config) -o application.o -c synchrorep-core/applicative/application.cpp

md5sum.o:
	gcc -o md5sum.o -c synchrorep-core/technical/md5.cc

ac_logs.o:
	gcc $(lib_config) $(include_config) -o ac_logs.o -c synchrorep-core/accessors/ac_logs.cpp

widget_synchronization_log.o:
	gcc $(lib_config) $(include_config) -o widget_synchronization_log.o -c synchrorep-core/interface/widget_synchronization_log.cpp

window_log.o:
	gcc $(lib_config) $(include_config) -o window_log.o -c synchrorep-core/interface/window_log.cpp

tc_mount.o:
	gcc $(lib_config) $(include_config) -o tc_mount.o -c synchrorep-core/technical/tc_mount.cpp

plugin.o:
	gcc -fPIC -D_REENTRANT $(lib_config) -I/usr/include/nautilus  -I/usr/include/i386-linux-gnu/nautilus -lnautilus-extension -c -o plugin.o synchrorep-extension/plugin.c

clean:
	rm -f *.o
	rm -f synchrorep
	rm -f libnautilus-synchrorep.so

install:
	test -d /usr || mkdir /usr
	test -d /usr/bin || mkdir /usr/bin
	cp synchrorep /usr/bin/synchrorep
	test -d /usr/lib || mkdir /usr/lib
	test -d /usr/lib/nautilus || mkdir /usr/lib/nautilus
	test -d /usr/lib/nautilus/extensions-2.0 || mkdir /usr/lib/nautilus/extensions-2.0
	cp libnautilus-synchrorep.so /usr/lib/nautilus/extensions-2.0/libnautilus-synchrorep.so
	test -d /usr/lib/nautilus/extensions-3.0 || mkdir /usr/lib/nautilus/extensions-3.0
	cp libnautilus-synchrorep.so /usr/lib/nautilus/extensions-3.0/libnautilus-synchrorep.so
	test -d /usr/share || mkdir /usr/share
	test -d /usr/share/doc || mkdir /usr/share/doc
	test -d /usr/share/doc/synchrorep || mkdir /usr/share/doc/synchrorep
	test -d /usr/share/synchrorep || mkdir /usr/share/synchrorep
	test -d /usr/share/applications || mkdir /usr/share/applications
	test -d /usr/share/pixmaps || mkdir /usr/share/pixmaps
	cp synchrorep-core/ressources/copyright /usr/share/doc/synchrorep/copyright
	cp synchrorep-core/ressources/licence /usr/share/doc/synchrorep/licence
	cp synchrorep-core/ressources/synchrorep.config.desktop /usr/share/applications
	cp -p synchrorep-core/ressources/synchrorep.desktop /usr/share/synchrorep/synchrorep.desktop
	cp synchrorep-core/ressources/synchrorep.group.menu /usr/share/synchrorep/synchrorep.group.menu
	cp synchrorep-core/ressources/synchrorep.group.directory /usr/share/synchrorep/synchrorep.group.directory
	cp synchrorep-core/ressources/synchrorep.png /usr/share/pixmaps/synchrorep.png
	cp synchrorep-core/ressources/copy_arrow.png /usr/share/synchrorep/copy_arrow.png
	cp synchrorep-core/ressources/sync_arrow.png /usr/share/synchrorep/sync_arrow.png
	cp synchrorep-core/ressources/synchrorep.group.png /usr/share/pixmaps/synchrorep.group.png
	test -d /usr/share/locale-langpack || mkdir /usr/share/locale-langpack
	test -d /usr/share/locale-langpack/es || mkdir /usr/share/locale-langpack/es
	test -d /usr/share/locale-langpack/es/LC_MESSAGES || mkdir /usr/share/locale-langpack/es/LC_MESSAGES
	test -d /usr/share/locale-langpack/fr || mkdir /usr/share/locale-langpack/fr
	test -d /usr/share/locale-langpack/fr/LC_MESSAGES || mkdir /usr/share/locale-langpack/fr/LC_MESSAGES
	test -d /usr/share/locale-langpack/pl || mkdir /usr/share/locale-langpack/pl
	test -d /usr/share/locale-langpack/pl/LC_MESSAGES || mkdir /usr/share/locale-langpack/pl/LC_MESSAGES
	test -d /usr/share/locale-langpack/ru || mkdir /usr/share/locale-langpack/ru
	test -d /usr/share/locale-langpack/ru/LC_MESSAGES || mkdir /usr/share/locale-langpack/ru/LC_MESSAGES
	test -d /usr/share/locale-langpack/de || mkdir /usr/share/locale-langpack/de
	test -d /usr/share/locale-langpack/de/LC_MESSAGES || mkdir /usr/share/locale-langpack/de/LC_MESSAGES
	cp locale-langpack/es/LC_MESSAGES/synchrorep.mo /usr/share/locale-langpack/es/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/fr/LC_MESSAGES/synchrorep.mo /usr/share/locale-langpack/fr/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/pl/LC_MESSAGES/synchrorep.mo /usr/share/locale-langpack/pl/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/ru/LC_MESSAGES/synchrorep.mo /usr/share/locale-langpack/ru/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/de/LC_MESSAGES/synchrorep.mo /usr/share/locale-langpack/de/LC_MESSAGES/synchrorep.mo

package:
	test -d synchrorep-package/usr/bin || mkdir -p synchrorep-package/usr/bin
	cp synchrorep synchrorep-package/usr/bin/synchrorep
	test -d synchrorep-package/usr/lib/nautilus/extensions-2.0 || mkdir -p synchrorep-package/usr/lib/nautilus/extensions-2.0
	cp libnautilus-synchrorep.so synchrorep-package/usr/lib/nautilus/extensions-2.0/libnautilus-synchrorep.so
	test -d synchrorep-package/usr/lib/nautilus/extensions-3.0 || mkdir -p synchrorep-package/usr/lib/nautilus/extensions-3.0
	cp libnautilus-synchrorep.so synchrorep-package/usr/lib/nautilus/extensions-3.0/libnautilus-synchrorep.so
	test -d synchrorep-package/usr/share/doc/synchrorep || mkdir -p synchrorep-package/usr/share/doc/synchrorep
	test -d synchrorep-package/usr/share/synchrorep || mkdir -p synchrorep-package/usr/share/synchrorep
	test -d synchrorep-package/usr/share/applications || mkdir -p synchrorep-package/usr/share/applications
	test -d synchrorep-package/usr/share/pixmaps || mkdir -p synchrorep-package/usr/share/pixmaps
	cp synchrorep-core/ressources/copyright synchrorep-package/usr/share/doc/synchrorep/copyright
	cp synchrorep-core/ressources/licence synchrorep-package/usr/share/doc/synchrorep/licence
	cp synchrorep-core/ressources/synchrorep.config.desktop synchrorep-package/usr/share/applications
	cp -p synchrorep-core/ressources/synchrorep.desktop synchrorep-package/usr/share/synchrorep/synchrorep.desktop
	cp synchrorep-core/ressources/synchrorep.group.menu synchrorep-package/usr/share/synchrorep/synchrorep.group.menu
	cp synchrorep-core/ressources/synchrorep.group.directory synchrorep-package/usr/share/synchrorep/synchrorep.group.directory
	cp synchrorep-core/ressources/synchrorep.png synchrorep-package/usr/share/pixmaps/synchrorep.png
	cp synchrorep-core/ressources/copy_arrow.png synchrorep-package/usr/share/synchrorep/copy_arrow.png
	cp synchrorep-core/ressources/sync_arrow.png synchrorep-package/usr/share/synchrorep/sync_arrow.png
	cp synchrorep-core/ressources/synchrorep.group.png synchrorep-package/usr/share/pixmaps/synchrorep.group.png
	test -d synchrorep-package/usr/share/locale-langpack/es/LC_MESSAGES || mkdir -p synchrorep-package/usr/share/locale-langpack/es/LC_MESSAGES
	test -d synchrorep-package/usr/share/locale-langpack/fr/LC_MESSAGES || mkdir -p synchrorep-package/usr/share/locale-langpack/fr/LC_MESSAGES
	test -d synchrorep-package/usr/share/locale-langpack/pl/LC_MESSAGES || mkdir -p synchrorep-package/usr/share/locale-langpack/pl/LC_MESSAGES
	test -d synchrorep-package/usr/share/locale-langpack/ru/LC_MESSAGES || mkdir -p synchrorep-package/usr/share/locale-langpack/ru/LC_MESSAGES
	test -d synchrorep-package/usr/share/locale-langpack/de/LC_MESSAGES || mkdir -p synchrorep-package/usr/share/locale-langpack/de/LC_MESSAGES
	cp locale-langpack/es/LC_MESSAGES/synchrorep.mo synchrorep-package/usr/share/locale-langpack/es/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/fr/LC_MESSAGES/synchrorep.mo synchrorep-package/usr/share/locale-langpack/fr/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/pl/LC_MESSAGES/synchrorep.mo synchrorep-package/usr/share/locale-langpack/pl/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/ru/LC_MESSAGES/synchrorep.mo synchrorep-package/usr/share/locale-langpack/ru/LC_MESSAGES/synchrorep.mo
	cp locale-langpack/de/LC_MESSAGES/synchrorep.mo synchrorep-package/usr/share/locale-langpack/de/LC_MESSAGES/synchrorep.mo
	mkdir -p synchrorep-package/DEBIAN
	cp control64 synchrorep-package/DEBIAN/control
	chown -R root:root synchrorep-package
	chmod 775 synchrorep-package
	chmod 775 synchrorep-package/DEBIAN
	chmod 744 synchrorep-package/usr/share/synchrorep/synchrorep.desktop
	dpkg-deb --build synchrorep-package

clean-package:
	rm -rf synchrorep-package/usr
	rm synchrorep-package.deb
