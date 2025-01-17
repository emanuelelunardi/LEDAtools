#pragma once 
#include "binomials.hpp"
#include <NTL/ZZ.h>
#include <NTL/RR.h>
#include <iomanip>

/***************************Classic ISDs***************************************/

double isd_log_cost_classic_BJMM_approx(const uint32_t n, 
                                        const uint32_t k,
                                        const uint32_t t) {
    return ((double)t) * - log((1.0 - (double) k / (double) n)) / log(2);
}

// computes the probability of a random k * k being invertible
const NTL::RR log_probability_k_by_k_is_inv(const NTL::RR &k) {
    NTL::RR log_pinv = NTL::RR(0.5);
    for(long i = 2 ; i <=k ; i++){
        log_pinv = log_pinv * (NTL::RR(1) - NTL::power2_RR(-i));
    }
    return NTL::log(log_pinv);
}

const NTL::RR probability_k_by_k_is_inv(const NTL::RR &k) {
    NTL::RR log_pinv = NTL::RR(0.5);
    for(long i = 2 ; i <=k ; i++){
        log_pinv = log_pinv * (NTL::RR(1) - NTL::power2_RR(-i));
    }
    return log_pinv;
}

const NTL::RR classic_rref_red_cost(const NTL::RR &n, const NTL::RR & r){
    /* simple reduced row echelon form transform, as it is not likely to be the 
     * bottleneck */
    NTL::RR k = n-r;
    return r*r*n/NTL::RR(2) + 
           (n*r)/NTL::RR(2) - 
           r*r*r / NTL::RR(6) +
           r*r +
           r / NTL::RR(6) - NTL::RR(1);
}

const NTL::RR classic_IS_candidate_cost(const NTL::RR &n, const NTL::RR & r){
    return classic_rref_red_cost(n,r)/probability_k_by_k_is_inv(r) + r*r;
}

const NTL::RR Fin_Send_rref_red_cost(const NTL::RR &n,
                                     const NTL::RR &r,
                                     const NTL::RR l){
    /* reduced size reduced row echelon form transformation, only yields an
     * (r-l) sized identity matrix */
    NTL::RR k = n-r;
    return  - l*l*l / NTL::RR(3) 
            - l*l*n / NTL::RR(2) 
            + l*l*r / NTL::RR(2) 
            - 3*l*l / NTL::RR(2) 
            - 3*l*n / NTL::RR(2) 
            +   l*r / NTL::RR(2) 
            -  13*l / NTL::RR(6) 
            + n*r*r / NTL::RR(2) 
            +   n*r / NTL::RR(2) 
            - r*r*r / NTL::RR(6) 
            +   r*r 
            +     r / NTL::RR(6) 
            - NTL::RR(1);
}

const NTL::RR Fin_Send_IS_candidate_cost(const NTL::RR &n,
                                         const NTL::RR &r,
                                         const NTL::RR &l){
    return  Fin_Send_rref_red_cost(n,r,l)/probability_k_by_k_is_inv(r-l) + r*r;
}

double isd_log_cost_classic_Prange(const uint32_t n, 
                                   const uint32_t k,
                                   const uint32_t t) {
   NTL::RR n_real = NTL::RR(n);
   NTL::RR k_real = NTL::RR(k);
   NTL::RR t_real = NTL::RR(t);

   NTL::RR cost_iter = classic_IS_candidate_cost(n_real,n_real-k_real);
   NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                       NTL::to_RR(binomial_wrapper(n-k,t));

   NTL::RR log_cost = log2_RR(num_iter)+ log2_RR(cost_iter);
   return NTL::conv<double>( log_cost );
}

