#include "main.h"

static int heightmsg(char_t *str, int right, uint16_t length)
{
    int y = 0, x = 0;
    char_t *a = str, *b = str, *end = str + length;
    while(1) {
        if(a == end || *a == '\n' || *a == ' ') {
            int count = a - b, w = textwidth(b, a - b);
            if(x + w > right) {
                if(x != 0 || *a == '\n') {
                    y += font_msg_lineheight;
                    int l = utf8_len(b);
                    count -= l;
                    b += l;
                }
                x = 0;
                w = textwidth(b, count);
            }

            x += w;
            b = a;

            if(a == end) {
                break;
            }

            if(*a == '\n') {
                b += utf8_len(b);
                y += font_msg_lineheight;
                x = 0;
            }
        }
        a += utf8_len(a);
    }

    y += font_msg_lineheight;

    return y;
}

void messages_draw(MESSAGES *m, int x, int y, int width, int height)
{
    setcolor(0);
    setfont(FONT_MSG);

    uint8_t lastauthor = 0xFF;

    void **p = m->data->data;
    int i, n = m->data->n;

    for(i = 0; i != n; i++) {
        MESSAGE *msg = *p++;

        if(y + msg->height <= 0) { //! NOTE: should not be constant 0
            y += msg->height;
            continue;
        }

        if(y >= height + 100) { //! NOTE: should not be constant 100
            break;
        }

        setcolor(LIST_MAIN);
        setfont(FONT_MISC);
        char timestr[6];
        int len;
        len = sprintf(timestr, "%u:%.2u", msg->time / 60, msg->time % 60);
        drawtext(x + width - TIME_WIDTH, y, (uint8_t*)timestr, len);

        if(m->type) {
            /* group */
            setcolor(0);
            setfont(FONT_MSG_NAME);
            drawtextwidth_right(x, MESSAGES_X - NAME_OFFSET, y, &msg->msg[msg->length] + 1, (uint16_t)msg->msg[msg->length]);
        } else {
            FRIEND *f = &friend[m->data->id];
            uint8_t author = msg->flags & 1;
            if(author != lastauthor) {
                setfont(FONT_MSG_NAME);
                if(!author) {
                    setcolor(0);
                    drawtextwidth_right(x, MESSAGES_X - NAME_OFFSET, y, f->name, f->name_length);
                } else {
                    setcolor(CHAT_SELF);
                    drawtextwidth_right(x, MESSAGES_X - NAME_OFFSET, y, self.name, self.name_length);
                }
                lastauthor = author;
            } else {
                if(!author) {
                    setcolor(0);
                } else {
                    setcolor(CHAT_SELF);
                }
            }
        }
        /**/

        switch(msg->flags) {
        case 0:
        case 1:
        case 2:
        case 3: {
            /* normal message */
            int h1 = 0xFFFF, h2 = 0xFFFF;
            if(i == m->data->istart) {
                h1 = m->data->start;
                h2 = ((i == m->data->iend) ? m->data->end : msg->length);
            } else if(i == m->data->iend) {
                h1 = 0;
                h2 = m->data->end;
            } else if(i > m->data->istart && i < m->data->iend) {
                h1 = 0;
                h2 = msg->length;
            }

            if(m->data->istart == m->data->iend && m->data->start == m->data->end) {
                h1 = 0xFFFF;
                h2 = 0xFFFF;
            }

            setfont(FONT_MSG);
            int ny = drawtextmultiline(x + MESSAGES_X, x + width - TIME_WIDTH, y, font_msg_lineheight, msg->msg, msg->length, h1, h2 - h1, 1);
            if(ny - y != msg->height - MESSAGES_SPACING) {
                debug("error101\n");
            }
            y = ny;

            break;
        }

        case 4:
        case 5: {
            /* image */
            MSG_IMG *img = (void*)msg;
            //SIZE size;

            //SelectObject(hdcMem, img->bitmap);
            //GetBitmapDimensionEx(img->bitmap, &size);
            //BitBlt(hdc, x, y, size.cx, size.cy, hdcMem, 0, 0, SRCCOPY);
            y += img->height;
            break;
        }

        case 6:
        case 7: {
            MSG_FILE *file = (void*)msg;
            int dx = MESSAGES_X;
            int xx = x + dx;
            _Bool mo = (m->iover == i);

            uint8_t size[16];
            int sizelen = sprint_bytes(size, file->size);

            setcolor(WHITE);

            if(file->status == FILE_DONE) {
                drawalpha(BM_FT, xx, y, BM_FT_WIDTH, BM_FT_HEIGHT, C_GREEN);
            } else if(file->status == FILE_KILLED) {
                drawalpha(BM_FT, xx, y, BM_FT_WIDTH, BM_FT_HEIGHT, C_RED);
            } else {
                if(file->status == FILE_BROKEN) {
                    drawalpha(BM_FTM, xx, y, BM_FTM_WIDTH, BM_FT_HEIGHT, C_YELLOW);
                } else {
                    drawalpha(BM_FTM, xx, y, BM_FTM_WIDTH, BM_FT_HEIGHT, C_GRAY);
                    setcolor(GRAY(98));
                }

                int x =  xx + BM_FTM_WIDTH + SCALE;
                drawalpha(BM_FTB1, x, y, BM_FTB_WIDTH, BM_FTB_HEIGHT + SCALE, (mo && m->over == 1) ? C_GREEN_LIGHT : C_GREEN);
                drawalpha(BM_NO, x + (BM_FTB_WIDTH - BM_FB_WIDTH) / 2, y + SCALE * 4, BM_FB_WIDTH, BM_FB_HEIGHT, WHITE);

                uint32_t color = (msg->flags == 7 && (file->status == FILE_PENDING || file->status == FILE_BROKEN)) ? C_GRAY: ((mo && m->over == 2) ? C_GREEN_LIGHT : C_GREEN);
                drawalpha(BM_FTB2, x, y + BM_FTB_HEIGHT + SCALE * 2, BM_FTB_WIDTH, BM_FTB_HEIGHT, color);
                drawalpha((msg->flags == 6 && file->status ==  FILE_PENDING) ? BM_YES : BM_PAUSE, x + (BM_FTB_WIDTH - BM_FB_WIDTH) / 2, y + BM_FTB_HEIGHT + SCALE * 5, BM_FB_WIDTH, BM_FB_HEIGHT, color == C_GRAY ? LIST_MAIN : WHITE);
            }

            setfont(FONT_MISC);


            drawtext(xx + 35 * SCALE, y + 3 * SCALE, file->name, file->name_length);
            drawtext(xx + 35 * SCALE, y + 10 * SCALE, size, sizelen);

            y += BM_FT_HEIGHT;

            break;
        }
        }

        y += MESSAGES_SPACING;
    }
}

