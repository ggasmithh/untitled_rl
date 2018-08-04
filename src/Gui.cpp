#include <stdarg.h>
#include <stdio.h>

#include "main.hpp"


static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 20;
static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT - 1;

Gui::Gui() {
    con = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
}

Gui::~Gui() {
    delete con;
    log.clearAndDelete();
}

void Gui::render() {

    // clear the gui
    con->setDefaultBackground(TCODColor::black);
    con->clear();
    // draw the health bar
    renderBar(1,1,BAR_WIDTH,"HP",engine.player->destructible->hp,
	    engine.player->destructible->maxHp,
	    TCODColor::lightRed,TCODColor::darkerRed);


    // draw the message log 
    //
    // (consider moving this to a new function so we can have Messages without 
    // a Bar or vice versa)
    int y = 1;
    float colorCoef = 0.4f;
    for (Message **it = log.begin(); it != log.end(); it++) {
        Message *message = *it;
        con->setDefaultForeground(message->col * colorCoef);
        con->print(MSG_X, y, message->text);
        if (colorCoef < 1.0) {
            colorCoef += 0.3;
        }
    }

    // blit the GUI console to the Root console
    TCODConsole::blit(con, 0, 0, engine.screenWidth, PANEL_HEIGHT,
    TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);

}

void Gui::renderBar(int x, int y, int width, const char *name,
    float value, float maxValue, const TCODColor &barColor,
    const TCODColor &backColor) {
    
    // fill background
    con->setDefaultBackground(backColor);
    con->rect(x, y, width, 1, false, TCOD_BKGND_SET);

    // calculate how much of the bar is filled
    int barWidth = (int)(value / maxValue * width);
    if (barWidth > 0) {
        // draw the bar
        con->setDefaultBackground(barColor);
        con->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
    }

    // print text over the bar
    con->setDefaultForeground(TCODColor::white);
    con->printEx(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER, 
        "%s: %g/%g", name, value, maxValue);


}

Gui::Message::Message(const char *text, const TCODColor &col):
    text(strdup(text)), col(col) {
}

Gui::Message::~Message() {
    free(text);
}

void Gui::message(const TCODColor &col, const char *text, ...) {
    /*  it's right about here where this tutorial loses me
        like i get what we're doing, i just don't know why
        we're doing it this way specifically. */

    // build the text
    va_list ap; 
    char buf[128];

    va_start(ap, text);
    vsprintf(buf, text, ap);
    va_end(ap);

    char *lineBegin = buf;
    char *lineEnd;

    do {
        // make room for the new message (not a big fan of the condition on this "if")
        if (log.size() == MSG_HEIGHT) { // i just wanna decouple these two...
            Message *toRemove = log.get(0);
            log.remove(toRemove);
            delete toRemove;
        }

        // detect newline character. . . 
        lineEnd = strchr(lineBegin, '\n');

        // . . . which apparently isn't a C newline character?
        // replace it with a C newline character? (\0???)
        if (lineEnd) {
            *lineEnd = '\0';
        }

        /*  OH right because we were using C functions 
            elsewhere to handle our message.  */

        // add newly splitted message to our log
        Message *msg = new Message(lineBegin, col);
        log.push(msg);

        // start at the first character after the end of our last line
        lineBegin = lineEnd + 1;

    } while (lineEnd);
}