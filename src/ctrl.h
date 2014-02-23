#ifndef CTRL_H
#define CTRL_H

struct Control {
	Control();
	virtual ~Control();
	void draw();
};

struct Window : public Control { // system.cpp
	Window();
	virtual ~Window();
	void msgLoop();
};

#endif CTRL_H