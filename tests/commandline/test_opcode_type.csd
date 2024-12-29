<CsoundSynthesizer>
<CsOptions>
-n 
</CsOptions>
<CsInstruments>
0dbfs=1

instr 1
   osc:OpcodeRef init "oscili"
   opcodeinfo osc
   oobj:OpcodeObj create osc
   opcodeinfo oobj

   a1 init oobj, p4, p5, p6
      perf oobj
      out a1
endin

</CsInstruments>
<CsScore>
i1 0 1 0.5 440 -1
</CsScore>
</CsoundSynthesizer>

