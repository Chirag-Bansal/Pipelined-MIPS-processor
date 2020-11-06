#include <iostream>
#include <bits/stdc++.h> 
#include <fstream>
#include <string> // for string and to_string() 
#include <vector>
#include <queue>
#include <stack>

using namespace std;

bool control_hazard = false;

bool data_hazard = false;

int data_hazard_registor = 0;

typedef struct helpStall
{
	string function;
	int regNumber;


	helpStall(string func, int id){
		this->function = func;
		this->regNumber = id;
	}

}helpStall;

typedef struct Instruction
{
	std::vector<string> tokens;
	
	bool empty;

	bool r_type;
	bool branch;
	bool jump;
	bool memory_related;

	int id;

	int clock_cycles;
	int registor[3];

	int val;
	
	string function;

	int stage_value;
}Instruction;



string memory[4096];
int registors[32];


unordered_map<int,Instruction*> registor_map;

int numOfInstructions = 0;

int ClockCycles = 0;
int i = 0;
int programCounter = 0;

int addr(int first, int second){
	return registors[first]+registors[second];
}
int subr(int first, int second){
	return registors[first]-registors[second];
}
int multr(int first, int second){
	return registors[first]*registors[second];
}
int divr(int first, int second){
	return registors[first]/registors[second];
}


Instruction* fetch(int i){
	string line = memory[i];

	cout << "Fetching Instruction: " << i << endl;
	Instruction* ins = new Instruction();
	
	ins->id = i;
	ins->stage_value = 1;

	stringstream check1(line);
	string intermediate; 

    while(getline(check1, intermediate, ' ')) { 
      	ins->tokens.push_back(intermediate); 
    }

    return ins;
}

void decode(Instruction* ins){

	string function = ins->tokens[0];

	cout << "Decoding Instruction : " << ins->id << endl;

	ins->stage_value = 2;

	if(function == "add" || function == "sub" || function == "div" || function == "mult"){
		ins->r_type = true;
		ins->branch = false;
		ins->jump = false;
		ins->memory_related = false;
		ins->registor[0] = stoi(ins->tokens[1]);
	    ins->registor[1] = stoi(ins->tokens[2]);
	    ins->registor[2] = stoi(ins->tokens[3]);

	    registor_map[ins->registor[0]] = ins;

	    if(registor_map[ins->registor[1]] != NULL){
	    	data_hazard = true;
	    	data_hazard_registor = ins->registor[1];
	    }
	    if (registor_map[ins->registor[2]] != NULL) {
	    	data_hazard = true;
	    	data_hazard_registor = ins->registor[2];
	    }

	}
	else if(function == "beq" || function == "bne" || function == "bgtz" || function == "blez"){
		ins->r_type = false;
		ins->branch = true;
		ins->jump = false;
		ins->memory_related = false;
		if(function == "beq" || function == "bne"){
			ins->registor[0] = stoi(ins->tokens[1]);
		    ins->registor[1] = stoi(ins->tokens[2]);
		    ins->registor[2] = stoi(ins->tokens[3]);
		    if(registor_map[ins->registor[1]] != NULL){
		    	data_hazard = true;
		    	data_hazard_registor = ins->registor[1];
		    }
		    if (registor_map[ins->registor[0]] != NULL) {
		    	data_hazard = true;
		    	data_hazard_registor = ins->registor[0];
		    }
		}
		else if(function == "bgtz" || function == "blez"){
			ins->registor[0] = stoi(ins->tokens[1]);
		    ins->registor[1] = stoi(ins->tokens[2]);
		    if (registor_map[ins->registor[0]] != NULL) {
		    	data_hazard = true;
		    	data_hazard_registor = ins->registor[0];
		    }
		}
	}
	else if(function == "j" || function == "jal" || function == "jr"){
		ins->r_type = false;
		ins->branch = false;
		ins->jump = true;
		ins->memory_related = false;
		if(function != "jr" ) ins->registor[0] = stoi(ins->tokens[1]);

		if(function == "j"){
			i = ins->registor[0]-1;
		}
		else if(function == "jal"){
			i = ins->registor[0]-1;
			registors[31] = i;
		}
		else if(function == "jr"){
			i = registors[31]-1;
		}

		control_hazard = true;
	}
	else if(function == "lw" || function == "sw"){
		ins->r_type = false;
		ins->branch = false;
		ins->jump = false;
		ins->memory_related = true;
		ins->registor[0] = stoi(ins->tokens[1]);
	    ins->registor[1] = stoi(ins->tokens[2]);
	    ins->registor[2] = stoi(ins->tokens[3]);

	    registor_map[ins->registor[0]] = ins;

	    if (registor_map[ins->registor[1]] != NULL) {
		    	data_hazard = true;
		    	data_hazard_registor = ins->registor[1];
		}
	}
	else if(function == "li"){
		ins->r_type = true;
		ins->branch = false;
		ins->jump = false;
		ins->memory_related = false;
		ins->registor[0] = stoi(ins->tokens[1]);
		ins->registor[1] = stoi(ins->tokens[2]);

    	registor_map[ins->registor[0]] = ins;
	}
	else{
		// These are functions srl and sll
		ins->r_type = true;
		ins->branch = false;
		ins->jump = false;
		ins->memory_related = false;
		ins->registor[0] = stoi(ins->tokens[1]);
	    ins->registor[1] = stoi(ins->tokens[2]);
	    ins->registor[2] = stoi(ins->tokens[3]);

	    registor_map[ins->registor[0]] = ins;

	    if(registor_map[ins->registor[1]] != NULL){
	    	data_hazard = true;
	    	data_hazard_registor = ins->registor[1];
	    }
	    if (registor_map[ins->registor[2]] != NULL) {
	    	data_hazard = true;
	    	data_hazard_registor = ins->registor[2];
	    }
	}

	ins->clock_cycles = stoi(ins->tokens[ins->tokens.size()-1]);
	ins->function = function;
}


