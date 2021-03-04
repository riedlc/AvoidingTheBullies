/* The Network class Implementation (Network).cpp) */
#include "Network.h" // user-defined header in the same directory
#include <iostream>
#include <fstream>
#include <iterator>
#include <math.h>
#include <numeric>

// Constructor
// default values shall only be specified in the declaration,
// cannot be repeated in definition
Network::Network(int pop, float strategy_learning_speed, float network_learning_speed, float strategy_discount, float network_discount, float strategy_tremble, float network_tremble, bool strategy_sym, bool network_sym, float score_copy_prob, float copy_error, float explore_prob){

    this->pop = pop;
    double fill_value = 19.0/(pop-1);
    for(int i = 0; i < pop; i++){
        Agent A(i,fill_value,strategy_learning_speed, network_learning_speed, strategy_discount, network_discount, strategy_tremble, network_tremble, strategy_sym, network_sym, score_copy_prob, copy_error, explore_prob);
        
        // Create friend weights vector
        std::vector<double> myfriends(pop);
        std::fill(myfriends.begin(),myfriends.end(),fill_value);
        myfriends.at(i) = 0;
        A.setFriends(myfriends);
        
        A.updateAgent();
        
        agents.push_back(A);
        
       // for (auto i: agents.at(i).getStrats(0))
        //    std::cout << i << ' ';
    }
}

Network::Network(int pop, std::string strat_filepath, float strategy_learning_speed, float network_learning_speed, float strategy_discount, float network_discount, float strategy_tremble, float network_tremble, bool strategy_sym, bool network_sym, float score_copy_prob, float copy_error, float explore_prob){
    
    this->pop = pop;
    
    double net_fill = 19.0/(pop-1);
    
    std::ifstream stratstream(strat_filepath);
    std::istream_iterator<double> startstrat(stratstream), endstrat;
    std::vector<double> strat_matrix(startstrat, endstrat);
        
    for(int i = 0; i < pop; i++){
        Agent A(i,strat_matrix,strategy_learning_speed, network_learning_speed, strategy_discount, network_discount, strategy_tremble, network_tremble, strategy_sym, network_sym, score_copy_prob, copy_error, explore_prob);
        
        // Create friend weights vector
        std::vector<double> myfriends(pop);
        std::fill(myfriends.begin(),myfriends.end(),net_fill);
        myfriends.at(i) = 0;
        A.setFriends(myfriends);
        
        A.updateAgent();
        
        agents.push_back(A);
        
        // for (auto i: agents.at(i).getStrats(0))
        //    std::cout << i << ' ';
    }
}

Network::Network(std::string net_filepath, std::string strat_filepath, float strategy_learning_speed, float network_learning_speed, float strategy_discount, float network_discount, float strategy_tremble, float network_tremble, bool strategy_sym, bool network_sym, float score_copy_prob, float copy_error, float explore_prob) {  

    std::ifstream netstream(net_filepath);
    std::istream_iterator<double> startnet(netstream), endnet;
    std::vector<double> net_matrix(startnet, endnet);
    std::cout << "Read " << net_matrix.size() << " numbers" << std::endl;
    
    this->pop = pow(net_matrix.size(),0.5);
    
    // print the numbers to stdout
    std::cout << "numbers read in:\n";
    std::copy(net_matrix.begin(), net_matrix.end(), 
              std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;
    
    std::ifstream stratstream(strat_filepath);
    std::istream_iterator<double> startstrat(stratstream), endstrat;
    std::vector<double> strat_matrix(startstrat, endstrat);
    
    for(int i = 0; i < pop; i++){  
        Agent A(i,strat_matrix,strategy_learning_speed, network_learning_speed, strategy_discount, network_discount, strategy_tremble, network_tremble, strategy_sym, network_sym, score_copy_prob, copy_error, explore_prob);

        // Create friend weights vector
        std::vector<double>::const_iterator first = net_matrix.begin() + (i * pop);
        std::vector<double>::const_iterator last = net_matrix.begin() + ((i+1)*pop);
        std::vector<double> myfriends(first,last);
        
        A.setFriends(myfriends);
        
        A.updateAgent();
        
        agents.push_back(A);
    }
}

Agent& Network::GetAgent(std::vector<Agent>::size_type AgentNumber)
{
    return agents[AgentNumber];
}

void Network::AddAgent(const Agent& NewAgent)
{
    agents.push_back(NewAgent);
}

int Network::getPop(){
    return pop;
}

std::vector<Agent> Network::getAgents(){
    return agents;
}
