<CsoundSynthesizer>
<CsInstruments>

sr = 44100
ksmps = 32
nchnls = 2
0dbfs  = 1

instr 1

  k1   duserrnd 1
  asig poscil .5, 220*k1, 2	;multiply frequency with random value
  outs asig, asig

endin
</CsInstruments>
<CsScore>
f1 0 -20 -41  2 .1 8 .9	;choose 2 at 10% probability, and 8 at 90

f2 0 8192 10 1

i1 0 2
e
</CsScore>
</CsoundSynthesizer>
