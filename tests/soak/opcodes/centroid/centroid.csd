<CsoundSynthesizer>
<CsInstruments>

sr = 44100
ksmps = 32
nchnls = 2
0dbfs  = 1

giSine ftgen 0, 0, 16384, 10, 1

instr 1
  ktrig init 1				;calculate centroid
  a1   oscil3 0.5, p4, giSine		;of the sine wave
  k1   centroid a1, ktrig, 16384
  asig oscil3 0.5, k1, giSine
  outs a1, asig			;left = original, right = centroid signal

endin
</CsInstruments>
<CsScore>

i1 0 2 20
i1 + 2 200
i1 + 2 2000
e

</CsScore>
</CsoundSynthesizer>
