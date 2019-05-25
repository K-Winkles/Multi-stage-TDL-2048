#include<iostream>
#include<ctime>
#include<cstdlib>
#include<cmath>
#include<fstream>
#include <vector>
#include <algorithm>

using namespace std;

#define grid_size 4
#define max_iterations 1000000 //change to 1,000,000 in actual experiment

#define splitter1 45000 //adjust as needed
#define splitter2 65000 //adjust as needed

#define alpha 0.001 

//split flags (global variables)
bool splitter1_flag = true;
bool splitter2_flag = true;
bool stage1=true;
bool stage2=false;
bool stage3=false;

/*
	Experiment Set-Up:
	Stage 1: no. of empty tiles and valid moves
	Stage 2: no. of distinct tiles and tiles with v-tile and 2v-tile
	Stage 3: no. of empty tiles and valid moves
*/



class state{
		
public:
	
	int grid[grid_size][grid_size];
	double r;
	double r_true;
	double monotonicity;
	double emptytiles;
	double mergeabletiles;
	double distinct_tiles;
	double v2vtiles;
	double validmoves;
	bool has2048;
	bool has4096;
	bool has8192;
	bool has16384;
	bool has32768;
	bool has65536;
	bool has131072;
	
	void init_game_state();
	void add_random_tile();
	void print_state();
	void calc_v2vtiles();
	void calc_monotonicity();
	void calc_emptytiles();
	void calc_mergeabletiles();
	void calc_validmoves();
	void calc_distinct_tiles();
	void check_big_tiles();

	state();

};

class Tuple{

public:

	double LUT[50625];
	int i[grid_size];
	int j[grid_size];

	Tuple();

};

bool is_terminal_state(state);
bool is_valid_move(int,state);
bool check_win(state);
state compute_afterstate(int, state);
state fill_space(int, state);
state merge_tiles(int, state);

class Game{
	
public:
	
	int score;
	state s;
	state s1;
	state s2;
	
	void make_move(int);
	void fill_space(int);
	void merge_tiles(int);

	Game();
	
};

class Agent{
	
public:

	int a;
	double exploration_rate;
	bool learning_enabled;
	Tuple tuple_TDL[17];


	void construct_n_tuple_network();
	void play_game(Game*);
	int evaluate(state);
	void learn_evaluation(state,int,double,state,state);
	void update_LUT(state, double);

	double V(state);

	Agent();
	
};


int main(){ 


	//to keep track of statistics
	int i, wins = 0;
	float win_rate = 0;
	int total_score = 0;
	int average_score;

	

	//creation of relevant objects
	Agent agent;
	Game exec;
	fstream myfile;
	fstream myfile2;
	fstream myfile3;
	ofstream reset;
	
	srand(time(NULL)); //seed random
	
	cout << "Executing" << endl;
	
	reset.open("score_data_001_expt.txt", ios::trunc);
	reset.close();
	reset.open("win_rate_data_001_expt.txt", ios::trunc);
	reset.close();
	reset.open("data_table.txt",ios::trunc);
	reset.close();
	
	for(i = 0; i < max_iterations; i++){
		
		agent.play_game(&exec);

		if((i+1) % 1000 == 0){
			
			agent.learning_enabled = false;
			
			for(int j = 0; j < 1000; j++){
				
				agent.play_game(&exec);
				
				if(check_win(exec.s))
					wins++;
				
				total_score += exec.score;
				
			}
			
			win_rate = (float)wins/(float)1000;
			average_score = total_score/1000;

			myfile.open("score_data_001_expt.txt", ios::app);
			myfile2.open("win_rate_data_001_expt.txt", ios::app);
			myfile3.open("date_table.txt",ios::app);

			myfile << "hold on;\nplot(" << i+1 << "," << average_score << ",'b.');\r\n";
			myfile2 << "hold on;\nplot(" << i+1 << "," << win_rate << ",'b.');\r\n";
			myfile3 << ""<<i+1<<" & "<<average_score<<" & "<<win_rate<<"\\\\"<<"\n";
				

			myfile.close();
			myfile2.close();
			myfile3.close();
			
			cout << "Game " << i+1 << " | Wins: " << wins << " | Win Rate: " << win_rate << " | Average Score: " << average_score << endl;

			//indicate splits, update stage flags
			if ((average_score >= splitter1) && (splitter1_flag)){
				cout << "Stage 1-2 milestone has been reached. \n"; splitter1_flag=false;stage1=false;stage2=true;
			}else if ((average_score >= splitter2) && (splitter2_flag)){
				cout << "Stage 2-3 milestone has been reached. \n"; splitter2_flag = false;stage2=false;stage3=true;
			}//end if


			wins = 0;
			total_score = 0;
			agent.learning_enabled = true;
			
		}//end if

	}//end for


	
	
	return 0;
}//end class

