#include <algorithm>
#include <random>
#include <iostream>
#include <chrono>
#include <climits>
#include <map>
#include <assert.h>

#include "board.hpp"
#include "engine.hpp"

#define WT_MULTIPLIER 1000

bool time_check(std::chrono::high_resolution_clock::time_point& start_time){
    auto cur_time=std::chrono::high_resolution_clock::now();
    auto duration=std::chrono::duration_cast<std::chrono::milliseconds>(cur_time-start_time);
    if(duration.count()>1900)
    {
        std::cout<<"Time up"<<std::endl;
        return 0;
    }
    return 1;
}

int evaluation_func(Board* board_copy,PlayerColor my_col)
{
    // int mult1=20000,mult2=500,mult3=1;
    int w_king=7*WT_MULTIPLIER,w_bishop=5*WT_MULTIPLIER,w_rook=3*WT_MULTIPLIER,w_pawn=1*WT_MULTIPLIER;    
    // int w_threat_bishop=3,w_threat_rook=2,w_threats_pawn=1;
    int w_moves=1;    

    PlayerColor curr_col=board_copy->data.player_to_play;
    std::unordered_set<U16> moveset[2];     //0-WHITE, 1-BLACK
    
    board_copy->data.player_to_play=WHITE;
    moveset[0]=board_copy->get_legal_moves();
    board_copy->data.player_to_play=BLACK;
    moveset[1]=board_copy->get_legal_moves();

    board_copy->data.player_to_play=curr_col;
    
    BoardData *d = &(board_copy->data);




    if( ( (curr_col==WHITE && moveset[0].size()==0) || (curr_col==BLACK && moveset[1].size()==0) )  && board_copy->in_check())  //checkmate
    {
        if(curr_col==my_col)
            return INT_MIN;
        else
            return INT_MAX;

    }
    //  In stalement points pieces alive can determine the points obtained
    // else if( (curr_col==WHITE && moveset[0].size()==0) || (curr_col==BLACK && moveset[1].size()==0) )  //stalemate
    //     return Value(0,0,0);
    
    U8 black_pawn_1= (d->b_pawn_bs!=DEAD) ? d->board_0[d->b_pawn_bs] : 0;
    U8 black_pawn_2= (d->b_pawn_ws!=DEAD) ? d->board_0[d->b_pawn_ws] : 0;
    U8 white_pawn_1= (d->w_pawn_bs!=DEAD) ? d->board_0[d->w_pawn_bs] : 0;
    U8 white_pawn_2= (d->w_pawn_ws!=DEAD) ? d->board_0[d->w_pawn_ws] : 0;
    
    int num_rooks[2]={0};
    int num_bishops[2]={0};
    int num_pawns[2]={0};
    num_rooks[0]=(d->w_rook_bs!=DEAD) + (d->w_rook_ws!=DEAD) + ((white_pawn_1 & ROOK)!=0) + ((white_pawn_2 & ROOK)!=0);
    num_rooks[1]=(d->b_rook_bs!=DEAD) + (d->b_rook_ws!=DEAD) + ((black_pawn_1 & ROOK)!=0) + ((black_pawn_2 & ROOK)!=0);
    num_bishops[0]=(d->w_bishop!=DEAD) + ((white_pawn_1 & BISHOP)!=0) + ((white_pawn_2 & BISHOP)!=0);
    num_bishops[1]=(d->b_bishop!=DEAD) + ((black_pawn_1 & BISHOP)!=0) + ((black_pawn_2 & BISHOP)!=0);
    num_pawns[0]=((white_pawn_1 & PAWN)!=0) + ((white_pawn_2 & PAWN)!=0);
    num_pawns[1]=((black_pawn_1 & PAWN)!=0) + ((black_pawn_2 & PAWN)!=0);


    //threat calculation
    // int threat_pawns[2]={0};
    // int threat_rooks[2]={0};
    // int threat_bishops[2]={0};
    // for(int i=0;i<2;i++)
    // {
    //     for(auto move:moveset[1-i])
    //     {
    //         U8 position=getp1(move);           
    //         U8 piecetype=board_copy->data.board_0[position];
    //         if(piecetype!=EMPTY)
    //         {
    //             if(piecetype & PAWN)            threat_pawns[i]+=1;
    //             else if(piecetype & ROOK)       threat_rooks[i]+=1;
    //             else if(piecetype & BISHOP)     threat_bishops[i]+=1;
    //         }
    //     }
    // }



    int evaluation=0;
    
    evaluation+=(num_rooks[0]-num_rooks[1])*w_rook;
    evaluation+=(num_bishops[0]-num_bishops[1])*w_bishop;
    evaluation+=(num_pawns[0]-num_pawns[1])*w_pawn;

    // evaluation+=(threat_bishops[0]-threat_bishops[1])*w_threat_bishop;
    // evaluation+=(threat_rooks[0]-threat_rooks[1])*w_threat_rook;
    // evaluation+=(threat_pawns[0]-threat_pawns[1])*w_threats_pawn;
    
    // evaluation+=(moveset[0].size()-moveset[1].size())*w_moves;
    
    if(my_col==BLACK)
    {
        evaluation=-evaluation;
    }

    if(my_col==curr_col)
        evaluation -= w_king*(board_copy->in_check());
    else
        evaluation += w_king*(board_copy->in_check());

    //now can also add weight of pieces in threat
    // if(count>=3)
    // {
        // std::cout<<"YAY Parinting"<<std::endl;
        // std::cout<<"Pawns = "<<num_pawns[0]<<","<<num_pawns[1]<<std::endl;
        // std::cout<<"Rooks = "<<num_rooks[0]<<","<<num_rooks[1]<<std::endl;
        // std::cout<<"Bishops = "<<num_bishops[0]<<","<<num_bishops[1]<<std::endl;
        // std::cout<<"Moves = "<<moveset[0].size()<<","<<moveset[1].size()<<std::endl;
    // }
    return (evaluation+(moveset[0].size()-moveset[1].size())*w_moves);      

}

