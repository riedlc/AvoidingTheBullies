/* The Network class Implementation (Network).cpp) */
#include "Network.h" // user-defined header in the same directory
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iterator>
#include <math.h>
#include <numeric>

std::vector<int> Game::num_strats;

// Constructor
// default values shall only be specified in the declaration,
// cannot be repeated in definition
Game::Game(std::vector<std::vector<double>> gamePayoffs,std::string gameName, double base_payoff, std::vector<int> num_strats, std::string coupling_effect, std::vector<double> fight_bonus){
    this->gamePayoffs = gamePayoffs;
    this->gameName = gameName;
    this->base_payoff = base_payoff;
    this->num_strats = num_strats;
    this->coupling_effect = coupling_effect;
    this->fight_bonus = fight_bonus;
}
Game::Game(std::string payoff_filepath, std::string gameName, double base_payoff, std::string coupling_effect){
    this->gameName = gameName;
    this->base_payoff = base_payoff;
    this->coupling_effect = coupling_effect;

    std::ifstream infile(payoff_filepath);
    
    std::string line;
    
    int i = 0;
    
    while (std::getline(infile, line))
    {
        std::istringstream iss(line);
                
        if(i == 0){
            std::istream_iterator<int> start(iss), end;
            std::vector<int> this_line(start, end);
            num_strats = this_line;
        }else if(i < 3){
            std::istream_iterator<double> start(iss), end;
            std::vector<double> this_line(start, end);
            gamePayoffs.push_back(this_line);
        }else{
            std::istream_iterator<double> start(iss), end;
            std::vector<double> this_line(start, end);
            fight_bonus = this_line;
        }
        i++;
    }
    
}

std::string Game::getName(){
    return gameName;
}

void Game::setPayoffs(std::vector<std::vector<double>> gamePayoffs){
    this->gamePayoffs = gamePayoffs;
}

std::vector<std::vector<double>> Game::getPayoffs(){
    return gamePayoffs;
}

double Game::getBasePayoff(){
    return base_payoff;
};

std::vector<double> Game::playGame(UGenerator rng, Agent &visitor, Agent &host){
    std::vector<double> these_payoffs(2);
    
    double visitScore = visitor.getScore();
    double hostScore = host.getScore();
    
    int visitStrategy = visitor.getCurrentStrategy();
    int hostStrategy = host.getCurrentStrategy();
    
    int interaction_number = 2 * visitStrategy + hostStrategy;
    
    visitor.updateInteractions(interaction_number);
    host.updateInteractions(interaction_number);
    
    std::vector<double> bonus_vec;
    
    if(coupling_effect == "Fight"){
        bonus_vec = FightBonus(visitStrategy, hostStrategy, visitScore, hostScore);
    }else if(coupling_effect == "FightSplit"){
        bonus_vec = FightSplitBonus(visitStrategy, hostStrategy, visitScore, hostScore);
    }else if(coupling_effect == "FightRand"){
        bonus_vec = FightRand(rng,visitStrategy, hostStrategy, visitScore, hostScore);
    }else if(coupling_effect == "StagHunt"){
        bonus_vec = StagHuntBonus(visitStrategy, hostStrategy, visitScore, hostScore);
    }else if(coupling_effect == "FSH"){
        bonus_vec = FSHBonus(visitStrategy, hostStrategy, visitScore, hostScore);
    }else if(coupling_effect == "None"){
        bonus_vec = {0.0,0.0};
    }
    
    double visitPayoff = gamePayoffs.at(0).at(visitStrategy * 2 + hostStrategy) + base_payoff + bonus_vec.at(0);
    double hostPayoff = gamePayoffs.at(1).at(visitStrategy * 2 + hostStrategy) + base_payoff + bonus_vec.at(1);
    
    if(visitPayoff < base_payoff){
        visitPayoff = base_payoff;
    }
    if(hostPayoff < base_payoff){
        hostPayoff = base_payoff;
    }
    
    
    these_payoffs.at(0) = visitPayoff;
    these_payoffs.at(1) = hostPayoff;
    
    visitor.setCurrentPayoff(visitPayoff);
    host.setCurrentPayoff(hostPayoff);
    
    visitor.recordInteraction(visitPayoff,hostPayoff);
    
    // Function uses A.getScore() and B.getScore() to affect payoffs of each player
    return these_payoffs;
}