#define P_MAX_LB 20
double isd_log_cost_classic_LB(const uint32_t n, 
                               const uint32_t k,
                               const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost;
    uint32_t best_p = 1;
    uint32_t constrained_max_p = P_MAX_LB > t ? t : P_MAX_LB;
    NTL::RR IS_candidate_cost;
    IS_candidate_cost = classic_IS_candidate_cost(n_real,n_real-k_real);
    for(uint32_t p = 1 ;p < constrained_max_p; p++ ){
       NTL::RR p_real = NTL::RR(p);
       NTL::RR cost_iter = IS_candidate_cost +
                           NTL::to_RR(binomial_wrapper(k,p)*p*(n-k));
       NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                           NTL::to_RR( binomial_wrapper(k,p) * 
                                       binomial_wrapper(n-k,t-p) );
       log_cost = (NTL::log(num_iter)+NTL::log(cost_iter)) / NTL::log(NTL::RR(2));
       if(min_log_cost > log_cost){
           min_log_cost = log_cost;
           best_p=p;
       }
   }
//    std::cerr << std::endl << "Lee-Brickell best p: " << best_p << std::endl;
   return NTL::conv<double>( min_log_cost );
}

#define P_MAX_Leon P_MAX_LB
#define L_MAX_Leon 200
double isd_log_cost_classic_Leon(const uint32_t n, 
                                 const uint32_t k,
                                 const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost;
    uint32_t best_l=0,best_p=1, constrained_max_l, constrained_max_p;

    NTL::RR IS_candidate_cost;
    IS_candidate_cost = classic_IS_candidate_cost(n_real,n_real-k_real);
    constrained_max_p = P_MAX_Leon > t ? t : P_MAX_Leon;
    for(uint32_t p = 1; p < constrained_max_p; p++ ){
      constrained_max_l = ( L_MAX_Leon > (n-k-(t-p)) ? (n-k-(t-p)) : L_MAX_Leon);
      NTL::RR p_real = NTL::RR(p);
      for(uint32_t l = 0; l < constrained_max_l; l++){
          NTL::RR KChooseP = NTL::to_RR( binomial_wrapper(k,p) );
          NTL::RR cost_iter = IS_candidate_cost +
                   KChooseP * p_real * NTL::to_RR(l) +
                   ( KChooseP / NTL::power2_RR(l))* NTL::RR(p * (n-k - l));
          NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                              NTL::to_RR( binomial_wrapper(k,p) *
                                          binomial_wrapper(n-k-l,t-p) );
          log_cost = ( NTL::log(num_iter) + NTL::log(cost_iter) ) / NTL::log(NTL::RR(2));
          if(min_log_cost > log_cost){
              min_log_cost = log_cost;
              best_l = l;
              best_p = p;
          }
       }
    }
   return NTL::conv<double>( min_log_cost );
}


#define P_MAX_Stern P_MAX_Leon
#define L_MAX_Stern L_MAX_Leon
double isd_log_cost_classic_Stern(const uint32_t n, 
                                 const uint32_t k,
                                 const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost;
    uint32_t best_l = 0,best_p = 2, constrained_max_l, constrained_max_p;

    NTL::RR IS_candidate_cost;
    IS_candidate_cost = classic_IS_candidate_cost(n_real,n_real-k_real); 

    constrained_max_p = P_MAX_Stern > t ? t : P_MAX_Stern;
    for(uint32_t p = 2; p < constrained_max_p; p = p+2 ){
      constrained_max_l = ( L_MAX_Stern > (n-k-(t-p)) ? (n-k-(t-p)) : L_MAX_Stern);
      NTL::ZZ kHalfChoosePHalf;
      for(uint32_t  l = 0; l < constrained_max_l; l++){
          NTL::RR p_real = NTL::RR(p);
          kHalfChoosePHalf = binomial_wrapper(k/2,p/2);
          NTL::RR kHalfChoosePHalf_real = NTL::to_RR(kHalfChoosePHalf);

          NTL::RR cost_iter = IS_candidate_cost +
                  kHalfChoosePHalf_real * 
                              ( NTL::to_RR(l)*p_real + 
                                (kHalfChoosePHalf_real / NTL::power2_RR(l))  * NTL::RR(p * (n-k - l)) 
                              );
// #if LOG_COST_CRITERION == 1
          NTL::RR log_stern_list_size = kHalfChoosePHalf_real * 
                                   ( p_real/NTL::RR(2) * NTL::log( k_real/NTL::RR(2))/NTL::log(NTL::RR(2) ) +NTL::to_RR(l)); 
                  log_stern_list_size = NTL::log(log_stern_list_size) / NTL::log(NTL::RR(2));
                  cost_iter = cost_iter*log_stern_list_size;
// #endif
          NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                              NTL::to_RR( kHalfChoosePHalf*kHalfChoosePHalf *    
                                          binomial_wrapper(n-k-l,t-p) );
          log_cost = log2_RR(num_iter) + log2_RR(cost_iter);
          if(min_log_cost > log_cost){
              min_log_cost = log_cost;
              best_l = l;
              best_p = p;
          }
       }
    }

//     std::cerr << std::endl << "Stern Best l: " << best_l << " best p: " << best_p << std::endl;
   return NTL::conv<double>( min_log_cost );
}