bool is_terminal_state(state s){
	
	int i;
	
	for(i = 0; i < 4; i++)
		if(is_valid_move(i+1, s))
			return false;
	
	return true;
	
}//end method

bool is_valid_move(int a, state s){
	int i, j;
	
	switch(a){
		
		case 1: //up
			for(i = 1; i < grid_size; i++)
				for(j = 0; j < grid_size; j++)
					if(s.grid[i][j] != 0 && (s.grid[i][j] == s.grid[i-1][j] || s.grid[i-1][j] == 0))
						return true;
			break;
			
		case 2: //down
			for(i = 0; i < grid_size-1; i++)
				for(j = 0; j < grid_size; j++)
					if(s.grid[i][j] != 0 && (s.grid[i][j] == s.grid[i+1][j] || s.grid[i+1][j] == 0))
						return true;
			break;
			
		case 3: //left
			for(i = 0; i < grid_size; i++)
				for(j = 1; j < grid_size; j++)
					if(s.grid[i][j] != 0 && (s.grid[i][j] == s.grid[i][j-1] || s.grid[i][j-1] == 0))
						return true;
			break;
			
		case 4: //right
			for(i = 0; i < grid_size; i++)
				for(j = 0; j < grid_size-1; j++)
					if(s.grid[i][j] != 0 && (s.grid[i][j] == s.grid[i][j+1] || s.grid[i][j+1] == 0))
						return true;
			break;
	}
	return false;
	
}//end method

bool check_win(state s){

	int i, j;

	for(i = 0; i < grid_size; i++)
		for(j = 0; j < grid_size; j++)
			if(s.grid[i][j] >= 2048)
				return true;

	return false;

}

state compute_afterstate(int temp_a, state s){
	
	s = fill_space(temp_a, s);
	s = merge_tiles(temp_a, s);
	/*
	if (stage1==true){
		s.calc_emptytiles();
		s.calc_validmoves();
	}else if (stage2==true){
		s.calc_v2vtiles(); //segfault here, check access in grids
		s.calc_distinct_tiles();
	}else if (stage3==true){
		//s.calc_mergeabletiles(); 
		s.calc_emptytiles();
		s.calc_validmoves();
	}//end if
	*/
	s.calc_emptytiles();
	s.calc_validmoves();
	s = fill_space(temp_a, s);
	
	return s;

}

