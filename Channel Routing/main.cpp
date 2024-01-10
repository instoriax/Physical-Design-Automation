#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <algorithm>

using namespace std;

struct Wire{
    int track;
    int start;
    int end;
    Wire(int a, int b, int c){
        track=a;
        start=b;
        end=c;
    }
};

struct Net{
    vector<Wire> wires;
    vector<int> dogleg;
};

struct Interval{
    int start;
    int end;
    Interval(int a, int b){
        start=a;
        end=b;
    }
};

struct Sort_interval{
    int net;
    int start;
    int end;
    Sort_interval(int a, int b, int c){
        net=a;
        start=b;
        end=c;
    }
};

auto cmp = [](Sort_interval a, Sort_interval b) { return a.start<b.start; };
int pin_length, max_pin_index, track_number;
vector<vector<bool>> t_channel,b_channel;
vector<int> t_pin, b_pin;
vector<vector<Interval>> intervals;
vector<vector<int>> constrain;
vector<int> const_count; 
vector<Net> nets;
set<Sort_interval, decltype(cmp)> sort_intervals(cmp);

void read_file(ifstream &input){
    pin_length=0;
    max_pin_index=0;
    string keyword;
    input >> keyword;
    while(keyword[0]=='T'){
        keyword=keyword.substr(1);
        int index=stoi(keyword);
        if(index>=t_channel.size()){
            t_channel.resize(index+1);
        }
        int start,end;
        input >> start >> end;
        pin_length=max(pin_length,end+1);
        t_channel[index].resize(pin_length);
        for(int i=start; i<=end; i++){
            t_channel[index][i]=1;
        }
        input >> keyword;
    }
    t_channel[0].resize(pin_length);
    for(int i=1; i<t_channel.size(); i++){
        t_channel[i].resize(pin_length);
        for(int j=0; j<pin_length; j++){
            if(t_channel[i-1][j]==1)
                t_channel[i][j]=1;
        }
    }

    while(keyword[0]=='B'){
        keyword=keyword.substr(1);
        int index=stoi(keyword);
        if(index>=b_channel.size()){
            b_channel.resize(index+1);
        }
        int start,end;
        input >> start >> end;
        b_channel[index].resize(pin_length);
        for(int i=start; i<=end; i++){
            b_channel[index][i]=1;
        }
        input >> keyword;
    }
    for(int i=1; i<b_channel.size(); i++){
        for(int j=0; j<pin_length; j++){
            if(b_channel[i-1][j]==1)
                b_channel[i][j]=1;
        }
    }

    t_pin.push_back(stoi(keyword));
    int tmp;
    for(int i=1; i<pin_length; i++){
        input >> tmp;
        max_pin_index=max(max_pin_index, tmp);
        t_pin.push_back(tmp);
    }
    for(int i=0; i<pin_length; i++){
        input >> tmp;
        max_pin_index=max(max_pin_index, tmp);
        b_pin.push_back(tmp);
    }  
}

void construct_graph(){
    intervals.resize(max_pin_index+1);
    constrain.resize(max_pin_index+1);
    const_count.resize(max_pin_index+1);
    vector<vector<int>> location(max_pin_index+1);
    for(int i=0; i<pin_length; i++){
        int t=t_pin[i], b=b_pin[i];
        location[t].push_back(i);
        location[b].push_back(i);

        if(t!=0 && b!=0){
            constrain[t].push_back(b);
            const_count[b]++;
        }
    }
    for(int i=1; i<=max_pin_index; i++){
        for(int j=1; j<location[i].size(); j++){
            intervals[i].push_back(Interval(location[i][j-1], location[i][j]));
        }
    }
}

void adj_deg(int index, int net){
    if(t_pin[index]==net)
        const_count[b_pin[index]]--;
    return;
}

void route_bound(){
    vector<vector<Interval>> bound_space(t_channel.size());
    for(int i=0; i<t_channel.size(); i++){
        Interval tmp(-1,-1);
        for(int j=0; j<pin_length; j++){
            if(t_channel[i][j]){
                if(tmp.start!=-1){
                    bound_space[i].push_back(tmp);
                    tmp.start=-1;
                    tmp.end=-1;
                }
            }
            else{
                if(tmp.start==-1){
                    tmp.start=j;
                    tmp.end=j;
                }
                else{
                    tmp.end=j;
                }
            }
        }
        if(tmp.start!=-1){
            bound_space[i].push_back(tmp);
        }

    }
    int track_label=-1;
    nets.resize(max_pin_index+1);
    for(int track=t_channel.size()-2; track>=0; track--){
        for(int space=0; space<bound_space[track].size(); space++){
            for(int index=1; index<const_count.size(); index++){
                if(const_count[index]==0){
                    for(auto inter:intervals[index]){
                        sort_intervals.insert(Sort_interval(index, inter.start, inter.end));
                    }
                    const_count[index]=-1;
                }
            }
            int watermark=bound_space[track][space].start;
            int end=bound_space[track][space].end;
            Sort_interval si(watermark,watermark,watermark);
            auto it=sort_intervals.lower_bound(si);
            int curr_net=it->net;
            vector<set<Sort_interval, decltype(cmp)>::iterator> routed;
            while(it!=sort_intervals.end() && it->start<end){
                if(it->start>=watermark && it->end<=end){
                    if(curr_net==it->net){
                        nets[it->net].wires.push_back(Wire(track_label, it->start, it->end));
                        routed.push_back(it);
                        watermark=it->end;
                        Sort_interval tmp(watermark,watermark,watermark);
                        it=sort_intervals.lower_bound(tmp);
                    }
                    else{
                        it++;
                        curr_net=it->net;
                    }
                }
                else{
                    it++;
                    curr_net=it->net;
                }
            }   
        
            for(int i=routed.size()-1; i>=0; i--){
                int n=routed[i]->net;
                for(int j=intervals[n].size()-1; j>=0; j--){
                    if(intervals[n][j].start==routed[i]->start && intervals[n][j].end==routed[i]->end){
                        if(j==0){
                            adj_deg(intervals[n][j].start, n);
                        }
                        else{
                            if(intervals[n][j-1].end!=intervals[n][j].start){
                                adj_deg(intervals[n][j].start, n);
                            }
                        }
                            
                        if(j==intervals[n].size()-1)
                            adj_deg(intervals[n][j].end, n);
                        else{
                            if(intervals[n][j+1].start!=intervals[n][j].end){
                                adj_deg(intervals[n][j].end, n);
                            }                            
                        }
                        intervals[n].erase(intervals[n].begin()+j);
                        sort_intervals.erase(routed[i]);
                        break;
                    }
                }
            }
        }
        track_label--;
    }
}

