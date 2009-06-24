mkdir break_dev
mkdir prod
mkdir break_prod

"../../../tools/rombreaker/rombreaker.exe" -i twl_0E8A.srl -b HEADER -o "break_dev/twl_0E8A.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0E9A.nand.srl -b HEADER -o "break_dev/twl_0E9A.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EAA.srl -b ARM9FLX -o "break_dev/twl_0EAA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EBA.nand.srl -b ARM9FLX -o "break_dev/twl_0EBA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0ECA.srl -b ARM7FLX -o "break_dev/twl_0ECA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EDA.nand.srl -b ARM7FLX -o "break_dev/twl_0EDA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EEA.srl -b ARM9LTD -o "break_dev/twl_0EEA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EFA.nand.srl -b ARM9LTD -o "break_dev/twl_0EFA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EGA.srl -b ARM7LTD -o "break_dev/twl_0EGA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EHA.nand.srl -b ARM7LTD -o "break_dev/twl_0EHA.nand.srl"

"../../../tools/rombreaker/rombreaker.exe" -i twl_0EIA.nand.srl -b HEADER -o "break_dev/twl_0EIA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EJA.nand.srl -b ARM9FLX -o "break_dev/twl_0EJA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EKA.nand.srl -b ARM7FLX -o "break_dev/twl_0EKA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0ELA.nand.srl -b ARM9LTD -o "break_dev/twl_0ELA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EMA.nand.srl -b ARM7LTD -o "break_dev/twl_0EMA.nand.srl"

"../../../tools/rombreaker/rombreaker.exe" -i twl_0ENA.srl -b HEADER -o "break_dev/twl_0ENA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EOA.nand.srl -b HEADER -o "break_dev/twl_0EOA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EPA.srl -b ARM9FLX -o "break_dev/twl_0EPA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EQA.nand.srl -b ARM9FLX -o "break_dev/twl_0EQA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0ERA.srl -b ARM7FLX -o "break_dev/twl_0ERA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0ESA.nand.srl -b ARM7FLX -o "break_dev/twl_0ESA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0ETA.srl -b ARM9LTD -o "break_dev/twl_0ETA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EUA.nand.srl -b ARM9LTD -o "break_dev/twl_0EUA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EVA.srl -b ARM7LTD -o "break_dev/twl_0EVA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i twl_0EWA.nand.srl -b ARM7LTD -o "break_dev/twl_0EWA.nand.srl"

"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0E8A.srl "prod/twl_0E8A.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0E9A.nand.srl "prod/twl_0E9A.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EAA.srl "prod/twl_0EAA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EBA.nand.srl "prod/twl_0EBA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ECA.srl "prod/twl_0ECA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EDA.nand.srl "prod/twl_0EDA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EEA.srl "prod/twl_0EEA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EFA.nand.srl "prod/twl_0EFA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EGA.srl "prod/twl_0EGA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EHA.nand.srl "prod/twl_0EHA.nand.srl"

"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EIA.nand.srl "prod/twl_0EIA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EJA.nand.srl "prod/twl_0EJA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EKA.nand.srl "prod/twl_0EKA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ELA.nand.srl "prod/twl_0ELA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EMA.nand.srl "prod/twl_0EMA.nand.srl"

"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ENA.srl "prod/twl_0ENA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EOA.nand.srl "prod/twl_0EOA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EPA.srl "prod/twl_0EPA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EQA.nand.srl "prod/twl_0EQA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ERA.srl "prod/twl_0ERA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ESA.nand.srl "prod/twl_0ESA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0ETA.srl "prod/twl_0ETA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EUA.nand.srl "prod/twl_0EUA.nand.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EVA.srl "prod/twl_0EVA.srl"
"%TWL_IPL_RED_PRIVATE_ROOT%/tools/bin/mastering.TWL.exe" -t twl_0EWA.nand.srl "prod/twl_0EWA.nand.srl"

"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0E8A.srl -b HEADER -o "break_prod/twl_0E8A.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0E9A.nand.srl -b HEADER -o "break_prod/twl_0E9A.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EAA.srl -b ARM9FLX -o "break_prod/twl_0EAA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EBA.nand.srl -b ARM9FLX -o "break_prod/twl_0EBA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ECA.srl -b ARM7FLX -o "break_prod/twl_0ECA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EDA.nand.srl -b ARM7FLX -o "break_prod/twl_0EDA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EEA.srl -b ARM9LTD -o "break_prod/twl_0EEA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EFA.nand.srl -b ARM9LTD -o "break_prod/twl_0EFA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EGA.srl -b ARM7LTD -o "break_prod/twl_0EGA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EHA.nand.srl -b ARM7LTD -o "break_prod/twl_0EHA.nand.srl"

"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EIA.nand.srl -b HEADER -o "break_prod/twl_0EIA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EJA.nand.srl -b ARM9FLX -o "break_prod/twl_0EJA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EKA.nand.srl -b ARM7FLX -o "break_prod/twl_0EKA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ELA.nand.srl -b ARM9LTD -o "break_prod/twl_0ELA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EMA.nand.srl -b ARM7LTD -o "break_prod/twl_0EMA.nand.srl"