#define P_MAX_FS P_MAX_Stern 
#define L_MAX_FS L_MAX_Stern 
double isd_log_cost_classic_FS(const uint32_t n, 
                                 const uint32_t k,
                                 const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost;
    uint32_t best_l = 0, best_p = 2,constrained_max_l, constrained_max_p;

    NTL::RR IS_candidate_cost;
    constrained_max_p = P_MAX_Stern > t ? t : P_MAX_Stern;
    for(uint32_t p = 2; p < constrained_max_p; p = p+2 ){
      constrained_max_l = ( L_MAX_Stern > (n-k-(t-p)) ? (n-k-(t-p)) : L_MAX_Stern);
      NTL::RR p_real = NTL::RR(p);
      NTL::ZZ kPlusLHalfChoosePHalf;
      for(uint32_t  l = 0; l < constrained_max_l; l++){
       IS_candidate_cost = Fin_Send_IS_candidate_cost(n_real,n_real-k_real,NTL::RR(l));
          kPlusLHalfChoosePHalf = binomial_wrapper((k+l)/2,p/2);
          NTL::RR kPlusLHalfChoosePHalf_real = NTL::to_RR(kPlusLHalfChoosePHalf);
          NTL::RR cost_iter = IS_candidate_cost +
                  kPlusLHalfChoosePHalf_real * 
                              ( NTL::to_RR(l)*p_real + 
                                ( kPlusLHalfChoosePHalf_real / NTL::power2_RR(l)) * 
                                  NTL::RR(p * (n-k - l)) 
                              );
// #if LOG_COST_CRITERION == 1
          NTL::RR l_real = NTL::to_RR(l);
          NTL::RR log_FS_list_size = kPlusLHalfChoosePHalf_real * 
                                   ( p_real/NTL::RR(2) * NTL::log( (k_real+l_real)/NTL::RR(2))/NTL::log(NTL::RR(2) ) +l_real); 
                  log_FS_list_size = log2_RR(log_FS_list_size);
                  cost_iter = cost_iter*log_FS_list_size;
// #endif
          NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                              NTL::to_RR( kPlusLHalfChoosePHalf * kPlusLHalfChoosePHalf *
                                          binomial_wrapper(n-k-l,t-p) );
                              
          log_cost = log2_RR(num_iter) + log2_RR(cost_iter);
          if(min_log_cost > log_cost){
              min_log_cost = log_cost;
              best_l = l;
              best_p = p;
          }
       }
    }
//     std::cerr << std::endl << "FS Best l: " << best_l << " best p: " << best_p << std::endl;
   return NTL::conv<double>( min_log_cost );
}

#define P_MAX_MMT (P_MAX_FS+25) // P_MAX_MMT
#define L_MAX_MMT 350 //L_MAX_MMT
#define L_MIN_MMT 110

