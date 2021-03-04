#define NK_LEN 1048576

/* The Agent class Implementation (Agent).cpp) */
#include "Network.h" // user-defined header in the same directory
#include <iostream>
#include <fstream>
#include <iterator>
#include <bitset>
#include <math.h>
#include <numeric>
#include <algorithm>
#include <vector>
#include <tr1/functional>

// Constructor
// default values shall only be specified in the declaration,
// cannot be repeated in definition
Agent::Agent(int agent_id, double fill_value, float strategy_learning_speed, float network_learning_speed, float strategy_discount, float network_discount, float strategy_tremble, float network_tremble, bool strategy_sym, bool network_sym, float score_copy_prob, float copy_error, float explore_prob){
    this->agent_id = agent_id;
    this->network_learning_speed = network_learning_speed;
    this->strategy_discount = strategy_discount;
    this->network_discount = network_discount;
    this->strategy_tremble = strategy_tremble;
    this->network_tremble = network_tremble;
    this->strategy_sym = strategy_sym;
    this->network_sym = network_sym;
    this->score_copy_prob = score_copy_prob;
    this->copy_error = copy_error;
    this->explore_prob = explore_prob;
    
    for(int k = 0; k < 4; k ++){
        my_interactions.push_back(0);
    }
    
    if(network_learning_speed == 0){
        this->network_discount = 0;
    }
    if(strategy_learning_speed == 0){
        this->strategy_discount = 0;
    }
        
    for(size_t i = 0; i<Game::num_strats.size(); i++)
    {
        std::vector<double> myvector;
        for(int j = 0; j<Game::num_strats.at(i); j++)
        {
            myvector.push_back(fill_value);
        }
        new_strategy_profile.push_back(myvector);
    }
    cur_strategy_profile = new_strategy_profile;
    cur_score = 0;
    new_score = 0;
}

Agent::Agent(int agent_id, std::vector<double> fill_values, float strategy_learning_speed, float network_learning_speed, float strategy_discount, float network_discount, float strategy_tremble, float network_tremble, bool strategy_sym, bool network_sym, float score_copy_prob, float copy_error, float explore_prob){
    this->agent_id = agent_id;
    this->strategy_learning_speed = strategy_learning_speed;
    this->network_learning_speed = network_learning_speed;
    this->strategy_discount = strategy_discount;
    this->network_discount = network_discount;
    this->strategy_tremble = strategy_tremble;
    this->network_tremble = network_tremble;
    this->strategy_sym = strategy_sym;
    this->network_sym = network_sym;
    this->score_copy_prob = score_copy_prob;
    this->copy_error = copy_error;
    this->explore_prob = explore_prob;
    
    for(int k = 0; k < 4; k ++){
        my_interactions.push_back(0);
    }
    
    if(this->network_learning_speed == 0){
        this->network_discount = 0;
    }
    if(this->strategy_learning_speed == 0){
        this->strategy_discount = 0;
    }
    
    int num_strats_i = (int) Game::num_strats.size();
    int num_strats_j;
    int index;
    
    for(int i = 0; i<num_strats_i; i++){
        num_strats_j = (int) Game::num_strats.at(i);
        
        std::vector<double> myvector;
        for(int j = 0; j<num_strats_j; j++)
        {
            index = agent_id * num_strats_i * num_strats_j + i * num_strats_i + j;
            myvector.push_back(fill_values.at(index));
        }
        new_strategy_profile.push_back(myvector);
    }
    cur_strategy_profile = new_strategy_profile;
}

std::vector<double> Agent::getStrats(int strat_num){
    return cur_strategy_profile.at(strat_num);
}

void Agent::discountStrategy(int strat_num){
    std::transform(new_strategy_profile.at(strat_num).begin(),new_strategy_profile.at(strat_num).end(),new_strategy_profile.at(strat_num).begin(), std::bind2nd(std::multiplies<double>(),(1-strategy_discount)));
}

void Agent::discountNeighbors(){
    int pop = cur_friends.size();
    
    //Iterate over neighbors
    for(int nid = 0; nid < pop; nid++)
    {
        // Discount neighbors
        new_friends.at(nid) = new_friends.at(nid) * (1-network_discount);
    }
}

void Agent::setFriends(std::vector<double> friends){
    new_friends = friends;
}

std::vector<double> Agent::getFriends() const{
    return cur_friends;
}


void Agent::updateAgent(int t){
    cur_friends = new_friends;
    cur_strategy_profile = new_strategy_profile;
    
    if(t % 1 == 0){
        cur_score = new_score;
        perceived_cur_score = perceived_new_score;
    }
}

int Agent::chooseFriend(UGenerator rng, std::vector<int> temp_agent_seq){
    
    int friend_ind = -1; // Flag (goes >= 0 as the index) for when the neighbor is picked
    int nid;
    float rand_tremble = rng();
    
    int pop = cur_friends.size();
    
    // If agent doesn't make an error
    if(rand_tremble > network_tremble){
        std::vector<double> sum_vec(pop);
        //float sum_vec[pop];
        
        std::partial_sum(cur_friends.begin(), cur_friends.end(), sum_vec.begin());
        
        double interaction_random_draw = rng() * sum_vec[pop-1];
        
        //Iterate over neighbors
        for(nid = 0; nid < pop; nid++)
        {
            // Add viable neighbor
            // If it's time to copy
            
            if(friend_ind == -1 && interaction_random_draw <= sum_vec.at(nid) && nid != agent_id){
                friend_ind = nid; // This is agents partner
            }
            // Discount neighbors
            new_friends.at(nid) = new_friends.at(nid) * (1-network_discount);
        }
    }else{ // If agent makes an error
        for(nid = 0; nid < pop; nid++)
        {
            // Discount all neighbors
            new_friends.at(nid) = new_friends.at(nid) * (1-network_discount);
        }
        // Choose random neighbor
        int temp_friend_ind = (int) (rng() * (pop-1));
        friend_ind = temp_agent_seq.at(temp_friend_ind);
        
    }
    currentFriend = friend_ind;
    
    return friend_ind;
}