state fill_space(int a, state temp_state){
	//cout << "Executing function fill_space\n";
	switch(a)
	{
		case 1: //up
			for(int i=0;i<grid_size;i++)
				for(int j=0;j<grid_size;j++)
				{
					if(temp_state.grid[j][i] == 0)
					{
						for(int k=j+1;k<grid_size;k++)
							if(temp_state.grid[k][i] != 0)
							{

								temp_state.grid[j][i]=temp_state.grid[k][i];
								temp_state.grid[k][i]=0;
								break;
							}
					}

				}break;

		case 2: //down
			for(int i=0;i<grid_size;i++)
				for(int j=grid_size-1;j>=0;j--)
				{
					if(temp_state.grid[j][i] == 0)
					{
						for(int k=j-1;k>=0;k--)
							if(temp_state.grid[k][i] != 0)
							{
								temp_state.grid[j][i]=temp_state.grid[k][i];
								temp_state.grid[k][i]=0;
								break;
							}
					}

				}break;
		case 3: //left
			for(int i=0;i<grid_size;i++)
				for(int j=0;j<grid_size;j++)
				{
					if(temp_state.grid[i][j] == 0)
					{
						for(int k=j+1;k<grid_size;k++)
							if(temp_state.grid[i][k] != 0)
							{
								temp_state.grid[i][j]=temp_state.grid[i][k];
								temp_state.grid[i][k]=0;
								break;
							}
					}

				}break;


		case 4:	//right
			for(int i=0;i<grid_size;i++)
				for(int j=grid_size-1;j>=0;j--)
				{
					if(temp_state.grid[i][j] == 0)
					{
						for(int k=j-1;k>=0;k--)
							if(temp_state.grid[i][k] != 0)
							{
								temp_state.grid[i][j]=temp_state.grid[i][k];
								temp_state.grid[i][k]=0;
								break;
							}
					}

				}break; 

	}
	
	return temp_state;
	
}

state merge_tiles(int a, state temp_state){
	switch (a){
		case 1: //up
			for(int i=0;i<grid_size;i++){
				for(int j=0;j<grid_size-1;j++){
					if(temp_state.grid[j][i] != 0 && temp_state.grid[j][i]==temp_state.grid[j+1][i]){
						temp_state.grid[j][i]+=temp_state.grid[j+1][i];
						temp_state.grid[j+1][i]=0;
						temp_state.r_true += temp_state.grid[j][i];
						temp_state.r += log2(temp_state.grid[j][i]);
					}
				}
			}
			break;
		case 2: //down
			for(int i=0;i<grid_size;i++){
				for(int j=grid_size-1;j>0;j--){
					if(temp_state.grid[j][i] != 0 && temp_state.grid[j][i]==temp_state.grid[j-1][i]){
						temp_state.grid[j][i]+=temp_state.grid[j-1][i];
						temp_state.grid[j-1][i]=0;
						temp_state.r_true += temp_state.grid[j][i];
						temp_state.r += log2(temp_state.grid[j][i]);
					}
				}
			}
			break;
		case 3: //left
			for(int i=0;i<grid_size;i++){
				for(int j=0;j<grid_size-1;j++){
					if(temp_state.grid[i][j] != 0 && temp_state.grid[i][j]==temp_state.grid[i][j+1]){
						temp_state.grid[i][j]+=temp_state.grid[i][j+1];
						temp_state.grid[i][j+1]=0;
						temp_state.r_true += temp_state.grid[j][i];
						temp_state.r += log2(temp_state.grid[i][j]);
					}
				}
			}
			break;
		case 4: //right
			for(int i=0;i<grid_size;i++){
				for(int j=grid_size-1;j>0;j--){
					if(temp_state.grid[i][j] != 0 && temp_state.grid[i][j]==temp_state.grid[i][j-1]){
						temp_state.grid[i][j]+=temp_state.grid[i][j-1];
						temp_state.grid[i][j-1]=0;
						temp_state.r_true += temp_state.grid[j][i];
						temp_state.r += log2(temp_state.grid[i][j]);
					}
				}
			}
			break;
	}//end switch
	
	return temp_state;
	
}//end function	


Agent::Agent(){

	exploration_rate = 0.001;
	learning_enabled = true;

	construct_n_tuple_network();

}//end function

void Agent::construct_n_tuple_network(){

	int i, j;

	for(i = 0; i < 4; i++)
		for(j = 0; j < 4; j++){

			//Construction of vertical n-tuple_TDLs
			tuple_TDL[i].i[j] = j;
			tuple_TDL[i].j[j] = i;

			//Construction of horizontal n-tuple_TDLs
			tuple_TDL[4+i].i[j] = i;
			tuple_TDL[4+i].j[j] = j;

		}//end for

	//Construction of square n-tuple_TDLs
	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++){
			tuple_TDL[8 + (3*i + j)].i[0] = i;
			tuple_TDL[8 + (3*i + j)].j[0] = j;

			tuple_TDL[8 + (3*i + j)].i[1] = i+1;
			tuple_TDL[8 + (3*i + j)].j[1] = j;

			tuple_TDL[8 + (3*i + j)].i[2] = i+1;
			tuple_TDL[8 + (3*i + j)].j[2] = j+1;

			tuple_TDL[8 + (3*i + j)].i[3] = i;
			tuple_TDL[8 + (3*i + j)].j[3] = j+1;
		}//end for

}//end function

