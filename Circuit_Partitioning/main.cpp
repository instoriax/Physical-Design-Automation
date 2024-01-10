#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cmath>

using namespace std;

struct Cell{
    Cell *prev;
    Cell *next;
    int id;
    int gain;
    Cell(){
        prev=nullptr;
        next=nullptr;
        id=0;
        gain=0;
    }
};

struct Net{
    int zero;
    int one;
    Net(){
        zero=0;
        one=0;
    }
};

struct Rec{
    int id, total_gain, pardif;
};

int best_index, Pmax=0;
vector<Rec> record;
vector<Cell*> gain_list,cell_pointer;
vector<Net*> net_pointer;

void insert(Cell *c){
    int index=c->gain+Pmax;
    if(gain_list[index]!=nullptr){
        gain_list[index]->prev=c;
        c->next=gain_list[index];
    }
    else
        c->next=nullptr;
    gain_list[index]=c;
}

void pull(Cell *c){
    if(gain_list[c->gain+Pmax]==c)
        gain_list[c->gain+Pmax]=c->next;
    else{
        c->prev->next=c->next;
        if(c->next!=nullptr)
            c->next->prev=c->prev;
    }
    c->prev=nullptr;
}

void change(const int &c, const int &val){
    Cell* tmp=cell_pointer[c];
    pull(tmp);
    tmp->gain+=val;
    insert(tmp);
}

