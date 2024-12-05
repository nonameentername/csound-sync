<CsoundSynthesizer>
<CsInstruments>
sr=44100
ksmps=1
nchnls=1


instr 1
  ix = 0
  bi = ix == 10
  until bi do
   ix += 1
  od
endin


</CsInstruments>
<CsScore>
i1 0 0
e
</CsScore>
</CsoundSynthesizer>