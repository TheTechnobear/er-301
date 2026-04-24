#pragma once

#include <od/graphics/FrameBuffer.h>

namespace od
{

#define CURSOR_HEIGHT 5

	enum CursorOrientation
	{
		cursorDown,
		cursorUp,
		cursorLeft,
		cursorRight
	};

	struct CursorState
	{
		void copyAttributes(CursorState &cs);

		CursorOrientation orientation = cursorDown;
		Color color = WHITE;
		int x = -10;
		int y = -10;
		bool active = false;
		bool show = true;
	};

	class Cursor
	{
	public:
		Cursor();
		virtual ~Cursor();
		static float defaultTweenStep();
		static float defaultBreathStep();
		static void setAnimationSteps(float tweenStep, float breathStep);
		static void resetAnimationSteps();
		void draw(CursorState &target, FrameBuffer &fb);
		void setPosition(int x, int y);

	private:
		static float sTweenStep;
		static float sBreathStep;
		float mX = 0;
		float mY = 0;
		float mBreath = 0;
		int mBreathState = 0;
	};

} /* namespace od */
