/* The Network class Header (Network.h) */
#include <stdio.h>
#include <string>   // using string
#include <vector>
#include <bitset>
#include <fstream>
#include <memory>
#include <iostream>
#include <cmath>
#include <boost/range/numeric.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>

typedef boost::mt19937 Engine;
typedef boost::uniform_real<double> UDistribution;
typedef boost::variate_generator< Engine &, UDistribution > UGenerator;
typedef boost::normal_distribution<double> NDistribution;   // Normal Distribution
typedef boost::variate_generator<Engine &, NDistribution > NGenerator;    // Variate generator

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args){
    size_t size = 1 + std::snprintf(nullptr, 0, format.c_str(), args ...);
    std::unique_ptr<char[]> buf(new char[size]);
    snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size);
}

// Random number generator
struct MersenneRNG {
    MersenneRNG() : dist(0.0, 1.0), rng(eng, dist) {}
    
    Engine eng;
    UDistribution dist;
    UGenerator rng;
};

//Agent Class

class Agent{
    private:
    
        // Neighbors with weights
        std::vector<double> cur_friends;
        std::vector<double> new_friends;
    
        // Strategies with weights (vector of 2 size 2 vectors)
        std::vector<std::vector<double>> cur_strategy_profile;
        std::vector<std::vector<double>> new_strategy_profile;
    
        std::vector<int> my_interactions;
    
        // Innovation space location
        int cur_location_int;
        int new_location_int;
        std::bitset<20> cur_location_bin;
        std::bitset<20> new_location_bin;
        
        // Innovation space score at current location
        double cur_score;
        double new_score;
    
        // Perceived score
        double perceived_cur_score;
        double perceived_new_score;
        
        // Agent id
        int agent_id;
    
        // Track last interaction
        double past_p1_payoff;
        double past_p2_payoff;
    
        double total_payoff;
    
        // Last strategy played
        int currentStrategy;
        // Last payoff earned
        double currentPayoff;
        // Last friend visited
        int currentFriend;
        int last_visit;
    
        // Learning Speeds
        float strategy_learning_speed;
        float network_learning_speed;
        
        // Past Discounting (Memory)
        float strategy_discount;
        float network_discount;
        
        // Tremble (Error rate)
        float strategy_tremble;
        float network_tremble;
        
        // Symmetry (For strategy: Are strategies in different contexts (home/away) correlated, for network: are directed network links (in/out) correlated).
        bool strategy_sym;
        bool network_sym;
        
        // Innovation variables
        float score_copy_prob; // Probability of copying neighbor
        float copy_error; // Error in copying different bits
        float explore_prob; // Probability of exploring
    
        int rounds_since_copy;
    public:
        Agent(const int agent_id, double fill_value = 1, float strategy_learning_speed = 1, float network_learning_speed = 1, float strategy_discount = 0.01, float network_discount = 0.01, float strategy_tremble = 0.01, float network_tremble = 0.01, bool strategy_sym = 0, bool network_sym = 0, float score_copy_prob = 0.1, float copy_error = 0.1, float explore_prob = 1);
        Agent(const int agent_id, std::vector<double> fill_values, float strategy_learning_speed = 1, float network_learning_speed = 1, float strategy_discount = 0.01, float network_discount = 0.01, float strategy_tremble = 0.01, float network_tremble = 0.01, bool strategy_sym = 0, bool network_sym = 0, float score_copy_prob = 0.1, float copy_error = 0.1, float explore_prob = 1);
        
        // Get Agent ID (shouldn't be necessary)
        int getID();
        
        // Get Agent strategy profile for given strategy set (defined by strat_num)
        std::vector<double> getStrats(int strat_num);
        void discountStrategy(int strat_num);
        void discountNeighbors();
    
        std::vector<int> getInteractions();
    
        // Set Agent friends from network
        void setFriends(std::vector<double> friends);
        std::vector<double> getFriends() const;
        
        void updateAgent();
    
