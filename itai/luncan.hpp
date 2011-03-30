#ifndef LUNCAN_H_INCLUDED
#define LUNCAN_H_INCLUDED

#include <iostream>

class Room
{
private:
	int m_floor, m_number, m_beds;

public:
	Room(int floor, int number, int beds);
	int get_room_number() const;
	int get_room_floor() const;
	int get_num_of_beds() const;
};

class Room_Chain
{
private:
	const Room& m_room;
	Room_Chain * m_next;
public:
	Room_Chain(const Room& room, Room_Chain * next);
};

class Rooms_List
{
private:
	Room_Chain * m_head;

public:
	Rooms_List();

	//add a room in the beginning of the list 
	void add_room(const Room& room);
};

class Hotel
{
private:
	Rooms_List m_rooms;

public:
	Hotel(Rooms_List& rooms);
	const Rooms_List& get_rooms() const;
};

class Date
{
private:
	int m_year, m_month, m_day;
	
	int get_ordinal() const;

public:
	Date(int year, int month, int day);
	void print_date() const;
	std::string to_string() const;

	int compare(const Date& other) const;
	int delta(const Date& other) const;
	static Date advance(const Date& from, int num_of_days);
};

class Order
{
private:
	// allow a maximum of 900 beds per order -- that's the number of beds in 
	// the hotel, so we're safe... that's because you don't allow us to use 
	// std::vector<T>
	static const int max_rooms_per_order = 900; 

	Date m_check_in, m_check_out;
	int m_owner_id, m_order_id;
	int m_num_of_rooms;
	int m_num_of_beds_per_room[max_rooms_per_order]; 

public:
	Order(const Date& check_in, const Date& check_out, int owner_id, int order_id);
	Order(const Date& check_in, int num_of_nights, int owner_id, int order_id);

	static int generate_order_id();

	void set_room_and_bed(int room_number, int num_of_beds);

	int get_price_for(int num_of_beds) const;

	const Date& get_check_in() const;
	const Date& get_check_out() const;
	int get_num_of_rooms() const;

	std::string get_check_in_date() const;
	std::string get_check_out_date() const;
	int get_num_of_nights() const;
	int get_owner_id() const; 
	int get_order_id() const;

	void set_check_in_date(const Date& check_in);
	void set_check_out_date (const Date& check_out);
	void set_num_of_nights(int num_of_nights);

	void print_order_info();

	void move_order(const Date& new_check_in, const Date& new_check_out);
	void move_order(const Date& new_check_out);
	void move_order(const Date& new_check_in, int new_num_of_nights);
};

struct Order_Chain
{
	Order * m_order;
	Order_Chain * m_next;

	Order_Chain(Order * order, Order_Chain * next);
};

class Orders_List
{
public:
	Order_Chain * m_head;

	Orders_List();

	Order * find(int order_id) const;
	void insert(Order * order);
	void remove(int order_id);
};

class OrderManager
{
private:
	Orders_List list;
	int get_num_of_rooms_for_date(const Date& d) const;

public:
	OrderManager();

	void Add_order(Order* order);
	void print_orders(const Date& check_in);
	void print_orders(const Date& check_in, const Date& check_out);
	void print_orders(const Date& check_in, int days);

	Order* get_order(int order_id);

	void update_order(Order* order);
	void remove_order(Order* order);
	void remove_order(int order_id);
};



#endif // LUNCAN_H_INCLUDED
