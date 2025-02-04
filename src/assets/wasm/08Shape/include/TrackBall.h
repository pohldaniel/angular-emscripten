#ifndef __TrackBallH__
#define __TrackBallH__
class TrackBall {

public:

	enum Button {
		ENoButton = 0x0,
		ELeftButton = 0x1,
		EMiddleButton = 0x2,
		ERightButton = 0x4
	};

	enum Modifier {
		ENoModifier = 0x0,
		EShiftModifier = 0x1,
		EControlModifier = 0x2,
		EAltModifier = 0x3
	};
};
#endif