int main(int argc , char *argv[]){
    ifstream input;
    ofstream output;
    input.open(argv[1]);
    output.open(argv[2]);
    vector<vector<int>> cell_array, net_array;
    vector<int> gain,balance_size;
    vector<short> start,partition,lock;
    double balance_factor, balance_max, balance_min;
    string keyword;
    int cell_amount=0, best_gain, max_cell_id=0, max_net_id=0;
//讀檔
    input>>balance_factor;
    while(!input.eof()){
        input>>keyword;
        if(keyword!="NET")
            break;
        input>>keyword;
        string net=keyword.substr(1);
        int n=stoi(net);
        if(n>max_net_id){
            max_net_id=n;
            net_array.resize(n+1);
        }
        input>>keyword;
        while(keyword!=";"){
            string ctmp=keyword.substr(1);
            int c=stoi(ctmp);
            if(c>max_cell_id){
                max_cell_id=c;
                cell_array.resize(c+1);
            }
            net_array[n].push_back(c);
            cell_array[c].push_back(n);
            input>>keyword;
        }
    }
    input.close();
    for(auto cellcount:cell_array){
        if(cellcount.size()!=0)
            cell_amount++;
    }

    balance_max=((1+balance_factor)*0.5)*cell_amount;
    balance_min=cell_amount*0.5-int(balance_factor*cell_amount*0.5);
    start.resize(max_cell_id+1,-1);
//算Pmax & 隨便分成兩半
    best_gain=cell_amount*0.01;
    int half=cell_amount>>1;
    int count=0;
    for(int i=1; i<cell_array.size(); i++){
        if(cell_array[i].size()>0){
            Pmax=max((int)(cell_array[i].size()),Pmax);
            count++;
            if(count<=half)
                start[i]=0;
            else
                start[i]=1;
        }
    }
    net_pointer.resize(max_net_id+1, nullptr);
    cell_pointer.resize(max_cell_id+1, nullptr);
    gain.resize(max_cell_id+1);
    partition.resize(max_cell_id+1,-1);
    lock.resize(max_cell_id+1,-1);
    int round_count=0;
    Rec best;
    while(true){
        lock.clear();
        lock.resize(max_cell_id+1,-1);
        int max_gain=0;
        if(round_count){
            for(int i=1; i<=best_index; i++)
                start[record[i].id]= !start[record[i].id];           
//輸出檔案並結束
            if(best.total_gain<=best_gain){
                int cutsize=0;
                for(auto n:net_array){
                    if(n.size()!=0){
                        int f=0, t=0;
                        for(auto p:n){
                            if(start[p])
                                f++;
                            else
                                t++;
                        }
                        if((f!=0)&&(t!=0))
                            cutsize++;
                    }
                }
                output << "Cutsize = " << cutsize << endl;
                vector<int> G1,G2;
                for(int index=1; index < start.size(); index++){
                    if(start[index]==1)
                        G1.push_back(index);
                    else if(start[index]==0)
                        G2.push_back(index);
                }

                output << "G1 " << G1.size() << endl;
                for(auto cid:G1)
                    output << "c" << cid << " ";
                output << ";" << endl;
                output << "G2 " << G2.size() << endl;
                for(auto cid:G2)
                    output << "c" << cid << " ";
                output << ";" << endl;
                output.close();
                return 0;
            }
        }

        partition=start;
        balance_size={0,0};
        for(auto p:partition){
            if(p!=-1)
                balance_size[p]++;
        }        
        best.pardif=abs(balance_size[0]-balance_size[1]);
        best.total_gain=0;
        record.clear();
        record.push_back(best);
//算gain
        for(int i=1; i<cell_array.size(); i++){
            if(cell_array[i].size()!=0){
                lock[i]=0;
                gain[i]=0;
                bool p=partition[i];
                for(auto net:cell_array[i]){
                        int f=0, t=0;
                        for(auto c:net_array[net]){
                            if(partition[c]==p)
                                f++;
                            else
                                t++;
                        }
                        if(f==1)
                            gain[i]++;
                        if(t==0)
                            gain[i]--;
                }
            }
        }
//建cell pointer & net pointer
        for(int i=1; i<net_array.size(); i++){
            if(net_array[i].size()==0)
                continue;
            delete net_pointer[i];
            Net *n = new Net;
            net_pointer[i]=n;
        }

        for(int i=1; i<cell_array.size(); i++){
            if(cell_array[i].size()==0)
                continue;
            Cell *c = new Cell;
            c->id=i;
            c->gain=gain[i];
            delete cell_pointer[i];
            cell_pointer[i]=c;
            bool party=partition[i];
            for(auto n:cell_array[i]){
                if(party)
                    net_pointer[n]->one++;
                else
                    net_pointer[n]->zero++;
            }
        }
//把cell根據gain插到gain_list上
        gain_list = vector<Cell*>(2*Pmax+1, nullptr);
        for(auto s:cell_pointer){
            if(s==nullptr)
                continue;
            insert(s);
        }

        for(int move=0; move<cell_amount; move++){
//找最大gain & 且移動後為balance，紀錄它的id並拔掉它
            int id=-1;
            for(int i=gain_list.size()-1; i>=0; i--){
                if(gain_list[i]!=nullptr){
                    Cell* tmp = gain_list[i];
                    while(tmp!=nullptr){
                        if(balance_size[partition[tmp->id]]-1 >= balance_min){
                            id=tmp->id;
                            pull(tmp);
                            lock[id]=1;
                            for(auto n:cell_array[id]){
                                if(partition[id])
                                    net_pointer[n]->one--;
                                else
                                    net_pointer[n]->zero--;
                            }
                            break;
                        }
                        else
                            tmp=tmp->next;
                    }
                    if(id!=-1)
                        break;
                }      
            }
//更新gain
            unordered_map<int,int> crement;
            bool f=partition[id];
            for(auto net:cell_array[id]){
                if((net_pointer[net]->one>=2)&&(net_pointer[net]->zero>=2))
                    continue;
                if(f){
                    //front=0
                    if(net_pointer[net]->one==0){
                        for(auto cell:net_array[net]){
                            if(!lock[cell])
                                crement[cell]--;
                        }   
                    }
                    //front=1
                    else if(net_pointer[net]->one==1){
                        for(auto cell:net_array[net]){
                            if((partition[cell]==1)&&(cell!=id)){
                                if(!lock[cell])
                                    crement[cell]++;
                                break;
                            }
                        }  
                    }
                    //to=0
                    if(net_pointer[net]->zero==0){
                        for(auto cell:net_array[net]){
                            if(!lock[cell])
                                crement[cell]++;                        
                        }
                    }
                    //to=1
                    else if(net_pointer[net]->zero==1){
                        for(auto cell:net_array[net]){
                            if(partition[cell]==0){
                                if(!lock[cell])
                                    crement[cell]--;
                                break;
                            }
                        }  
                    }
                }
                else{
                    //front=0
                    if(net_pointer[net]->zero==0){
                        for(auto cell:net_array[net]){
                            if(!lock[cell])
                                crement[cell]--;
                        }   
                    }
                    //front=1
                    else if(net_pointer[net]->zero==1){
                        for(auto cell:net_array[net]){
                            if((partition[cell]==0)&&(cell!=id)){
                                if(!lock[cell])
                                    crement[cell]++;
                                break;
                            }
                        }  
                    }
                    //to=0
                    if(net_pointer[net]->one==0){
                        for(auto cell:net_array[net]){
                            if(!lock[cell])
                                crement[cell]++;                        
                        }
                    }
                    //to=1
                    else if(net_pointer[net]->one==1){
                        for(auto cell:net_array[net]){
                            if(partition[cell]==1){
                                if(!lock[cell])
                                    crement[cell]--;
                                break;
                            }
                        }  
                    }               
                }
            }
            for(auto cre:crement){
                if(cre.second!=0)
                    change(cre.first, cre.second);
            }
//移動
            balance_size[partition[id]]--;
            partition[id]=!partition[id];
            balance_size[partition[id]]++;
            for(auto net:cell_array[id]){
                if(partition[id])
                    net_pointer[net]->one++;
                else
                    net_pointer[net]->zero++;
            }
//紀錄
            Rec tmp;
            tmp.total_gain=record[move].total_gain+cell_pointer[id]->gain;
            
            tmp.id=id;
            if(tmp.total_gain>max_gain){
                best_index=move+1;
                max_gain=tmp.total_gain;
                tmp.pardif=abs(balance_size[0]-balance_size[1]);
                best=tmp;
            }
            else if(tmp.total_gain==max_gain){
                int tp=abs(balance_size[0]-balance_size[1]);
                int bp=best.pardif;
                if(tp<bp){
                    best_index=move+1;           
                    tmp.pardif=tp;
                    best=tmp;                
                }
            }     
            record.push_back(tmp);      
        }
        round_count++;
    }
    return 0;
}