void route_track(){
    int track_label=0;
    while(true){
        for(int index=1; index<const_count.size(); index++){
            if(const_count[index]==0){
                for(auto inter:intervals[index]){
                    sort_intervals.insert(Sort_interval(index, inter.start, inter.end));
                }
                const_count[index]=-1;
            }
        }  
        if(sort_intervals.empty())
            break;
        int watermark=0;
        int end=pin_length-1;
        Sort_interval si(watermark,watermark,watermark);
        auto it=sort_intervals.lower_bound(si);
        int curr_net=it->net;
        vector<set<Sort_interval, decltype(cmp)>::iterator> routed;
        while(it!=sort_intervals.end() && it->start<end){
            if(it->start>=watermark && it->end<=end){
                if(curr_net==it->net){
                    nets[it->net].wires.push_back(Wire(track_label, it->start, it->end));
                    routed.push_back(it);
                    watermark=it->end;
                    Sort_interval tmp(watermark,watermark,watermark);
                    it=sort_intervals.lower_bound(tmp);
                }
                else{
                    it++;
                    curr_net=it->net;
                }
            }
            else{
                it++;
                curr_net=it->net;
            }
        }   
        for(int i=routed.size()-1; i>=0; i--){
            int n=routed[i]->net;
            for(int j=intervals[n].size()-1; j>=0; j--){
                if(intervals[n][j].start==routed[i]->start && intervals[n][j].end==routed[i]->end){
                    if(j==0){
                        adj_deg(intervals[n][j].start, n);
                    }
                    else{
                        if(intervals[n][j-1].end!=intervals[n][j].start){
                            adj_deg(intervals[n][j].start, n);
                        }
                    }
                        
                    if(j==intervals[n].size()-1)
                        adj_deg(intervals[n][j].end, n);
                    else{
                        if(intervals[n][j+1].start!=intervals[n][j].end){
                            adj_deg(intervals[n][j].end, n);
                        }                            
                    }
                    intervals[n].erase(intervals[n].begin()+j);
                    sort_intervals.erase(routed[i]);
                    break;
                }
            }
        }
        track_label++;
    }
    track_number=track_label;
}

void write_file(ofstream &output){
    auto wcmp = [](Wire a, Wire b) { return a.start<b.start; };
    output << "Channel density: " << track_number << endl;
    for(int n=1; n<nets.size(); n++){
        output << "Net " << n << endl; 
        if(nets[n].wires.size()>1){
            for(int w=1; w<nets[n].wires.size(); w++){
                if(nets[n].wires[w-1].track==nets[n].wires[w].track){
                    if(nets[n].wires[w-1].end==nets[n].wires[w].start){
                        nets[n].wires[w-1].end=nets[n].wires[w].end;
                        nets[n].wires.erase(nets[n].wires.begin()+w);
                        w--;
                    }
                }
            }
        }
        sort(nets[n].wires.begin(), nets[n].wires.end(), wcmp);
        for(int w=0; w<nets[n].wires.size(); w++){
            if(nets[n].wires[w].track<0)
                output << "T" << nets[n].wires[w].track+t_channel.size()-1 << " ";
            else
                output << "C" << track_number-nets[n].wires[w].track << " ";
            output << nets[n].wires[w].start << " " << nets[n].wires[w].end << endl;
            if(w!=0){
                if(nets[n].wires[w].track!=nets[n].wires[w-1].track){
                    if(nets[n].wires[w].start==nets[n].wires[w-1].end){
                        output << "Dogleg " << nets[n].wires[w].start << endl;
                    }
                }
            }
        }
    }
}

int main(int argc , char *argv[]){
    ifstream input;
    input.open(argv[1]);
    read_file(input);
    input.close();
    construct_graph();
    route_bound();
    route_track();
    ofstream output;
    output.open(argv[2]);
    write_file(output);
    output.close();
    return 0;
}