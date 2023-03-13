#include "Input.h"

Slick::Input::InputManager::InputManager() 
	:
	mKeys({}),
	mButtons({}),
	mCursorState(0),
	mCursorX(0), mCursorY(0),
	mOldCursorX(0), mOldCursorY(0)
{
	mKeys.fill({false, false});
	mButtons.fill({false, false});
}

Slick::Input::InputManager::~InputManager() {}

void Slick::Input::InputManager::update() {
	for(auto& t : mKeys)
		t.handled = false;
	for(auto& t : mButtons)
		t.handled = false;

	for (auto& ke : mKeyEvents) {
		if (!mKeys[(u32)ke.kc].handled) {
			mKeys[(u32)ke.kc].handled = true;
			mKeys[(u32)ke.kc].state = ke.state;
			ke.handled = true;
		}
	}
	std::erase_if(mKeyEvents, [](KeyEvent& ke){ return ke.handled; });

	for (auto& ke : mButtonEvents) {
		if (!mButtons[(u32)ke.kc].handled) {
			mButtons[(u32)ke.kc].handled = true;
			mButtons[(u32)ke.kc].state = ke.state;
			ke.handled = true;
		}
	}
	std::erase_if(mButtonEvents, [](ButtonEvent& ke){ return ke.handled; });

	if(mCursorState < 2)
		mCursorState++;
	mOldCursorX = mCursorX;
	mOldCursorY = mCursorY;
	mCursorX = mNewX;
	mCursorY = mNewY;
}

void Slick::Input::InputManager::on_key(Key kc, bool state) {
	mKeyEvents.push_back(KeyEvent{
		.kc = kc,
		.state = state,
		.handled = false
	});
}

void Slick::Input::InputManager::on_button(Button kc, bool state) {
	mButtonEvents.push_back(ButtonEvent{
		.kc = kc,
		.state = state,
		.handled = false
	});
}
