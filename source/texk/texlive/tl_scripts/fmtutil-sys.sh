#!/bin/sh
# $Id: fmtutil-sys.sh 35626 2014-11-20 19:08:47Z karl $
# fmtutil-sys - arrange for fmtutil to affect system directories. 
# Public domain.  Originally written by Thomas Esser.

test -f /bin/ksh && test -z "$RUNNING_KSH" \
  && { UNAMES=`uname -s`; test "x$UNAMES" = xULTRIX; } 2>/dev/null \
  && { RUNNING_KSH=true; export RUNNING_KSH; exec /bin/ksh $0 ${1+"$@"}; }
unset RUNNING_KSH

test -f /bin/bsh && test -z "$RUNNING_BSH" \
  && { UNAMES=`uname -s`; test "x$UNAMES" = xAIX; } 2>/dev/null \
  && { RUNNING_BSH=true; export RUNNING_BSH; exec /bin/bsh $0 ${1+"$@"}; }
unset RUNNING_BSH

# hack around a bug in zsh:
test -n "${ZSH_VERSION+set}" && alias -g '${1+"$@"}'='"$@"'

# preferentially use subprograms from our own directory.
mydir=`echo "$0" | sed 's,/[^/]*$,,'`
mydir=`cd "$mydir" && pwd`
PATH="$mydir:$PATH"; export PATH

v=`kpsewhich -var-value TEXMFSYSVAR`
c=`kpsewhich -var-value TEXMFSYSCONFIG`

TEXMFVAR="$v"
TEXMFCONFIG="$c"
export TEXMFVAR TEXMFCONFIG

exec fmtutil ${1+"$@"}