        int chooseFriend(UGenerator rng,std::vector<int> temp_agent_seq);
        int getCurrentFriend();
        void setCurrentFriend(int friend_id);
    
        void chooseStrategy(UGenerator rng, int strategy_role);
        int getCurrentStrategy();
    
        void setCurrentPayoff(double currentPayoff);
        double getCurrentPayoff();

        void trackPayoffs(double last_payoff);
    
        void addStrategyPayoff(int strategy_role);
    
        void addNetworkPayoff();
    
        void setInitScore(UGenerator rng);
    
        void updateInteractions(int inter_number);
        
        //void exploreSpace(int num_bits_to_flip, UGenerator rng, NGenerator nrng,const std::vector<double> &space_data);
        //void setInitLocation(UGenerator rng, NGenerator nrng,const std::vector<double> &space_data);
    
        //void copyNeighborLocationBit(Agent neighbor, UGenerator rng, NGenerator nrng,const std::vector<double> &space_data);
        //void copyNeighborLocationFull(Agent neighbor, UGenerator rng, NGenerator nrng,const std::vector<double> &space_data);
    
        //void copyNeighborLocationDiff(Agent neighbor, UGenerator rng, NGenerator nrng,const std::vector<double> &space_data);
    
    
        int convertBin2Dec(std::bitset<20> bin);
    
        int getLocationInt();
        std::bitset<20> getLocationBin();
    

        double getScore();
        double getTotalPayoff();
    
        void recordInteraction(double visitPayoff, double hostPayoff);
        
        double getPastVisitPayoff();
        double getPastHostPayoff();
        int getPastVisitPartner();
    
        void setStrategyLearning(float strategy_learning_speed);
        float getStrategyLearning();
        
        void setNetworkLearning(float network_learning_speed);
        float getNetworkLearning();
        
        void setStrategyDiscount(float strategy_discount);
        float getStrategyDiscount();
        
        void setNetworkDiscount(float network_discount);
        float getNetworkDiscount();
        
        void setStrategyTremble(float strategy_tremble);
        float getStrategyTremble();
        
        void setNetworkTremble(float network_tremble);
        float getNetworkTremble();
        
        void setStrategySym(bool strategy_sym);
        bool getStrategySym();
        
        void setNetworkSym(bool network_sym);
        bool getNetworkSym();
        
        void setScoreCopyProb(float score_copy_prob);
        float getScoreCopyProb();
        
        void setCopyError(float copy_error);
        float getCopyError();
        
        void setExploreProb(float explore_prob);
        float getExploreProb();
};

//Network Class
class Network{
    private:
        // Population of network
        int pop;
        
        // Vector of Agents
        std::vector<Agent> agents;
    
    public:
        Network(int pop = 20, float strategy_learning_speed = 1, float network_learning_speed = 1, float strategy_discount = 0.01, float network_discount = 0.01, float strategy_tremble = 0.01, float network_tremble = 0.01, bool strategy_sym = 0, bool network_sym = 0, float score_copy_prob = 0.1, float copy_error = 0.1, float explore_prob = 1);
        Network(int pop, std::string strat_filepath, float strategy_learning_speed = 1, float network_learning_speed = 1, float strategy_discount = 0.01, float network_discount = 0.01, float strategy_tremble = 0.01, float network_tremble = 0.01, bool strategy_sym = 0, bool network_sym = 0, float score_copy_prob = 0.1, float copy_error = 0.1, float explore_prob = 1);
        Network(std::string net_filepath, std::string strat_filepath, float strategy_learning_speed = 1, float network_learning_speed = 1, float strategy_discount = 0.01, float network_discount = 0.01, float strategy_tremble = 0.01, float network_tremble = 0.01, bool strategy_sym = 0, bool network_sym = 0, float score_copy_prob = 0.1, float copy_error = 0.1, float explore_prob = 1);
    
        std::vector<int> agent_seq;
    
        int getPop();
        std::vector<Agent> getAgents();
    
