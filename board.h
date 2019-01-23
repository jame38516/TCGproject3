#pragma once
#include <array>
#include <iostream>
#include <iomanip>
#include <cmath>

/**
 * array-based board for 2048
 *
 * index (1-d form):
 *  (0)  (1)  (2)  
 *  (4)  (5)  (6)  
 *
 */
class board {
public:
	typedef uint32_t cell;
	typedef std::array<cell, 3> row;
	typedef std::array<row, 2> grid;
	
	typedef std::array<cell, 2> row2;
	typedef std::array<row, 3> grid2;
	
	typedef uint64_t data;
	typedef int reward;

public:
	board() : tile(), attr(0) {}
	board(const grid& b, data v = 0) : tile(b), attr(v) {}
	board(const board& b) = default;
	board& operator =(const board& b) = default;

	operator grid&() { return tile; }
	operator const grid&() const { return tile; }
	row& operator [](unsigned i) { return tile[i]; }
	const row& operator [](unsigned i) const { return tile[i]; }
	cell& operator ()(unsigned i) { return tile[i / 3][i % 3]; }
	const cell& operator ()(unsigned i) const { return tile[i / 3][i % 3]; }

	data info() const { return attr; }
	data info(data dat) { data old = attr; attr = dat; return old; }

public:
	bool operator ==(const board& b) const { return tile == b.tile; }
	bool operator < (const board& b) const { return tile <  b.tile; }
	bool operator !=(const board& b) const { return !(*this == b); }
	bool operator > (const board& b) const { return b < *this; }
	bool operator <=(const board& b) const { return !(b < *this); }
	bool operator >=(const board& b) const { return !(*this < b); }

public:

	/**
	 * place a tile (index value) to the specific position (1-d form index)
	 * return 0 if the action is valid, or -1 if not
	 */
	reward place(unsigned pos, cell tile) {
		if (pos >= 6) return -1;
		if (tile != 1 && tile != 2 && tile != 3) return -1;
		operator()(pos) = tile;
		return 0;
	}

	/**
	 * apply an action to the board
	 * return the reward of the action, or -1 if the action is illegal
	 */
	reward slide(unsigned opcode) {
		switch (opcode & 0b11) {
		case 0: return slide_up();
		case 1: return slide_right();
		case 2: return slide_down();
		case 3: return slide_left();
		default: return -1;
		}
	}

	reward slide_left() {
		board prev = *this;
		reward score = 0;
		for (int r = 0; r < 2; r++) {
			auto& row = tile[r];
			int top = 0, hold = 0, hold_c = 0, place_index = 0;
			bool score_flag = false, zero_flag = false;
			for (int c = 0; c < 3; c++) {
				int tile = row[c];
				if (tile == 0){
					zero_flag = true;
					place_index = c;
					continue;
				} 
				row[c] = 0;
				if (!zero_flag){
					if (hold) {
						if (( tile == hold||(tile==2 && hold==1) ) && c-place_index==1 && !( (tile==1 && hold==1)||(tile==2 && hold==2) )) {
							row[top++] = ++tile;
							score += (1 << tile);
							zero_flag = true;
							place_index = c;
							hold = 0;
						} 
						else if ((tile==1 && hold==2) && c-place_index==1){
							tile = 3;
							row[top++] = tile;
							score += (1 << tile);
							zero_flag = true;
							place_index = c;
							hold = 0;
						}
						else {
							row[top++] = hold;
							hold = tile;
							place_index = c;
						}
					}
					else {
						hold = tile;
					}
				}
				else{
					row[place_index] = tile;
					place_index = c;
				}
			}
			if (hold) tile[r][top] = hold;
		}
		return (*this != prev) ? score : -1;
	}
	reward slide_right() {
		reflect_horizontal();
		reward score = slide_left();
		reflect_horizontal();
		return score;
	}
	reward slide_up() {
		
		board prev = *this;
		reward score = 0;
		for (int c=0; c<2; c++){
			auto& row = tile[c];
			for (int r=0; r<3; r++){
				trans_tile[r][c] = row[r];
		//		std::cout << trans_tile[r][c] << " ";
			}
		}
			
				
		
		for (int r = 0; r < 3; r++) {
			auto& row2 = trans_tile[r];
			int top = 0, hold = 0, hold_c = 0, place_index = 0;
			bool score_flag = false, zero_flag = false;
			for (int c = 0; c < 2; c++) {
				int tile = row2[c];
				
				if (tile == 0){
					zero_flag = true;
					place_index = c;
					continue;
				} 
				row2[c] = 0;
				if (!zero_flag){
					if (hold) {
						if (( tile == hold||(tile==2 && hold==1) ) && c-place_index==1 && !( (tile==1 && hold==1)||(tile==2 && hold==2) )) {
							row2[top++] = ++tile;
							score += (1 << tile);
							zero_flag = true;
							place_index = c;
							hold = 0;
						} 
						else if ((tile==1 && hold==2) && c-place_index==1){
							tile = 3;
							row2[top++] = tile;
							score += (1 << tile);
							zero_flag = true;
							place_index = c;
							hold = 0;
						}
						else {
							row2[top++] = hold;
							hold = tile;
							place_index = c;
						}
					}
					else {
						hold = tile;
					}
				}
				else{
					row2[place_index] = tile;
					place_index = c;
				}
			}
			if (hold) trans_tile[r][top] = hold;
		}
		for (int r=0; r<3; r++){
			auto& row = trans_tile[r];
			for (int c=0; c<2; c++){
				tile[c][r] = row[c];
		//		std::cout << trans_tile[r][c] << " ";
			}
		}
			
		return (*this != prev) ? score : -1;
	}
	reward slide_down() {
		reflect_vertical();
		reward score = slide_up();
		reflect_vertical();
		return score;
	}

	void transpose() {
		for (int r = 0; r < 2; r++) {
			for (int c = r + 1; c < 3; c++) {
				std::swap(tile[r][c], tile[c][r]);
			}
		}
	}

	void reflect_horizontal() {
		for (int r = 0; r < 2; r++) 
			std::swap(tile[r][0], tile[r][2]);
	}

	void reflect_vertical() {
		for (int c = 0; c < 3; c++) 
			std::swap(tile[0][c], tile[1][c]);
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(int r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
	friend std::ostream& operator <<(std::ostream& out, const board& b) {
	
		for (int i = 0; i < 6; i++) {
			if (b(i) >= 3)
				out << std::setw(std::min(i, 1)) << "" << (3*std::pow(2, b(i)-3));
			else
				out << std::setw(std::min(i, 1)) << "" << b(i);
		}
		return out;
	}
	friend std::istream& operator >>(std::istream& in, board& b) {
		for (int i = 0; i < 6; i++) {
			while (!std::isdigit(in.peek()) && in.good()) in.ignore(1);
			in >> b(i);
			if (b(i) >= 3)
				b(i) = std::log2(b(i)/3)+3;

		}
		return in;
		
	}

private:
	grid tile;
	grid2 trans_tile;
	data attr;
};
