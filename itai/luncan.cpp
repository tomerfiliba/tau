#include <sstream>
#include "luncan.hpp"


Room::Room(int floor, int number, int beds) :
	m_floor(floor), m_number(number), m_beds(beds)
{
}

int Room::get_room_number() const
{
	return m_number;
}

int Room::get_room_floor() const
{
	return m_floor;
}

int Room::get_num_of_beds() const
{
	return m_beds;
}

///////////////////////////////////////////////////////////////////////////////

Room_Chain::Room_Chain(const Room& room, Room_Chain * next) : 
m_room(room), m_next(next)
{
}

Rooms_List::Rooms_List() : m_head(0)
{
}

void Rooms_List::add_room(const Room& room)
{
	Room_Chain * next = m_head;
	m_head = new Room_Chain(room, next);
}


///////////////////////////////////////////////////////////////////////////////

Hotel::Hotel(Rooms_List& rooms) : m_rooms(m_rooms)
{
}

const Rooms_List& Hotel::get_rooms() const
{
	return m_rooms;
}

///////////////////////////////////////////////////////////////////////////////

Date::Date(int year, int month, int day) : 
	m_year(year), m_month(month), m_day(day)
{
	if (month < 1 || month > 12 || day < 1 || day > 31) {
		std::cout << "Date " << to_string() << " is invalid" << std::endl;
	}
}

void Date::print_date() const
{
	std::cout << to_string();
}

std::string Date::to_string() const
{
	std::stringstream ss;
	ss << m_day << "/" << m_month << "/" << m_year;
	return ss.str();
}

int Date::get_ordinal() const
{
	// assume every month has 31 days... that's what they said in the
	// instructions
	return (m_year * 12 + m_month) * 31 + m_day;
}

Date Date::advance(const Date& from, int num_of_days)
{
	int ordinal = from.get_ordinal() + num_of_days;
	int d = ordinal % 31;
	ordinal /= 31;
	int m = ordinal % 12;
	int y = ordinal / 12;
	return Date(y, m, d);
}

int Date::delta(const Date& other) const
{
	return other.get_ordinal() - get_ordinal();
}

int Date::compare(const Date& other) const
{
	// 0 == equals
	// negative == this < other
	// positive == this > other
	return get_ordinal() - other.get_ordinal();
}

///////////////////////////////////////////////////////////////////////////////

Order::Order(const Date& check_in, const Date& check_out, int owner_id, int order_id) :
	m_check_in(check_in), m_check_out(check_out), m_owner_id(owner_id), m_order_id(order_id),
	m_num_of_rooms(0)
{
	for (int i = 0; i < max_rooms_per_order; i++) {
		m_num_of_beds_per_room[i] = -1;
	}
}

Order::Order(const Date& check_in, int num_of_nights, int owner_id, int order_id) :
	m_check_in(check_in), m_check_out(Date::advance(check_in, num_of_nights)), 
	m_owner_id(owner_id), m_order_id(order_id), m_num_of_rooms(0)
{
	for (int i = 0; i < max_rooms_per_order; i++) {
		m_num_of_beds_per_room[i] = -1;
	}
}

int Order::generate_order_id()
{
	static int last_order_id = 0;
	last_order_id++;
	return last_order_id;
}

void Order::set_room_and_bed(int room_number, int num_of_beds)
{
	if (room_number < 1 || room_number > max_rooms_per_order) {
		std::cout << "Only up to " << max_rooms_per_order << " rooms per order!" << std::endl;
		return;
	}
	room_number--;
	if (m_num_of_beds_per_room[room_number] < 0) {
		m_num_of_rooms++;
	}
	m_num_of_beds_per_room[room_number] = num_of_beds;
}

const Date& Order::get_check_in() const
{
	return m_check_in;
}

const Date& Order::get_check_out() const
{
	return m_check_out;
}

std::string Order::get_check_in_date() const
{
	return m_check_in.to_string();
}

std::string Order::get_check_out_date() const
{
	return m_check_out.to_string();
}

int Order::get_num_of_nights() const
{
	return m_check_in.delta(m_check_out);
}

int Order::get_owner_id() const
{
	return m_owner_id;
}

int Order::get_order_id() const
{
	return m_order_id;
}

void Order::set_check_in_date(const Date& check_in)
{
	m_check_in = check_in; // calls copy c'tor
}

void Order::set_check_out_date (const Date& check_out)
{
	m_check_out = check_out; // calls copy c'tor
}

void Order::set_num_of_nights(int num_of_nights)
{
	m_check_out = Date::advance(m_check_in, num_of_nights); // calls copy c'tor
}

int Order::get_price_for(int num_of_beds) const
{
	if (num_of_beds == 1) {
		return 300;
	}
	else if (num_of_beds == 2) {
		return 500;
	}
	else {
		return 500 + 150 * (num_of_beds - 2); 
	}
}

int Order::get_num_of_rooms() const
{
	return m_num_of_rooms;
}

