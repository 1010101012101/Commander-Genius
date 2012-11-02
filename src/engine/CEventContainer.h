/*
 * CEventContainer.h
 *
 *  Created on: 03.08.2010
 *      Author: gerstrong
 *
 * This will store a list of events
 */

#ifndef CEVENTCONTAINER_H_
#define CEVENTCONTAINER_H_

#include "CEvent.h"
#include <deque>


class CEventContainer
{
public:

	size_t size() { return m_EventList.size(); }
	bool empty() { return m_EventList.empty(); }
	void clear() { m_EventList.clear(); }
	
	void add(std::unique_ptr<CEvent>& ev) 
	{ 
	    m_EventList.push_back(move(ev)); 	    
	}
	
	void add(CEvent *ev) 
	{ 
	    m_EventList.push_back(std::unique_ptr<CEvent>(ev)); 	    
	}
	std::deque< std::unique_ptr<CEvent> >::iterator erase(std::deque< std::unique_ptr<CEvent> >::iterator &it)
	{	return m_EventList.erase(it);	}
	std::deque< std::unique_ptr<CEvent> >::iterator begin() { return m_EventList.begin(); }
	std::deque< std::unique_ptr<CEvent> >::iterator end() { return m_EventList.end(); }
	template<typename T> T* occurredEvent();
	void pop_Event() { m_EventList.pop_front(); }

private:
	std::deque< std::unique_ptr<CEvent> > m_EventList;
};

template<typename T>
T* CEventContainer::occurredEvent()
{
	if(m_EventList.empty())
		return NULL;

	return dynamic_cast<T*> (m_EventList.front().get());
}


#endif /* CEVENTCONTAINER_H_ */