        Agent& GetAgent(std::vector<Agent>::size_type ElementNumber);
        void AddAgent(const Agent& NewAgent);

};

class Game{
    private:
        std::string gameName;
        double base_payoff;
        std::vector<std::vector<double>> gamePayoffs;
        std::string coupling_effect;
        std::vector<double> fight_bonus;
        
    public:
        static std::vector<int> num_strats;
    
        Game(std::vector<std::vector<double>> gamePayoffs = {{0,1,0.2,0.6},{0,0.2,1,0.6}}, std::string gameName = "HDInnov", double base_payoff = 0.0001, std::vector<int> num_strats = {2,2}, std::string coupling_effect = "Fight", std::vector<double> fight_bonus = {0,0});
            
        Game(std::string payoff_filepath, std::string gameName = "HDInnov", double base_payoff = 0.0001, std::string coupling_effect = "Fight");
    
        std::string getName();
        std::string getCouplingEffect();
        void setPayoffs(std::vector<std::vector<double>> gamePayoffs);
        std::vector<std::vector<double>> getPayoffs();
        double getBasePayoff();
        std::vector<double> playGame(UGenerator rng,Agent &visitor, Agent &host);
    
        std::vector<double> StagHuntBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore);
        std::vector<double> FightBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore);
        std::vector<double> FightRand(UGenerator rng, int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore);
        std::vector<double> FightSplitBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore);
        std::vector<double> FSHBonus(int currentAgentStrategy, int friendAgentStrategy, double visitScore, double hostScore);
};

class Environment{
    private:
        int size;
    
};

struct SimTracking{
    char key[20];
    const char out_folder_complete_path[100] = "/Users/bobloblaw/Dropbox/Research/Evolutionary_Modeling";
    int current_seed;
    
    std::string out_network_file;
    std::string out_stats_file;
    std::string out_p1strat_file;
    std::string out_p2strat_file;
    std::string out_innov_scores;
    std::string out_innov_locs;
    std::string out_partners;
    std::string out_p1_payoffs;
    std::string out_p2_payoffs;
    std::string out_tp_file;
    std::string out_inter_file;
    
    std::vector<std::vector<double>> player_strategies_p1_t;
    std::vector<std::vector<double>> player_strategies_p2_t;
    std::vector<std::vector<double>> network_weights_t;
    std::vector<std::vector<double>> player_p1_payoffs_t;
    std::vector<std::vector<double>> player_p2_payoffs_t;
    std::vector<std::vector<int>> player_partners_t;
    std::vector<double> strategy_correlation_t;
    std::vector<std::vector<double>> prop_interactions_t;
    std::vector<std::vector<double>> strategy_mean_t;
    std::vector<std::vector<double>> strategy_variance_t;
    std::vector<double> instrength_variance_t;
    std::vector<int> times_tracked;
    std::vector<std::vector<double>> total_payoffs_t;
    
    std::vector<std::vector<int>> all_interactions_t;

    
    std::vector<std::vector<int>> innovation_locations_t;
    std::vector<std::vector<double>> innovation_scores_t;
    
    int max_time;
    
    std::string out_file_evostats;
    
