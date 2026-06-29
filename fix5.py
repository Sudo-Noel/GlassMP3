import re

# Fix player.h
with open("players/player.h", "r") as f:
    c = f.read()
c = c.replace("short AT3_OutputBuffer", "extern short AT3_OutputBuffer")
c = c.replace("short *AT3_OutputPtr;", "extern short *AT3_OutputPtr;")
with open("players/player.h", "w") as f:
    f.write(c)

# Fix player.c
with open("players/player.c", "r") as f:
    c = f.read()
if "short AT3_OutputBuffer" not in c:
    c = c.replace('#include "player.h"', '#include "player.h"\n\nshort AT3_OutputBuffer[2 * 2048 * 2];\nshort *AT3_OutputPtr;')
with open("players/player.c", "w") as f:
    f.write(c)

# Fix skinsettings.h
with open("gui/skinsettings.h", "r") as f:
    c = f.read()
c = c.replace("struct skinsList skinsList;", "extern struct skinsList skinsList;")
with open("gui/skinsettings.h", "w") as f:
    f.write(c)

# Fix skinsettings.c
with open("gui/skinsettings.c", "r") as f:
    c = f.read()
if "struct skinsList skinsList;" not in c:
    c = c.replace('#include "skinsettings.h"', '#include "skinsettings.h"\n\nstruct skinsList skinsList;')
with open("gui/skinsettings.c", "w") as f:
    f.write(c)

# Fix languages.h
with open("gui/languages.h", "r") as f:
    c = f.read()
c = c.replace("struct languagesList languagesList;", "extern struct languagesList languagesList;")
with open("gui/languages.h", "w") as f:
    f.write(c)

# Fix languages.c
with open("gui/languages.c", "r") as f:
    c = f.read()
if "struct languagesList languagesList;" not in c:
    c = c.replace('#include "languages.h"', '#include "languages.h"\n\nstruct languagesList languagesList;')
with open("gui/languages.c", "w") as f:
    f.write(c)

# Add utimensat stub to main.c
with open("main.c", "r") as f:
    c = f.read()
if "utimensat" not in c:
    stub = """
#include <sys/stat.h>
int utimensat(int dirfd, const char *pathname, const struct timespec times[2], int flags) {
    return 0;
}
"""
    c = c + stub
with open("main.c", "w") as f:
    f.write(c)

