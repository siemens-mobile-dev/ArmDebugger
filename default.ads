[Main]
ID=15451
FlashAddress=-1610612736
FlashSize=67108864
NucleusLow=-1
NucleusHigh=-1
[BootCommands]
C0="C" a0000000 00040000
C1="O"
C2="A"
C3="E"
C4="I"
[Editor]
Source0=temp2.asm
Target0=-1
Source1=asm\cgsn_patch.asm
Target1=-1
Source2=asm\arm_internal.asm
Target2=-1
Source3=asm\arm_checkL2T.asm
Target3=-1
Source4=program1.asm
Target4=-1
Source5=D:\M65\x65flasher\asm\loader.asm
Target5=-1
Source6=D:\M65\x65flasher\asm\loader_s2.asm
Target6=-1
Source7=pvboot.asm
Target7=-1
Source8=asm\arm_readCP15.asm
Target8=-1
[Appearance]
RP1address=0
RP2address=0
CPaddress=0
[Names]
00089380=int_code
[Patches]
Count=0
[Snappoints]
[Sidebar]
Count=3
Type0=3
Text0=QW
Type1=4
Data1=1122FFFF
Type2=5
Address2=-1610612736
[Watches]
Count=0
[AddressHistory]
Count=0
0=R4
1=SP
2=A0145000
3=A0845000
4=A0835000
5=A0825000
6=A0821000
7=89000
8=8E000
9=A028AEE7
10=A1720100
11=A17C0000
12=A0000000
13=A8100000
14=A8000000