void Agent::play_game(Game * game){ //Main Function
	
	double r;
	
	game->s.init_game_state();
	game->score = 0;
	
	
	while (!is_terminal_state(game->s)){
		
		r = (double) rand() / (RAND_MAX);
		
		if(r <= exploration_rate){
			while(true){
				a = rand() % 4 + 1;
				if(is_valid_move(a, game->s))
					break;
			}
		}else{
			a = evaluate(game->s);
		}

		game->make_move(a);
		
		if (learning_enabled){
			learn_evaluation(game->s, a, game->s1.r, game->s1, game->s2);
		}//end if
		
		game->s = game->s2;
		game->score = game->score + game->s1.r_true;

		
	}// end while
	game->s.check_big_tiles();
	game->s.has2048=false;
	game->s.has4096=false;
	game->s.has8192=false;
	game->s.has16384=false;
	game->s.has32768=false;
	game->s.has65536=false;
	game->s.has131072=false;
}//end function

int Agent::evaluate(state s){

	int i, action;
	int count = 0;
	int rand_action[4];
	double temp_r[4];
	double max;
	bool assigned[4], first = true;
	state temp_state;

	for(i = 0; i < 4; i++){
		if(is_valid_move(i+1,s)){
			temp_state = compute_afterstate(i+1, s);
			temp_r[i] = temp_state.r + V(temp_state);
			
			assigned[i] = true;	
		}
		else assigned[i] = false;
	}

	for(i = 0; i < 4; i++)
		if(assigned[i]){
			if(first){
				first = false;
				max = temp_r[i];
				action = i+1;
				rand_action[0] = i+1;
			}
			else if(temp_r[i] > max){
				max = temp_r[i];
				action = i+1;
			}
			else if(temp_r[i] == max){
				rand_action[count] = i+1;
				count ++;
			}
		}
		
	if(count > 0)
		action = rand_action[rand() % (count + 1)];
		
	return action;
}//end function

void Agent::learn_evaluation(state s,int a,double r,state s1,state s2){

	int a_next;
	double V_s;
	state temp_state;

	a_next = evaluate(s2);

	temp_state = compute_afterstate(a_next, s2);

	float features=0;
	/*
	if (stage1==true){
		features = temp_state.emptytiles + temp_state.validmoves;
	}else if (stage2==true){
		features = temp_state.v2vtiles + temp_state.distinct_tiles;
	}else if (stage3==true){
		//features = temp_state.mergeabletiles;
		features = temp_state.emptytiles + temp_state.validmoves;
	}//end if
	*/
	features = temp_state.emptytiles + temp_state.validmoves;
	features=features+temp_state.r; //linear summation of reward and features
	
	V_s = alpha*(features + V(temp_state) - V(s1));
	update_LUT(s1, V_s);

}//end function

double Agent::V(state s){ //this function is used to access relevant parts of the LUT

	double sum = 0;
	int i, j;
	int index;

	for(i = 0; i < 17; i++){

		index = 0;

		for(j = 0; j < 4; j++)
			if(s.grid[tuple_TDL[i].i[j]][tuple_TDL[i].j[j]] > 0)
				index += log2(s.grid[tuple_TDL[i].i[j]][tuple_TDL[i].j[j]]) * pow(15,j);

		sum += tuple_TDL[i].LUT[index];
	}//end for

	return sum;
}//end function

