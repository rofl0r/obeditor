obeddir="$(top_srcdir)/release"
obeddatadir="$(obeddir)/ressources"

obeddata_DATA = \
	manual.txt remaps.txt manual_stage.txt \
	delete-small.png \
	copy-small.png \
	delete.png \
	zoom-in.png zoom-out.png zoom-x1.png \
	loop.png \
	noloop.png \
	aktion.png \
	error_small.png \
	warning_small.png \
	info_small.png \
	stop.png \
	noimage.jpg \
	cursor_pipette_plus.png cursor_pipette_simple.png \
	cursor_pipette_plus-win.png cursor_pipette_simple-win.png \
	cursor_move.png cursor_move-win.png \
	sound-on.png sound-off.png help.png \
	ArrowDown.png ArrowLeft.png ArrowRight.png ArrowUp.png \
	ent_shadow.png \
	pSpawn_1.png pSpawn_2.png pSpawn_3.png pSpawn_4.png


all:
	@for dep in $(obeddata_DATA); do \
		echo "updating ressource = $$dep"; \
		cp -fu $(top_srcdir)/ressources/$$dep .; \
	done
	echo "top_srcdir = $(top_srcdir)"

