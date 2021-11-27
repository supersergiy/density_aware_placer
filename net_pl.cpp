#include "net_pl.h"
#include "basic_types.h"
#include "design_info.h"
#include "utilities.h"
#include <set>
#include <map>

using namespace std;

double hpwl;

#ifdef DEBUG
extern time_t tGetFourLimits;
#endif

/* ************************************************** */
Coor Net_Pl::getPinCoorNetIndex(int pos_inside_net)
{
	Coor cell_center = design.cell_lib[cell[pos_inside_net].CellPosition].getCenter();
	Coor result;

	result.x = cell_center.x + cell[pos_inside_net].pin_xoffset;
	result.y = cell_center.y + cell[pos_inside_net].pin_yoffset;
	return result;
}
/* ************************************************** */
void Net_Pl::addNetPin(int cell_index, Coor pin_coor)
{
	NetPin x_pin, y_pin;

	x_pin.pin_coor = pin_coor.x;
	y_pin.pin_coor = pin_coor.y;

	x_pin.cell_index = cell_index;
	y_pin.cell_index = cell_index;

	x_pins.insert(x_pin);
	y_pins.insert(y_pin);
}
/* ************************************************** */
void Net_Pl::removeNetPin(int cell_index, Coor pin_coor)
{
	multiset<NetPin>::iterator it_x, it_y;
	NetPin x_pin, y_pin;

	x_pin.pin_coor = pin_coor.x;
	y_pin.pin_coor = pin_coor.y;

	x_pin.cell_index = cell_index;
	y_pin.cell_index = cell_index;

	it_x = x_pins.find(x_pin);
	it_y = y_pins.find(y_pin);

	if (it_y == y_pins.end()){
		cout << "BUG!!!!!!!!!!!!!!!!" << endl;
	}

	if (it_x == x_pins.end()){
		cout << "BUG!!!!!!!!!!!!!!!!" << endl;
	}
	x_pins.erase(it_x);
	y_pins.erase(it_y);

}
/* ************************************************** */
void Net_Pl::moveNetPin(int cell_index, Coor original_coor, Coor destination_coor)
{
	removeNetPin(cell_index, original_coor);
	addNetPin(cell_index, destination_coor);
}
/* ************************************************** */
void Net_Pl::initializeNet(void)
{
	for (int i = 0; i < cell.size(); i++){
		addNetPin(cell[i].CellPosition, getPinCoorNetIndex(i));
	}
}
/* ************************************************** */
coor_type Net_Pl::getHPWL(void)
{
	coor_type left_limit, right_limit, top_limit, bot_limit;

	if (one_cell_net){
		return 0;
	}

	left_limit = (x_pins.begin())->pin_coor;
	right_limit = (x_pins.rbegin())->pin_coor;

	bot_limit = (y_pins.begin())->pin_coor;
	top_limit = (y_pins.rbegin())->pin_coor;

	return (right_limit - left_limit) + (top_limit - bot_limit);	
}
/* ************************************************** */
void Net_Pl::getFourLimits(coor_type &left_limit, coor_type &right_limit, coor_type &top_limit, coor_type &bot_limit)
{

	left_limit = (x_pins.begin())->pin_coor;
	right_limit = (x_pins.rbegin())->pin_coor;

	bot_limit = (y_pins.begin())->pin_coor;
	top_limit = (y_pins.rbegin())->pin_coor;

}
/* ************************************************** */
void Net_Pl::getFourLimits(coor_type &left_limit, coor_type &right_limit, coor_type &top_limit, coor_type &bot_limit, int cell_to_ignore)
{
	multiset<NetPin>::iterator it;
	multiset<NetPin>::reverse_iterator r_it;

	if (one_cell_net){
		cout << "Warning: single cell net" << endl;
	}

	it = x_pins.begin();
	while (cell_to_ignore == it->cell_index){
		it++;
	}
	left_limit = it->pin_coor;

	r_it = x_pins.rbegin();
	while (cell_to_ignore == r_it->cell_index){
		r_it++;
	}
	right_limit = r_it->pin_coor;

	it = y_pins.begin();
	while (cell_to_ignore == it->cell_index){
		it++;
	}
	bot_limit = it->pin_coor;

	r_it = y_pins.rbegin();
	while (cell_to_ignore == r_it->cell_index){
		r_it++;
	}
	top_limit = r_it->pin_coor;

}
/* ************************************************** */
void Net_Pl::initialOneCellNet(void)
{
	map<int, int> pin_stat;
	for (multiset<NetPin>::iterator it = x_pins.begin(); it != x_pins.end(); it++){
		pin_stat[it->cell_index]++;
	}
	if (pin_stat.size() == 1){
		one_cell_net = true;
	}

	for (multiset<NetPin>::iterator it = y_pins.begin(); it != y_pins.end(); it++){
		pin_stat[it->cell_index]--;
	}

	for (map<int, int>::iterator it = pin_stat.begin(); it != pin_stat.end(); it++){
		if (it->second != 0){
			cout << "x and y are not consistent" << endl;
			break;
		}
	}
}
/* ************************************************** */
bool Net_Pl::checkNetConsistency(void)
{
	map<int, int> pin_stat;
	for (multiset<NetPin>::iterator it = x_pins.begin(); it != x_pins.end(); it++){
		pin_stat[it->cell_index]++;
	}
	if ((pin_stat.size() == 1) != (one_cell_net)){
		cout << "one cell net bug" << endl;
	}

	for (multiset<NetPin>::iterator it = y_pins.begin(); it != y_pins.end(); it++){
		pin_stat[it->cell_index]--;
	}

	for (map<int, int>::iterator it = pin_stat.begin(); it != pin_stat.end(); it++){
		if (it->second != 0){
			cout << "x and y are not consistent" << endl;
			return false;
		}
	}
	return true;
}
