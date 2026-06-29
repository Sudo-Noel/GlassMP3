import re

# Fix common.c
with open("gui/common.c", "r") as f:
    c = f.read()
c = c.replace("pspTime currTime;", "ScePspDateTime currTime;")
c = c.replace("currTime.minutes", "currTime.minute")
c = c.replace("int drawToolbars(){", "int drawToolbars(struct settings tSettings){")
c = c.replace("oslIntraFontSetStyle(f, size, color, shadowColor, options);", "oslIntraFontSetStyle(f, size, color, shadowColor, 0.0f, options);")
c = c.replace("stricmp", "strcasecmp").replace("strnicmp", "strncasecmp")
with open("gui/common.c", "w") as f:
    f.write(c)

# Fix menu.c
with open("gui/menu.c", "r") as f:
    c = f.read()
c = c.replace("stricmp", "strcasecmp").replace("strnicmp", "strncasecmp")

c = c.replace("menu->elements[i]->xPos = oslIntraFontPrintColumn(fontMenuNormal, menu->elements[i]->xPos, yPos, menu->width  - (xPos - menu->xPos) - 8, 0, menu->elements[i]->text);", 
              "menu->elements[i]->xPos = oslIntraFontPrintColumn(fontMenuNormal, menu->elements[i]->xPos, yPos, menu->width  - (xPos - menu->xPos) - 8, menu->elements[i]->text);")
c = c.replace("oslIntraFontPrintColumn(fontMenuNormal, xPos, yPos, menu->width - (xPos - menu->xPos), 0, menu->elements[i]->text);", 
              "oslIntraFontPrintColumn(fontMenuNormal, xPos, yPos, menu->width - (xPos - menu->xPos), menu->elements[i]->text);")

c = re.sub(r'(\s+)if \(!fontMenuNormal\)\s+if \( skinGetString\("STR_FONT_ALT_NAME", buffer\) == 0 \)', 
           r'\1if (!fontMenuNormal) {\n\1    if ( skinGetString("STR_FONT_ALT_NAME", buffer) == 0 )\n\1}', c)

c = re.sub(r'(\s+)if \(menu->dataFeedFunction != NULL\)\s+menu->dataFeedFunction\(menu->selected\);\s+menu->selected \+= menu->maxNumberVisible;',
           r'\1if (menu->dataFeedFunction != NULL) {\n\1    menu->dataFeedFunction(menu->selected);\n\1}\n\1menu->selected += menu->maxNumberVisible;', c)

with open("gui/menu.c", "w") as f:
    f.write(c)