// Setters
void Agent::updateInteractions(int inter_number){
    my_interactions.at(inter_number) = my_interactions.at(inter_number) + 1;
}

void Agent::setStrategyLearning(float strategy_learning_speed){
    this->strategy_learning_speed = strategy_learning_speed;
}
void Agent::setNetworkLearning(float network_learning_speed){
    this->network_learning_speed = network_learning_speed;
}

void Agent::setStrategyDiscount(float strategy_discount){
    this->strategy_discount = strategy_discount;
}
void Agent::setNetworkDiscount(float network_discount){
    this->network_discount = network_discount;
}

void Agent::setStrategyTremble(float strategy_tremble){
    this->strategy_tremble = strategy_tremble;
}
void Agent::setNetworkTremble(float network_tremble){
    this->network_tremble = network_tremble;
}

void Agent::setStrategySym(bool strategy_sym){
    this->strategy_sym = strategy_sym;
}
void Agent::setNetworkSym(bool network_sym){
    this->network_sym = network_sym;
}

void Agent::setScoreCopyProb(float score_copy_prob){
    this->score_copy_prob = score_copy_prob;
}

void Agent::setCopyError(float copy_error){
    this->copy_error = copy_error;
}

void Agent::setExploreProb(float explore_prob){
    this->explore_prob = explore_prob;
}

void Agent::addStrategyPayoff(int send_rec){
    
    new_strategy_profile.at(send_rec).at(currentStrategy) =  new_strategy_profile.at(send_rec).at(currentStrategy) + currentPayoff * strategy_learning_speed;
    
    new_score = new_score + currentPayoff;
    
}

void Agent::addNetworkPayoff(){
    new_friends.at(currentFriend) = new_friends.at(currentFriend) + currentPayoff * network_learning_speed;
}

void Agent::setCurrentFriend(int friend_id){
    currentFriend = friend_id;
}

void Agent::chooseStrategy(UGenerator rng, int send_rec){
    
    int strat_draw;
    
    double tremble_strat_draw;
    double weight_strat_draw;
    double tremble_draw = rng();
    
    if(tremble_draw < strategy_tremble){
        tremble_strat_draw = rng();
        strat_draw = (int) (tremble_strat_draw + 0.5);
    }else{
        std::vector<double> strat_sum_vec(2);
        
        std::partial_sum(cur_strategy_profile.at(send_rec).begin(), cur_strategy_profile.at(send_rec).end(), strat_sum_vec.begin());
        
        weight_strat_draw = rng();
        if(weight_strat_draw * strat_sum_vec.at(1) < strat_sum_vec.at(0)){
            strat_draw = 0;
        }else{
            strat_draw = 1;
        }
    }
    
    currentStrategy = strat_draw;
    
}

void Agent::setCurrentPayoff(double currentPayoff){
    this->currentPayoff = currentPayoff;
}

void Agent::recordInteraction(double visitPayoff, double hostPayoff){
    past_p1_payoff = visitPayoff;
    past_p2_payoff = hostPayoff;
    last_visit = currentFriend;
}
void Agent::setInitScore(UGenerator rng){
    cur_score = rng();
    new_score = cur_score;
}

//Getters
std::vector<int> Agent::getInteractions(){
    return my_interactions;
}
int Agent::getID(){
    return agent_id;
}
float Agent::getStrategyLearning(){
    return strategy_learning_speed;
}
float Agent::getNetworkLearning(){
    return network_learning_speed;
}

float Agent::getStrategyDiscount(){
    return strategy_discount;
}
float Agent::getNetworkDiscount(){
    return network_discount;
}

float Agent::getStrategyTremble(){
    return strategy_tremble;
}
float Agent::getNetworkTremble(){
    return network_tremble;
}

bool Agent::getStrategySym(){
    return strategy_sym;
}
bool Agent::getNetworkSym(){
    return network_sym;
}

float Agent::getScoreCopyProb(){
    return score_copy_prob;
}

float Agent::getCopyError(){
    return copy_error;
}

float Agent::getExploreProb(){
    return explore_prob;
}

int Agent::getCurrentStrategy(){
    return currentStrategy;
}

double Agent::getScore(){
    return cur_score;
}

int Agent::getLocationInt(){
    return cur_location_int;
}

std::bitset<20> Agent::getLocationBin(){
    return cur_location_bin;
}

double Agent::getPastVisitPayoff(){
    return past_p1_payoff;
}

double Agent::getPastHostPayoff(){
    return past_p2_payoff;
}

int Agent::getPastVisitPartner(){
    return last_visit;
}
double Agent::getCurrentPayoff(){
    return currentPayoff;
}

double Agent::getTotalPayoff(){
    return cur_score;
}

// Helpers

int Agent::convertBin2Dec(std::bitset<20> bin){
    return static_cast<int>(bin.to_ulong());
}


