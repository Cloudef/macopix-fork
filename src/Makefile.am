bin_PROGRAMS    = macopix
macopix_SOURCES   =\
	 main.c callbacks.c pixmap.c gui.c balloon.c clock.c configfile.c\
	 utils.c\
         mail.c pop.c md5c.c md5ify.c nokkari.c bmp.c bmpwrite.c sockmsg.c\
         codeconv.c unmime.c base64.c quoted-printable.c unlha.c untar.c\
	 trayicon.c alpha.c ssl.c sslmanager.c dnd.c\
	 intl.h main.h configfile.h libpop.h md5.h nokkari.h\
	 bmp.h sockmsg.h codeconv.h unmime.h base64.h quoted-printable.h\
         macopix_icon.h tar32api.h ssl.h

if HAVE_WINDRES
MACOPIX_RES = macopix.res
macopix.res: macopix_res.rc
	$(WINDRES) $< -O coff -o $@
else
MACOPIX_RES = 
endif

EXTRA_DIST = \
	macopix_res.rc.in\
        macopix.ico\
	def_biff.xpm\
	def_biff_r.xpm\
	tar32.dll\
	libtar32.a

INCLUDES        = @GTK_CFLAGS@  @GDK_PIXBUF_CFLAGS@ $(GNUTLS_CFLAGS)\
		  $(DISABLE_DEPRECATED)
LDADD           = @GTK_LIBS@  @GDK_PIXBUF_LIBS@ $(PNGLIB_LIBS) $(WINMM_LIBS)\
                   $(MACOPIX_RES)  $(WINSOCK_LIBS) $(UNLHA32_LIBS)  $(TAR32_LIBS) $(GNUTLS_LIBS) $(WIN32_ICONV_LIBS)
CLEANFILES      = *~
DISTCLEANFILES  = .deps/*.P
AM_CPPFLAGS = -DLOCALEDIR=\""$(localedir)"\"  -DCOMMON_DIR=\""$(commondir)"\"\
            -DPREFIX_DIR=\""$(prefixdir)"\" 

