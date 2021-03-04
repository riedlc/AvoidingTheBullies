""" This code corresponds to simulations run in "Conflict and Convention in Dynamic Networks" by Foley, Forber, Smead, Riedl
    Royal Society Interface
    Referred to hereafter as CCDN
    IF ATTEMPTING TO REPLICATE CCDN RESULTS, DO NOT CHANGE VARIABLES COMMENTED WITH "FIXED IN CCDN"
    Otherwise, feel free to play around with any variables
    
    Readme available in main folder
    
   """

import os, sys
import copy
from itertools import product
import numpy as np
import random
import string
import pandas as pd
import re
from itertools import combinations
from subprocess import call
from pathlib import Path

# Helper function to flatten list of lists
flatten = lambda l: [item for sublist in l for item in sublist] 

base_path = os.path.abspath(os.path.dirname(__file__))

# Header of input files (these are all parameters that C++ simulation code uses)
full_input_list = ['Base','Net_in','TMax','NetDiscount','StratDiscount','NetLearningSpeed','StratLearningSpeed','NetSymmetric','StratSymmetric','NetTremble','StratTremble','CopyProb','CopyError','ExploreProb','InnovNoise','CoupleEffect','Game','OutFolder','Key']

def build_prev_inputs(full_inputpath):
    
    existing_inputs = []
    if os.path.exists(full_inputpath):
        existing_inputs = [a for a in os.listdir(full_inputpath) if a.endswith("conf")]
        
        all_input_dfs = []
        
        for inp_file in existing_inputs:
            full_inpfile_path = os.path.abspath(os.path.join(full_inputpath,inp_file))
            inp_df = pd.read_csv(full_inpfile_path,sep=" ",dtype=str)
            all_input_dfs.append(inp_df)
        
        all_inputs = pd.concat(all_input_dfs)
        
        all_inputs.set_index("Key",inplace=True)
        
        rest_df = pd.DataFrame()
        
        for key in list(all_inputs.index):
            full_payfile_path = os.path.abspath(os.path.join(full_inputpath,"Payoffs","Payoffs_" + key + ".csv"))
            full_stratfile_path = os.path.abspath(os.path.join(full_inputpath,"Strategy","Strategy_" + key + ".csv"))
            pay_df = pd.read_csv(full_payfile_path,header=None,sep=" ",skiprows=1,dtype=str,nrows=2)
            bonus_df = pd.read_csv(full_payfile_path,header=None,sep=" ",skiprows=3,dtype=str)
            strat_df = pd.read_csv(full_stratfile_path,header=None,sep=" ",dtype=str)
            
            existing_payoffs = flatten(pay_df.values)
            existing_bonus = flatten(bonus_df.values)
            existing_strats = flatten(strat_df.values)
            
            existing_rest = existing_payoffs + existing_bonus + existing_strats
            
            rest_df[key] = existing_rest
        
        all_inputs = pd.concat([all_inputs,rest_df.T],axis=1)
        all_inputs.columns = range(all_inputs.shape[1])  
    else:
        all_inputs = pd.DataFrame()
    
    return all_inputs

