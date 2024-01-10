#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <unordered_map>
#include <string>
#include <algorithm>

using namespace std;

struct Mos{
    int name;
    int left;
    int gate;
    int right;
};

struct cmp {
    bool operator() (Mos i,Mos j) {
        return (i.gate<j.gate);
    }
};

struct Location{
    int index;
    bool left_right;//0是left, 1是right
    Location(int i, bool l){
        index=i;
        left_right=l;
    }
};

struct NetData{   
    vector<Location> pmos;
    vector<Location> nmos;
};

struct Result{
    vector<Mos> nmos;
    vector<Mos> pmos;
    vector<NetData> NetsData;
    vector<double> netsHPWL;
    vector<vector<int>> gate_loc;
};

vector<Mos> nmos,pmos;
vector<NetData> NetsData;
vector<double> netsHPWL;
vector<int> modify_net;
vector<vector<int>> gate_loc;
unordered_map<int, int> gate;
unordered_map<string, int> instance, net;
unordered_map<int, string> reverse_net, reverse_instance;
double p_act_vw,n_act_vw;
Result result;
int remove_dummy_num;

void read_file(ifstream &input){
    string keyword;
    getline(input,keyword);
    int mos_size=1;
    int net_size=1;
    while(input >> keyword){
        if(keyword==".ENDS")
            break;
        Mos tmp;
        keyword=keyword.substr(1);
        instance[keyword]=mos_size;
        tmp.name=mos_size;
        ++mos_size;

        input >> keyword;
        if(!net[keyword]){
            net[keyword]=net_size;
            tmp.left=net_size;
            ++net_size;
        }
        else{
            tmp.left=net[keyword];
        }

        input >> keyword;
        if(!net[keyword]){
            net[keyword]=net_size;
            tmp.gate=net_size;
            ++net_size;
        }
        else{
            tmp.gate=net[keyword];
        }

        input >> keyword;
        if(!net[keyword]){
            net[keyword]=net_size;
            tmp.right=net_size;
            ++net_size;
        }
        else{
            tmp.right=net[keyword];
        }
        input >> keyword;
        input >> keyword;
        char t=keyword[0];
        if(t=='n')
            nmos.push_back(tmp);
        else
            pmos.push_back(tmp);
        input >> keyword;
        input >> keyword;
        input >> keyword;
        keyword=keyword.substr(5);
        if(t=='n'){
            n_act_vw=stoi(keyword)*27;
        }
        else{
            p_act_vw=stoi(keyword)*27;
        }
    }
    for(auto n:net){
        reverse_net[n.second]=n.first;
    }
    for(auto n:instance){
        reverse_instance[n.second]=n.first;
    } 
}

