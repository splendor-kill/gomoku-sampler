#include <fstream>
#include <regex>
#include <iostream>
#include <time.h>
#include <conio.h>
using namespace std;

#define USE_OPENMP

#include "mcts.h"

#include "gomoku.h"

void main_program()
{
	using namespace std;

	bool human_player = false;

	MCTS::ComputeOptions player1_options, player2_options;
	player1_options.max_iterations = 1000;
	//player1_options.max_time = 10;
	player1_options.verbose = true;
	player2_options.max_iterations = 100;
	//player1_options.max_time = 10;
	player2_options.verbose = true;

	GomokuState state;
	while (state.has_moves()) {
		cout << endl << "State: " << state << endl;

		GomokuState::Move move = GomokuState::no_move;
		if (state.player_to_move == 1) {
			move = MCTS::compute_move(state, player1_options);
			state.do_move(move);
		}
		else {
			if (human_player) {
				while (true) {
					cout << "Input your move: ";

					move = GomokuState::no_move;
					string input;
					cin >> input;
					if (input.size() != 2) continue;
					int row = GomokuState::LABLE_POS.at(toupper(input[0]));
					int col = GomokuState::LABLE_POS.at(toupper(input[1]));
					move = make_pair(row, col);
					try {
						state.do_move(move);
						break;
					}
					catch (std::exception&) {
						cout << "Invalid move." << endl;
					}
				}
			}
			else {
				move = MCTS::compute_move(state, player2_options);
				state.do_move(move);
			}
		}
	}

	cout << endl << "Final state: " << state << endl;

	if (state.get_result(2) == 1.0) {
		cout << "Player 1 wins!" << endl;
	}
	else if (state.get_result(1) == 1.0) {
		cout << "Player 2 wins!" << endl;
	}
	else {
		cout << "Nobody wins!" << endl;
	}
}

void read_config(MCTS::ComputeOptions& options)
{
	ifstream myfile("config.txt");
	if (myfile.is_open()) {
		string line;
		while (getline(myfile, line)) {
			std::regex base_regex("(\\S+)\\s*=\\s*(.*)", regex_constants::ECMAScript);
			std::smatch base_match;

			if (std::regex_match(line, base_match, base_regex)) {
				string key = base_match[1].str();
				string value = base_match[2].str();
				if (key == "iterations_param1") {
					options.iterations_param1 = stoi(value);
				}
				else if (key == "iterations_param2") {
					options.iterations_param2 = stoi(value);
				}
				else if (key == "number_of_repeat") {
					options.number_of_repeat = stoi(value);
				}
				else if (key == "number_of_threads") {
					options.number_of_threads = stoi(value);
				}
				else if (key == "max_time") {
					options.max_time = stoi(value);
				}
				else if (key == "file_size(K)") {
					options.file_size = stoi(value) * 1024;
				}
				else if (key == "verbose") {
					options.verbose = value == "true";
				}
				else if (key == "make_dataset") {
					options.make_dataset = value == "true";
				}
				else if (key == "dat_dir") {
					options.dat_dir = value;
				}
				else if (key == "file_prefix") {
					options.file_prefix = value;
				}
				else if (key == "board_size") {
					options.board_size = stoi(value);
				}
			}

		}
		myfile.close();
	}
	else {
		throw std::runtime_error("what's matter with the config files?");
	}
}

unique_ptr<ofstream>& prepare_file(const MCTS::ComputeOptions& options, size_t &file_size)
{
	static unique_ptr<ofstream> outfile;

	if (outfile) {
		if (file_size >= options.file_size) {
			outfile->close();
			outfile = nullptr;
			file_size = 0;
		}
	}

	if (!outfile) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		struct tm newtime;
		localtime_s(&newtime, &in_time_t);
		std::stringstream ss;
		ss << std::put_time(&newtime, "%F_%H-%M-%S");
		string filename = options.dat_dir + "\\" + options.file_prefix + ss.str() + ".txt";
		outfile = std::move(unique_ptr<ofstream>(new ofstream(filename)));
		if (!outfile->is_open()) {
			throw runtime_error("cannot open file " + filename);
		}
	}

	return outfile;
}

void save_dataset(const GomokuState& state, const MCTS::ComputeOptions& options)
{
	if (!options.make_dataset)
		return;
	static size_t file_size = 0;

	string line = state.str() + to_string(options.best_visits) + "," + to_string(options.best_wins);
	auto &file = prepare_file(options, file_size);
	*file << line << endl;
	file_size += line.size();

	if (options.quit)
		file->close();
}

void self_play()
{
	using namespace std;
	cout << "self play to high" << endl;
	MCTS::ComputeOptions player1_options;
	read_config(player1_options);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> dist(player1_options.iterations_param1, player1_options.iterations_param2);

	while (true) {
		player1_options.max_iterations = dist(gen);
		MCTS::ComputeOptions player2_options = player1_options;

		for (int i = 0; i < player1_options.number_of_repeat; i++) {
			GomokuState state(player1_options.board_size);
			while (state.has_moves()) {
				GomokuState::Move move = GomokuState::no_move;
				if (state.player_to_move == 1) {
					move = MCTS::compute_move(state, player1_options);
					state.do_move(move);
					save_dataset(state, player1_options);
				}
				else {
					move = MCTS::compute_move(state, player2_options);
					state.do_move(move);
					save_dataset(state, player2_options);
				}

				if (_kbhit()) {
					int ch = _getch();
					if (ch == 'q' || ch == 'Q') {
						player1_options.quit = true;
						save_dataset(state, player1_options);
						goto HERE;
					}
				}
			}
		}
	}
HERE:
	cout << "the end" << endl;

}

int main()
{
	try {
		//main_program();
		self_play();
	}
	catch (std::runtime_error& error) {
		std::cerr << "ERROR: " << error.what() << std::endl;
		return 1;
	}
}

