#ifndef BACKGROUND_H
#define BACKGROUND_H

int initBackground();    // call once, after glutCreateWindow
void drawBackground();   // call every frame, right after glClear and glLoadIdentity

#endif