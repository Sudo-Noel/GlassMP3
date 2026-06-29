import re

# Fix common.c
with open("gui/common.c", "r") as f:
    c = f.read()
c = c.replace("int drawToolbars(struct settings tSettings){", "int drawToolbars(struct settings *tSettings){")
c = c.replace("tSettings.dateTimeFormat", "tSettings->dateTimeFormat")

# Fix indentation warnings
c = re.sub(r'(\s+)if \(!fontNormal\)\s+if \( fontNormal->fontType', r'\1if (!fontNormal) {\n\1    if ( fontNormal->fontType', c)
c = c.replace('            fontNormal = oslLoadFontFile(buffer);', '            fontNormal = oslLoadFontFile(buffer);\n    }')

with open("gui/common.c", "w") as f:
    f.write(c)

# Fix menu.c indentation warnings
with open("gui/menu.c", "r") as f:
    c = f.read()

c = re.sub(r'(\t+)if \(!fontMenuNormal\)\s+if \( skinGetString', r'\1if (!fontMenuNormal) {\n\1    if ( skinGetString', c)
c = c.replace('            fontMenuNormal = oslLoadFontFile(buffer);', '            fontMenuNormal = oslLoadFontFile(buffer);\n    }')

c = re.sub(r'(\s+)if \(menu->dataFeedFunction != NULL\)\s+menu->dataFeedFunction\(menu->selected\);\s+menu->selected', 
           r'\1if (menu->dataFeedFunction != NULL) {\n\1    menu->dataFeedFunction(menu->selected);\n\1}\n\1menu->selected', c)

with open("gui/menu.c", "w") as f:
    f.write(c)