double isd_log_cost_classic_MMT(const uint32_t  n,
                                 const uint32_t k,
                                 const uint32_t t) {
    uint32_t r = n-k;
    NTL::RR n_real = NTL::RR(n);
    NTL::RR r_real = NTL::RR(r);
    NTL::RR k_real = n_real-r_real;


    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost, log_mem_cost;
    uint32_t best_l= L_MIN_MMT, best_l1, best_p = 4,
             constrained_max_l = 0, constrained_max_p;

    NTL::RR FS_IS_candidate_cost;
    constrained_max_p = P_MAX_MMT > t ? t : P_MAX_MMT;
    /* p should be divisible by 4 in MMT */
    for(uint32_t p = 4; p <= constrained_max_p; p = p+4 ){
      constrained_max_l = ( L_MAX_MMT > (n-k-(t-p)) ? (n-k-(t-p)) : L_MAX_MMT );
             for(uint32_t l = L_MIN_MMT; l <= constrained_max_l; l++){
                NTL::RR l_real = NTL::to_RR(l);
                NTL::ZZ kPlusLHalfChoosePHalf = binomial_wrapper((k+l)/2,p/2);
                NTL::RR num_iter  = NTL::to_RR(binomial_wrapper(n,t)) /
                              NTL::to_RR( kPlusLHalfChoosePHalf * kPlusLHalfChoosePHalf *
                                          binomial_wrapper(n-k-l,t-p) );
                FS_IS_candidate_cost = Fin_Send_IS_candidate_cost(n_real,r_real,l_real);
                NTL::ZZ  kPlusLHalfChoosePFourths = binomial_wrapper((k+l)/2,p/4);
                NTL::RR  kPlusLHalfChoosePFourths_real = NTL::to_RR(kPlusLHalfChoosePFourths);
                NTL::RR  minOperandRight, min;
                NTL::RR  PChoosePHalf = NTL::to_RR(binomial_wrapper(p,p/2));
                NTL::RR  kPlusLChoosePHalf = NTL::to_RR(binomial_wrapper((k+l),p/2));
                minOperandRight = NTL::to_RR(binomial_wrapper((k+l)/2,p/2)) / PChoosePHalf;
                min =  kPlusLHalfChoosePFourths_real > minOperandRight ? minOperandRight : kPlusLHalfChoosePFourths_real;

               /* hoist out anything not depending on l_1/l_2 split*/
#if defined(EXPLORE_REPRS)
               for(uint32_t l_1 = 1 ; l_1 <= l ; l_1++){
                  uint32_t l_2= l-l_1;
#else
                  uint32_t l_2 = NTL::conv<unsigned int>(kPlusLHalfChoosePFourths / NTL::ZZ(p/4));
                  /*clamp l_2 to a safe value , 0 < l_2 < l*/
                  l_2 = l_2 <= 0 ? 1 : l_2;
                  l_2 = l_2 >= l ? l-1 : l_2;

                  uint32_t l_1= l - l_2;
#endif
                  NTL::RR interm = kPlusLHalfChoosePFourths_real / NTL::power2_RR(l_2) *
                                         NTL::to_RR(p/2*l_1);

                  NTL::RR otherFactor = ( NTL::to_RR(p/4*l_2) + interm );
                  NTL::RR cost_iter = FS_IS_candidate_cost +
                                      min*otherFactor +
                                      kPlusLHalfChoosePFourths_real * NTL::to_RR(p/2*l_2);

                  NTL::RR lastAddend = otherFactor +
                                        kPlusLHalfChoosePFourths_real * 
                                         kPlusLChoosePHalf * PChoosePHalf /
                                         NTL::power2_RR(l)   *  
                                       NTL::to_RR( p*(r-l) );
                  lastAddend = lastAddend * kPlusLHalfChoosePFourths_real;
                  cost_iter += lastAddend;
// #if 0

          NTL::RR log_MMT_space = r_real*n_real +
                                  kPlusLHalfChoosePFourths_real *
                                       (NTL::to_RR(p/4)* log2_RR(NTL::to_RR(k+l/2))+ NTL::to_RR(l_2) )+
                                  NTL::to_RR(min) * (NTL::to_RR(p/2)* log2_RR(NTL::to_RR(k+l))+ NTL::to_RR(l) );
                  log_MMT_space = log2_RR(log_MMT_space);
                  cost_iter = cost_iter*log_MMT_space;
// #endif
                  log_cost = log2_RR(num_iter) + log2_RR(cost_iter);
                  if(min_log_cost > log_cost){
                      min_log_cost = log_cost;
                      best_l = l;
                      best_l1 = l_1;
                      best_p = p;
                      log_mem_cost = log_MMT_space;
                  }
#if defined(EXPLORE_REPRS)
               }
#endif
            }
    }
    std::cerr << std::endl << "MMT Best l: " << best_l
                           << " best p: "   << best_p
                           << " best l1: "  << best_l1
                           << std::endl;
   if(best_p == constrained_max_p){
          std::cerr << std::endl << "Warning: p on exploration edge! " << std::endl;
   }
   if(best_l == constrained_max_l){
          std::cerr << std::endl << "Warning: l on exploration edge! " << std::endl;
   }
   std::cerr << log_mem_cost << " ";
   return NTL::conv<double>( min_log_cost );
}