int minval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col);

int maxval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col)
{
    if(curr_depth<=0  || time_check(start_time)==0)
    {
        int temp=evaluation_func(b,my_col);
        std::cout<<"MAXVAL Output = "<<temp<<std::endl;
        return temp;
    }   

    auto moveset=b->get_legal_moves();
    int ans=INT_MIN;
    for(auto move:moveset)
    {
        Board* temp_b=b->copy();
        temp_b->do_move(move);
        int temp_ans=minval(temp_b,curr_depth-1,alpha,beta,start_time,my_col);
        ans=std::max(ans,temp_ans);
        delete temp_b;
        alpha=std::max(alpha,ans);
        if(alpha>=beta)
            break;
    }
    std::cout<<"MAXVAL Output = "<<ans<<std::endl;
    return ans;
}

int minval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col)
{
    if(curr_depth<=0 || time_check(start_time)==0)
    {
        int temp=evaluation_func(b,my_col);
        std::cout<<"MINVAL Output = "<<temp<<std::endl;
        return temp;
    }
    auto moveset=b->get_legal_moves();
    int ans=INT_MAX;
    for(auto move:moveset)
    {
        Board* temp_b=b->copy();
        temp_b->do_move(move);
        int temp_ans=maxval(temp_b,curr_depth-1,alpha,beta,start_time,my_col);
        ans=std::min(ans,temp_ans);
        delete temp_b;
        beta=std::min(beta,ans);
        if(alpha>=beta)
            break;
    }
    std::cout<<"MINVAL Output = "<<ans<<std::endl;
    return ans;
}

std::vector<std::pair<int,U16>> minimax(Board *b, int curr_depth,std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col)
{
    std::vector<std::pair<int,U16>> ans;
    auto moveset=b->get_legal_moves();
    for(auto move : moveset)
    {
        Board* temp_b=b->copy();
        temp_b->do_move(move);
        int val = minval(temp_b,curr_depth-1,INT_MIN,INT_MAX,start_time,my_col);
        delete temp_b;
        ans.push_back(std::pair<int,U16>(val,move));
    }
    sort(ans.begin(),ans.end());
    return ans;
}

void Engine::find_best_move(const Board& b) 
{
    static int counter=-1;
    auto start_time=std::chrono::high_resolution_clock::now();
    auto moveset = b.get_legal_moves();
    if (moveset.size() == 0) 
    {
        this->best_move = 0;
    }
    else
    {
        int opening=1;
        if(b.data.player_to_play==BLACK)
        {
            if(b.data.b_bishop==DEAD || b.data.b_rook_bs==DEAD || b.data.b_rook_ws==DEAD || b.data.b_pawn_bs==DEAD || b.data.b_pawn_ws==DEAD || b.in_check())
                opening=0;
        }
        else
        {
            if(b.data.w_bishop==DEAD || b.data.w_rook_bs==DEAD || b.data.w_rook_ws==DEAD || b.data.w_pawn_bs==DEAD || b.data.w_pawn_ws==DEAD || b.in_check())            
                opening=0;
        }
        counter++;
        if(opening)
        {
            U16 preferred_move=0;
            if(b.data.player_to_play==WHITE)
            {
                switch(counter)
                {
                    case 0: preferred_move=move(pos(4,1),pos(5,1));
                    break;
                    case 1: preferred_move=move(pos(3,0),pos(4,1));
                    break;
                    case 2: preferred_move=move(pos(2,0),pos(1,1));
                    break;
                    case 3: preferred_move=move(pos(4,0),pos(0,0));
                    break;
                    case 4: preferred_move=move(pos(5,1),pos(5,0));
                    break;
                }
            }
            else
            {
                switch(counter)
                {
                    case 0: preferred_move=move(pos(2,5),pos(1,5));
                    break;
                    case 1: preferred_move=move(pos(3,6),pos(2,5));
                    break;
                    case 2: preferred_move=move(pos(4,6),pos(5,5));
                    break;
                    case 3: preferred_move=move(pos(2,6),pos(6,6));
                    break;
                    case 4: preferred_move=move(pos(1,5),pos(1,6));
                    break;
                }
            }
            
            if(preferred_move!=0)
            {
                for(auto move:moveset)
                {
                    if(preferred_move==move)
                    {
                        std::cout<<"----------------Doing Preferred Move--------------"<<std::endl;
                        this->best_move=preferred_move;
                        return;
                    }
                }
            }
        }

        Board* temp_b=b.copy();
        int max_depth=3;
        auto ans=minimax(temp_b,max_depth,start_time,b.data.player_to_play);
        delete temp_b;
        this->best_move=ans[ans.size()-1].second;

        // if(preferred_move==0)
        // {
        //     for(auto p:ans)
        //     {
        //         std::cout<<"Value="<<p.first<<" "<<"Move="<<move_to_str(p.second)<<std::endl;
        //     }
        //     std::cout<<"selected move = "<<move_to_str(this->best_move)<<std::endl;
        //     // exit(0);
        // }
    }
}
