<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>

0dbfs = 1
if floatsize() > 4 then
i1 ftgen 1, 0, 2^30, 2, 0, 2^30, 0
else
i1 ftgen 1, 0, 2^24, 2, 0, 2^24, 0
endif

instr 1
endin

</CsInstruments>
<CsScore>
i1 0 0
</CsScore>
</CsoundSynthesizer>