#define P_MAX_BJMM 20 // P_MAX_MMT
#define L_MAX_BJMM 90 //L_MAX_MMT
#define Eps1_MAX_BJMM 4
#define Eps2_MAX_BJMM 4
double isd_log_cost_classic_BJMM(const uint32_t n, 
                                 const uint32_t k,
                                 const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    uint32_t r = n-k;
    NTL::RR r_real = NTL::RR(r);

    NTL::RR min_log_cost = n_real; // unreachable upper bound
    NTL::RR log_cost;
    uint32_t best_l, best_p, 
             best_eps_1, best_eps_2, 
             constrained_max_l, constrained_max_p;

    NTL::RR FS_IS_candidate_cost;
    constrained_max_p = P_MAX_BJMM > t ? t : P_MAX_BJMM;
    /*p should be divisible by 2 in BJMM */
    for(uint32_t p = 2; p < constrained_max_p; p = p+2 ){
        /* sweep over all the valid eps1 knowing that p/2 + eps1 should be a 
         * multiple of 4*/ 
        for(uint32_t eps1 = 2+(p%2) ; eps1 < Eps1_MAX_BJMM; eps1 = eps1 + 2) {
           uint32_t p_1 = p/2 + eps1;
           /* sweep over all the valid eps2 knowing that p_1/2 + eps2 should 
            * be even */ 
           for(uint32_t eps2 = (p_1%2) ; eps2 < Eps2_MAX_BJMM; eps2 = eps2 + 2){
              uint32_t p_2 = p_1/2 + eps2;

             constrained_max_l = ( L_MAX_BJMM > (n-k-(t-p)) ? (n-k-(t-p)) : L_MAX_BJMM );
             for(uint32_t  l = 0; l < constrained_max_l; l++){
              /* Available parameters p, p_1,p_2,p_3, l */
                NTL::RR l_real = NTL::RR(l);
                FS_IS_candidate_cost = Fin_Send_IS_candidate_cost(n_real,n_real-k_real,l_real); 
                uint32_t p_3 = p_2/2;

                NTL::ZZ L3_list_len = binomial_wrapper((k+l)/2,p_3);
                NTL::RR L3_list_len_real = NTL::to_RR(L3_list_len);
                /* the BJMM number of iterations depends only on L3 parameters
                 * precompute it */
                NTL::RR num_iter  = NTL::to_RR( binomial_wrapper(n,t) ) /
                                    NTL::to_RR( binomial_wrapper((k+l),p) *
                                                binomial_wrapper(r-l,t-p) 
                                              );
                NTL::RR P_invalid_splits = NTL::power(L3_list_len_real,2) /
                                         NTL::to_RR( binomial_wrapper(k+l,p_2));
                num_iter = num_iter / NTL::power(P_invalid_splits,4);

                /* lengths of lists 2 to 0 have to be divided by the number of repr.s*/
                NTL::RR L2_list_len = NTL::to_RR(binomial_wrapper(k+l,p_2)) * 
                                      NTL::power(P_invalid_splits,1);
                NTL::RR L1_list_len = NTL::to_RR(binomial_wrapper(k+l,p_1)) * 
                                      NTL::power(P_invalid_splits,2);
                /* estimating the range for r_1 and r_2 requires to compute the
                 * number of representations rho_1 and rho_2 */

                NTL::ZZ rho_2 = binomial_wrapper(p_1,p_1/2) * 
                                binomial_wrapper(k+l-p_1,eps2);
                NTL::ZZ rho_1 = binomial_wrapper(p,p/2) * 
                                binomial_wrapper(k+l-p,eps1);
                int min_r2 = NTL::conv<int>(NTL::log(NTL::to_RR(rho_2)) / 
                             NTL::log(NTL::RR(2)));
                int max_r1 = NTL::conv<int>(NTL::log(NTL::to_RR(rho_1)) / 
                             NTL::log(NTL::RR(2)));

                /*enumerate r_1 and r_2 over the suggested range 
                 * log(rho_2) < r2 < r_1 < log(rho_1)*/
                /* clamp to safe values */
                min_r2 = min_r2 > 0 ? min_r2 : 1;
                max_r1 = max_r1 < (int)l ? max_r1 : l-1;

                NTL::RR p_real = NTL::RR(p);
                for(int r_2 = min_r2 ; r_2 < max_r1 - 1; r_2++){
                    for(int r_1 = r_2+1; r_1 < max_r1 ; r_1++){
                       /*add the cost of building Layer 3 to cost_iter */
                        NTL::RR cost_iter = NTL::to_RR(8) *
                                            L3_list_len_real *
                                            NTL::to_RR(p_3*r_2);

                       /* add the cost of building Layer 2 */
                       cost_iter += NTL::to_RR(4*(k+l+p_2*(r_1-r_2))) *
                                    L3_list_len_real *
                                    L3_list_len_real /
                                    NTL::power2_RR(r_2);

                       /* add the cost of building Layer 1 */
                       NTL::RR b1 = NTL::to_RR(binomial_wrapper(k+l,p_1));
                       cost_iter += NTL::to_RR(2) / NTL::power2_RR(r_1) * (
                                       NTL::power(L3_list_len_real,4) / 
                                       NTL::power2_RR(r_2) *  
                                       NTL::to_RR(k+l) + 
                                       b1 * NTL::power(P_invalid_splits,2) * 
                                       NTL::to_RR(p_1*(l-r_1))
                                    );

                       /* add the cost of building L0 */
                       cost_iter +=  NTL::power(P_invalid_splits,4) / 
                                     NTL::power2_RR(l) * (
                                        NTL::power(b1,2) / NTL::power2_RR(r_1) *
                                        NTL::to_RR(k+l)+ 
                                        NTL::to_RR(binomial_wrapper(k+l,p)) *
                                        NTL::to_RR(p*(n-k-l))
                                    );

                       NTL::RR L1_list_len_full = L1_list_len / NTL::power2_RR(r_1);
                       NTL::RR L2_list_len_full = L2_list_len / NTL::power2_RR(r_2);
                       /* add cost for sorting all lists */
                       cost_iter +=  NTL::RR(2) * L1_list_len_full * 
                                     NTL::log(L1_list_len_full) +
                                     NTL::RR(4) * L2_list_len_full * 
                                     NTL::log(L2_list_len_full) +
                                     NTL::RR(8) * L3_list_len_real * 
                                     NTL::log(L3_list_len_real);
// #if LOG_COST_CRITERION == 1
          
          NTL::RR log_BJMM_space = L3_list_len_real * ( p_3 * log2_RR( (k_real+l_real)/NTL::RR(2) )  + NTL::to_RR(r_2)) + 
                                   L2_list_len_full * ( p_2 * log2_RR( (k_real+l_real)/NTL::RR(2) )  + NTL::to_RR(r_1+r_2)) +
                                   L1_list_len_full * ( p_1 * log2_RR( (k_real+l_real)/NTL::RR(2) )  + l_real) 
                                   /* L0 is never materialized, just hot-tested */ ; 
                  log_BJMM_space = log2_RR(log_BJMM_space);
                  cost_iter = cost_iter*log_BJMM_space;
// #endif
                       log_cost = log2_RR(num_iter) + log2_RR(cost_iter);
                       if(min_log_cost > log_cost){
                           min_log_cost = log_cost;
                           best_l = l;
                           best_p = p;
                           best_eps_1 = eps1;
                           best_eps_2 = eps2;
                       }
                    }
                }

             } /*end of iteration over l */
         /* to review up to to here */  
        } /* end for over eps2 */     
      } /* end for over eps1 */
    } /* end for over p*/
//     std::cerr << std::endl << "BJMM Best l: " << best_l 
//                            << " best p: "   << best_p 
//                            << " best eps1: "  << best_eps_1
//                            << " best eps2: "  << best_eps_2
//                            << std::endl;
   return NTL::conv<double>( min_log_cost );
}

