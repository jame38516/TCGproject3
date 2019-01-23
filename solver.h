#pragma once
#include <iostream>
#include <algorithm>
#include <cmath>
#include "board.h"
#include <numeric>
#include <string>
#include <vector>
#include <cstring>
#include <sstream>
#include <math.h>

using namespace std;

class answer {
	typedef float value_t;
	public:
		answer(value_t min = -1, value_t avg = -1, value_t max = -1) : min(min), avg(avg), max(max) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return !std::isnan(ans.avg) ? (out << ans.min << " " << ans.avg << " " << ans.max) : (out << "-1") << std::endl;
		}
	public:
		value_t min, avg, max;
	};


class state_type {
public:
	enum type : char {
		before  = 'b',
		after   = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		return out << char(type.t);
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};

class state_hint {
public:
	state_hint(const board& state) : state(const_cast<board&>(state)) {}

	char type() const { return state.info() ? state.info() + '0' : 'x'; }
	operator board::cell() const { return state.info(); }

public:
	friend std::istream& operator >>(std::istream& in, state_hint& hint) {
		while (in.peek() != '+' && in.good()) in.ignore(1);
		char v; in.ignore(1) >> v;
		hint.state.info(v != 'x' ? v - '0' : 0);
		return in;
	}
	friend std::ostream& operator <<(std::ostream& out, const state_hint& hint) {
		return out << "+" << hint.type();
	}

private:
	board& state;
};


class solver {

public:
	bool before_state = true;
	answer construct_table(board state, int op, vector<int> hint_bag, int bag_left_index, vector<int> space){
		if( before_state ){  
			int hint = hint_bag[bag_left_index];

			if (before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min != -1){
				int avg = before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].avg;
				int min = before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min;
				int max = before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].max;
				answer value(min, avg, max);
				return value;
			}
				
			
			double avg_max=0, min, max;
			bool move_flag = false;
			
			for (int mv=0; mv<4; mv++){
				
				board tmp = state;		
				int reward = tmp.slide(mv);	
				if (reward != -1)
				{
					before_state = false;
					move_flag = true;
					space.clear();
					if (mv == 0)
						space.assign({ 3, 4, 5 });
					else if (mv == 1)
						space.assign({ 0, 3 });
					else if (mv == 2)
						space.assign({ 0, 1, 2 });
					else
						space.assign({ 2, 5 });	
					
					answer v = construct_table(tmp, mv, hint_bag, bag_left_index, space);
					if (v.avg > avg_max){
						avg_max = v.avg;
						min = v.min;
						max = v.max;
					}
				}

				
			}
			
			if (move_flag){
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].avg = avg_max;
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min = min;
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].max = max;
				answer value(min, avg_max, max);
				
				return value;
			}
			else{
				double v=0;
				
				for (int i=0; i<6; i++)
					if (state(i) >= 3){
						int value = (3*pow(2, state(i)-3));
						v += pow(3, log2( value/3 ) + 1);
					}
						
				answer value(v, v, v);
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].avg = v;
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min = v;
				before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].max = v;
				return value;
			}
				
		}
		
		// after state
		else{				
			int env_tile = hint_bag[bag_left_index];	
			hint_bag.erase(hint_bag.begin()+bag_left_index);
			if (hint_bag.empty())
				hint_bag = { 1, 2, 3 };
			if (after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min != -1){
				int min = after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min;
				int max = after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].max;
				int avg = after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].avg;
				answer value(min, avg, max);
				return value;
			}
			double min=10000, max=0, avg=0;	
			int child_count = 0;   //<-----------------------------		
			for (int bag_left_index=0; bag_left_index<hint_bag.size(); bag_left_index++){						
				for (int pos : space) {	
					if (state(pos) != 0)
						continue;							
					board tmp = state;
					tmp(pos) = env_tile;				
					before_state = true;
					answer v = construct_table(tmp, op, hint_bag, bag_left_index, space);
					
					if (v.max > max)
						max = v.max;
					if (v.min < min)
						min = v.min;
					avg += v.avg;
					child_count++;
				}
			}
			avg = avg / child_count;
			after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min = min;
			after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].max = max;
			after_table[op][env_tile-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].avg = avg;
			answer value(min, avg, max);
			return value;
		}
	}
	
	
	
	solver(const std::string& args) {
		board state;
		
		for(int i=0; i<6; i++)
			state(i) = 0;  
	
		vector<int> space;
		space.assign({ 0, 1, 2, 3, 4, 5 });

		vector<int> hint_bag = {{1, 2, 3}};
		double min=10000, max=0, avg=0;
		int child_count = 0;	
		for (int pos : space) {	
//			cout << "pos:" << pos << endl;
			for (int bag_index=0; bag_index<hint_bag.size(); bag_index++){
				board tmp = state;
				int env_tile = hint_bag[bag_index];
				tmp(pos) = env_tile;
				
				vector<int> tmp_bag = hint_bag;
				hint_bag.erase(hint_bag.begin()+bag_index);
				if (hint_bag.empty())
					hint_bag.assign({ 1, 2, 3 });

				for (int bag_left_index=0; bag_left_index<hint_bag.size(); bag_left_index++){
					cout << "-----root state-----:"; ///////////////////////
					cout << "+" << env_tile << " {";  ///////////////////////
					for (int i=0; i<hint_bag.size(); i++)
						cout << hint_bag[i] << " "; //////////
					cout << "}" << endl;
					int op = -1;
					cout << "op:" << op << endl; ////////////////////////
					cout << endl << tmp << endl;  ///////////////////////
					
					before_state = true;
					answer v = construct_table(tmp, op, hint_bag, bag_left_index, space);
					child_count++;
				}
				
				hint_bag = tmp_bag;
					
			}
		}
	}

	answer solve(const board& state, state_type type = state_type::before) {
		board::cell hint = state_hint(state);    // <-------------------

		if ( type.is_after() ){
			for (int i=0; i<4; i++){
				
				if (after_table[i][hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)].min != -1)
					return after_table[i][hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)];
			}
			return after_table[0][hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)];
		}
			
			
		else if ( type.is_before() )
			return before_table[hint-1][state(0)][state(1)][state(2)][state(3)][state(4)][state(5)];
	}

private:
	// TODO: place your transposition table here
 	static answer before_table[3][10][10][10][10][10][10];  //first 6 for board; last for hint
	static answer after_table[4][3][10][10][10][10][10][10];  //first 6 for board; 7th for hint; last for last move


};

answer solver::before_table[3][10][10][10][10][10][10];
answer solver::after_table[4][3][10][10][10][10][10][10];
