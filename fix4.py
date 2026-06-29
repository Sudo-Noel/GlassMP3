import re

with open("gui/settings.h", "r") as f:
    c = f.read()
c = c.replace("int SETTINGS_save(struct settings tSettings);", "int SETTINGS_save(struct settings *tSettings);")
with open("gui/settings.h", "w") as f:
    f.write(c)

with open("gui/settings.c", "r") as f:
    c = f.read()
c = c.replace("int SETTINGS_save(struct settings tSettings){", "int SETTINGS_save(struct settings *tSettings){")
c = c.replace("tSettings.", "tSettings->")
with open("gui/settings.c", "w") as f:
    f.write(c)

with open("gui/gui_playlistEditor.c", "r") as f:
    c = f.read()
c = re.sub(r'(\s+)if \(!trackInfoBkg\)\s+errorLoadImage\(buffer\);\s+M3U_clear\(\);', 
           r'\1if (!trackInfoBkg)\n\1    errorLoadImage(buffer);\n\1M3U_clear();', c)
with open("gui/gui_playlistEditor.c", "w") as f:
    f.write(c)

with open("gui/gui_settings.c", "r") as f:
    c = f.read()

# Fix indentations for CLOCK settings
for var in ['CLOCK_GUI', 'CLOCK_MP3', 'CLOCK_MP3ME', 'CLOCK_OGG', 'CLOCK_FLAC', 'CLOCK_AA3', 'CLOCK_WMA']:
    old_str = f"if (userSettings->{var} + delta <= 222 && userSettings->{var} + delta >= getMinCPUClock())\n"
    c = re.sub(
        rf'(\s+)if \(userSettings->{var} \+ delta <= 222 && userSettings->{var} \+ delta >= getMinCPUClock()\)\s+userSettings->{var} \+= delta;\s+',
        rf'\1if (userSettings->{var} + delta <= 222 && userSettings->{var} + delta >= getMinCPUClock()) {{\n\1    userSettings->{var} += delta;\n\1}}\n\1', c)

with open("gui/gui_settings.c", "w") as f:
    f.write(c)