/***************************Quantum ISDs***************************************/


const NTL::RR quantum_gauss_red_cost(const NTL::RR &n, 
                                     const NTL::RR & k) {
    return 0.5* NTL::power(n-k,3) + k*NTL::power((n-k),2);
}


double isd_log_cost_quantum_LB(const uint32_t n, 
                               const uint32_t k,
                               const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR log_pi_fourths = NTL::log(pi*0.25);
    NTL::RR log_pinv = log_probability_k_by_k_is_inv(k_real);
    NTL::RR iteration_cost = quantum_gauss_red_cost(n_real,k_real) +
                             NTL::to_RR(binomial_wrapper(k,2) * 2 * (n-k));
    NTL::RR log_cost = (lnBinom(n_real,t_real) - 
                        (lnBinom(k_real,NTL::RR(2)) + 
                         lnBinom(n_real-k_real,t-NTL::RR(2))) 
                       )*0.5 + 
                       log_pi_fourths; 
    log_cost += NTL::log(iteration_cost);
    log_cost = log_cost / NTL::log(NTL::RR(2));
    return NTL::conv<double>( log_cost );
}


#define MAX_M (t/2)

double isd_log_cost_quantum_stern(const uint32_t n,
                                  const uint32_t k,
                                  const uint32_t t) {
    NTL::RR n_real = NTL::RR(n);
    NTL::RR k_real = NTL::RR(k);
    NTL::RR t_real = NTL::RR(t);
    NTL::RR current_complexity, log_p_success, c_it, c_dec;

    // Start computing Stern's parameter invariant portions of complexity
    NTL::RR log_pi_fourths = NTL::log(pi*0.25);
    // compute the probability of a random k * k being invertible
    NTL::RR log_pinv = log_probability_k_by_k_is_inv(k_real);
    // compute the cost of inverting the matrix, in a quantum execution env.
    NTL::RR c_inv = quantum_gauss_red_cost(n_real,k_real);

    // optimize Stern's parameters :
    // m : the # of errors in half of the chosen dimensions
    // l : the length of the run of zeroes in the not chosen dimensions
    // done via exhaustive parameter space search, minimizing the total 
    // complexity. 
    // Initial value set to codeword bruteforce to ensure the minimum is found.
    NTL::RR min_stern_complexity = NTL::RR(n)*NTL::log(NTL::RR(2));

    for(long m = 1; m <= MAX_M; m++){
        NTL::RR m_real = NTL::RR(m);
        /* previous best complexity as a function of l alone. 
         * initialize to bruteforce-equivalent,  break optimization loop as soon
         * as a minimum is found */
        NTL::RR prev_best_complexity = NTL::RR(t);
        for(long l = 0; l < (n-k-(t-2*m)); l++ ){

          NTL::RR  l_real = NTL::RR(l);
          log_p_success = lnBinom(t_real, 2*m_real) + 
                   lnBinom(n_real-t_real, k_real-2*m_real) + 
                   lnBinom(2*m_real,m_real) +
                   lnBinom(n_real-k_real-t_real+2*m_real,l_real);
          log_p_success = log_p_success - ( m_real*NTL::log(NTL::RR(4)) + 
                              lnBinom(n_real,k_real) +
                              lnBinom(n_real -k_real, l_real));
         current_complexity = -(log_p_success+log_pinv)*0.5 + log_pi_fourths;
         /* to match specifications , the term should be 
          * (n_real-k_real), as per in deVries, although
          * David Hobach thesis mentions it to be 
          * (n_real-k_real-l_real), and it seems to match.
          * amend specs for the typo. */
         c_it = l_real + 
            (n_real-k_real-l_real)* NTL::to_RR(binomial_wrapper(k/2,m)) / 
            NTL::power2_RR(-l);

         c_it = c_it * 2*m_real * NTL::to_RR(binomial_wrapper(k/2,m));
#if IGNORE_DECODING_COST == 1
        c_dec = 0.0;
#elif IGNORE_DECODING_COST == 0
        /*cost of decoding estimated as per Golomb CWDEC 
         * decoding an n-bit vector with weight k is 
         * CWDEC_cost(k,n)=O(n^2 log_2(n))  and following deVries, where
         * c_dec = CWDEC_cost(n-k, n) + k + CWDEC_cost(l,n-k)*/
        c_dec = n_real*n_real*NTL::log(n_real) + k_real +
                (n_real-k_real)*(n_real-k_real)*NTL::log((n_real-k_real));
#endif
         current_complexity = current_complexity + NTL::log(c_it+c_inv+c_dec);
          if(current_complexity < prev_best_complexity){
             prev_best_complexity = current_complexity;
          } else{
             break;
          }
        }
       if(current_complexity < min_stern_complexity){
           min_stern_complexity = current_complexity;
       }
    }
    return NTL::conv<double>( min_stern_complexity / NTL::log(NTL::RR(2.0)) );
}


