//========================================================//
//  predictor.h                                           //
//  Header file for the Branch Predictor                  //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//

#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdint.h>
#include <stdlib.h>

//------------------------------------//
//      Global Predictor Defines      //
//------------------------------------//
#define NOTTAKEN  0
#define TAKEN     1

// The Different Predictor Types
#define STATIC      0
#define GSHARE      1
#define TOURNAMENT  2
#define CUSTOM      3
extern const char *bpName[];

// Definitions for 2-bit counters
#define SN  0            // predict NT, strong not taken; strong select G
#define WN  1            // predict NT, weak not taken; week select G
#define WT  2            // predict T, weak taken; week select L
#define ST  3            // predict T, strong taken; strong select L

// Definitions for Custom predictor
#define CUSTOM_GHISTORYBITS 9
#define CUSTOM_PCINDEXBITS 8

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//
extern int ghistoryBits; // Number of bits used for Global History
extern int lhistoryBits; // Number of bits used for Local History
extern int pcIndexBits;  // Number of bits used for PC index
extern int bpType;       // Branch Prediction Type
extern int verbose;

//------------------------------------//
//    Predictor Function Prototypes   //
//------------------------------------//

// Initialize the predictor
//
void init_Gshare(void);
void init_Tournament(void);
void init_Custom(void);
void init_predictor(void);

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t predict_Gshare(uint32_t pc);
uint8_t predict_localTournament(uint32_t pc);
uint8_t predict_globalTournament(uint32_t pc);
uint8_t predict_Tournament(uint32_t pc);
int16_t predict_Custom(uint32_t pc);
uint8_t make_prediction(uint32_t pc);

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void shift_Left(uint8_t *BH, uint8_t outcome);
void train_Gshare(uint32_t pc, uint8_t outcome);
void train_Tournament(uint32_t pc, uint8_t outcome);
void train_Custom(uint32_t pc, uint8_t outcome);
void train_predictor(uint32_t pc, uint8_t outcome);

#endif
