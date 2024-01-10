#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <list>
#include <unordered_map>

using namespace std;

class Block{
public:
    int id;
    int width;
    int height;
    int x;
    int y;
    bool is_rotate;
    Block *parent;
    Block *l_child;
    Block *r_child;
    Block(int a, int b, int c){
        id=a;
        width=b;
        height=c;
        x=0;
        y=0;
        is_rotate=0;
        parent=nullptr;
        l_child=nullptr;
        r_child=nullptr;
    }
    void rotate(){
        this->is_rotate=!this->is_rotate;
        int tmp=width;
        this->width=this->height;
        this->height=tmp;
    }
};

class  Result_block{
public:
    bool exist;
    int id;
    int width;
    int height;
    int x;
    int y;
    int is_rotate;
    int parent;
    int l_child;
    int r_child;

    Result_block(Block *b){
        if(b==nullptr)
            exist=0;
        else{
            exist=1;
            id=b->id;
            width=b->width;
            height=b->height;
            x=b->x;
            y=b->y;
            is_rotate=b->is_rotate;
            if(b->parent!=nullptr)
                parent=b->parent->id;
            else
                parent=-1;
            if(b->l_child!=nullptr)
                l_child=b->l_child->id;
            else
                l_child=-1;
            if(b->r_child!=nullptr)
                r_child=b->r_child->id;
            else
                r_child=-1;
        }
    }
};

struct Contour{
    int start;
    int end;
    int y;
    Contour(int a, int b, int c){
        start=a;
        end=b;
        y=c;
    }
};

vector<Block*> blocks;
Block *root;
double R_lower, R_upper;
list<Contour> y_contour;
vector<Result_block> result_vec, best_vec;
pair<int,int> best_wl;
int best_area=2147483647, best_root_index, store_root_index;
unordered_map<string, int> map;

Block *initial_tree(){
    Block *prev, *root;
    root=blocks[0];
    root->parent=nullptr;
    store_root_index=0;
    prev=blocks[0];

    srand(time(NULL));
    for(int i=1; i<blocks.size(); i++){
        if(blocks[i]!=nullptr){
            bool lr=rand()%2;
            if(lr){
                prev->l_child=blocks[i];
                prev->r_child=nullptr;
            }
            else{
                prev->r_child=blocks[i];
                prev->l_child=nullptr;
            }
            blocks[i]->parent=prev;
            prev=blocks[i];
        }
    }
    prev->l_child=nullptr;
    prev->r_child=nullptr;
    return root;
}

int update_contour(const int &start, const int &end, const int &height){
    int max_y=0;
    auto it=y_contour.begin();
    auto from=y_contour.end(), to=y_contour.end();

    for(it; it!=y_contour.end(); it++){
        if(it->end>start){//代表進到start跟end的範圍內
            from=it;
            to=it;
            break;
        }
    }
    while(it!=y_contour.end()){
        if(it->start<end){
            max_y=max(max_y,it->y);
            ++it;
        }
        else{
            --it;
            to=it;
            break;
        }
    }
    if(it==y_contour.end())
        to=--it;

    it=to;
    if(from->start<start)//左邊沒重疊到
        y_contour.insert(from,Contour(from->start, start, from->y));
    if(to->end>end){//右邊沒重疊到
        it++;
        y_contour.insert(it,Contour(end, to->end, to->y));
        --it;
        --it;
    }
    if(from==to){//只跟一個方塊重疊
        it->start=start;
        it->end=end;
        it->y=max_y+height;
    }
    else{//跟多個方塊重疊
        it=y_contour.erase(from,to);
        it=to;
        it++;
        y_contour.erase(to);
        y_contour.insert(it, Contour(start,end,max_y+height));
    }
    return max_y;
}

void cal_xy(Block *node, const bool &is_left, int &x, int &y){
    if(node==nullptr)
        return;
    if(is_left)
        node->x=node->parent->x+node->parent->width;
    else
        node->x=node->parent->x;
    int y_coo=update_contour(node->x, node->x+node->width, node->height);
    node->y=y_coo;
    x=max(x, node->x+node->width);
    y=max(y, node->y+node->height);
    cal_xy(node->l_child, 1, x, y);
    cal_xy(node->r_child, 0, x, y);    
}

