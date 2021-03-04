"""This is the driver code for the model described in "Conflict and Convention in Dynamic Networks", by Foley, Forber, Smead, Riedl, Royal Society Interface (citation to come)
    
    After compiling "Conflict_Dynamic_Networks_SimCode.cpp",
    Choose parameter lists of interest below and then run this file (i.e. python driver.py)
    All arguments are configured in this file, and this file directly calls the C++ simulation code
    
    """

import sys,os
from setup_sim import setup_simulation
from get_payoffs import get_payoffs
import numpy as np

"""
    Set Input Parameters
    
    """

# Run_now (run locally, immediately when calling setup_simulation
run_now = 0

num_seeds = 4 # Change this as you see fit

base = 0.0001

# If run_now == 0, see readme for details

# Population (size of network)
# CHOOSE LIST OF POPULATIONS TO SIMULATE (INTEGER ONLY)
pop_list_in = [20]

# Initial strategy string - Options "uniform" or "random"
# "uniform" means they're all equal to 1, "random" is random draw, which is later renormalized
init_strategy_str_list_in = ["uniform"]

# Network and strategy discounts
# Discounting determines memory of agents
# Acceptable values are in [0,1] range inclusive
# IN CCDN, STRATEGY AND NETWORK DISCOUNTS ARE EQUAL
net_discount_list_in = [0.01] #[0,0.01,0.1]
strat_discount_list_in = [0.01]

# Network and strategy tremble (0.01 is default)
# CCDN COVERS [0,0.001,0.01]
# STRATEGY AND NETWORK TREMBLE EQUAL IN CCDN
net_tremble_list_in = [0.01]# [0,0.01,0.1]
strat_tremble_list_in = net_tremble_list_in

# Network and strategy learning speed 
# CCDN COVERS [0.1,1,10]
# 1 is default
net_speed_list_in = [0,1] # [0,1,10]
strat_speed_list_in = [1] # [1,10]

net_sym_list_in = [0] # [0,1]
strat_sym_list_in = [0] # [0,1]


# Copy probability (0.001 default)
copy_prob_list_in = [0] # [0.0001,0.001]

# Copy error (0.5 default)
copy_error_list_in = [0] # [0.1,0.5]

# Explore prob (1 default)
explore_prob_list_in = [0] # [0.001,0.01]

# Coupling effect or coupling function
# Options are: "Fight", "None", "StagHunt", "FSH"
couple_effect = "Fight"

## Diff 0.4 ===> 1.5, 0
## 0.25 + (Agent - Friend) 
## 0.25 + (Friend - Agent)

total_weight = 2

t_max = 1000000

# INITIAL CONDITIONS FOR VISITOR AND HOST STRATEGY
# SET AT INITIAL PERCENTAGE TO PLAY HAWK (P1 IS VISITOR, P2 IS HOST)
# DEFAULT 50, 50 (EQUAL CHANCE TO PLAY HAWK OR DOVE FOR BOTH VISITOR AND HOST AT SIMULATION START)
init_cond_hawk_p1_list_in = [50]
init_cond_hawk_p2_list_in = [50]

innov_noise_list_in = [0]

dh = float(sys.argv[1])
dd = float(sys.argv[2])

## User input payoffs (see get_payoffs.py for full details)
payoff_type = "User"
if payoff_type == "User":
    user_p1_payoff = [[0,1],[dh,dd]]
    user_p2_payoff = [[0,dh],[1,dd]]

# Get Payoffs 
# See "get_payoffs.py" to change arguments from defaults
payoffs = get_payoffs(payoff_type,user_p1_payoff = user_p1_payoff, user_p2_payoff = user_p2_payoff)

#f_list = np.array([0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0])
f_list = [0.6]
bonus_vec_list_in = [[np.around(a,2),np.around(a/3.,3)] for a in f_list]

print(bonus_vec_list_in)

"""
    Run simulation with given parameters
    """

setup_simulation(base, payoffs, pop_list_in, net_discount_list_in, strat_discount_list_in, init_cond_hawk_p1_list_in, init_cond_hawk_p2_list_in, net_speed_list_in, strat_speed_list_in,net_tremble_list_in, strat_tremble_list_in,net_sym_list_in,
  strat_sym_list_in, total_weight, t_max, copy_prob_list_in, copy_error_list_in, explore_prob_list_in,innov_noise_list_in,couple_effect,bonus_vec_list_in, run_now,num_seeds)