_Bool messages_mmove(MESSAGES *m, int mx, int my, int dy, int width, int height)
{
    if(my < 0) {
        my = 0;
    }

    if(mx < 0) {
        if(m->iover != ~0) {
            m->iover = ~0;
            return 1;
        }
        return 0;
    }

    setfont(FONT_MSG);

    void **p = m->data->data;
    int i = 0, n = m->data->n;
    _Bool redraw = 0;

    while(i != n) {
        MESSAGE *msg = *p++;

        int dy = msg->height;

        if((my >= 0 && my < dy) || i == n - 1) {
            switch(msg->flags) {
            case 0:
            case 1:
            case 2:
            case 3: {
                /* normal message */
                m->over = hittextmultiline(mx - MESSAGES_X, width - MESSAGES_X - TIME_WIDTH, my, msg->height, font_msg_lineheight, msg->msg, msg->length, 1);
                m->urlover = 0xFFFF;

                if(my >= dy || mx < MESSAGES_X || m->over == msg->length) {
                    break;
                }

                overtext = 1;

                char_t *str = msg->msg + m->over;
                while(str != msg->msg) {
                    str--;
                    if(*str == ' ' || *str == '\n') {
                        str++;
                        break;
                    }
                }

                char_t *end = msg->msg + msg->length;
                while(str != end && *str != ' ' && *str != '\n') {
                    if(end - str >= 7 && strcmp2(str, "http://") == 0) {
                        hand = 1;
                        m->urlover = str - msg->msg;
                    }

                    if(end - str >= 8 && strcmp2(str, "https://") == 0) {
                        hand = 1;
                        m->urlover = str - msg->msg;
                    }

                    str++;
                }

                if(m->urlover != 0xFFFF) {
                    m->urllen = (str - msg->msg) - m->urlover;
                }

                break;
            }

            case 4:
            case 5: {
                m->over = 0;
                break;
            }

            case 6:
            case 7: {
                if(my >= BM_FT_HEIGHT) {
                    break;
                }

                uint8_t over = 0;

                mx -= MESSAGES_X;
                if(mx >= 0 && mx < BM_FT_WIDTH && my >= 0 && my < BM_FT_HEIGHT) {
                    over = 3;
                    mx -= BM_FTM_WIDTH + SCALE;
                    if(mx >= 0) {
                        if(my < BM_FTB_HEIGHT + SCALE) {
                            over = 1;
                        } else if(my >= BM_FTB_HEIGHT + SCALE * 2) {
                            over = 2;
                        }
                    }
                }

                if(over != m->over || i != m->iover) {
                    redraw = 1;
                    m->over = over;
                }

                break;
            }
            }

            m->iover = i;

            if(m->select) {
                int start, end, istart, iend;
                if(i > m->idown) {
                    istart = m->idown;
                    iend = i;

                    start = m->down;
                    end = m->over;
                } else if(i < m->idown) {
                    iend = m->idown;
                    istart = i;

                    end = m->down;
                    start = m->over;
                } else {
                    istart = iend = i;
                    if(m->over >= m->down) {
                        start = m->down;
                        end = m->over;
                    } else {
                        end = m->down;
                        start = m->over;
                    }
                }

                if(start != m->data->start || istart != m->data->istart || end != m->data->end || iend != m->data->iend) {
                    m->data->start = start;
                    m->data->end = end;
                    m->data->istart = istart;
                    m->data->iend = iend;

                    setselection();
                    redraw = 1;
                }

            }
            return redraw;
        }

        my -= dy;

        i++;
    }

    return 0;
}