pair<int,int> cal_WL(){
    int x=0, y=0;
    y_contour.clear();
    y_contour.push_back(Contour(0,2147483647,0));
    root->x=0;
    root->y=0;
    update_contour(0, root->width, root->height);
    cal_xy(root->l_child, 1, x, y);
    cal_xy(root->r_child, 0, x, y);
    return {x,y};
}

void perturbation(){
    int op = rand()%3;  
    if(op==0){//rotate a macro
        int index=rand()%blocks.size();
        while(blocks[index]==nullptr)
            index++;
        blocks[index]->rotate();
    }
    else if(op==1){//swap 2 nodes
        int a_index=rand()%blocks.size();
        int b_index=rand()%blocks.size();
        while(blocks[a_index]==nullptr)
            a_index=rand()%blocks.size();
        while((a_index==b_index)||(blocks[b_index]==nullptr))
            b_index=rand()%blocks.size();
        Block *a=blocks[a_index], *b=blocks[b_index];
  
        if((b->parent==a)||(a->parent==b)){//a是b parent
            if(a->parent==b){//b是a parent
                swap(a_index, b_index);
                a=blocks[a_index];
                b=blocks[b_index];
            }
            Block *a_p=a->parent;
            Block *a_l=a->l_child;
            Block *a_r=a->r_child;
            Block *b_p=b->parent;
            Block *b_l=b->l_child;
            Block *b_r=b->r_child;      
            if(a_p!=nullptr){
                if(a_p->l_child==a)
                    a_p->l_child=b;
                else if(a_p->r_child==a)
                    a_p->r_child=b;
                b->parent=a_p;
            }
            else
                b->parent=nullptr;
            a->parent=b;

            if(a->l_child==b){
                a->l_child=b_l;
                a->r_child=b_r;
                b->l_child=a;
                b->r_child=a_r;
                if(a_r!=nullptr)
                    a_r->parent=b;
                if(b_l!=nullptr)
                    b_l->parent=a;
                if(b_r!=nullptr)
                    b_r->parent=a;
            }
            else if(a->r_child==b){
                a->l_child=b_l;
                a->r_child=b_r;
                b->l_child=a_l;
                b->r_child=a;
                if(a_l!=nullptr)
                    a_l->parent=b;
                if(b_l!=nullptr)
                    b_l->parent=a;
                if(b_r!=nullptr)
                    b_r->parent=a;
            }
            if(a_p==nullptr)
                root=blocks[b_index];
            else if(b_p==nullptr)
                root=blocks[a_index];
        }
        else{
            Block *a_p=a->parent;
            Block *a_l=a->l_child;
            Block *a_r=a->r_child;
            Block *b_p=b->parent;
            Block *b_l=b->l_child;
            Block *b_r=b->r_child;   

            if(a_p==nullptr)
                root=blocks[b_index];
            else if(b_p==nullptr)
                root=blocks[a_index];

            a->l_child=b_l;
            a->r_child=b_r;
            a->parent=b_p;
            if(b_l!=nullptr)
                b_l->parent=a;
            if(b_r!=nullptr)
                b_r->parent=a;
            if(b_p!=nullptr){
                if(b_p->l_child==b)
                    b_p->l_child=a;
                else if(b_p->r_child==b)
                    b_p->r_child=a;
            }

            b->l_child=a_l;
            b->r_child=a_r;
            b->parent=a_p;
            if(a_l!=nullptr)
                a_l->parent=b;
            if(a_r!=nullptr)
                a_r->parent=b;
            if(a_p!=nullptr){
                if(a_p->l_child==a)
                    a_p->l_child=b;
                else if(a_p->r_child==a)
                    a_p->r_child=b;
            }
        }
    }
    else{//delete & insert
        int a_index=rand()%blocks.size();
        while(blocks[a_index]==nullptr)
            a_index=rand()%blocks.size();
        Block *a=blocks[a_index];

        while(true){
            if(a->l_child!=nullptr)
                a=a->l_child;
            else if(a->r_child!=nullptr)
                a=a->r_child;
            else
                break;
        }

        a_index=a->id;
        int b_index=rand()%blocks.size();
        while((a_index==b_index)||(blocks[b_index]==nullptr)||(blocks[b_index]->l_child!=nullptr && blocks[b_index]->r_child!=nullptr))
            b_index=rand()%blocks.size();

        Block *b=blocks[b_index];

        if(a->parent->l_child==a)
            a->parent->l_child=nullptr;
        else if(a->parent->r_child==a)
            a->parent->r_child=nullptr;

        a->parent=b;

        if(b->l_child==nullptr)
            b->l_child=a;
        else if(b->r_child==nullptr)
            b->r_child=a;
    }
}