std::vector<double> Game::StagHuntBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore){
    
    double score_diff = visitScore - hostScore;
    double score_total = (visitScore + hostScore)/2;

    return {(-0.6 + score_total - score_diff * (bool) (score_diff > 0)) * (bool) (friendAgentStrategy + currentAgentStrategy == 2), (-0.6 + score_total + score_diff * (bool) (score_diff < 0)) * (bool) (friendAgentStrategy + currentAgentStrategy == 2)};
}
std::vector<double> Game::FightBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore){
    
    double visit_advantage;
    double host_advantage;
    
    if(visitScore > hostScore){
        //visit_advantage = -log(1.01-visitScore)/(0.9*(-log(1.01-visitScore)-log(1.01-hostScore)));
        //host_advantage = 1 - visit_advantage;
        visit_advantage = fight_bonus.at(0);
        host_advantage = 0;
    }
    else if(hostScore > visitScore){
        //host_advantage = -log(1.01-hostScore)/(0.9*(-log(1.01-visitScore)-log(1.01-hostScore)));
        //visit_advantage = 1 - host_advantage;
        visit_advantage = 0;
        host_advantage = fight_bonus.at(0);
    }else{
        visit_advantage = fight_bonus.at(1);
        host_advantage = fight_bonus.at(1);
    }
    
    return {visit_advantage * (bool) (friendAgentStrategy + currentAgentStrategy == 0),host_advantage * (bool) (currentAgentStrategy + friendAgentStrategy == 0)};
}
std::vector<double> Game::FightRand(UGenerator rng, int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore){
    
    double visit_advantage;
    double host_advantage;
    
    int prob_power = 6;
    
    double denom_total = pow(visitScore,prob_power) + pow(hostScore,prob_power);
    double visit_prob = pow(visitScore,prob_power)/denom_total;
    
    if(rng() < visit_prob){
        //visit_advantage = -log(1.01-visitScore)/(0.9*(-log(1.01-visitScore)-log(1.01-hostScore)));
        //host_advantage = 1 - visit_advantage;
        visit_advantage = fight_bonus.at(0);
        host_advantage = 0;
    }
    else{
        //host_advantage = -log(1.01-hostScore)/(0.9*(-log(1.01-visitScore)-log(1.01-hostScore)));
        //visit_advantage = 1 - host_advantage;
        visit_advantage = 0;
        host_advantage = fight_bonus.at(0);
    }
    return {visit_advantage * (bool) (friendAgentStrategy + currentAgentStrategy == 0),host_advantage * (bool) (currentAgentStrategy + friendAgentStrategy == 0)};
}
std::vector<double> Game::FightSplitBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore){
    
    double visit_prop;
    double host_prop;
    double visit_advantage;
    double host_advantage;
    
    if(visitScore > hostScore){
        visit_prop = -log(1.001-visitScore)/((-log(1.001-visitScore)-log(1.001-hostScore)));
        host_prop = 1 - visit_prop;
        
        visit_advantage = visit_prop * fight_bonus.at(0);
        host_advantage = host_prop * fight_bonus.at(0);
    }
    else if(hostScore > visitScore){
        host_prop = -log(1.001-hostScore)/((-log(1.001-visitScore)-log(1.001-hostScore)));
        visit_prop = 1 - host_prop;
        
        host_advantage = host_prop * fight_bonus.at(0);
        visit_advantage = visit_prop * fight_bonus.at(0);
    }else{
        visit_advantage = fight_bonus.at(1);
        host_advantage = fight_bonus.at(1);
    }
    
    return {visit_advantage * (bool) (friendAgentStrategy + currentAgentStrategy == 0),host_advantage * (bool) (currentAgentStrategy + friendAgentStrategy == 0)};
}
std::vector<double> Game::FSHBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore){
    
    double score_diff = visitScore - hostScore;
    double score_total = (visitScore + hostScore)/2;

    return {((0.25 + score_diff) * (bool) (friendAgentStrategy + currentAgentStrategy == 0)) + (-0.6 + score_total - score_diff * (bool) (score_diff > 0)) * (bool) (friendAgentStrategy + currentAgentStrategy == 2), ((0.25 - score_diff) * (bool) (currentAgentStrategy + friendAgentStrategy == 0)) + (-0.6 + score_total + score_diff * (bool) (score_diff < 0)) * (bool) (friendAgentStrategy + currentAgentStrategy == 2)};
}