void Order::print_order_info()
{
	//order for guest: 045963282 
	//check in date: 01/03/2011 
	//check out date: 02/03/2011 
	//total nights: 1 
	//total rooms: 2 
	//total persons for room 1: 2 
	//total persons for room 2: 3 
	//price for room 1: 500 ILS 
	//price for room 2: 650 ILS 
	//total price: 1150 ILS

	std::cout << "order for guest: " << m_owner_id << std::endl;
	std::cout << "check in date: " << m_check_in.to_string() << std::endl;
	std::cout << "check out date: " << m_check_out.to_string() << std::endl;
	std::cout << "total rooms: " << m_num_of_rooms << std::endl;

	int room_num = 1;
	for (int i = 0; i < max_rooms_per_order; i++) {
		if (m_num_of_beds_per_room[i] > 0) {
			std::cout << "total persons for room " << room_num << ": " << m_num_of_beds_per_room[i] << std::endl;
			room_num++;
		}
	}

	room_num = 1;
	int total_price = 0;
	for (int i = 0; i < max_rooms_per_order; i++) {
		if (m_num_of_beds_per_room[i] > 0) {
			int price = get_price_for(m_num_of_beds_per_room[i]);
			std::cout << "price for room " << room_num << ": " << price << " ILS" << std::endl;
			total_price += price;
			room_num++;
		}
	}
	std::cout << "total price: " << total_price << " ILS" << std::endl;
}

void Order::move_order(const Date& new_check_in, const Date& new_check_out)
{
	set_check_in_date(new_check_in);
	set_check_out_date(new_check_out);
}

void Order::move_order(const Date& new_check_out)
{
	set_check_out_date(new_check_out);
}

void Order::move_order(const Date& new_check_in, int new_num_of_nights)
{
	set_check_in_date(new_check_in);
	set_num_of_nights(new_num_of_nights);
}

//////////////////////////////////////////////////////////////////////////////

Order_Chain::Order_Chain(Order * order, Order_Chain * next) : 
	m_order(order), m_next(next)
{
}

Orders_List::Orders_List() : m_head(0)
{
}

Order * Orders_List::find(int order_id) const
{
	for(Order_Chain * curr = m_head; curr != 0; curr = curr->m_next) {
		if (order_id == curr->m_order->get_order_id()) {
			return curr->m_order;
		}
	}
	return 0;
}

void Orders_List::insert(Order * order)
{
	if (m_head == 0) {
		m_head = new Order_Chain(order, 0);
	}
	else {
		for(Order_Chain * prev = 0, * curr = m_head; curr != 0;
				prev = curr, curr = curr->m_next) {
			if (order->get_check_in().compare(curr->m_order->get_check_in()) > 0) {
				continue;
			}
			if (order->get_order_id() > curr->m_order->get_order_id()) {
				continue;
			}
			if (prev == 0) {
				m_head = new Order_Chain(order, curr);
			}
			else {
				prev->m_next = new Order_Chain(order, curr);
			}
			break;
		}
	}
}

void Orders_List::remove(int order_id)
{
	for(Order_Chain * prev = m_head, *curr = m_head; curr != 0;
			prev = curr, curr = curr->m_next) {
		if (order_id == curr->m_order->get_order_id()) {
			if (curr == m_head) {
				m_head = curr->m_next;
			}
			else {
				prev->m_next = curr->m_next;
			}
			delete curr;
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

OrderManager::OrderManager() : list()
{
}

void OrderManager::Add_order(Order* order)
{
	for (int i = 0; i < order->get_num_of_nights(); i++) {
		Date d = Date::advance(order->get_check_in(), i);
		int rooms = get_num_of_rooms_for_date(d);
		if (rooms + order->get_num_of_rooms() > 900) {
			std::cout << "No available rooms for order no. " << order->get_order_id() << std::endl;
			return;
		}
	}

	list.insert(order);
}

int OrderManager::get_num_of_rooms_for_date(const Date& d) const
{
	int rooms = 0;
	for(Order_Chain * curr = list.m_head; curr != 0; curr = curr->m_next) {
		if (d.compare(curr->m_order->get_check_in()) >= 0 && 
				d.compare(curr->m_order->get_check_in()) <= 0) {
			rooms += curr->m_order->get_num_of_rooms();
		}
	}
	return rooms;
}

void OrderManager::print_orders(const Date& check_in)
{
	static Date infinity(9999,12,31);
	print_orders(check_in, infinity);
}

void OrderManager::print_orders(const Date& check_in, const Date& check_out)
{
	for(Order_Chain * curr = list.m_head; curr != 0; curr = curr->m_next) {
		if (check_in.compare(curr->m_order->get_check_in()) <= 0 && 
				check_out.compare(curr->m_order->get_check_out()) >= 0) {
			curr->m_order->print_order_info();
			std::cout << std::endl; // empty line
		}
	}
}

void OrderManager::print_orders(const Date& check_in, int days)
{
	if (days == 1) {
		// special case: if days == 1, print only check_in
		print_orders(check_in, Date::advance(check_in, 0));
	}
	else {
		print_orders(check_in, Date::advance(check_in, days));
	}
}

Order* OrderManager::get_order(int order_id)
{
	return list.find(order_id);
}

void OrderManager::update_order(Order* order)
{
	list.remove(order->generate_order_id());
	Add_order(order);
}

void OrderManager::remove_order(Order* order)
{
	remove_order(order->get_order_id());
}

void OrderManager::remove_order(int order_id)
{
	list.remove(order_id);
}