def setup_simulation(base_in, payoffs,pop_list_in = [20],net_discount_list_in = [0.01],strat_discount_list_in = [0.01],init_cond_hawk_p1_list_in = [50], init_cond_hawk_p2_list_in = [50], net_learningspeed_list_in = [1], strat_learningspeed_list_in = [1], net_tremble_list_in = [0.01], strat_tremble_list_in = [0.01],net_sym_list_in = [0], strat_sym_list_in = [0], total_weight=2,tmax_in = 1000000, copy_prob_list_in = [0.001], copy_error_list_in = [0.5], explore_prob_list_in = [0.5],innov_noise_list_in = [0.01], couple_effect="None",bonus_vec_list_in = [[0,0]], run_now = 0, num_seeds = 1, ruggednessk = 7):
    """ 
    THE FOLLOWING SETS ALL INPUT PARAMETERS FOR MODEL
    
    pop_list_in = list of simulation populations (integer)
    init_strategy_str_list_in = list of strings that determine initial strategies ("random" or "uniform")
    net_discount_list_in = list of network discount values (float range [0,1])
    strat_discount_list_in = list of strategy discount values (float range[0,1])
    net_speed_list_in = list of network learning speeds (float >= 0, where 0 implies no learning)
    strat_speed_list_in = list of strategy learning speeds (float >= 0, where 0 implies no learning)
    net_tremble_list_in = list of network trembles (called errors in paper) (float range[0,1])
    strat_tremble_list_in = list of strategy trembles (float range[0,1])
    init_cond_hawk_p1_list_in = list of initial hawk strategy percentages for visitor
    init_cond_hawk_p2_list_in = list of initial hawk strategy percentages for host
    """

    # Each element in these lists corresponds to one parameter point.  For payoff symmetry, set dd1s = dd2s, dh1s == dh2s. (Can do manually or just add an if statement).  Must have equal number of elements in all 4 lists!

    # THE TWO STRATEGIES ARE HAWK AND DOVE FOR BOTH HOST AND VISITOR
    # FIXED IN CCDN
    num_strats_p1 = 2
    num_strats_p2 = 2

    # Game Type (Hawk Dove)
    # FIXED IN CCDN, THIS IS JUST FOR FILE NAMING
    game = "HDInnov"

    # FOLDER WHERE INPUT FILES ARE STORED
    input_folder = game + "_Input/"

    # Fixed base at 0.0001 in hawk-dove
    # This number is added to all payoffs (to avoid computational issues with floats below minimum representable float)
    ## FIXED IN CCDN

    # Network and strategy symmetry (0 for asymmetric, 1 for symmetric)
    # FIXED IN CCDN
    unzip_payoff = copy.deepcopy(payoffs)
    unzip_payoff = *unzip_payoff,
    unzip_payoff = flatten(unzip_payoff[0][0])
    
    # Parameters to string - leave this
    nd_str = "-".join(map(str, net_discount_list_in))
    sd_str = "-".join(map(str, strat_discount_list_in))
    nls_str = "-".join(map(str, net_learningspeed_list_in))
    sls_str = "-".join(map(str, strat_learningspeed_list_in))
    net_tremble_str = "-".join(map(str, net_tremble_list_in))
    strat_tremble_str = "-".join(map(str, strat_tremble_list_in))
    n_sym_str = "-".join(map(str, net_sym_list_in))
    s_sym_str = "-".join(map(str, strat_sym_list_in))
    pop_str = "-".join(map(str, pop_list_in))
    copy_prob_str = "-".join(map(str, copy_prob_list_in))
    copy_error_str = "-".join(map(str, copy_error_list_in))
    explore_prob_str = "-".join(map(str, explore_prob_list_in))
    bonus_str = "-".join(map(str, [a[0] for a in bonus_vec_list_in]))
    payoff_str = "-".join(map(str, unzip_payoff))
    innov_noise_str = "-".join(map(str,innov_noise_list_in))
    
    strats_p1_str = str(num_strats_p1)
    strats_p2_str = str(num_strats_p2)
    

    time_str = str(tmax_in/1000000) + "M"
    if tmax_in < 1000000:
        time_str = str(tmax_in)

    # Simulation description
    description_in = "Pop-"+ pop_str + "_Discount-N-" + nd_str + "_S-" + sd_str + "_Tremble-" + strat_tremble_str + "_NLS-" + nls_str + "_SLS-" + sls_str + "_SymN-"+ n_sym_str + "_SymS-" + s_sym_str  + "_CopyProb-" + copy_prob_str + "_CopyError-" + copy_error_str + "_ExpProb-" + explore_prob_str + "_InnovNoise-" + innov_noise_str + "_CoupleFn-" + couple_effect + "_P-" + payoff_str + "_Bon-" + bonus_str 
    
    data_description = game + "_" + description_in

    input_subfolder = "Input_" + data_description

    full_inputpath = os.path.abspath(os.path.join(base_path,"..",input_folder,input_subfolder))

    if not os.path.exists(game + "_Output_Data"):
        os.makedirs(game + "_Output_Data")
    
    print(data_description)

    all_inputs = build_prev_inputs(full_inputpath)
        
    # Get all combinations of parameter lists above
    param_combos = list(product(payoffs,pop_list_in,net_discount_list_in,strat_discount_list_in,init_cond_hawk_p1_list_in,init_cond_hawk_p2_list_in,net_learningspeed_list_in,strat_learningspeed_list_in,net_sym_list_in,strat_sym_list_in, net_tremble_list_in,strat_tremble_list_in, copy_prob_list_in, copy_error_list_in, explore_prob_list_in, innov_noise_list_in, bonus_vec_list_in))

    input_params_list = []
    for ind,i in enumerate(param_combos):
                
        # CONVERT PERCENTAGES TO INITIAL WEIGHTS (TOTAL 2)
        hawk_p1_weight = np.round(total_weight * i[4]/100.,1)
        hawk_p2_weight = np.round(total_weight * i[5]/100.,1)
        
        # Set local parameter variables
        p1_payoffs_in = i[0][0].flatten()
        p2_payoffs_in = i[0][1].flatten()
        payoffs_in = pd.concat([pd.DataFrame(p1_payoffs_in).T,pd.DataFrame(p2_payoffs_in).T])
        pop_in = i[1]
        init_p1strategy_fill_in = np.array(np.ones(shape=(pop_in,2)) * np.array([hawk_p1_weight,total_weight-hawk_p1_weight]))
        init_p2strategy_fill_in = np.array(np.ones(shape=(pop_in,2)) * np.array([hawk_p2_weight,total_weight-hawk_p2_weight]))
        
        init_strategy_fill_in = np.concatenate((init_p1strategy_fill_in, init_p2strategy_fill_in),axis=1)
        net_discount_in = i[2]
        strat_discount_in = i[3]
        net_learningspeed_in = i[6]
        strat_learningspeed_in = i[7]
        net_sym_in = i[8]
        strat_sym_in = i[9]
        net_tremble_in = i[10]
        strat_tremble_in = i[11]
        copy_prob_in = i[12]
        copy_error_in = i[13]
        explore_prob_in = i[14]
        innov_noise_in = i[15]
        bonus_vec_in = i[16]
        
        if pop_in < 100:
            file_lines = 8
        elif pop_in < 500:
            file_lines = 2
        else:
            file_lines = 1
        
        key = ''.join(random.SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(8))
        
        payoff_folder = os.path.abspath(os.path.join(full_inputpath,"Payoffs"))
        initw_folder = os.path.abspath(os.path.join(full_inputpath,"Strategy"))
        initnet_folder = os.path.abspath(os.path.join(full_inputpath,"Network"))
        
        payoff_path = os.path.abspath(os.path.join(payoff_folder,"Payoffs_" + key + ".csv"))
        initw_path = os.path.abspath(os.path.join(initw_folder,"Strategy_" + key + ".csv"))
        #initnet_path = os.path.abspath(os.path.join(initnet_folder,"Network_" + key + ".csv"))

        
        input_params = [base_in,pop_in,tmax_in,net_discount_in,strat_discount_in,net_learningspeed_in,strat_learningspeed_in,net_sym_in,strat_sym_in,net_tremble_in,strat_tremble_in,copy_prob_in,copy_error_in, explore_prob_in, innov_noise_in, couple_effect, game,data_description,key]
        
        rest_df = flatten(payoffs_in.values) + bonus_vec_in + flatten(init_strategy_fill_in)

        full_row = np.array(input_params[:-1] + rest_df)
        
        #if not (all_inputs == full_row).all(1).any():
            
        if not os.path.exists(payoff_folder):
            os.makedirs(payoff_folder)
            os.makedirs(initw_folder)
        #os.makedirs(initnet_folder)
        
        print(key)
        
        pd.DataFrame([num_strats_p1,num_strats_p2]).T.to_csv(payoff_path,sep = " ", header=None,index=False)
        
        with open(payoff_path, 'a') as f:
            pd.DataFrame(payoffs_in).to_csv(f,sep = " ",header=None,index=False)
            pd.DataFrame(bonus_vec_in).T.to_csv(f,sep= " ",header=None,index=False)
        
        pd.DataFrame(init_strategy_fill_in).to_csv(initw_path,sep= " ",header=None,index=False)
        
        if input_params not in input_params_list:
            input_params_list.append(input_params)
        else:
            print("Corresponding Input Already Exists")

    if len(input_params_list) > 0:
        input_param_df = pd.DataFrame(input_params_list)    
        
        input_param_df.columns = full_input_list
            
        dataframes = []
        while len(input_param_df) > file_lines:
            top = input_param_df[:file_lines]
            dataframes.append(top)
            input_param_df = input_param_df[file_lines:]
        
        dataframes.append(input_param_df)
        
        if not os.path.exists(full_inputpath):
            os.makedirs(full_inputpath)
        
        for ind, frame in enumerate(dataframes):
            input_filename = "Input_" + data_description + "_" + str(ind) + ".conf"
            full_conf_path = os.path.abspath(os.path.join(full_inputpath,input_filename))
            if os.path.exists(full_conf_path):
                with open(full_conf_path,'a') as f:
                    frame.to_csv(f,index=False,sep= " ",header=None)
            else:
                frame.to_csv(full_conf_path, index=False, sep = " ")
        
    if run_now:
        call(["./CulturalEvo" + " " + description_in + " " + "0" + " 4 " + str(num_seeds) + " " + "0 0" + " " + str(ruggednessk), "-l"],shell=True)
        call(["python Analysis/Code/plot_nets_HD.py " + description_in + " 0 0 4 4 ALL", "-l"],shell=True)
