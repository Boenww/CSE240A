//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <string.h> // memset
#include <math.h> //round

//
// TODO:Student Information
//
const char *studentName = "Bowen Zhang";
const char *studentID   = "A53241738";
const char *email       = "boz084@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
    "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
uint8_t *globalBHT;
uint32_t GHR;
uint8_t *localBHT;
uint32_t *localPHT;
uint8_t *choicePredictor;
int16_t perceptronT[1 << CUSTOM_PCINDEXBITS][CUSTOM_GHISTORYBITS + 1];
int8_t *gHistory;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//
// Initialize the predictor
//
void
init_Gshare() {
    GHR = 0;
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
}

void
init_Tournament() {
    GHR = 0;
    localBHT = malloc((1 << lhistoryBits) * sizeof(uint8_t));
    memset(localBHT, WN, (1 << lhistoryBits) * sizeof(uint8_t));
    localPHT = malloc((1 << pcIndexBits) * sizeof(uint32_t));
    memset(localPHT, 0, (1 << pcIndexBits) * sizeof(uint32_t));
    globalBHT = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(globalBHT, WN, (1 << ghistoryBits) * sizeof(uint8_t));
    choicePredictor = malloc((1 << ghistoryBits) * sizeof(uint8_t));
    memset(choicePredictor, WN, (1 << ghistoryBits) * sizeof(uint8_t));
}

//Modify perceptron predictor based on Jimenez and Lin, “Dynamic Branch Prediction with Perceptrons,” HPCA 2001
//mainly consists of a GHR, a Perceptron Table containing weights of different history bits
//size: 2^(CUSTOM_PCINDEXBITS) * (CUSTOM_GHISTORYBITS + 1) * (1 + log(1.93 * CUSTOM_GHISTORYBITS + 14)) + CUSTOM_GHISTORYBITS * 2 + CUSTOM_PCINDEXBITS
//CUSTOM_PCINDEXBITS: 8, CUSTOM_GHISTORYBITS: 9, size: ≈15Kb
//
void
init_Custom() {
    GHR = 0;
    memset(perceptronT, 0, (1 << CUSTOM_PCINDEXBITS) * (CUSTOM_GHISTORYBITS + 1) * sizeof(int16_t));//
    gHistory = malloc((1 << CUSTOM_GHISTORYBITS) * sizeof(uint8_t));
    memset(gHistory, -1, (1 << CUSTOM_GHISTORYBITS) * sizeof(uint8_t));
}

