/*todo: replace windows functions, multiline edits, add missing edit functions (ex: double click to select word)*/

struct edit {
    PANEL panel;

    _Bool multiline, mouseover;
    uint16_t mouseover_char, length, maxlength;
    char_t *data;
    void (*onenter)(void);
    void (*ontab)(void);
};

void edit_draw(EDIT *edit, int x, int y, int width, int height);

_Bool edit_mmove(EDIT *edit, int x, int y, int dy, int width, int height);
_Bool edit_mdown(EDIT *edit);
_Bool edit_dclick(EDIT *edit, _Bool triclick);
_Bool edit_mright(EDIT *edit);
_Bool edit_mwheel(EDIT *edit, int height, double d);
_Bool edit_mup(EDIT *edit);
_Bool edit_mleave(EDIT *edit);

void edit_char(uint32_t ch, _Bool control);

int edit_copy(char_t *data, int len);
void edit_paste(char_t *data, int len);
void edit_delete(void);
void edit_selectall(void);
void edit_clear(void);

_Bool edit_active(void);

void edit_resetfocus(void);
void edit_setstr(EDIT *edit, char_t *str, uint16_t length);
