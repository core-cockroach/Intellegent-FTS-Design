savedcmd_startrtos.mod := printf '%s\n'   startrtos.o | awk '!x[$$0]++ { print("./"$$0) }' > startrtos.mod
