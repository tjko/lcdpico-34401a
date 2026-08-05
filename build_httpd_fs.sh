#!/bin/sh
#
# build_httpd_fs.sh
#

SERVER="lwIP/2.2.1 (LCDpico)"
EXCLUDE="html~,shtml~,json~,csv~,~"

SSIFILENAME=src/httpd-fs_ssi.list
FSDIR=src/httpd-fs/
FSDATAFILE=src/lcdpico_fsdata.c

fatal() { echo "`basename $0`: $*"; exit 1; }

[ -d "$FSDIR" ] || fatal "cannot find fs directory: $FSDIR"


#./contrib/makefsdata ${FSDIR} -m -svr:"${SERVER}" -ssi:${SSIFILENAME} -f:${FSDATAFILE}.old -x:"${EXCLUDE}"
#[ $? -eq 0 ] || fatal "makefsdata failed"
#dos2unix ${FSDATAFILE}
#[ $? -eq 0 ] || fatal "dos2unix failed"

./contrib/makefsdata.py ${FSDIR} -m -svr "${SERVER}" -ssi "${SSIFILENAME}" -f ${FSDATAFILE} -x "${EXCLUDE}" -v

