#define NK_LEN 1048576

// If compiled with -fopenmp, include omp (for multithreading)
#ifdef _OPENMP
    #include <omp.h>
#else
    #define omp_get_thread_num() 0
    #define omp_get_max_threads() 1
#endif

/* A test driver for the Network class (NetworkDriver.cpp) */
#include <stdio.h>
#include <sys/stat.h>
#include <iostream>
#include <memory>
#include <fstream>
#include <iterator>
#include <numeric>
#include <string>
#include <algorithm>
#include <bitset>
#include <iomanip>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/normal_distribution.hpp>
#include "Network.h"   // using Network class

typedef boost::mt19937 Engine;
typedef boost::uniform_real<double> UDistribution;
typedef boost::variate_generator< Engine &, UDistribution > UGenerator;
typedef boost::normal_distribution<double> NDistribution;   // Normal Distribution
typedef boost::variate_generator<Engine &, NDistribution > NGenerator;    // Variate generator

template <typename Stream, typename Iter, typename Infix>
inline Stream& infix (Stream &os, Iter from, Iter to, Infix infix_) {
    if (from == to) return os;
    os << *from;
    for (++from; from!=to; ++from) {
        os << infix_ << *from;
    }
    return os;
}

template <typename Stream, typename Iter>
inline Stream& comma_seperated (Stream &os, Iter from, Iter to) {
    return infix (os, from, to, ", ");
}


void run_model(UGenerator rng, NGenerator nrng, Game &g, SimTracking &tracking_vars, Network &net);
void run_timestep(UGenerator rng, NGenerator nrng, Game &g, SimTracking &tracking_vars, Network &net, int t, std::vector<int> agent_seq);
bool is_number(const std::string& s);
bool file_exists (const std::string& name);

