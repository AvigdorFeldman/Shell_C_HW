# Compiles each of the shells
cc Standard_shell.c -o StandShell
cc Advanced_shell.c -o Adv
cc Security_shell.c -o Security
cc exit.c -o exit
# Compiles functions of the Advanced shell
cc merge.c -o Merge
cc unmerge.c -o UnMerge
cc size.c -o Size
cc findmax.c -o FindMax
cc delete.c -o Delete
# This program is useble to both the Advanced and Security shells
cc history.c -o History
# Compiles functions of the Security shell
cc enc.c -o Enc
cc dec.c -o Dec
cc AdvEnc.c -o AdvEnc
cc AdvDec.c -o AdvDec
# Starts the Standard shell
./StandShell