void Agent::update_LUT(state s, double V_s){ //used to access relevant parts of LUT and update value in slot
	int i, j;
	int index;

	for(i = 0; i < 17; i++){

		index = 0;

		for(j = 0; j < 4; j++)
			if(s.grid[tuple_TDL[i].i[j]][tuple_TDL[i].j[j]] > 0)
				index += log2(s.grid[tuple_TDL[i].i[j]][tuple_TDL[i].j[j]]) * pow(15,j);
			
		tuple_TDL[i].LUT[index] += V_s;

	}//end for
}//end function

Game::Game(){
	score = 0;
}//end function

void Game::make_move(int a){
	
	s1 = s;
	
	s1 = compute_afterstate(a, s1);
	
	s2 = s1;
	s2.add_random_tile();
	s2.r = 0;
	s2.r_true=0;
			
}//end function

state::state(){
	r = 0;
	has2048=false;
	has4096=false;
	has8192=false;
	has16384=false;
	has32768=false;
	has65536=false;
	has131072=false;
}//end function

void state::init_game_state(){ //Finished
	int i, j;
	
	for(i = 0; i < grid_size; i++)
		for(j = 0; j < grid_size; j++)
				grid[i][j] = 0;
	
	add_random_tile();
	add_random_tile();
}//end function

void state::add_random_tile(){
	int i, j, k;
	bool flag=false;
	k = rand() % 10;

	while (!flag){
		i = rand() % grid_size;
		j = rand() % grid_size;
		
		if (grid[i][j]==0){
			if(k < 1)
				grid[i][j] = 4;
			else
				grid[i][j] = 2;
			
			flag=true;
			return;
		}//end if
	}//end while
}//end function