    void init_Trackers(int pop){
        // Strategy initialization
        std::vector<double> init_strategy(pop*2);
        std::fill(init_strategy.begin(),init_strategy.end(),0.5);
        player_strategies_p1_t.push_back(init_strategy);
        player_strategies_p2_t.push_back(init_strategy);
        
        // Network initialization
        std::vector<double> init_netweights(pop*pop);
        double fill_val = 1.0/(pop-1);
        std::fill(init_netweights.begin(),init_netweights.end(),fill_val);
        for(int i = 0; i < pop; i++){
            init_netweights.at(i * pop + i) = 0;
        }
        network_weights_t.push_back(init_netweights);
        //std::vector<std::vector<double>> player_payoffs_t;
        //std::vector<double> strategy_correlation_t;
        prop_interactions_t.push_back({0, 0.25,0.25,0.25,0.25});
        //std::vector<std::vector<double>> strategy_mean_t;
        //std::vector<std::vector<double>> strategy_variance_t;
        //std::vector<double> instrength_variance_t;
        
        times_tracked = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, 25, 50, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000, 2000, 3000, 4000, 5000, 6000,
            7000, 8000, 9000, 10000, 20000, 30000, 40000, 50000, 60000, 70000, 80000, 90000, 100000, 110000, 120000, 130000, 140000, 150000, 160000, 170000, 180000, 190000, 200000, 210000, 220000, 230000, 240000, 250000, 260000, 270000, 280000, 290000, 300000, 310000, 320000, 330000, 340000, 350000, 360000, 370000, 380000, 390000, 400000, 410000, 420000, 430000, 440000, 450000, 460000, 470000, 480000, 490000, 500000, 550000, 600000, 650000, 700000, 750000, 800000, 900000, 1000000};          
        
    }
    
    void updateData(Network &net, UGenerator rng, NGenerator nrng, int time_t){
        int pop = net.getPop();
        
        std::vector<double> player_strategies_p1;
        player_strategies_p1.reserve(pop*2);
        
        std::vector<double> player_strategies_p2;
        player_strategies_p2.reserve(pop*2);
        
        std::vector<double> prop_interactions(5,0.0);
        
        std::vector<double> network_weights;
        network_weights.reserve(pop);
        
        std::vector<double> innovation_scores;
        innovation_scores.reserve(pop);
        
        std::vector<int> innovation_locations;
        innovation_locations.reserve(pop);
        
        std::vector<double> player_p1_payoffs;
        player_p1_payoffs.reserve(pop);
        
        std::vector<double> player_p2_payoffs;
        player_p2_payoffs.reserve(pop);
        
        std::vector<int> player_partners;
        player_partners.reserve(pop);
        
        std::vector<double> total_payoffs;

        std::vector<int> all_interactions;
        //all_interactions.reserve(pop * 4);
        
        for(int agent_num = 0; agent_num < pop; agent_num++){
            
            Agent &curAgent = net.GetAgent(agent_num);
            //curAgent.exploreSpace(1,rng,nrng,space_data);
            curAgent.updateAgent();
            
            std::vector<int> agent_interactions = curAgent.getInteractions();
            
            all_interactions.insert(all_interactions.end(), agent_interactions.begin(), agent_interactions.begin() + 4);
            
            // Update visitor strategy tracker
            std::vector<double> p1_strats = curAgent.getStrats(0);
            double p1_strat_sum = std::accumulate(p1_strats.begin(),p1_strats.end(), 0.0);
            std::transform(p1_strats.begin(), p1_strats.end(), p1_strats.begin(),
                           std::bind2nd(std::multiplies<double>(), 1.0/p1_strat_sum));
            
            player_strategies_p1.insert(player_strategies_p1.end(), p1_strats.begin(), p1_strats.end());
            
            // Update host strategy tracker
            std::vector<double> p2_strats = curAgent.getStrats(1);
            double p2_strat_sum = std::accumulate(p2_strats.begin(),p2_strats.end(), 0.0);
            
            std::transform(p2_strats.begin(), p2_strats.end(), p2_strats.begin(),
                           std::bind2nd(std::multiplies<double>(), 1.0/p2_strat_sum));
            player_strategies_p2.insert(player_strategies_p2.end(), p2_strats.begin(), p2_strats.end());
            
            // Update network tracker
            std::vector<double> friends = curAgent.getFriends();
            double net_sum = std::accumulate(friends.begin(),friends.end(), 0.0);
            
            std::transform(friends.begin(), friends.end(), friends.begin(),
                           std::bind2nd(std::multiplies<double>(), 1.0/net_sum));
            
            network_weights.insert(network_weights.end(), friends.begin(), friends.end());
            
            double score = curAgent.getScore();
            innovation_scores.push_back(score);
            
            int location = curAgent.getLocationInt();
            innovation_locations.push_back(location);
            
            total_payoffs.push_back(curAgent.getTotalPayoff());
            
            player_p1_payoffs.push_back(curAgent.getPastVisitPayoff());
            player_p2_payoffs.push_back(curAgent.getPastHostPayoff());
            player_partners.push_back(curAgent.getPastVisitPartner());
        }
        int strat_1;
        int strat_2;
        
        prop_interactions.at(0) = time_t;
        for(int pop_ind_1 = 0; pop_ind_1 < pop; pop_ind_1++){
            for(int pop_ind_2 = 0; pop_ind_2 < pop; pop_ind_2++){
                for(int strategy_role = 0; strategy_role < 4; strategy_role++){
                    strat_1 = strategy_role/2;
                    strat_2 = strategy_role%2;
                    
                    /*
                    if(!std::isfinite(network_weights.at(pop_ind_1 * pop + pop_ind_2))){
                        printf("%lf\n",network_weights.at(pop_ind_1 * pop + pop_ind_2));
                        printf("%d\n",pop_ind_1);
                        printf("%d\n",pop_ind_2);

                    }
                    */
                    //printf("Prop = %f\n",(network_weights.at(pop_ind_1 * pop + pop_ind_2)));
                    
                    prop_interactions.at(strategy_role + 1) += (network_weights.at(pop_ind_1 * pop + pop_ind_2) * player_strategies_p1.at(pop_ind_1 * 2 + strat_1) * player_strategies_p2.at(pop_ind_2 * 2 + strat_2))/pop;
                }
            }
        }
        prop_interactions_t.push_back(prop_interactions);
        player_strategies_p1_t.push_back(player_strategies_p1);
        player_strategies_p2_t.push_back(player_strategies_p2);
        network_weights_t.push_back(network_weights);
        player_p1_payoffs_t.push_back(player_p1_payoffs);
        player_p2_payoffs_t.push_back(player_p2_payoffs);
        player_partners_t.push_back(player_partners);
        
        innovation_scores_t.push_back(innovation_scores);
        innovation_locations_t.push_back(innovation_locations);
        all_interactions_t.push_back(all_interactions);
        total_payoffs_t.push_back(total_payoffs);
        
    };
    void initTrackLocation(Network &net, UGenerator rng, NGenerator nrng){

        int pop = net.getPop();
        
        std::vector<double> innovation_scores;
        innovation_scores.reserve(pop);
        
        std::vector<int> innovation_locations;
        innovation_locations.reserve(pop);
        
        for(int agent_num = 0; agent_num < pop; agent_num++){
            
            net.agent_seq.push_back(agent_num);
            
            Agent &curAgent = net.GetAgent(agent_num);
            curAgent.setInitScore(rng);
            
            double score = curAgent.getScore();
            innovation_scores.push_back(score);
            
            int location = curAgent.getLocationInt();
            innovation_locations.push_back(location);
            
        }
       
        innovation_scores_t.push_back(innovation_scores);
        innovation_locations_t.push_back(innovation_locations);
    
};
    // =snprintf("%s/%s_StrategyP1t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    //snprintf(out_file_p2t,sizeof(out_file_p2t),"%s/%s_StrategyP2t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    //snprintf(out_file_p1_s1_t,sizeof(out_file_p1_s1_t),"%s/%s_ActionsP1t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    //snprintf(out_file_p2_s1_t,sizeof(out_file_p2_s1_t),"%s/%s_ActionsP2t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    // snprintf(out_file_stats,sizeof(out_file_stats),"%s/%s_EvoStats_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    //snprintf(out_file_payoffs_p1_t,sizeof(out_file_payoffs_p1_t),"%s/%s_PayoffsP1t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
    //snprintf(out_file_payoffs_p2_t,sizeof(out_file_payoffs_p2_t),"%s/%s_PayoffsP2t_%s_%d.csv",out_folder_complete_path,game,key,seeds[seed_ind]);
};
