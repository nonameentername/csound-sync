<CsoundSynthesizer>
<CsOptions>
-n
</CsOptions>
<CsInstruments>

0dbfs = 1
if floatsize() > 4 then
i1 ftgen 1, 0, 2^30-1, 2, 0, 2^30-1, 0
else
i1 ftgen 1, 0, 2^30-1, 2, 0, 2^24-1, 0
endif

instr 1
if floatsize() > 4 then
if ftlen(1) != 1073741823 then
exitnow(-1)
endif
else
if ftlen(1) != 16777215 then
exitnow(-1)
endif
endif
endin

</CsInstruments>
<CsScore>
i1 0 0
</CsScore>
</CsoundSynthesizer>
