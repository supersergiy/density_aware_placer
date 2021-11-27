#pragma once

#include "basic_types.h"
#include "SubRow.h"

class RowInfo
{
public:
    RowInfo():Coordinate(0.0), height(0.0), Sitewidth(0.0),
    Sitespacing(0.0), Siteorient(false), Sitesymmetry(true),
    SubrowOrigin(0.0), NumSites(0.0), RESubRow(0.0) {}

    void initialCoor(coor_type initial) {Coordinate = initial;}
    void initialHeight(coor_type initial) {height = initial;}
    void initialWidth(coor_type initial) {width = initial;}
    void initialSitewidth(coor_type initial) {Sitewidth = initial;}
    void initialSitespacing(coor_type initial) {Sitespacing = initial;}
    void initialSiteorient(bool initial) {Siteorient = initial;}
    void initialSitesymmetry(bool initial) {Sitesymmetry = initial;}
    void initialSubrowOrigin(coor_type initial) {SubrowOrigin = initial;}
    void initialNumSites(coor_type initial) {NumSites = initial;}

    void calRESubRow() {RESubRow = SubrowOrigin + (NumSites * Sitespacing);}
    void clear() {RowInfo();}

    coor_type getCoor(){ return Coordinate;}
    double getWidth() {return width;}
    coor_type getHeight(){ return height;}
    coor_type getSitewidth(){ return Sitewidth;}
    coor_type getSitespacing(){ return Sitespacing;}
    bool getSiteorient(){ return Siteorient;}
    bool getSitesymmetry(){ return Sitesymmetry;}
    coor_type getSubrowOrigin(){ return SubrowOrigin;}
    coor_type getNumSites(){ return NumSites;}
    coor_type getRESubRow(){ return RESubRow;}

    // Macro
	void AddMacro(size_t p_macro_pos){Macro.push_back(p_macro_pos);}
	size_t getMacro(size_t p_pos){return Macro.at(p_pos);}
	size_t getMacroNum(){return Macro.size();}
	// SubRow
	void addSubRow(SubRow p_segment) {Segment.push_back(p_segment);}
	SubRow& getSubRow(size_t p_pos) {return Segment.at(p_pos);}
	size_t getSubRowNum() {return Segment.size();}
	void eraseSubRow(size_t p_seg_pos) {Segment.erase(Segment.begin() + p_seg_pos);}
	void sortSubrow() {sort(Segment.begin(), Segment.end());}
private:
    coor_type Coordinate;
    coor_type width, height;
    coor_type Sitewidth;
    coor_type Sitespacing;
    bool Siteorient;// Y=true, N=false. Not using in contest.
    bool Sitesymmetry;// Y=true, N=false. Not using in contest.
    coor_type SubrowOrigin;
    coor_type NumSites;
    coor_type RESubRow;// Right-Edge Subrow
    // Abacus legalization using
    vector<size_t> Macro;
    vector<SubRow> Segment;
}; // RowInfo
