#pragma once

/****************************************/
/*										*/
/*	Author:		Eskalante				*/
/*	Name:		Drag					*/
/*	Version:	1.1						*/
/*	Desc.:		Tool for mouse drag.	*/
/*										*/
/****************************************/

// point coordinate
struct point {
	int x;
	int y;
};
// rectangle coordinate
struct rect {
	rect() {

	}
	rect(int t, int l, int r, int b) :top(t), left(l), right(r), bottom(b) {

	}
	signed int top;
	signed int left;
	signed int right;
	signed int bottom;
};

class drag {
	// C&D
public:
	// provide starting, no border will be applied
	drag(rect sArea) :bDragState(0), bFalse(0), sButtonArea(sArea) {
		point nill;
		nill.x = 0;
		nill.y = 0;
		sStart = nill;
		sEnd = nill;
		sBorder.bottom = 0;
		sBorder.left = 0;
		sBorder.right = 0;
		sBorder.top = 0;
	}
	// provide starting area and border
	drag(rect sArea, rect sBorders) :bDragState(0), bFalse(0), sButtonArea(sArea), sBorder(sBorders) {
		point nill;
		nill.x = 0;
		nill.y = 0;
		sStart = nill;
		sEnd = nill;
	}
	virtual	~drag() {

	}

	// SYS
public:
	// call in main cycle
	// INPUT: current cursor position, state of mouse left button (down=true)
	// OUTPUT: state of drag (dragging=true)
	bool	update(point sCursor, bool bLeftButton = false) {
		if (bLeftButton) {
			if (bDragState) {
				// update
				sEnd.x = sCursor.x;
				sEnd.y = sCursor.y;
				if (!isIn(sCursor, sBorder) || sBorder.right > 0) {
					sEnd = border(sEnd, sBorder);
				}
			}
			else {
				if (isIn(sCursor, sButtonArea) && !bFalse) {
					// on left button down in area
					sStart.x = sCursor.x;
					sStart.y = sCursor.y;
					sEnd.x = sCursor.x;
					sEnd.y = sCursor.y;
					// start dragging
					bDragState = true;
				}
				else
					// false activation, button was allready pushed outside
					bFalse = true;
			}
		}
		else {
			// reset false state
			bFalse = false;
			if (bDragState) {
				// on left button up
				sEnd.x = sCursor.x;
				sEnd.y = sCursor.y;
				if (!isIn(sCursor, sBorder) || sBorder.right > 0) {
					sEnd = border(sEnd, sBorder);
				}
				// end dragging
				bDragState = false;
			}
		}
		return bDragState;
	}

	// MAIN
public:
	rect	getActiveArea() const {
		return calcActiveArea();
	}
	point	getStart() const {
		return sStart;
	}
	point	getEnd() const {
		return sEnd;
	}
	point	getTopLeft() const {
		point ret;
		ret.x = calcActiveArea().left;
		ret.y = calcActiveArea().top;
		return ret;
	}
	point	getTopRight() const {
		point ret;
		ret.x = calcActiveArea().right;
		ret.y = calcActiveArea().top;
		return ret;
	}
	point	getBottomLeft() const {
		point ret;
		ret.x = calcActiveArea().left;
		ret.y = calcActiveArea().bottom;
		return ret;
	}
	point	getBottomRight() const {
		point ret;
		ret.x = calcActiveArea().right;
		ret.y = calcActiveArea().bottom;
		return ret;
	}
	// return axis distances between start and end points
	point	getResult() const {
		point ret;
		ret.x = sEnd.x - sStart.x;
		ret.y = sEnd.y - sStart.y;
		return ret;
	}

	// ACC
public:
	rect	getButtonArea() const {
		return sButtonArea;
	}
	void	setButtonArea(rect sNewArea) {
		sButtonArea = sNewArea;
	}
	rect	getBorderArea() const {
		return sBorder;
	}
	void	setBorderArea(rect sNewBorder) {
		sBorder = sNewBorder;
	}

	// ATR
private:
	rect	sButtonArea;	// area from which the drag starts
	rect	sBorder;		// border of dragging
	point	sStart;			// starting point
	point	sEnd;			// ending point
	bool	bDragState;		// status of the drag
	bool	bFalse;			// allreadz pushed cursor entered working area

							// HELP
private:
	// calculate if point is in rectangle
	bool	isIn(point sCursor, rect sArea) {
		if (sCursor.x <= sArea.right &&
			sCursor.x >= sArea.left &&
			sCursor.y <= sArea.bottom &&
			sCursor.y >= sArea.top) {
			return true;
		}
		return false;
	}
	// calculate rectangle from two points (top left and bottom right)
	rect	calcActiveArea() const {
		rect ret;
		// find top&bottom points
		if (sStart.y >= sEnd.y) {
			ret.bottom = sStart.y;
			ret.top = sEnd.y;
		}
		else {
			ret.bottom = sEnd.y;
			ret.top = sStart.y;
		}
		// find left&right points
		if (sStart.x >= sEnd.x) {
			ret.right = sStart.x;
			ret.left = sEnd.x;
		}
		else {
			ret.right = sEnd.x;
			ret.left = sStart.x;
		}
		return ret;
	}
	// cut position up to the borders
	point	border(point sIn, rect sBor) {
		if (sIn.x > sBor.right)
			sIn.x = sBor.right;
		if (sIn.x < sBor.left)
			sIn.x = sBor.left;
		if (sIn.y > sBor.bottom)
			sIn.y = sBor.bottom;
		if (sIn.y < sBor.top)
			sIn.y = sBor.top;
		return sIn;
	}
};