"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ENA.srl -b HEADER -o "break_prod/twl_0ENA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EOA.nand.srl -b HEADER -o "break_prod/twl_0EOA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EPA.srl -b ARM9FLX -o "break_prod/twl_0EPA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EQA.nand.srl -b ARM9FLX -o "break_prod/twl_0EQA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ERA.srl -b ARM7FLX -o "break_prod/twl_0ERA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ESA.nand.srl -b ARM7FLX -o "break_prod/twl_0ESA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0ETA.srl -b ARM9LTD -o "break_prod/twl_0ETA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EUA.nand.srl -b ARM9LTD -o "break_prod/twl_0EUA.nand.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EVA.srl -b ARM7LTD -o "break_prod/twl_0EVA.srl"
"../../../tools/rombreaker/rombreaker.exe" -i prod/twl_0EWA.nand.srl -b ARM7LTD -o "break_prod/twl_0EWA.nand.srl"


mkdir "break_dev/tad"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0E9A.nand.srl -v 0 -o break_dev/tad/twl_0E9A.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EBA.nand.srl -v 0 -o break_dev/tad/twl_0EBA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EDA.nand.srl -v 0 -o break_dev/tad/twl_0EDA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EFA.nand.srl -v 0 -o break_dev/tad/twl_0EFA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EHA.nand.srl -v 0 -o break_dev/tad/twl_0EHA.tad

%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EIA.nand.srl -s -v 0 -o break_dev/tad/twl_0EIA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EJA.nand.srl -s -v 0 -o break_dev/tad/twl_0EJA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EKA.nand.srl -s -v 0 -o break_dev/tad/twl_0EKA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0ELA.nand.srl -s -v 0 -o break_dev/tad/twl_0ELA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EMA.nand.srl -s -v 0 -o break_dev/tad/twl_0EMA.tad

%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EOA.nand.srl -s -v 0 -o break_dev/tad/twl_0EOA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EQA.nand.srl -s -v 0 -o break_dev/tad/twl_0EQA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0ESA.nand.srl -s -v 0 -o break_dev/tad/twl_0ESA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EUA.nand.srl -s -v 0 -o break_dev/tad/twl_0EUA.tad
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe break_dev/twl_0EWA.nand.srl -s -v 0 -o break_dev/tad/twl_0EWA.tad

mkdir "break_prod/0E9A/v0"
cd "break_prod/0E9A/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0E9A.nand.srl -v 0 -p -o twl_0E9A.tad
mv properties ../
cd ../../

mkdir "0EBA/v0"
cd "0EBA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EBA.nand.srl -v 0 -p -o twl_0EBA.tad
mv properties ../
cd ../../

mkdir "0EDA/v0"
cd "0EDA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EDA.nand.srl -v 0 -p -o twl_0EDA.tad
mv properties ../
cd ../../

mkdir "0EFA/v0"
cd "0EFA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EFA.nand.srl -v 0 -p -o twl_0EFA.tad
mv properties ../
cd ../../

mkdir "0EHA/v0"
cd "0EHA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EHA.nand.srl -v 0 -p -o twl_0EHA.tad
mv properties ../
cd ../../


mkdir "0EIA/v0"
cd "0EIA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EIA.nand.srl -s -v 0 -p -o twl_0EIA.tad
mv properties ../
cd ../../

mkdir "0EJA/v0"
cd "0EJA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EJA.nand.srl -s -v 0 -p -o twl_0EJA.tad
mv properties ../
cd ../../

mkdir "0EKA/v0"
cd "0EKA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EKA.nand.srl -s -v 0 -p -o twl_0EKA.tad
mv properties ../
cd ../../

mkdir "0ELA/v0"
cd "0ELA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0ELA.nand.srl -s -v 0 -p -o twl_0ELA.tad
mv properties ../
cd ../../

mkdir "0EMA/v0"
cd "0EMA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EMA.nand.srl -s -v 0 -p -o twl_0EMA.tad
mv properties ../
cd ../../


mkdir "0EOA/v0"
cd "0EOA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EOA.nand.srl -s -v 0 -p -o twl_0EOA.tad
mv properties ../
cd ../../

mkdir "0EQA/v0"
cd "0EQA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EQA.nand.srl -s -v 0 -p -o twl_0EQA.tad
mv properties ../
cd ../../

mkdir "0ESA/v0"
cd "0ESA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0ESA.nand.srl -s -v 0 -p -o twl_0ESA.tad
mv properties ../
cd ../../

mkdir "0EUA/v0"
cd "0EUA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EUA.nand.srl -s -v 0 -p -o twl_0EUA.tad
mv properties ../
cd ../../

mkdir "0EWA/v0"
cd "0EWA/v0"
%TWLSDK_ROOT%/build/tools/maketad/bin/maketad.exe ../../twl_0EWA.nand.srl -s -v 0 -p -o twl_0EWA.tad
mv properties ../
cd ../../
