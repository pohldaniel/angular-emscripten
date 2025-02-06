#pragma once

#include <interfaces/IStateMachine.h>
#include "Event.h"

enum States {
	SHAPE
};

class State;
class StateMachine : public IStateMachine<State> {

	friend class IStateMachine<State>;

public:

	StateMachine(const float& dt, const float& fdt);

	void fixedUpdate();
	void update();
	void render();
	void resizeState(int deltaW, int deltaH, States state);

	const float& m_fdt;
	const float& m_dt;

private:

};

class State : public IState<State> {

public:

	State(StateMachine& machine, States currentState);
	virtual ~State();

	States getCurrentState();

	virtual void OnMouseMotion(const Event::MouseMoveEvent& event);
	virtual void OnMouseButtonDown(const Event::MouseButtonEvent& event);
	virtual void OnMouseButtonUp(const Event::MouseButtonEvent& event);

protected:

	StateMachine& m_machine;
	const float& m_fdt;
	const float& m_dt;

	States m_currentState;
};