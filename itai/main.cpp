#include <iostream>
#include "luncan.hpp"


int main()
{
	OrderManager om;
	Order o1(Date(2011,03,27), Date(2011,03,29), 111222, Order::generate_order_id());
	o1.set_room_and_bed(1, 2);
	o1.set_room_and_bed(2, 4);
	Order o2(Date(2011,04,05), Date(2011,04,10), 111333, Order::generate_order_id());
	o2.set_room_and_bed(1, 3);
	Order o3(Date(2011,04,19), Date(2011,04,20), 111222, Order::generate_order_id());
	o3.set_room_and_bed(1, 1);
	o3.set_room_and_bed(2, 1);
	Order o4(Date(2011,04,19), Date(2011,04,25), 111333, Order::generate_order_id());
	o4.set_room_and_bed(1, 3);
	o4.set_room_and_bed(2, 1);

	std::cout << "add o4" << std::endl;
	om.Add_order(&o4);
	std::cout << "add o1" << std::endl;
	om.Add_order(&o1);
	std::cout << "add o2" << std::endl;
	om.Add_order(&o2);
	std::cout << "add o3" << std::endl;
	om.Add_order(&o3);
	std::cout << "print" << std::endl;
	om.print_orders(Date(2011,04,01));




	char tmp[10];
	std::cout << "-- press ENTER to exit --" << std::endl;
	std::cin.getline(tmp, sizeof(tmp));
	return 0;
}
