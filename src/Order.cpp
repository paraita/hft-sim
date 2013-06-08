#include "Order.h"
#include <iostream>

Order::Order()
{
}
Order::Order(int a_asset , int a_owner , double	a_time , int a_price,
			 int	a_volume , OrderType a_type ,
			 int a_globalOrderIdentifier, OrderState state)
{
	m_asset					= a_asset;
	m_owner					= a_owner;
	m_time					= a_time;
	m_price					= a_price;
	m_initialVolume			= a_volume;
	m_volume				=a_volume;
	m_type					= a_type;
	m_state					= ALIVE;
	m_globalOrderIdentifier = a_globalOrderIdentifier;
	m_state					= state;
	m_priority				= 0;
}

Order::~Order()
{

}
int Order::getUnderlying()
{
	return m_asset;
}
int Order::getIdentifier()
{
	return m_globalOrderIdentifier;
}
int Order::getOwner()
{
	return m_owner;
}
int Order::getVolume()
{
	return m_volume;
}
int Order::getInitialVolume()
{
	return m_initialVolume;
}
int Order::getPrice()
{
	return m_price;
}
OrderType Order::getType()
{
	return m_type;
}
double Order::getTime()
{
	return m_time;
}
void Order::printOrder()
{
	std::cout << m_globalOrderIdentifier << "\t" << m_owner << "\t"	
		<< m_asset << "\t" << m_type << "\t" << m_state << "\t"	
		<< m_initialVolume << "\t" << m_volume << "\t" << m_price << "\t"	
		<< std::endl ;	
}
void Order::setPrice(int a_price)
{
	m_price = a_price ;
}
void Order::setVolume(int a_volume)
{
	m_volume = a_volume ;
}
void Order::setType(OrderType a_type)
{
	m_type = a_type ;
}
void Order::setState(OrderState state)
{
	m_state = state ;
}
OrderState Order::getState()
{
	return m_state;
}
int Order::getPriority(){
	return m_priority;
}
void Order::setPriority(int priority){
	m_priority = priority;
}