void write_file(ofstream &output){
    vector<int> same;
    for(int i=0; i<pmos.size()-1; ++i){
        if((pmos[i].right==pmos[i+1].left)&&(nmos[i].right==nmos[i+1].left))
            same.push_back(i);
    }

    double vertical=(p_act_vw+n_act_vw)*0.5+27;
    double total=0;
    for(int i=1; i<NetsData.size(); ++i){
        double HPWL=0;
        int p=NetsData[i].pmos.size();
        int n=NetsData[i].nmos.size();
        if((p==0)&&(n==0))
            continue;
        if((p==1)&&(n==0))
            continue;
        if((p==0)&&(n==1))
            continue;

        bool rightest, leftest;
        int right_index=0, left_index=2147483647;
        if(p){
            for(auto m:NetsData[i].pmos){
                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }

                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        if(n){
            for(auto m:NetsData[i].nmos){
                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }

                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        int diff=right_index-left_index;
        if(diff==0){
            if(rightest==leftest)
                HPWL=vertical;
            else{
                if((right_index==0)||(right_index==pmos.size()-1))
                    HPWL=vertical+49.5;
                else
                    HPWL=vertical+54;
            }
        }
        else{
            HPWL+=(diff*162-88);
            for(int j=left_index; j<right_index; ++j){
                auto it = find(same.begin(), same.end(), j);
                if (it != same.end())
                    HPWL-=108;
            }
            if(rightest==0){
                HPWL+=17;
            }
            else{
                if(right_index==nmos.size()-1)
                    HPWL+=66.5;
                else
                    HPWL+=71;
            }
            if(leftest==0){
                if(left_index==0)
                    HPWL+=66.5;
                else
                    HPWL+=71;
            }
            else{
                HPWL+=17;
            }
            if((p)&&(n))
                HPWL+=vertical;
        }
        total+=HPWL;
    }
    output << total << endl;
    for(int i=0; i<pmos.size()-1; ++i){
        auto it = find(same.begin(), same.end(), i);
        if (it != same.end())
            output << reverse_instance[pmos[i].name] << " ";
        else
            output << reverse_instance[pmos[i].name] << " Dummy " ;
    }
    output << reverse_instance[pmos[pmos.size()-1].name] << endl;
    for(int i=0; i<pmos.size()-1; ++i){
        auto it = find(same.begin(), same.end(), i);
        if (it != same.end())
            output << reverse_net[pmos[i].left] << " " << reverse_net[pmos[i].gate] << " ";
        else
            output << reverse_net[pmos[i].left] << " " << reverse_net[pmos[i].gate] << " " <<reverse_net[pmos[i].right] <<" Dummy " ;
    }    
    output << reverse_net[pmos[pmos.size()-1].left] << " " << reverse_net[pmos[pmos.size()-1].gate] << " " <<reverse_net[pmos[pmos.size()-1].right] << endl;

    for(int i=0; i<nmos.size()-1; ++i){
        auto it = find(same.begin(), same.end(), i);
        if (it != same.end())
            output << reverse_instance[nmos[i].name] << " ";
        else
            output << reverse_instance[nmos[i].name] << " Dummy " ;
    }
    output << reverse_instance[nmos[nmos.size()-1].name] << endl;
    for(int i=0; i<nmos.size()-1; ++i){
        auto it = find(same.begin(), same.end(), i);
        if (it != same.end())
            output << reverse_net[nmos[i].left] << " " << reverse_net[nmos[i].gate] << " ";
        else
            output << reverse_net[nmos[i].left] << " " << reverse_net[nmos[i].gate] << " " <<reverse_net[nmos[i].right] <<" Dummy " ;
    }    
    output << reverse_net[nmos[nmos.size()-1].left] << " " << reverse_net[nmos[nmos.size()-1].gate] << " " <<reverse_net[nmos[nmos.size()-1].right];
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

void first_cal_HPWL(){
    double vertical=(p_act_vw+n_act_vw)*0.5+27;
    remove_dummy_num=0;
    for(int i=0; i<pmos.size()-1; ++i){
        if((pmos[i].right==pmos[i+1].left)&&(nmos[i].right==nmos[i+1].left))
            remove_dummy_num++;
    }

    for(int i=1; i<NetsData.size(); ++i){
        double HPWL=0;
        int p=NetsData[i].pmos.size();
        int n=NetsData[i].nmos.size();
        if((p==0)&&(n==0)){
            netsHPWL[i]=0;
            continue;
        }
        if((p==1)&&(n==0)){
            netsHPWL[i]=0;
            continue;            
        }
        if((p==0)&&(n==1)){
            netsHPWL[i]=0;
            continue;            
        }

        bool rightest, leftest;
        int right_index=0, left_index=2147483647;
        if(p){
            for(auto m:NetsData[i].pmos){

                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }

                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        if(n){
            for(auto m:NetsData[i].nmos){
                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }

                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        int diff=right_index-left_index;
        HPWL+=(diff*162-88);
        if(rightest==0){
            HPWL+=17;
        }
        else{
            if(right_index==nmos.size()-1)
                HPWL+=66.5;
            else
                HPWL+=71;
        }
        if(leftest==0){
            if(left_index==0)
                HPWL+=66.5;
            else
                HPWL+=71;
        }
        else{
            HPWL+=17;
        }
        if((p)&&(n))
            HPWL+=vertical;
        netsHPWL[i]=HPWL;
    }
}

double cal_HPWL(){
    double vertical=(p_act_vw+n_act_vw)*0.5+27;
    double output=0;
    for(auto i:modify_net){
        double HPWL=0;
        int p=NetsData[i].pmos.size();
        int n=NetsData[i].nmos.size();
        if((p==0)&&(n==0)){
            netsHPWL[i]=0;
            continue;
        }
        if((p==1)&&(n==0)){
            netsHPWL[i]=0;
            continue;            
        }
        if((p==0)&&(n==1)){
            netsHPWL[i]=0;
            continue;            
        }

        bool rightest, leftest;
        int right_index=0, left_index=2147483647;
        if(p){
            for(auto m:NetsData[i].pmos){
                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }
                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        if(n){
            for(auto m:NetsData[i].nmos){
                if(m.index>right_index){
                    right_index=m.index;
                    rightest=m.left_right;
                }
                else if(m.index==right_index){
                    if(rightest==0)
                        rightest=m.left_right;
                }
                if(m.index<left_index){
                    left_index=m.index;
                    leftest=m.left_right;
                }
                else if(m.index==left_index){
                    if(leftest==1)
                        leftest=m.left_right;
                }
            }
        }
        int diff=right_index-left_index;
        HPWL+=diff*162-88;
        if(rightest==0){
            HPWL+=17;
        }
        else{
            if(right_index==nmos.size()-1)
                HPWL+=66.5;
            else
                HPWL+=71;
        }
        if(leftest==0){
            if(left_index==0)
                HPWL+=66.5;
            else
                HPWL+=71;
        }
        else{
            HPWL+=17;
        }
        if((p)&&(n))
            HPWL+=vertical;
        output+=(HPWL-netsHPWL[i]);
        netsHPWL[i]=HPWL;
    }
    return output;
}

void perturbation(){
    modify_net.clear();
    int op = rand()%5;
    if(op==0){//swap left and right
        int p_or_n=rand()%2;
        int index=rand()%pmos.size();
        
        if(p_or_n){
            int tmp;
            int left=pmos[index].left;
            int right=pmos[index].right;

            modify_net.push_back(left);
            modify_net.push_back(right);
            pmos[index].right=left;
            pmos[index].left=right;
            for(int i=0; i<NetsData[left].pmos.size(); ++i){
                if(index==NetsData[left].pmos[i].index){
                    NetsData[left].pmos[i].left_right=1;
                    break;
                }                
            }
            for(int i=0; i<NetsData[right].pmos.size(); ++i){
                if(index==NetsData[right].pmos[i].index){
                    NetsData[right].pmos[i].left_right=0;
                    break;
                }                
            }
        }
        else{
            int tmp;
            int left=nmos[index].left;
            int right=nmos[index].right;
            modify_net.push_back(left);
            modify_net.push_back(right);
            nmos[index].right=left;
            nmos[index].left=right;     
            for(int i=0; i<NetsData[left].nmos.size(); ++i){
                if(index==NetsData[left].nmos[i].index){
                    NetsData[left].nmos[i].left_right=1;
                    break;
                }                
            }
            for(int i=0; i<NetsData[right].nmos.size(); ++i){
                if(index==NetsData[right].nmos[i].index){
                    NetsData[right].nmos[i].left_right=0;
                    break;
                }                
            }
        }

    }
    else if(op==4){//swap two pmos at same gate
        int index=rand()%gate_loc.size();
        int reindex=0;
        while(gate_loc[index].size()==1){
            ++reindex;
            if(reindex>100)
                return;
            index=rand()%gate_loc.size();
        }
            

        int index1=rand()%gate_loc[index].size();
        int index2=rand()%gate_loc[index].size();    
        while(index1==index2)
            index2=rand()%gate_loc[index].size(); 
        index1=gate_loc[index][index1];
        index2=gate_loc[index][index2];

        int gate1=pmos[index1].gate;
        int gate2=pmos[index2].gate;
        pmos[index1].gate=gate2;
        pmos[index2].gate=gate1;   

        swap(pmos[index1].name, pmos[index2].name);

        int p_i1_left=pmos[index1].left;  
        int p_i1_right=pmos[index1].right;
        int p_i2_left=pmos[index2].left;
        int p_i2_right=pmos[index2].right;

        modify_net.push_back(p_i1_left);
        modify_net.push_back(p_i1_right);
        modify_net.push_back(p_i2_left);
        modify_net.push_back(p_i2_right);
               
        pmos[index1].left=p_i2_left;
        pmos[index1].right=p_i2_right;
        pmos[index2].left=p_i1_left;
        pmos[index2].right=p_i1_right;

        int p_i1_left_index;
        int p_i1_right_index;
        int p_i2_left_index;
        int p_i2_right_index;

        for(int i=0; i<NetsData[p_i1_left].pmos.size(); ++i){
            if(index1==NetsData[p_i1_left].pmos[i].index){
                p_i1_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i1_right].pmos.size(); ++i){
            if(index1==NetsData[p_i1_right].pmos[i].index){
                p_i1_right_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i2_left].pmos.size(); ++i){
            if(index2==NetsData[p_i2_left].pmos[i].index){
                p_i2_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i2_right].pmos.size(); ++i){
            if(index2==NetsData[p_i2_right].pmos[i].index){
                p_i2_right_index=i;
                break;
            }            
        }

        NetsData[p_i1_left].pmos[p_i1_left_index].index=index2;
        NetsData[p_i1_right].pmos[p_i1_right_index].index=index2;
        NetsData[p_i2_left].pmos[p_i2_left_index].index=index1;
        NetsData[p_i2_right].pmos[p_i2_right_index].index=index1;

    }
    else{//swap two mos
        int index1=rand()%pmos.size();
        int index2=rand()%pmos.size();    
        int reindex=0;
        while(index1==index2){
            ++reindex;
            if(reindex>100)
                return;
            index2=rand()%pmos.size(); 
        }
            
        int gate1=pmos[index1].gate;
        int gate2=pmos[index2].gate;
        pmos[index1].gate=gate2;
        nmos[index1].gate=gate2;
        pmos[index2].gate=gate1;
        nmos[index2].gate=gate1;   

        int gate1_index=0;
        int gate2_index=0;
        for(int i=0; i<gate_loc[gate[gate1]].size(); ++i){
            if(gate_loc[gate[gate1]][i]==index1){
                gate1_index=i;
                break;
            }   
        }
        for(int i=0; i<gate_loc[gate[gate2]].size(); ++i){
            if(gate_loc[gate[gate2]][i]==index2){
                gate2_index=i;
                break;
            }   
        }
        gate_loc[gate[gate1]][gate1_index]=index2;
        gate_loc[gate[gate2]][gate2_index]=index1;
        swap(pmos[index1].name, pmos[index2].name);
        swap(nmos[index1].name, nmos[index2].name);
        int p_i1_left=pmos[index1].left;  
        int p_i1_right=pmos[index1].right;
        int p_i2_left=pmos[index2].left;
        int p_i2_right=pmos[index2].right;
        int n_i1_left=nmos[index1].left;  
        int n_i1_right=nmos[index1].right;
        int n_i2_left=nmos[index2].left;
        int n_i2_right=nmos[index2].right; 
        modify_net.push_back(p_i1_left);
        modify_net.push_back(p_i1_right);
        modify_net.push_back(p_i2_left);
        modify_net.push_back(p_i2_right);
        modify_net.push_back(n_i1_left);
        modify_net.push_back(n_i1_right);
        modify_net.push_back(n_i2_left);
        modify_net.push_back(n_i2_right);                
        pmos[index1].left=p_i2_left;
        pmos[index1].right=p_i2_right;
        pmos[index2].left=p_i1_left;
        pmos[index2].right=p_i1_right;
        nmos[index1].left=n_i2_left;
        nmos[index1].right=n_i2_right;
        nmos[index2].left=n_i1_left;
        nmos[index2].right=n_i1_right;
        int p_i1_left_index;
        int p_i1_right_index;
        int p_i2_left_index;
        int p_i2_right_index;
        int n_i1_left_index;
        int n_i1_right_index;
        int n_i2_left_index;
        int n_i2_right_index;

        for(int i=0; i<NetsData[p_i1_left].pmos.size(); ++i){
            if(index1==NetsData[p_i1_left].pmos[i].index){
                p_i1_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i1_right].pmos.size(); ++i){
            if(index1==NetsData[p_i1_right].pmos[i].index){
                p_i1_right_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i2_left].pmos.size(); ++i){
            if(index2==NetsData[p_i2_left].pmos[i].index){
                p_i2_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[p_i2_right].pmos.size(); ++i){
            if(index2==NetsData[p_i2_right].pmos[i].index){
                p_i2_right_index=i;
                break;
            }            
        }
        for(int i=0; i<NetsData[n_i1_left].nmos.size(); ++i){
            if(index1==NetsData[n_i1_left].nmos[i].index){
                n_i1_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[n_i1_right].nmos.size(); ++i){
            if(index1==NetsData[n_i1_right].nmos[i].index){
                n_i1_right_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[n_i2_left].nmos.size(); ++i){
            if(index2==NetsData[n_i2_left].nmos[i].index){
                n_i2_left_index=i;
                break;
            }            
        }

        for(int i=0; i<NetsData[n_i2_right].nmos.size(); ++i){
            if(index2==NetsData[n_i2_right].nmos[i].index){
                n_i2_right_index=i;
                break;
            }            
        } 
        NetsData[p_i1_left].pmos[p_i1_left_index].index=index2;
        NetsData[p_i1_right].pmos[p_i1_right_index].index=index2;
        NetsData[p_i2_left].pmos[p_i2_left_index].index=index1;
        NetsData[p_i2_right].pmos[p_i2_right_index].index=index1;
        NetsData[n_i1_left].nmos[n_i1_left_index].index=index2;
        NetsData[n_i1_right].nmos[n_i1_right_index].index=index2;
        NetsData[n_i2_left].nmos[n_i2_left_index].index=index1;
        NetsData[n_i2_right].nmos[n_i2_right_index].index=index1;
    }
}

void SA(){
    double temp=100000, cool=0.85;
    int P=50*pmos.size();

    while(temp>0.1){
        for(int p=0; p<P; ++p){
            perturbation();
            double cost=cal_HPWL();
            int remove_num=0;
            for(int i=0; i<pmos.size()-1; ++i){
                if((pmos[i].right==pmos[i+1].left)&&(nmos[i].right==nmos[i+1].left))
                    remove_num++;
            }
            cost+=((remove_dummy_num-remove_num)*pmos.size()*15);

            if(cost<0){
                result.pmos=pmos;
                result.nmos=nmos;
                result.NetsData=NetsData;
                result.netsHPWL=netsHPWL;    
                result.gate_loc=gate_loc;  
                remove_dummy_num=remove_num;
            }
            else{
                bool go=prob(cost, temp);
                if(go){
                    result.pmos=pmos;
                    result.nmos=nmos;
                    result.NetsData=NetsData;
                    result.netsHPWL=netsHPWL;
                    result.gate_loc=gate_loc;   
                    remove_dummy_num=remove_num;                
                }
                else{
                    pmos=result.pmos;
                    nmos=result.nmos;
                    NetsData=result.NetsData;
                    netsHPWL=result.netsHPWL;  
                    gate_loc=result.gate_loc;                     
                }
            }
        }
        temp*=cool;
    }
}

int main(int argc , char *argv[]){
    ifstream input;
    input.open(argv[1]);
    read_file(input);
    input.close();

    sort(pmos.begin(), pmos.end(), cmp());
    sort(nmos.begin(), nmos.end(), cmp());

    int current=pmos[0].gate;
    int count=1;
    int index=0;
    for(int i=1; i<pmos.size(); ++i){
        if(pmos[i].gate==current)
            ++count;
        else{
            if(count>1){
                gate[current]=index;
                gate_loc.resize(index+1);
                for(int j=i-count; j<i; ++j){
                    gate_loc[index].push_back(j);
                }
                current=pmos[i].gate;
                count=1;
                ++index;
            }
            else{
                gate[current]=index;
                gate_loc.resize(index+1);
                gate_loc[index].push_back(i-1);
                current=pmos[i].gate;
                count=1;
                ++index;
            }
        }
    }
    if(count>1){
        gate[current]=index;
        gate_loc.resize(index+1);
        for(int j=(int)pmos.size()-count; j<(int)pmos.size(); ++j){
            gate_loc[index].push_back(j);
        }
    }
    else{
        gate[current]=index;
        gate_loc.resize(index+1);
        gate_loc[index].push_back(pmos.size()-1);
    }    
    
    NetsData.resize(net.size()+1);
    netsHPWL.resize(net.size()+1);

    for(int i=0; i<pmos.size(); ++i){
        int left=pmos[i].left;
        int right=pmos[i].right;
        NetsData[left].pmos.push_back(Location(i,0));
        NetsData[right].pmos.push_back(Location(i,1));
    }
    for(int i=0; i<nmos.size(); ++i){
        int left=nmos[i].left;
        int right=nmos[i].right;
        NetsData[left].nmos.push_back(Location(i,0));
        NetsData[right].nmos.push_back(Location(i,1));
    }
    srand(1225048571);
    first_cal_HPWL();
    result.pmos=pmos;
    result.nmos=nmos;
    result.NetsData=NetsData;
    result.netsHPWL=netsHPWL;
    result.gate_loc=gate_loc;
    SA();
    ofstream output;
    output.open(argv[2]);
    write_file(output);
    output.close();
    return 0;
}