int main(int argc, char *argv[]){
    
    // Command line arguments at runtime
    char* inputFolder = argv[1]; // Name of input file (decide to include folder here)
    char* inputFileNumber = argv[2];  // Input file number
    int thread_ct = atoi(argv[3]); // Number of threads to run in parallel if fopenmp
    int num_seeds = atoi(argv[4]); // Number of seeds to run
    int base_seed = atoi(argv[5]); // Starting at this seed
    int start_key = atoi(argv[6]); // Starting at this key index
    int ruggednessk = atoi(argv[7]);
    
   /*
    time_tracker1 = atoi(argv[8]);
    time_tracker2 = atoi(argv[9]);
    time_tracker3 = atoi(argv[10]);
    burn_in_time = atoi(argv[11]);
     */
    
   
    
    
    // Read in seeds
    //////////////////////////////////////////////////////

    std::ifstream seedfile("../Helpers/ESD_Seeds_All_Ordered.csv");
    
    if (seedfile.fail()){
        std::cerr << "Error: " << strerror(errno) << "\n";
        _Exit(1);
    }
    
    std::vector<int> seeds;
    
    int seed_value;
    
    while (seedfile >> seed_value) {
        seeds.push_back(seed_value);
    }
    
    ////////////////////////////////////////////////////// End read


    // Read input file line by line
    std::string full_input_folder = string_format("HDInnov_Input/Input_HDInnov_%s",inputFolder);
    std::string inputFilename = string_format("%s/Input_HDInnov_%s_%s.conf",full_input_folder.c_str(),inputFolder,inputFileNumber);
    std::ifstream in(inputFilename);
    
    std::cout << inputFilename << "\n";

    if (in.fail()){
        std::cerr << "Error: " << strerror(errno) << "\n";
    }

    std::vector<std::vector<std::string>> all_inputs;
    
    std::string header;
    getline(in, header);
    
    if (in) {
        std::string line;
        
        while (std::getline(in, line)) {
            all_inputs.push_back(std::vector<std::string>());
            
            // Break down the row into column values
            std::stringstream split(line);
            std::string value;
            
            while (split >> value)
                all_inputs.back().push_back(value);
        }
    }
    
    ////////////////////////////////////////////////////// End read

    
    // Number of keys is number of lines, or size of all inputs first dimension
    int num_keys = (int) all_inputs.size();
    
    #ifdef _OPENMP
    {
        #pragma omp parallel for num_threads(thread_ct) //start thread_ct parallel for loops (each is one simulation)
    #endif
        for(int seeded_run = num_seeds*start_key; seeded_run < num_keys * num_seeds; seeded_run++)
        {
            // Set key index and seed index
            int run_num = seeded_run/num_seeds;
            int seed_ind = seeded_run%num_seeds;
            
            // Locate seed at the index
            int this_seed = seeds.at(base_seed + seed_ind);
            
            // Grab input vector and set Input parameters
            ////////////////////////////////////////////
            
            std::vector<std::string> these_inputs = all_inputs.at(run_num);
            
            double base_in = std::stod(these_inputs.at(0));
                
            int tmax_in = std::stoi(these_inputs.at(2));
            float netdiscount_in = std::stof(these_inputs.at(3));
            float stratdiscount_in = std::stof(these_inputs.at(4));
            float netlearningspeed_in = std::stof(these_inputs.at(5));
            float stratlearningspeed_in = std::stof(these_inputs.at(6));
            bool netsymmetric_in = boost::lexical_cast<bool>(these_inputs.at(7));
            bool stratsymmetric_in = boost::lexical_cast<bool>(these_inputs.at(8));
            float nettremble_in = std::stof(these_inputs.at(9));
            float strattremble_in = std::stof(these_inputs.at(10));
            float score_copy_prob = std::stof(these_inputs.at(11));
            float copy_error = std::stof(these_inputs.at(12));
            float explore_prob = std::stof(these_inputs.at(13));
            float innov_noise  = std::stof(these_inputs.at(14));
            std::string coupling_effect = these_inputs.at(15);
            std::string game_in = these_inputs.at(16);
            std::string outputDesc = these_inputs.at(17);
            std::string key = these_inputs.at(18);
            
            std::string mainOutputFolder = string_format("%s_Output_Data",game_in.c_str());

            std::string outputFolder = string_format("%s_Output_Data/Output_%s",game_in.c_str(),outputDesc.c_str());
            
            std::string strat_file = string_format("%s/Strategy/Strategy_%s.csv",full_input_folder.c_str(),key.c_str());
            
            struct stat st = {0};
            
            // If output directory doesn't exist, create it
            if(stat(mainOutputFolder.c_str(), &st) == -1){
                mkdir(mainOutputFolder.c_str(), 0700);
            }
    
            // If output directory doesn't exist, create it
            if(stat(outputFolder.c_str(), &st) == -1){
                mkdir(outputFolder.c_str(), 0700);
            }
            
            ////////////////////////////////////////////
            
            
            // Initialize tracking variables
            SimTracking tracking_vars;
            
            tracking_vars.out_network_file = string_format("%s/%s_Weights_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_stats_file = string_format("%s/%s_EvoStats_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_p1strat_file = string_format("%s/%s_StrategyVisit_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_p2strat_file = string_format("%s/%s_StrategyHost_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_innov_scores = string_format("%s/%s_Scores_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_innov_locs = string_format("%s/%s_Locations_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_p1_payoffs = string_format("%s/%s_P1Payoffs_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_p2_payoffs = string_format("%s/%s_P2Payoffs_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_partners = string_format("%s/%s_Partners_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_net_stds = string_format("%s/%s_NetSTD_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_full_strats = string_format("%s/%s_OutFS_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            tracking_vars.out_inscore = string_format("%s/%s_OutScore_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            
            tracking_vars.out_tp_file = string_format("%s/%s_TotalPayoff_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            
            tracking_vars.out_inter_file = string_format("%s/%s_TotalInteractions_%s_%d_%d.csv",outputFolder.c_str(),game_in.c_str(),key.c_str(), this_seed, ruggednessk);
            
            
            
            if(!(file_exists(tracking_vars.out_network_file) && file_exists(tracking_vars.out_stats_file) && file_exists(tracking_vars.out_p1strat_file) && file_exists(tracking_vars.out_p2strat_file) && file_exists(tracking_vars.out_innov_scores))){
                // && file_exists(tracking_vars.out_innov_locs) && file_exists(tracking_vars.out_p1_payoffs) && file_exists(tracking_vars.out_p2_payoffs) && file_exists(tracking_vars.out_partners)))
                // Set RNG and distributions
                Engine eng(this_seed);
                UDistribution udst(0.0, 1.0);
                UGenerator rng(eng, udst);
                
                NDistribution ndst(0.0, innov_noise);
                NGenerator nrng(eng, ndst);
                
                // Construct a Game
                std::string payoff_filename = string_format("%s/Payoffs/Payoffs_%s.csv",full_input_folder.c_str(),key.c_str());
                
                Game g(payoff_filename, game_in.c_str(), base_in, coupling_effect);
                
                //Construct a network of Agents
                //Network net("netfile.csv");
                int net_pop = std::stoi(these_inputs.at(1));
                Network net(net_pop,strat_file,stratlearningspeed_in, netlearningspeed_in, stratdiscount_in, netdiscount_in, strattremble_in,nettremble_in, stratsymmetric_in, netsymmetric_in, score_copy_prob, copy_error, explore_prob);
                //}else{
                //    std::string net_file = these_inputs.at(1);
                //    net = Network(net_file, strat_file, stratlearningspeed_in, netlearningspeed_in, stratdiscount_in, netdiscount_in, strattremble_in,nettremble_in, stratsymmetric_in, netsymmetric_in, score_copy_prob, copy_error, explore_prob);
                //} 
                
                tracking_vars.init_Trackers(net.getPop());
                tracking_vars.max_time = tmax_in;
                
            
            
                ////////////////////////////////////////////

                
                // Run single simulation
                run_model(rng, nrng,  g, tracking_vars, net);
                

                // Output tracking data
                /////////////////////////////////////////////
                std::ofstream net_out(tracking_vars.out_network_file.c_str());
                            
                net_out << std::setprecision(4);

                for(size_t time_i = 0; time_i < tracking_vars.network_weights_t.size(); time_i++){
                    comma_seperated(net_out, tracking_vars.network_weights_t.at(time_i).begin(), tracking_vars.network_weights_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream net_std_out(tracking_vars.out_net_stds.c_str());
                
                net_std_out << std::setprecision(4);
                
                for(size_t time_i = 0; time_i < tracking_vars.network_stds_t.size(); time_i++){
                    comma_seperated(net_std_out, tracking_vars.network_stds_t.at(time_i).begin(), tracking_vars.network_stds_t.at(time_i).end()) << std::endl;
                }
                

                std::ofstream p1_strat_out(tracking_vars.out_p1strat_file.c_str());
                
                p1_strat_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i < tracking_vars.player_strategies_p1_t.size(); time_i++){
                    comma_seperated(p1_strat_out, tracking_vars.player_strategies_p1_t.at(time_i).begin(), tracking_vars.player_strategies_p1_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream p2_strat_out(tracking_vars.out_p2strat_file.c_str());
                
                p2_strat_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i < tracking_vars.player_strategies_p2_t.size(); time_i++){
                    comma_seperated(p2_strat_out, tracking_vars.player_strategies_p2_t.at(time_i).begin(), tracking_vars.player_strategies_p2_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream scores_out(tracking_vars.out_innov_scores.c_str());
                
                scores_out << std::setprecision(8);
                
                for(size_t time_i = 0; time_i < tracking_vars.innovation_scores_t.size(); time_i++){
                    comma_seperated(scores_out, tracking_vars.innovation_scores_t.at(time_i).begin(), tracking_vars.innovation_scores_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream stats_out(tracking_vars.out_stats_file.c_str());
                
                stats_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i < tracking_vars.prop_interactions_t.size(); time_i++){
                    comma_seperated(stats_out, tracking_vars.prop_interactions_t.at(time_i).begin(), tracking_vars.prop_interactions_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream fs_out(tracking_vars.out_full_strats.c_str());
                
                fs_out << std::fixed;
                fs_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i < tracking_vars.full_strats_t.size(); time_i++){
                    comma_seperated(fs_out, tracking_vars.full_strats_t.at(time_i).begin(), tracking_vars.full_strats_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream inscore_out(tracking_vars.out_inscore.c_str());
                
                inscore_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i < tracking_vars.innov_score_t.size(); time_i++){
                    comma_seperated(inscore_out, tracking_vars.innov_score_t.at(time_i).begin(), tracking_vars.innov_score_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream tp_out(tracking_vars.out_tp_file.c_str());
                
                tp_out << std::setprecision(9);
                
                
                for(size_t time_i = 0; time_i < tracking_vars.total_payoffs_t.size(); time_i++){
                    comma_seperated(tp_out, tracking_vars.total_payoffs_t.at(time_i).begin(), tracking_vars.total_payoffs_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream inter_out(tracking_vars.out_inter_file.c_str());
                                
                
                for(size_t time_i = 0; time_i < tracking_vars.all_interactions_t.size(); time_i++){
                    comma_seperated(inter_out, tracking_vars.all_interactions_t.at(time_i).begin(), tracking_vars.all_interactions_t.at(time_i).end()) << std::endl;
                }
                
                /*
                
                std::ofstream locs_out(tracking_vars.out_innov_locs.c_str());
                
                for(size_t time_i = 0; time_i < tracking_vars.innovation_locations_t.size(); time_i++){
                    comma_seperated(locs_out, tracking_vars.innovation_locations_t.at(time_i).begin(), tracking_vars.innovation_locations_t.at(time_i).end()) << std::endl;
                }
            
                std::ofstream partners_out(tracking_vars.out_partners.c_str());
                
                for(size_t time_i = 0; time_i <tracking_vars.player_partners_t.size(); time_i++){
                    comma_seperated(partners_out, tracking_vars.player_partners_t.at(time_i).begin(), tracking_vars.player_partners_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream p1_payoffs_out(tracking_vars.out_p1_payoffs.c_str());
                
                p1_payoffs_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i <tracking_vars.player_p1_payoffs_t.size(); time_i++){
                    comma_seperated(p1_payoffs_out, tracking_vars.player_p1_payoffs_t.at(time_i).begin(), tracking_vars.player_p1_payoffs_t.at(time_i).end()) << std::endl;
                }
                
                std::ofstream p2_payoffs_out(tracking_vars.out_p2_payoffs.c_str());
                
                p2_payoffs_out << std::setprecision(3);
                
                for(size_t time_i = 0; time_i <tracking_vars.player_p2_payoffs_t.size(); time_i++){
                    comma_seperated(p2_payoffs_out, tracking_vars.player_p2_payoffs_t.at(time_i).begin(), tracking_vars.player_p2_payoffs_t.at(time_i).end()) << std::endl;
                }
                 */
                //////////////////////////////////////////// End output
            }

        }
    #ifdef _OPENMP
        }
    #endif
            
    return 0;
}
    
            
void run_model(UGenerator rng, NGenerator nrng,Game &g, SimTracking &tracking_vars, Network &net){
    //std::vector<double> past_payoffs_p1(pop,0.0); // Track visitor payoffs
    //std::vector<double> past_payoffs_p2(pop*2,0.0); // Track host payoffs and how many times a host got visited
    
    // Initialize node strength
    /*
    std::vector<double> node_strength(pop, 0.0);;
    
    long int total_p1 = 0;
    long int total_p2 = 0;
    
    for(int i = 0; i < pop; i++)
    {
        for(int k = 0; k < num_strats_p1; k++){
            p1_strategy_t.push_back(p1_strategy.at(i*num_strats_p1 + k));
        }
        for(int k = 0; k < num_strats_p2; k++){
            p2_strategy_t.push_back(p2_strategy.at(i*num_strats_p2 + k));
        }
    }
    
    player1_s1_t.push_back(total_p1);
    player2_s1_t.push_back(total_p2);
    
    
    adjacency_weights_t.insert(adjacency_weights_t.end(), adjacency_weights.begin(), adjacency_weights.end());
    
    //std::vector<float> adjacency_weights_new = adjacency_weights;
    
    std::vector<double> p1_strategy_new = p1_strategy;
    std::vector<double> p2_strategy_new = p2_strategy;
     
     */
    
    // Initialize agent sequence (to be randomized each round 
    tracking_vars.initTrackLocation(net, rng, nrng);
    
    // Run simulation for max_time timesteps 
    for (int t = 1; t < tracking_vars.max_time+1; t++)
    {
        // Update agent location at eac, h time step
        //std::fill(past_payoffs_p1.begin(),past_payoffs_p1.end(),0.0);
        //std::fill(past_payoffs_p2.begin(),past_payoffs_p2.end(),0.0);
        
        // Run simulation for one time step, loop through all agents once
        run_timestep(rng, nrng, g, tracking_vars, net, t, net.agent_seq);
    }
        
}


void run_timestep(UGenerator rng, NGenerator nrng, Game &g, SimTracking &tracking_vars, Network &net, int t, std::vector<int> agent_seq){
    
    // Shuffle agents in random order (updating is synchronous anyways so this only serves as another layer of randomness)

    std::random_shuffle(agent_seq.begin(), agent_seq.end());
        
    //////////////////////////////////////////
    
    int agent;
    

    // Loop through agents, random order
    for(int agent_num = 0; agent_num < net.getPop(); agent_num++)
    {        
        
        agent = agent_seq.at(agent_num); // Current agent (shuffled order)
        
        // Create temp vector for agent to choose random neighbor from
        std::vector<int> temp_agent_seq(agent_seq);
        
        temp_agent_seq.erase(std::remove(temp_agent_seq.begin(), temp_agent_seq.end(), agent), temp_agent_seq.end());
            
        // Get the current visitor agent
        Agent &currentAgent = net.GetAgent(agent);
                
         // Choose interaction partner according to network weights
        int friend_ind = currentAgent.chooseFriend(rng,temp_agent_seq);
        
        // Set friend agent
        Agent &friendAgent = net.GetAgent(friend_ind);
        friendAgent.setCurrentFriend(agent);
        
        //printf("Agent: %d\n", agent);
        //printf("Friend: %d\n", friend_ind);
        
        
        //Draw random numbers to determine strategy for host and visitor
        currentAgent.chooseStrategy(rng, 0);
        friendAgent.chooseStrategy(rng, 1);
        
        currentAgent.discountStrategy(0);
        currentAgent.discountStrategy(1);
        friendAgent.discountStrategy(1);
        friendAgent.discountStrategy(0);

        //printf("Player 1 Strategy: %d, Player 2 Strategy: %d\n",currentAgentStrategy,friendAgentStrategy);
        
        // See above for payoff matrix explanation: 0x0 is index 0, 0x1 is index 1, 1x0 index 2, 1x1 index 3
        
        /*
        if(p1_strat_ind == 0)
        {
            total_p1 += 1;
        }
        if(p2_strat_ind == 0)
        {
            total_p2 += 1;
        }
         */
        //printf("Payoff index: %d\n", payoff_index);
        
        /*
         Interact
         */
        g.playGame(rng,currentAgent, friendAgent);
        
        //int curAgentStrat = currentAgent.getCurrentStrategy();
        //int friendAgentStrat = friendAgent.getCurrentStrategy();
        
        float friend_copy_prob = friendAgent.getScoreCopyProb();
        float cur_copy_prob = currentAgent.getScoreCopyProb();

        /*
        if (curAgentStrat == 0) {
            friend_copy_prob = friend_copy_prob * 1/10;
        }
        if (friendAgentStrat == 0) {
            cur_copy_prob = cur_copy_prob * 1/10;
        }
         */
                
        //printf("Player 1 Payoff: %f, Player 2 Payoff: %f\n",currentAgentPayoff,friendAgentPayoff);

        /*
        past_payoffs_p1.at(agent) += currentAgentPayoff; // Track payoff for visitor
        past_payoffs_p2.at(friend_ind*2) += friendAgentPayoff; // Track payoff for host
        past_payoffs_p2.at(friend_ind*2 + 1) += 1; // Add one host interaction
        
         */
        /*
         Update network weights
         */
        
        
        currentAgent.addNetworkPayoff();
        
    
        if(friendAgent.getNetworkSym() == 1) // This mean that agents partner updates their network weights as well
        {
            friendAgent.discountNeighbors();
            friendAgent.addNetworkPayoff();
            
        }
         
        /*
         Update Strategy of visitor and host
         */
        
        currentAgent.addStrategyPayoff(0);        
        friendAgent.addStrategyPayoff(1);

        
        if(currentAgent.getStrategySym() == 1){  // If strategy is symmetric, agents are forced to have the same host and visitor strategies
            
            // Discount if updating
            currentAgent.discountStrategy(1);
            friendAgent.discountStrategy(0);
            
            // Update values
            currentAgent.addStrategyPayoff(1);
            friendAgent.addStrategyPayoff(0);
            
        }
        
    }
    
    tracking_vars.updateData(net,rng,nrng,t);
    /*
    if(std::find(tracking_vars.times_tracked.begin(), tracking_vars.times_tracked.end(), t) != tracking_vars.times_tracked.end()) {
        tracking_vars.updateData(net,rng,nrng,t);
    }else{
        for(int pop_ind_1 = 0; pop_ind_1 < net.getPop(); pop_ind_1++){
            Agent &curAgent = net.GetAgent(pop_ind_1);
            curAgent.updateAgent(t);
        }
        
    }
     */
        
        /*
        for (auto k: net.GetAgent(update_flag).getStrats(0))
            std::cout << k << ' ';
        
        printf("\n");
        for (auto k: net.GetAgent(update_flag).getStrats(1))
            std::cout << k << ' ';
         
         */
        
    
    // Update current adjacency weights and strategy weights to use for next time step
    /*
    adjacency_weights = adjacency_weights_new;
    p1_strategy = p1_strategy_new;
    p2_strategy = p2_strategy_new;
     
     */
}

bool is_number(const std::string& s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}
bool file_exists (const std::string& name) {
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0); 
}