_Bool messages_mdown(MESSAGES *m)
{
    if(m->iover != ~0) {
        MESSAGE *msg = m->data->data[m->iover];
        switch(msg->flags) {
        case 0 ... 3: {
            if(m->urlover != 0xFFFF) {
                char_t url[m->urllen + 1];
                memcpy(url, msg->msg + m->urlover, m->urllen * sizeof(char_t));
                url[m->urllen] = 0;

                openurl(url);
            }

            m->data->istart = m->data->iend = m->idown = m->iover;
            m->data->start = m->data->end = m->down = m->over;
            m->select = 1;
            break;
        }

        case 6 ... 7: {
            MSG_FILE *file = (void*)msg;
            if(m->over == 0) {
                break;
            }

            switch(file->status) {
            case FILE_PENDING: {
                if(msg->flags == 6) {
                    if(m->over == 2) {
                        savefilerecv(m->data->id, file);
                    } else if(m->over == 1) {
                        //decline
                        tox_postmessage(TOX_FILE_IN_CANCEL, m->data->id, file->filenumber, NULL);
                    }
                } else if(m->over == 1) {
                    //cancel
                    tox_postmessage(TOX_FILE_OUT_CANCEL, m->data->id, file->filenumber, NULL);
                }


                break;
            }

            case FILE_OK: {
                if(m->over == 2) {
                    //pause
                    tox_postmessage(TOX_FILE_IN_PAUSE + (msg->flags & 1), m->data->id, file->filenumber, NULL);
                } else if(m->over == 1) {
                    //cancel
                    tox_postmessage(TOX_FILE_IN_CANCEL + (msg->flags & 1), m->data->id, file->filenumber, NULL);
                }
                break;
            }

            case FILE_PAUSED: {
                if(m->over == 2) {
                    //resume
                    tox_postmessage(TOX_FILE_IN_RESUME + (msg->flags & 1), m->data->id, file->filenumber, NULL);
                } else if(m->over == 1) {
                    //cancel
                    tox_postmessage(TOX_FILE_IN_CANCEL + (msg->flags & 1), m->data->id, file->filenumber, NULL);
                }
                break;
            }

            case FILE_BROKEN: {
                //cancel
                if(m->over == 1) {
                    tox_postmessage(TOX_FILE_IN_CANCEL + (msg->flags & 1), m->data->id, file->filenumber, NULL);
                }
                break;
            }

            case FILE_DONE: {
                if(msg->flags == 6 && m->over == 3) {
                }
                break;
            }
            }
            break;
        }
        }

        return 1;
    }

    return 0;
}