/***************************Aggregation ***************************************/


double c_isd_log_cost(const uint32_t n, 
                      const uint32_t k,
                      const uint32_t t,
                      const uint32_t qc_order, 
                      const uint32_t is_kra) {
    double min_cost = n, current_cost;
    /* for key recovery attacks the advantage from quasi-cyclicity is p, 
     * for an ISD, the DOOM advantage is just sqrt(p) */
    double qc_red_factor= is_kra ? logl(qc_order) : logl(qc_order)/2.0;
    qc_red_factor = qc_red_factor/logl(2);

    std::cout << "Classic ";
    current_cost = isd_log_cost_classic_Prange(n,k,t) - qc_red_factor;
    std::cerr << "Classic Prange: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = current_cost;

    current_cost = isd_log_cost_classic_LB(n,k,t)- qc_red_factor;
    std::cerr << "Classic Lee-Brickell ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;

    current_cost = isd_log_cost_classic_Leon(n,k,t)- qc_red_factor;
     std::cerr << "Classic Leon ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;

    current_cost = isd_log_cost_classic_Stern(n,k,t)- qc_red_factor;
    std::cerr << "Classic Stern ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;

    current_cost = isd_log_cost_classic_FS(n,k,t)- qc_red_factor;
    std::cerr << "Classic Fin-Send ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;

    current_cost = isd_log_cost_classic_MMT(n,k,t)- qc_red_factor;
    std::cerr << "Classic MMT ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;

#if SKIP_BJMM == 0
    current_cost = isd_log_cost_classic_BJMM(n,k,t)- qc_red_factor;
    std::cerr << "Classic BJMM ISD: " << std::setprecision(5) << current_cost << std::endl;
    std::cout << current_cost << " ";
    min_cost = min_cost > current_cost ? current_cost : min_cost;
#endif
    std::cout << std::endl;

    return min_cost;
}

double q_isd_log_cost(const uint32_t n, 
                      const uint32_t k, 
                      const uint32_t t,
                      const uint32_t qc_order, 
                      const uint32_t is_kra) {
    double min_cost = n, current_cost;
    /* for key recovery attacks the advantage from quasi-cyclicity is p, 
     * for an ISD, the DOOM advantage is just sqrt(p) */
    double qc_red_factor= is_kra ? logl(qc_order) : logl(qc_order)/2.0;
    qc_red_factor = qc_red_factor/logl(2);
    std::cout << "Quantum ";

    current_cost = isd_log_cost_quantum_LB(n,k,t)- qc_red_factor;
    std::cout << current_cost << " ";
//     std::cout << " Q-Lee-Brickell ISD: " << /**/current_cost << std::endl;
    min_cost = current_cost;

    current_cost = isd_log_cost_quantum_stern(n, k, t)- qc_red_factor;
    std::cout << current_cost << " ";
//     std::cout << ", Q-Stern ISD: " << current_cost << std::endl;
    min_cost = min_cost > current_cost ? current_cost : min_cost;
    std::cout << std::endl;

    return min_cost;
}

