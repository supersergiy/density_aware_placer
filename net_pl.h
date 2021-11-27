#pragma once

#include "basic_types.h"
#include "multithread.h"

class NetPin{
public:
	coor_type pin_coor;
	int cell_index;

	bool operator<(const NetPin &p) const
	{
		return (pin_coor < p.pin_coor) || (pin_coor == p.pin_coor && cell_index < p.cell_index);
	}

	bool operator==(const NetPin &p) const
	{
		return (cell_index == p.cell_index) && (pin_coor == p.pin_coor);
	}
};

/* ************************************************** */
class Net_Pl
{

public:
    Net_Pl():NetDegree(0), one_cell_net(false){}
    void initialNetName(string initial) {NetName=initial;}
    void initialNetDegree(int initial) {NetDegree=initial;}
    void initialcell(NetNode initial) {cell.push_back(initial);}

    void clear() {Net_Pl(); NetName.clear(); cell.clear();}

    string getNetName() {return NetName;}
    int getNetDegree() {return NetDegree;}
    string getCellNodeName(int pos)
    {
    	string result =  cell.at(pos).NodeName;
    	return result;
    }
    bool getCellIorO(int pos) {return cell.at(pos).IorO;}
    double getXoffset(int pos) {return cell.at(pos).pin_xoffset;}
    double getYoffset(int pos) {return cell.at(pos).pin_yoffset;}

    //readers
	void getFourLimits(coor_type &left_limit, coor_type &right_limit, coor_type &top_limit, coor_type &bot_limit);
	void getFourLimits(coor_type &left_limit, coor_type &right_limit, coor_type &top_limit, coor_type &bot_limit, int cell_to_ignore);
    int getCellPosition(int pos)
    {
    	int result = cell.at(pos).CellPosition;
    }
	Coor getPinCoorNetIndex(int pos);
	coor_type getHPWL(void);

	//writers
	void addNetPin(int cell_index, Coor pin_coor);
	void removeNetPin(int cell_index, Coor pin_coor);
	void moveNetPin(int cell_index, Coor original_coor, Coor destination_coor);


	void initializeNet(void);

	void initialOneCellNet(void);
	bool checkNetConsistency(void);
	bool isOneCellNet(void) {return one_cell_net;}
private:

	coor_type getLeftLimit(void); //
	coor_type getRightLimit(void);
	coor_type getTopLimit(void);
	coor_type getBotLimit(void);
	bool one_cell_net;
    string NetName;
    int NetDegree;
    vector<NetNode> cell;

	multiset<NetPin> x_pins;
	multiset<NetPin> y_pins;
}; // Net_Pl
