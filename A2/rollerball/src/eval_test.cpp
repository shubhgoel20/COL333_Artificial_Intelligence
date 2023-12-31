#include <cstdint>
#include <string>
#include <climits>
#include <iostream>
#include <chrono>
#include <algorithm>
#include "board.hpp"


#define WT_MULTIPLIER 1000


constexpr U8 cw_90[64] = {
    48, 40, 32, 24, 16, 8,  0,  7,
    49, 41, 33, 25, 17, 9,  1,  15,
    50, 42, 18, 19, 20, 10, 2,  23,
    51, 43, 26, 27, 28, 11, 3,  31,
    52, 44, 34, 35, 36, 12, 4,  39,
    53, 45, 37, 29, 21, 13, 5,  47,
    54, 46, 38, 30, 22, 14, 6,  55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 acw_90[64] = {
     6, 14, 22, 30, 38, 46, 54, 7,
     5, 13, 21, 29, 37, 45, 53, 15,
     4, 12, 18, 19, 20, 44, 52, 23,
     3, 11, 26, 27, 28, 43, 51, 31,
     2, 10, 34, 35, 36, 42, 50, 39,
     1,  9, 17, 25, 33, 41, 49, 47,
     0,  8, 16, 24, 32, 40, 48, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 cw_180[64] = {
    54, 53, 52, 51, 50, 49, 48, 7,
    46, 45, 44, 43, 42, 41, 40, 15,
    38, 37, 18, 19, 20, 33, 32, 23,
    30, 29, 26, 27, 28, 25, 24, 31,
    22, 21, 34, 35, 36, 17, 16, 39,
    14, 13, 12, 11, 10,  9,  8, 47,
     6,  5,  4,  3,  2,  1,  0, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 id[64] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

#define cw_90_pos(p) cw_90[p]
#define cw_180_pos(p) cw_180[p]
#define acw_90_pos(p) acw_90[p]
#define cw_90_move(m) move_promo(cw_90[getp0(m)], cw_90[getp1(m)], getpromo(m))
#define acw_90_move(m) move_promo(acw_90[getp0(m)], acw_90[getp1(m)], getpromo(m))
#define cw_180_move(p) move_promo(cw_180[getp0(m)], cw_180[getp1(m)], getpromo(m))
#define color(p) ((PlayerColor)(p & (WHITE | BLACK)))

void rotate_board_new(U8 *src, U8 *tgt, const U8 *transform) {

    for (int i=0; i<64; i++) {
        tgt[transform[i]] = src[i];
    }
}


inline int weight(U8 piece)
{
    int wt_multiplier=1000;
    if(piece & KING)    return 2*wt_multiplier;
    else if(piece & BISHOP) return 4*wt_multiplier;
    else if(piece & ROOK)   return 3*wt_multiplier;
    else if(piece & PAWN)   return 2*wt_multiplier;
    else return 0;
}

inline int threat_weight(U8 piece)
{
    int wt_multiplier=5;
    if(piece & KING)    return 2*wt_multiplier;
    else if(piece & BISHOP) return 4*wt_multiplier;
    else if(piece & ROOK)   return 3*wt_multiplier;
    else if(piece & PAWN)   return 2*wt_multiplier;
    else return 0;
}



int evaluation_func(Board* board_copy,PlayerColor my_col)
{
    int w_moves=1;    

    int weights[7][7]={
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
        {1,0,1,0,1,0,1},
    };


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
    
    U8 black_pawn_1= (d->b_pawn_bs!=DEAD) ? d->board_0[d->b_pawn_bs] : 0;
    U8 black_pawn_2= (d->b_pawn_ws!=DEAD) ? d->board_0[d->b_pawn_ws] : 0;
    U8 white_pawn_1= (d->w_pawn_bs!=DEAD) ? d->board_0[d->w_pawn_bs] : 0;
    U8 white_pawn_2= (d->w_pawn_ws!=DEAD) ? d->board_0[d->w_pawn_ws] : 0;
    
    int num_rooks[2]={0};
    int num_bishops[2]={0};
    int num_pawns[2]={0};

    //promotion to rook is favoured over promotion to pawn
    num_rooks[0]=(d->w_rook_bs!=DEAD) + (d->w_rook_ws!=DEAD) + ((white_pawn_2 & BISHOP)!=0);
    num_rooks[1]=(d->b_rook_bs!=DEAD) + (d->b_rook_ws!=DEAD) + ((black_pawn_2 & BISHOP)!=0);

    num_bishops[0]=(d->w_bishop!=DEAD) + ((white_pawn_1 & BISHOP)!=0) + ((white_pawn_1 & ROOK)!=0) + ((white_pawn_2 & ROOK)!=0);
    num_bishops[1]=(d->b_bishop!=DEAD) + ((black_pawn_1 & BISHOP)!=0) + ((black_pawn_1 & ROOK)!=0) + ((black_pawn_2 & ROOK)!=0);

    num_pawns[0]=((white_pawn_1 & PAWN)!=0) + ((white_pawn_2 & PAWN)!=0);
    num_pawns[1]=((black_pawn_1 & PAWN)!=0) + ((black_pawn_2 & PAWN)!=0);

    int evaluation=0;
    
    evaluation+=(num_rooks[0]-num_rooks[1])*weight(ROOK);
    evaluation+=(num_bishops[0]-num_bishops[1])*weight(BISHOP);
    evaluation+=(num_pawns[0]-num_pawns[1])*weight(PAWN);
    evaluation+=(moveset[0].size()-moveset[1].size())*w_moves;

    int black_wt=0,white_wt=0;
    // if(board_copy->data.b_bishop!=DEAD)
    //     white_wt=1;
    // if(board_copy->data.w_bishop!=DEAD)
    //     black_wt=1;

    U8 *pieces = (U8*)board_copy;
    for (int i=0; i<12; i++) {
        U8 position = pieces[i];
        if(position==DEAD)  continue;
        U8 piece = board_copy->data.board_0[position];
        if(piece & WHITE)
            evaluation+=weights[getx(position)][gety(position)]*threat_weight(piece)*white_wt;
        else
            evaluation+=weights[getx(position)][gety(position)]*threat_weight(piece)*black_wt;
    }
   
    if(my_col==BLACK)
    {
        evaluation=-evaluation;
    }

    if(my_col==curr_col)
        evaluation -= weight(KING)*(board_copy->in_check());
    else
        evaluation += weight(KING)*(board_copy->in_check());

    return (evaluation);      
}


void set_pieces(Board * board)
{

    board->data.board_0[board->data.b_rook_ws]  = 0;
    board->data.board_0[board->data.b_rook_bs]  = 0;
    board->data.board_0[board->data.b_king   ]  = 0;
    board->data.board_0[board->data.b_bishop ]  = 0;
    board->data.board_0[board->data.b_pawn_ws]  = 0;
    board->data.board_0[board->data.b_pawn_bs]  = 0;

    board->data.board_0[board->data.w_rook_ws]  = 0;
    board->data.board_0[board->data.w_rook_bs]  = 0;
    board->data.board_0[board->data.w_king   ]  = 0;
    board->data.board_0[board->data.w_bishop ]  = 0;
    board->data.board_0[board->data.w_pawn_ws]  = 0;
    board->data.board_0[board->data.w_pawn_bs]  = 0;

    rotate_board_new(board->data.board_0, board->data.board_90, cw_90);
    rotate_board_new(board->data.board_0, board->data.board_180, cw_180);
    rotate_board_new(board->data.board_0, board->data.board_270, acw_90);

    board->data.b_king=pos(3,5);
    board->data.b_bishop=pos(7,7);
    board->data.b_rook_bs=pos(1,5);
    board->data.b_rook_ws=pos(5,6);
    board->data.b_pawn_bs=pos(6,6);
    board->data.b_pawn_ws=pos(5,2);

    board->data.w_king=pos(3,1);
    board->data.w_bishop=pos(2,1);
    board->data.w_rook_bs=pos(4,0);
    board->data.w_rook_ws=pos(5,1);
    board->data.w_pawn_bs=pos(7,7);
    board->data.w_pawn_ws=pos(1,3);
    
    // board->data.w_king=pos(3,0);
    // board->data.w_bishop=DEAD;
    // board->data.w_rook_bs=DEAD;
    // board->data.w_rook_ws=DEAD;
    // board->data.w_pawn_bs=DEAD;
    // board->data.w_pawn_ws=DEAD;

    board->data.board_0[board->data.b_rook_ws]  = BLACK | ROOK;
    board->data.board_0[board->data.b_rook_bs]  = BLACK | ROOK;
    board->data.board_0[board->data.b_king   ]  = BLACK | KING;
    board->data.board_0[board->data.b_bishop ]  = BLACK | BISHOP;
    board->data.board_0[board->data.b_pawn_ws]  = BLACK | PAWN;
    board->data.board_0[board->data.b_pawn_bs]  = BLACK | PAWN;

    board->data.board_0[board->data.w_rook_ws]  = WHITE | ROOK;
    board->data.board_0[board->data.w_rook_bs]  = WHITE | ROOK;
    board->data.board_0[board->data.w_king   ]  = WHITE | KING;
    board->data.board_0[board->data.w_bishop ]  = WHITE | BISHOP;
    board->data.board_0[board->data.w_pawn_ws]  = WHITE | PAWN;
    board->data.board_0[board->data.w_pawn_bs]  = WHITE | PAWN;

    rotate_board_new(board->data.board_0, board->data.board_90, cw_90);
    rotate_board_new(board->data.board_0, board->data.board_180, cw_180);
    rotate_board_new(board->data.board_0, board->data.board_270, acw_90);

    board->data.player_to_play=WHITE;
}


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

int minval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col);

int maxval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col)
{
    if(curr_depth<=0  || time_check(start_time)==0)
    {
        int temp=evaluation_func(b,my_col);
        // std::cout<<"MAXVAL Output = "<<temp<<std::endl;
        return temp;
    }   

    auto moveset=b->get_legal_moves();
    int ans=INT_MIN;
    for(auto move:moveset)
    {
        if(time_check(start_time)==0)   break;
        Board* temp_b=b->copy();
        temp_b->do_move(move);
        int temp_ans=minval(temp_b,curr_depth-1,alpha,beta,start_time,my_col);
        ans=std::max(ans,temp_ans);
        delete temp_b;
        alpha=std::max(alpha,ans);
        if(alpha>=beta)
            break;
    }
    // std::cout<<"MAXVAL Output = "<<ans<<std::endl;
    return ans;
}

int minval(Board *b, int curr_depth, int alpha, int beta, std::chrono::high_resolution_clock::time_point& start_time,PlayerColor my_col)
{
    if(curr_depth<=0 || time_check(start_time)==0)
    {
        int temp=evaluation_func(b,my_col);
        // std::cout<<"MINVAL Output = "<<temp<<std::endl;
        return temp;
    }
    auto moveset=b->get_legal_moves();

    int ans=INT_MAX;
    U16 best_move;
    std::string before_move=board_to_str(b->data.board_0);
    std::string after_move;
    for(auto move:moveset)
    {
        if(time_check(start_time)==0)   break;
        Board* temp_b=b->copy();
        temp_b->do_move(move);
        int temp_ans=maxval(temp_b,curr_depth-1,alpha,beta,start_time,my_col);
        if(ans>temp_ans)
        {
            best_move=move;
            after_move=board_to_str(temp_b->data.board_0);
        }
        ans=std::min(ans,temp_ans);
        delete temp_b;
        beta=std::min(beta,ans);
        if(alpha>=beta)
            break;
    }
    // std::cout<<"MINVAL Output = "<<ans<<std::endl;
    std::cout<<"OPTIMAL MOVE"<<std::endl;
    std::cout<<"Move = "<<move_to_str(best_move)<<std::endl;
    std::cout<<"Before Move:"<<std::endl;
    std::cout<<before_move;
    std::cout<<"After Move:"<<std::endl;
    std::cout<<after_move;
    std::cout<<"Value:"<<ans<<std::endl;
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


int main()
{
    Board new_board;
    set_pieces(&new_board);
    // std::cout<<board_to_str(new_board.data.board_0)<<std::endl;
    // std::cout<<all_boards_to_str(new_board)<<std::endl;
    // std::cout<<evaluation_func(&new_board,WHITE)<<std::endl;
    auto start_time=std::chrono::high_resolution_clock::now();
    // auto ans=minimax(&new_board,2,start_time,WHITE);
    // std::cout<<move_to_str(ans[ans.size()-1].second)<<std::endl;

    std::cout<<"Current value = "<<evaluation_func(&new_board,WHITE)<<std::endl;

    U16 move1=str_to_move("f2f3");
    U16 move2=str_to_move("f2f1");
    
    std::cout<<"MOVE 1"<<std::endl;
    Board* b_copy=new_board.copy();
    b_copy->do_move(move1);
    std::cout<<board_to_str(b_copy->data.board_0)<<std::endl;
    int val1=minval(b_copy,1,INT_MIN,INT_MAX,start_time,WHITE);
    delete b_copy;

    std::cout<<"MOVE 2"<<std::endl;
    b_copy=new_board.copy();
    b_copy->do_move(move2);
    std::cout<<board_to_str(b_copy->data.board_0)<<std::endl;
    int val2=minval(b_copy,1,INT_MIN,INT_MAX,start_time,WHITE);
    delete b_copy;

    std::cout<<val1<<" "<<val2<<std::endl;
    return 0;
}