/*
    Copyright (c) 2024-2025 Xuan Tan. All rights reserved.
    Licensed under the BSD-3-Clause.
*/

#pragma once


/*
    Required Libraries
*/

#ifdef _DEBUG

#pragma comment(lib, "ucrtd.lib")

#ifdef _MD
#pragma comment(lib, "vcruntime.lib")
#else
#pragma comment(lib, "libvcruntime.lib")
#endif

#else

#ifdef _MD
#pragma comment(lib, "ucrt.lib")
#pragma comment(lib, "vcruntime.lib")
#else
#pragma comment(lib, "libucrt.lib")
#pragma comment(lib, "libvcruntime.lib")
#endif

#endif

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "dbghelp.lib")


/*
    Exclude Multiple Technologies
*/


#define WIN32_LEAN_AND_MEAN

#define NOGDICAPMASKS
// #define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#define NOIME
#define NOFONTSIG

#define NOMMIDS
#define NONEWWAVE
#define NONEWRIFF
#define NOJPEGDIB
#define NONEWIC
#define NOBITMAP

// #define NOUSER
#define NOTOOLBAR
#define NOREBAR
#define NOUPDOWN
#define NOTOOLTIPS
#define NOSTATUSBAR
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOPROGRESS
#define NOHOTKEY
#define NOHEADER
#define NOIMAGEAPIS
#define NOLISTVIEW
#define NOTREEVIEW
#define NOUSEREXCONTROLS
#define NOTABCONTROL
#define NOANIMATE
#define NOMONTHCAL
#define NODATETIMEPICK
#define NOIPADDRESS
#define NOPAGESCROLLER
#define NONATIVEFONTCTL
#define NOBUTTON
#define NOSTATIC
#define NOEDIT
#define NOLISTBOX
#define NOCOMBOBOX
#define NOSCROLLBAR
#define NOTASKDIALOG
#define NOMUI
#define NOTRACKMOUSEEVENT
// #define NORESOURCE
#define NODESKTOP
#define NOWINDOWSTATION
#define NOSECURITY
#define NONCMESSAGES
#define NOMDI
#define NOSYSPARAMSINFO
#define NOWINABLE
#define NO_STATE_FLAGS

#define NODDRAWGDI
#define NODDEMLSPY
#define NO_COMMCTRL_DA
#define NO_DSHOW_STRSAFE
#define NODXINCLUDES
#define NODXMALLOC
#define NODXLCL
#define NODX95TYPES
#define NO_BASEINTERFACE_FUNCS
#define NOWINBASEINTERLOCK
#define NO_MEDIA_ENGINE_FACTORY
#define NO_INTSHCUT_GUIDS
#define NO_SHDOCVW_GUIDS
#define NO_WRAPPERS_FOR_ILCREATEFROMPATH
#define NO_SHOBJIDL_SORTDIRECTION
#define NO_WCN_PKEYS
#define NOEXTAPI
#define NO_WIA_DEBUG
// #define NOAPISET

#define NOSHLWAPI
// #define NO_SHLWAPI_STRFCNS
// #define NO_SHLWAPI_PATH
// #define NO_SHLWAPI_REG
// #define NO_SHLWAPI_STREAM
// #define NO_SHLWAPI_HTTP
// #define NO_SHLWAPI_ISOS
// #define NO_SHLWAPI_GDI

#define NOCOMPMAN
#define NODRAWDIB
#define NOVIDEO
#define NOAVIFMT
#define NOMMREG
#define NOAVIFILE
#define NOMCIWND
#define NOAVICAP
#define NOMSACM