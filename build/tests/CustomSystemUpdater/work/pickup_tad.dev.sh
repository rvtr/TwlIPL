#!/bin/sh
#! bash -f

#---- commands
LS='/bin/ls'
ECHO='/bin/echo'
HEAD='/bin/head'
FIND='/bin/find'
COPY='/bin/cp'
DATE='/bin/date'
XARGS='/bin/xargs'
MKDIR='/bin/mkdir'
CHDIR='cd'

#---- values
aqua_server='//10.116.1.5'
debug_dir="${aqua_server}/TWL_debug/sysmenu/rom/debug_rom"
redroms="${debug_dir}/sd.roms.template*"
clsbase="${debug_dir}/CLS_base"
datedir=`${DATE} +%Y%m%d`
regions='jp usa euro aus'

#---- call pickup_tad.pl
${ECHO} "===== getting latest apps files ====="
perl ./pickup_tad.pl

#---- gets the latest sd.roms.template roms
#redrom_dir=`${LS} -d --full-time ${redroms} | sort -k 7,10 | tail -n 1`
redrom_dir=`${LS} -dt ${redroms} | ${HEAD} -n 1`
${ECHO} -e "===== getting sd.roms.template* rom files ====="
${ECHO} -e "sd.roms.template dir : ${redrom_dir}"
${ECHO} -e "copy *.(tad|dat|nand) file -> ${datedir} dir\n"
${FIND} ${redrom_dir} -regextype posix-egrep -regex ".*.(tad|dat|nand)" | ${XARGS} ${COPY} -fut ${datedir} > /dev/null

#---- gets the cls base roms
${ECHO} -e "===== getting sd.roms.template* rom files ====="
${ECHO} -e "CLS_base dir : ${clsbase}"
${ECHO} -e "copy *.tad file -> ${datedir} dir"
${FIND} ${clsbase} -name "*.tad" | ${XARGS} ${COPY} -fut ${datedir} > /dev/null

#---- call mkcls.py
${ECHO} -e "===== create forcls dirs ====="
${MKDIR} ${datedir}_forcls
${CHDIR} ${datedir}_forcls
${FIND} ../${datedir} -name "*.tad" -exec ../mkcls.py {} \;

${ECHO} -e "===== copy *.nand *.dat files -> (jp|usa|euro|aus) dirs =====\n"
for region in ${regions}
do
	${MKDIR} ${region}
	${COPY} ../${datedir}/*.nand ${region}
	${COPY} ../${datedir}/*.dat ${region}
done

${COPY} ../cls.sh ./