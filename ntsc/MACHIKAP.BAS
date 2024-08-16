REM GP0-GP7: 8ビットI/O
REM GP10: MOSI, DC
REM GP11: MOSI, /WR
REM GP13: MOSI, /RD
REM GP12: MISO, /BUSY (GP27 for test by MachiKania)

cls

print "Will start in 5 seconds"
wait 60*5

exec 0xb672 :rem cpsid i

REM Initialize
gosub CMODE
gosub WRH
gosub RDH
in8L()

gosub SCOM,0x96 :REM cls

gosub SCOM,0x93 :REM printstr
rem t$="Hello MachiKania NTSC world!             "
t$="Hello MachiKania NTSC world!\n"
i=0
a=0
do
  c=peek(t+i)
  i=i+1
  gosub SDAT,c
  if 0=c then
    i=0
    a=a+1
    t$=dec$(a)+" Hello MachiKania NTSC world!\n"
  endif
  rem for j=1 to 5000:next
loop

label theend
exec 0xb662 :rem cpsie i
poke32 0x4001c004 + 4*25,0
poke32 0x40014004 + 8*25,5
poke32 0xd0000024,1<<25
do
  poke32 0xd0000014,1<<25
  wait 30
  poke32 0xd0000018,1<<25
  wait 30
loop

label CMODE
  poke32 0xd0000014,1<<10 :REM Command mode
return

label DMODE
  poke32 0xd0000018,1<<10 :REM Data mode
return

label WRH
  poke32 0xd0000014,1<<11 :REM /WR=H
return

label WRL
  poke32 0xd0000018,1<<11 :REM /WR=L
return

label RDH
  poke32 0xd0000014,1<<13 :REM /RD=H
return

label RDL
  poke32 0xd0000018,1<<13 :REM /RD=L
return

REM Send command
label SCOM
  do:loop until in(14)
  gosub CMODE
  out8l args(1)
  gosub WRL
  do:loop while in(14)
  gosub WRH
  in8L()
return

REM Send command
label SDAT
  do:loop until in(14)
  gosub DMODE
  out8l args(1)
  gosub WRL
  do:loop while in(14)
  gosub WRH
  in8L()
return