void execute_instruction(Instruction* ins){

	cout << "Executing Instruction: " << ins->id << endl;

	ins->stage_value = 3;

	string function = ins->function;

	if(function == "add"){
	    	ins->val = addr(ins->registor[1],ins->registor[2]);
	}
    else if(function == "sub"){
	    	ins->val = subr(ins->registor[1],ins->registor[2]);
	}
	else if(function == "div"){
	    	ins->val = divr(ins->registor[1],ins->registor[2]);
	}
	else if(function == "mult"){
	    	ins->val = multr(ins->registor[1],ins->registor[2]);
	}
	else if(function == "srl"){
	   		ins->val = registors[ins->registor[1]] * pow(2,-1*ins->registor[2]);
	}

	else if(function == "sll"){
	   	ins->val = registors[ins->registor[1]] * pow(2,ins->registor[2]);
	}

	else if(function == "li"){
		ins->val = ins->registor[1];
	}
	// In the branch functions i am assuming ins equal to 0 if they are satisfied
	else if(function == "beq"){
		ins->val = registors[ins->registor[0]] - registors[ins->registor[1]];
		if(ins->val == 0){
			i = i+ins->registor[2]-1;
			control_hazard = true;
		}
	}
	else if(function == "bne"){
		int temp = registors[ins->registor[0]] - registors[ins->registor[1]];
		ins->val = (temp != 0)? 0: 1;
		if(ins->val == 0){
			i = i+ins->registor[2]-1;
			control_hazard = true;
		}
	}
	else if(function == "bgtz"){
		int temp = registors[ins->registor[0]];
		ins->val = (temp > 0)? 0: 1;
		if(ins->val == 0){
			i = i+ins->registor[1]-1;
			control_hazard = true;
		}
	}
	else if(function == "blez"){
		int temp = registors[ins->registor[0]];
		ins->val = (temp <= 0)? 0: 1;
		if(ins->val == 0){
			i = i+ins->registor[2]-1;
			control_hazard = true;
		}
	}
	else if(function == "lw"){
		ins->val = 1000 + registors[ins->registor[1]] + ins->registor[2];
	}
	else if(function == "sw"){
		ins->val = 1000 + registors[ins->registor[1]] + ins->registor[2];
	}

	programCounter++;
	ClockCycles += ins->clock_cycles;
}

void memory_access(Instruction* ins){
	cout << "Memory Access for Instruction : " << ins->id << endl;

	ins->stage_value = 4;
	
	if(!ins->memory_related) return;

	string function = ins->function;

	if(function == "sw")
	{
		int vale = registors[ins->registor[0]];
		memory[ins->val] = to_string(vale);
	}
}

