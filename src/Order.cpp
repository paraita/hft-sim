#include <iostream>
#include "Order.h"

Order::Order() {
	asset = -1;
	owner = -1;
	time = 0.0;
	price = -1;
	initialVolume = -1;
	volume = -1;
	type = MARKET_SELL;
	state = ALIVE;
	globalOrderIdentifier = -1;
}

Order::Order(int a_asset,
	     int a_owner,
	     double a_time,
	     int a_price,
	     int a_volume,
	     OrderType a_type,
	     int a_globalOrderIdentifier) {
  asset = a_asset;
  owner = a_owner;
  time = a_time;
  price = a_price;
  initialVolume = a_volume;
  volume = a_volume;
  type = a_type;
  state = ALIVE;
  globalOrderIdentifier = a_globalOrderIdentifier;
}

Order::~Order() { }

int Order::getGlobalOrderIdentifier() {
	return globalOrderIdentifier;
}

int Order::getUnderlying() {
  return asset;
}

int Order::getIdentifier() {
  return globalOrderIdentifier;
}

int Order::getOwner() {
  return owner;
}

int Order::getVolume() {
  return volume;
}

int Order::getInitialVolume() {
  return initialVolume;
}

int Order::getPrice() {
  return price;
}

OrderType Order::getType() {
  return type;
}

double Order::getTime() {
  return time;
}

void Order::printOrder() {
  std::cout << globalOrderIdentifier << "\t" << owner << "\t"
	    << asset << "\t" << type << "\t" << state << "\t"
	    << initialVolume << "\t" << volume << "\t" << price << "\t"
	    << std::endl;	
}

void Order::setPrice(int a_price) {
  price = a_price;
}

void Order::setVolume(int a_volume) {
  volume = a_volume;
}

void Order::setType(OrderType a_type) {
  type = a_type;
}

void Order::setState(OrderState a_state) {
	state = a_state;
}