bool prob(const int &cost, const double &temp){
    double p=exp(-(cost/temp));
    p*=100;
    int r=rand()%100;
    if(r<p)
        return 1;
    else
        return 0;
}

void store_result(){
    result_vec.clear();
    for(auto n:blocks)
        result_vec.push_back(Result_block(n));
    store_root_index=root->id;
}

void restore_result(vector<Result_block> &result){
    for(int i=0; i<blocks.size(); i++){
        if(blocks[i]!=nullptr){
            blocks[i]->width=result[i].width;
            blocks[i]->height=result[i].height;
            blocks[i]->x=result[i].x;
            blocks[i]->y=result[i].y;
            blocks[i]->is_rotate=result[i].is_rotate;
            if(result[i].parent>=0)
                blocks[i]->parent=blocks[result[i].parent];
            else
                blocks[i]->parent=nullptr;

            if(result[i].l_child>=0)
                blocks[i]->l_child=blocks[result[i].l_child];
            else
                blocks[i]->l_child=nullptr;
            if(result[i].r_child>=0)
                blocks[i]->r_child=blocks[result[i].r_child];
            else
                blocks[i]->r_child=nullptr;
        }
    }
    root=blocks[store_root_index];
}

void SA(){
    root=initial_tree();
    pair<int,int> wl=cal_WL();
    store_result();
    double asprat=(double)wl.first/wl.second;
    int prevcost=wl.first*wl.second, nextcost;
    double temp=100000, cool=0.85;
    int P=10*blocks.size();
    int diff;
    while(temp>0.01){
        for(int i=0; i<P; i++){
            perturbation();             
            wl=cal_WL();
            nextcost=wl.first*wl.second;
            diff=nextcost-prevcost;
            asprat=(double)wl.first/wl.second;
            bool flag=0;
            if(diff<0){
                store_result(); 
                flag=1;      
            }
            else{
                bool go=prob(diff, temp);
                if(go){
                    store_result();     
                    flag=1;
                }
                              
                else            
                    restore_result(result_vec); 
            }
            if(flag){
                if((R_lower<asprat)&&(asprat<R_upper)){
                    if(best_area>nextcost){
                        best_vec=result_vec;
                        best_wl=wl;
                        best_area=nextcost;
                        best_root_index=store_root_index;                    
                    }
                }
            }
            prevcost=nextcost;
        }
        temp*=cool;
    }
}

int main(int argc , char *argv[]){
//read file
    ifstream input;
    ofstream output;
    input.open(argv[1]);
    output.open(argv[2]);
    input >> R_lower >> R_upper;
    string keyword;
    int size=0;
    while(!input.eof()){
        int tmp_w, tmp_h;
        input >> keyword;
        map[keyword]=size;
        input >> keyword;
        tmp_w=stoi(keyword);
        input >> keyword;
        tmp_h=stoi(keyword);
        Block *b=new Block(size, tmp_w, tmp_h);   
        blocks.push_back(b); 
        size++;    
    }
    input.close();
    while(true){
        SA();
        double asp=(double)best_wl.first/best_wl.second;
        if((R_lower<asp)&&(asp<R_upper)){
            for(int i=0; i<2; i++){
                SA();           
            }
            break;
        }
    }
    output << "A = "  << best_area << endl;
    output << "R = "  << (double)best_wl.first/best_wl.second << endl;
    for(auto b:map){
        if(best_vec[b.second].exist){
            output << b.first << " " << best_vec[b.second].x << " " << best_vec[b.second].y;
            if(best_vec[b.second].is_rotate)
                output << " R";
            if(size>1){
                output << endl;  
                size--;
            }                
        }
    }
    output.close();
    return 0;
}