_Bool messages_dclick(MESSAGES *m, _Bool triclick)
{
    if(m->iover != ~0) {
        MESSAGE *msg = m->data->data[m->iover];
        if(msg->flags >= 0 && msg->flags <= 3) {
            m->data->istart = m->data->iend = m->iover;

            uint8_t c = triclick ? '\n' : ' ';

            uint16_t i = m->over;
            while(i != 0 && msg->msg[i - 1] != c) {
                i -= utf8_unlen(msg->msg + i);
            }
            m->data->start = i;
            i = m->over;
            while(i != msg->length && msg->msg[i] != c) {
                i += utf8_len(msg->msg + i);
            }
            m->data->end = i;
            return 1;
        }
    }
    return 0;
}

_Bool messages_mright(MESSAGES *m)
{
    return 0;
}

_Bool messages_mwheel(MESSAGES *m, int height, double d)
{
    return 0;
}

_Bool messages_mup(MESSAGES *m)
{
    m->select = 0;
    return 0;
}

_Bool messages_mleave(MESSAGES *m)
{
    return 0;
}

int messages_selection(MESSAGES *m, void *data, uint32_t len)
{
    if(m->data->n == 0) {
        *(uint8_t*)data = 0;
        return 0;
    }

    int i = m->data->istart, n = m->data->iend + 1;
    void **dp = &m->data->data[i];

    char_t *p = data;

    while(i != n) {
        MESSAGE *msg = *dp++;

        if(m->type) {
            memcpy(p, &msg->msg[msg->length + 1], (uint16_t)msg->msg[msg->length] * 2);
            p += (uint8_t)msg->msg[msg->length];
        } else {
            FRIEND *f = &friend[m->data->id];
            uint8_t author = msg->flags & 1;

            if(!author) {
                memcpy(p, f->name, f->name_length);
                p += f->name_length;
            } else {
                memcpy(p, self.name, self.name_length);
                p += self.name_length;
            }
        }

        strcpy2(p, ": ");
        p += 2;

        switch(msg->flags) {
        case 0:
        case 1:
        case 2:
        case 3: {
            if(i == m->data->istart) {
                if(i == m->data->iend) {
                    memcpy(p, msg->msg + m->data->start, m->data->end - m->data->start);
                    p += m->data->end - m->data->start;
                } else {
                    memcpy(p, msg->msg + m->data->start, msg->length - m->data->start);
                    p += msg->length - m->data->start;
                }
            } else if(i == m->data->iend) {
                memcpy(p, msg->msg, m->data->end);
                p += m->data->end;
            } else {
                memcpy(p, msg->msg, msg->length);
                p += msg->length;
            }
            break;
        }
        }

        strcpy2(p, "\r\n");
        p += 2;

        i++;
    }
    *p = 0;

    return (void*)p - data;
}

static int msgheight(MESSAGE *msg, int width)
{
    switch(msg->flags) {
    case 0:
    case 1:
    case 2:
    case 3: {
        return heightmsg(msg->msg, width - MESSAGES_X - TIME_WIDTH, msg->length) + MESSAGES_SPACING;
    }

    case 6:
    case 7: {
        return BM_FT_HEIGHT + MESSAGES_SPACING;
    }

    }

    return 0;
}

void messages_updateheight(MESSAGES *m)
{
    MSG_DATA *data = m->data;
    if(!data || m->width == data->width) {
        return;
    }

    uint32_t height = 0;
    int i = 0;
    while(i < data->n) {
        MESSAGE *msg = data->data[i];
        msg->height = msgheight(msg, m->width);
        height += msg->height;
        i++;
    }

    m->height = height;
    data->height = height;
    data->width = width;
    m->panel.content_scroll->content_height = height;
}

static void message_setheight(MESSAGES *m, MESSAGE *msg, MSG_DATA *p)
{
    if(m->width == 0) {
        return;
    }
    msg->height = msgheight(msg, m->width);
    p->height += msg->height;
    if(m->data == p) {
        m->panel.content_scroll->content_height = p->height;
    }
}

void message_add(MESSAGES *m, MESSAGE *msg, MSG_DATA *p)
{
    time_t rawtime;
    struct tm *ti;
    time(&rawtime);
    ti = localtime(&rawtime);

    msg->time = ti->tm_hour * 60 + ti->tm_min;

    p->data = realloc(p->data, (p->n + 1) * sizeof(void*));
    p->data[p->n++] = msg;

    message_setheight(m, msg, p);
}