void
init_predictor()
{
    //
    //TODO: Initialize Branch Predictor Data Structures
    //
    switch (bpType) {
        case STATIC:
            break;
        case GSHARE:
            init_Gshare();
            break;
        case TOURNAMENT:
            init_Tournament();
            break;
        case CUSTOM:
            init_Custom();
            break;
        default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN: 1 indicates a prediction of taken; returning NOTTAKEN: 0
// indicates a prediction of not taken
//
uint8_t
predict_Gshare(uint32_t pc) {
    uint32_t index = (GHR ^ pc) & ((1 << ghistoryBits) - 1);
    if (globalBHT[index] <= 1) {
        return NOTTAKEN;
    } else {
        return TAKEN;
    }
}

uint8_t
predict_localTournament(uint32_t pc) {
    uint32_t localPHTIndex = pc & ((1 << pcIndexBits) - 1);
    uint32_t localBHTIndex = localPHT[localPHTIndex] & ((1 << lhistoryBits) - 1);
    if (localBHT[localBHTIndex] <= 1) {
        return NOTTAKEN;
    } else {
        return TAKEN;
    }
}

uint8_t
predict_globalTournament(uint32_t pc) {
    uint32_t globalIndex = GHR & ((1 << ghistoryBits) - 1);
    if (globalBHT[globalIndex] <= 1) {
        return NOTTAKEN;
    } else {
        return TAKEN;
    }
}

uint8_t
predict_Tournament(uint32_t pc) {
    uint32_t globalIndex = GHR & ((1 << ghistoryBits) - 1);
    if (choicePredictor[globalIndex] <= 1) {
        return predict_globalTournament(pc);
    } else {
        return predict_localTournament(pc);
    }
}

int16_t
predict_Custom(uint32_t pc) {
    uint32_t PTIndex = (pc ^ GHR) & ((1 << CUSTOM_PCINDEXBITS) - 1);
    int16_t sumOfDP = perceptronT[PTIndex][0];
    
    for (int i = 1;i <= CUSTOM_GHISTORYBITS;i++) {
        sumOfDP += perceptronT[PTIndex][i] * gHistory[i - 1];
    }
    
    return sumOfDP;
}

uint8_t
make_prediction(uint32_t pc) {
    //
    //TODO: Implement prediction scheme
    //
    
    // Make a prediction based on the bpType
    switch (bpType) {
        case STATIC:
            return TAKEN;
        case GSHARE:
            return predict_Gshare(pc);
        case TOURNAMENT:
            return predict_Tournament(pc);
        case CUSTOM:
            return predict_Custom(pc) >= 0 ? TAKEN:NOTTAKEN;
        default:
            break;
    }
    
    // If there is not a compatable bpType then return NOTTAKEN
    return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
shift_Left(uint8_t *BH, uint8_t outcome) {
    if (outcome == TAKEN) {
        if (*BH < ST) {
            (*BH)++;
        }
    } else {
        if (*BH > SN) {
            (*BH)--;
        }
    }
}

void
train_Gshare(uint32_t pc, uint8_t outcome) {
    uint32_t index = (pc ^ GHR) & ((1 << ghistoryBits) - 1);
    shift_Left(&(globalBHT[index]), outcome);
    GHR = ((GHR << 1) | outcome) & ((1 << ghistoryBits) - 1);
}

void
train_Tournament(uint32_t pc, uint8_t outcome) {
    uint32_t globalIndex = GHR & ((1 << ghistoryBits) - 1);
    uint32_t localPHTIndex = pc & ((1 << pcIndexBits) - 1);
    uint32_t localBHTIndex = localPHT[localPHTIndex] & ((1 << lhistoryBits) - 1);
    
    //update choicePredictor
    if (predict_localTournament(pc) != predict_globalTournament(pc)) {
        shift_Left(&(choicePredictor[globalIndex]), outcome == predict_localTournament(pc)? TAKEN:NOTTAKEN);
    }
    
    //update globalBHT, GHR, localBHT, localPHT
    shift_Left(&(globalBHT[globalIndex]), outcome);
    GHR = ((GHR << 1) | outcome) & ((1 << ghistoryBits) - 1);
    shift_Left(&(localBHT[localBHTIndex]), outcome);
    localPHT[localPHTIndex] = ((localPHT[localPHTIndex] << 1) | outcome) & ((1 << lhistoryBits) - 1);
}

void
train_Custom(uint32_t pc, uint8_t outcome) {
    int8_t t = outcome == TAKEN ? 1:-1;
    int16_t theta = round(1.93 * CUSTOM_GHISTORYBITS + 14);
    uint32_t PTIndex = (pc ^ GHR) & ((1 << CUSTOM_PCINDEXBITS) - 1);
    int16_t sumOfDP = predict_Custom(pc);
    
    if ((t * sumOfDP < 0) || abs(sumOfDP) <= theta) {
        perceptronT[PTIndex][0] += t;
        for (int i = 1;i <= CUSTOM_GHISTORYBITS;i++) {
            perceptronT[PTIndex][i] += t * gHistory[i - 1];
        }
    }
    for (int j = CUSTOM_GHISTORYBITS - 1;j > 0;j--) {
        gHistory[j] = gHistory[j - 1];
    }
    gHistory[0] = t;
    GHR = ((GHR << 1) | outcome) & ((1 << CUSTOM_PCINDEXBITS) - 1);
}

void
train_predictor(uint32_t pc, uint8_t outcome) {
    //
    //TODO: Implement Predictor training
    //
    switch (bpType) {
        case STATIC:
            break;
        case GSHARE:
            train_Gshare(pc, outcome);
            break;
        case TOURNAMENT:
            train_Tournament(pc, outcome);
            break;
        case CUSTOM:
            train_Custom(pc, outcome);
            break;
        default:
            break;
    }
}