void state::calc_v2vtiles(){ 
	//calculate number of v tile/2v-tile pairs. Checks every move for a corresponding 2v-tile
	v2vtiles=0;
	int i,j,k;
	for(i = 0; i < grid_size; i++){
		for(j = 0; j < grid_size; j++){
			if(grid[i][j] != 0 && (i!=0) && (grid[i][j] == grid[i-1][j] || grid[i-1][j] == 0)   ){ //up
				for(k = i-1; k > 0;k--){
					if (grid[k][j]==(grid[i][j]*2)){
						v2vtiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 && (i!=grid_size-1) && (grid[i][j] == grid[i+1][j] || grid[i+1][j] == 0)  ){ //down
				for(k = i+1; k < grid_size;k++){
					if (grid[k][j]==(grid[i][j]*2)){
						v2vtiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 &&  (j!=0) && (grid[i][j] == grid[i][j-1] || grid[i][j-1] == 0) ){ //left
				for(k = j-1; k > 0;k--){
					if (grid[i][k]==(grid[i][j]*2)){
						v2vtiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 && (j!=grid_size-1) && (grid[i][j] == grid[i][j+1] || grid[i][j+1] == 0)  ){ //right
				for(k = j+1; k < grid_size;k++){
					if (grid[i][k]==(grid[i][j]*2)){
						v2vtiles++;break;
					}//end if
				}//end for
			}//end if
			
		}//end for
	}//end for
}//end function

void state::calc_monotonicity(){
	//did not understand github code at all
	//but, the goal is to enforce good position in the row/column
	int i,j;
}

void state::calc_emptytiles(){
	emptytiles=0;
	int i,j;
	for(i = 0; i < grid_size;i++){
		for(j=0;j<grid_size;j++){
			if (grid[i][j]==0){
				emptytiles++;
			}//end if
		}//end for
	}//end for
}//end function

void state::calc_mergeabletiles(){ //function verified
	mergeabletiles=0;
	int i,j,k;
	for(i = 1; i < grid_size; i++){
		for(j = 0; j < grid_size; j++){
			if(grid[i][j] != 0 && (i!=0) && (grid[i][j] == grid[i-1][j] || grid[i-1][j] == 0)   ){ //up
				for(k = i-1; k > 0;k--){
					if (grid[k][j]==(grid[i][j])){
						mergeabletiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 && (i!=grid_size-1) && (grid[i][j] == grid[i+1][j] || grid[i+1][j] == 0)  ){ //down
				for(k = i+1; k < grid_size;k++){
					if (grid[k][j]==(grid[i][j])){
						mergeabletiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 &&  (j!=0) && (grid[i][j] == grid[i][j-1] || grid[i][j-1] == 0) ){ //left
				for(k = j-1; k > 0;k--){
					if (grid[i][k]==(grid[i][j])){
						mergeabletiles++;break;
					}//end if
				}//end for
			}//end if

			if(grid[i][j] != 0 && (j!=grid_size-1) && (grid[i][j] == grid[i][j+1] || grid[i][j+1] == 0)  ){ //right
				for(k = j+1; k < grid_size;k++){
					if (grid[i][k]==(grid[i][j])){
						mergeabletiles++;break;
					}//end if
				}//end for
			}//end if

			
		}//end for
	}//end for
}//end function

void state::calc_validmoves(){ //function verified
	validmoves=0;bool up = false; bool down = false; bool left = false; bool right = false;
	int i,j;
	for(i = 1; i < grid_size; i++){
		for(j = 0; j < grid_size; j++){
			if (!up){
				if(grid[i][j] != 0 && (grid[i][j] == grid[i-1][j] || grid[i-1][j] == 0)){
					validmoves++;up=true;
				}//end if
			}

			if (!down){
				if(grid[i][j] != 0 && (grid[i][j] == grid[i+1][j] || grid[i+1][j] == 0)){
					validmoves++;down=true;
				}//end if
			}

			if (!left){
				if(grid[i][j] != 0 && (grid[i][j] == grid[i][j-1] || grid[i][j-1] == 0)){
					validmoves++;left=true;
				}//end if
			}

			if (!right){
				if(grid[i][j] != 0 && (grid[i][j] == grid[i][j+1] || grid[i][j+1] == 0)){
					validmoves++;right = true;
				}//end if
			}

			if (up && down && left && right){
				break;
			}
		}//end for
	}//end for
}//end function

void state::calc_distinct_tiles(){ //function verified
	distinct_tiles=0;
	vector<int> tiles;


	for (int i = 0; i < grid_size; i++){
		for(int j = 0; j < grid_size; j++){
			if(!(find(tiles.begin(), tiles.end(), grid[i][j]) != tiles.end()) && (grid[i][j]!=0)) {
    			distinct_tiles++;
			} else if (grid[i][j]!=0){
    			tiles.push_back(grid[i][j]);
			}//end if
		}//end for
	}//end for
}//end function

void state::check_big_tiles(){
	for(int i = 0 ; i < grid_size;i++){
		for(int j = 0; j < grid_size;j++){
			/*
			if (grid[i][j]==2048 && !has2048){
				cout << "2048 encountered.\n";
				has2048=true;
				print_state();
			}
			*/
			/*
			if (grid[i][j]==4096 && !has4096){
				cout << "4096 encountered.\n";
				has4096=true;
				print_state();
			}
			*/
			if (grid[i][j]==8192 && !has8192){
				cout << "8192 encountered.\n";
				has8192=true;
				print_state();
			}
			if (grid[i][j]==16384 && !has16384){
				cout << "16384 encountered.\n";
				has16384=true;
				print_state();
			}
			if (grid[i][j]==32768 && !has32768){
				cout << "32768 encountered.\n";
				has32768=true;
				print_state();
			}
			if (grid[i][j]==65536 && !has65536){
				cout << "65536 encountered.\n";
				has65536=true;
				print_state();
			}
			if (grid[i][j]==131072 && !has131072){
				cout << "131072 encountered.\n";
				has131072=true;
				print_state();
			}
		}//end for
	}//end for
}//end method

void state::print_state(){
	
	int i, j;
	
	cout << "\n";
	for(i = 0; i < grid_size;i++){
		for(j=0;j<grid_size;j++)
			cout << grid[i][j] << "\t";
		
		cout << "\n";
	}//end for
}//end function

Tuple::Tuple(){

	int i;

	for(i = 0; i < 50625; i++)
		LUT[i] = 0;
}//end function