void write_back(Instruction* ins){

	cout << "Writing Back Instruction " << ins->id << endl;

	if(!ins->r_type && !(ins->function == "lw")) return;

	string function = ins->function;

	ins->stage_value = 5;

	if(ins->function == "lw")
	{
		if(memory[ins->val]=="")
			registors[ins->registor[0]] = 0;
		else
		{
			int vale = stoi(memory[ins->val]);
			registors[ins->registor[0]] = vale;
		}

		registor_map[ins->registor[0]] = NULL;
	}
	else if(ins->function == "li"){
		registors[ins->registor[0]] = ins->val;
		std::cout << "The value in the register " << ins->registor[0] << " is " << registors[ins->registor[0]] << '\n';
		registor_map[ins->registor[0]] = NULL;
	}
	else if(ins->function == "srl" || ins->function == "sll"){
		registors[ins->registor[0]] = ins->val;
		std::cout << "The value in the register " << ins->registor[0] << " is " << registors[ins->registor[0]] << '\n';
	   	std::cout << "The value in the register " << ins->registor[1] << " is " << registors[ins->registor[1]] << '\n';

	   	registor_map[ins->registor[0]] = NULL;
	}
	else{
		registors[ins->registor[0]] = ins->val;
		std::cout << "The value in the register " << ins->registor[0] << " is " << registors[ins->registor[0]] << '\n';
	   	std::cout << "The value in the register " << ins->registor[1] << " is " << registors[ins->registor[1]] << '\n';
	   	std::cout << "The value in the register " << ins->registor[2] << " is " << registors[ins->registor[2]] << '\n';

	   	registor_map[ins->registor[0]] = NULL;
	}

}

void execute(){

	Instruction* Instruction_first;
	Instruction* temp;

	queue<Instruction*> jobs;
	queue<Instruction*> buffer;

	if(numOfInstructions==0)
		return;	
	else{

		int cycle = 0;
		bool check2 = false; // check for data hazard

		while(i < numOfInstructions){

			cout << "This is cycle: " << cycle << endl;

			if(!check2)
			{
				Instruction_first = fetch(i++);
				jobs.push(Instruction_first);
			}

			while(!jobs.empty()){

				temp = jobs.front();
				jobs.pop();

				if(temp->stage_value==1){
					if(!control_hazard){
						decode(temp);
						if(data_hazard)
						{
							Instruction* hazardous_instruction = registor_map[data_hazard_registor];

							string function = hazardous_instruction->function;

							if(function == "lw"){
								check2 = true;
								temp->stage_value = 1;
								data_hazard = false;
							}
							else{
								int destination_registor = hazardous_instruction->registor[0];
								int value = hazardous_instruction->val;
								
								if(hazardous_instruction->stage_value >=3){
									registors[destination_registor] = value;
									data_hazard = false;
								}
								else{
									check2 = true;
									temp->stage_value = 1;
									data_hazard = false;
								}
							}
							
						}
						else
						{
							check2 = false;
						}
						buffer.push(temp);				
					}
					else
					{
						control_hazard = false;
					}
				}
				else if(temp->stage_value==2){
					execute_instruction(temp);
					buffer.push(temp);
				}
				else if(temp->stage_value==3){
					memory_access(temp);
					buffer.push(temp);
				}
				else if(temp->stage_value==4){
					write_back(temp);
				}
			}

			while(!buffer.empty()){
				jobs.push(buffer.front());
				buffer.pop();
			}

			cout << endl;
		
		cycle++;

		}

		while(!jobs.empty())
		{
			temp = jobs.front();
			jobs.pop();

			if(temp->stage_value==1){
				decode(temp);
				jobs.push(temp);
			}
			else if(temp->stage_value==2){
				execute_instruction(temp);
				jobs.push(temp);
			}
			else if(temp->stage_value==3){
				memory_access(temp);
				jobs.push(temp);
			}
			else if(temp->stage_value==4){
				write_back(temp);
			}
		}

		ClockCycles = cycle-1;
	}	

}

void read_to_memory(){
	ifstream myfile;
	string line;
	myfile.open ("input.txt");
	  
	if (myfile.is_open()){
		int i = 0;
		while (getline (myfile,line) ){
		  	memory[i] = line;
		  	i++;
		}
		numOfInstructions = i;
		myfile.close();
	}
}

void initialise(){
	// INIIALISE ALL THE REGISTORS HERE ...
	registors[1] = 5;
	registors[2] = 10;
	registors[3] = 20;
	registors[4] = 30;
	registors[10] = 1000;

  for(int i = 5; i < 31; i++) 
  {
  	if(i!=10)	
  	registors[i] = 0;
  }
}


int main(){

  read_to_memory();

  initialise();

  execute();

  float IPC = (float)programCounter/(float)ClockCycles;

	std::cout << "The number of Instructions executed are : " << programCounter << "\n";
	std::cout << "The number of clock cycles taken are : " << ClockCycles << "\n";
	std::cout << "The Instructions per cycle is : " << IPC << '\n';

  return 0;
}