import re

with open("gui/gui_player.c", "r") as f:
    c = f.read()

c = c.replace("scrollTitleX = oslIntraFontPrintColumn(fontNormal, scrollTitleX, tempPos[1], titleWidth, 0, info->title);",
              "scrollTitleX = oslIntraFontPrintColumn(fontNormal, scrollTitleX, tempPos[1], titleWidth, info->title);")

c = re.sub(r'(\s+)else if\(status == STATUS_CONFIRM_BOOKMARK\)\s+drawMessage\(userSettings, "STR_CONFIRM_BOOKMARK"\);\s+oslEndDrawing\(\);',
           r'\1else if(status == STATUS_CONFIRM_BOOKMARK) {\n\1    drawMessage(userSettings, "STR_CONFIRM_BOOKMARK");\n\1}\n\1oslEndDrawing();', c)

c = re.sub(r'(\s+)if \(\+\+userSettings->sleepMode > SLEEP_120\)\s+userSettings->sleepMode = 0;\s+if \(userSettings->sleepMode >= SLEEP_30\)',
           r'\1if (++userSettings->sleepMode > SLEEP_120) {\n\1    userSettings->sleepMode = 0;\n\1}\n\1if (userSettings->sleepMode >= SLEEP_30)', c)

with open("gui/gui_player.c", "w") as f:
    f.write(c)

