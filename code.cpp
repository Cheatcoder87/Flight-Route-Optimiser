#include <bits/stdc++.h>
using namespace std;

struct flight{
    int to;
    int distance;
    int time;
    int cost;
    string departure;
    string name;
    string code;
    flight(int to_,int d,int c,int t,string dept,string n,string c1){
        to=to_;
        distance=d;
        time=t;
        cost=c;
        departure=dept;
        name=n;
        code=c1;
    }
};

struct Path {
    vector<int> nodes;
    vector<int> flights_taken;
    int total_cost;
    int total_time;
    vector<int> layovers;
    string last_time;
    bool operator>(const Path& other) const {
        return total_cost > other.total_cost;
    }
};

class flightgraph {
public:
    map<string,int> airportindex;
    vector<string> indextoairport;
    map<string,int> flightindex;
    vector<flight> indextoflight;
    vector<vector<flight>> adj;

    void addairport(string x) {
        if(airportindex.count(x)) return;
        int ind=indextoairport.size();
        airportindex[x]=ind;
        indextoairport.push_back(x);
        adj.emplace_back();
    }

    void addflight(string from,string to,int d,int c,int t,string dept,string n,string c1) {
        addairport(from);
        addairport(to);
        int ind=indextoflight.size();
        flightindex[n]=ind;
        indextoflight.push_back(flight(airportindex[to],d,c,t,dept,n,c1));
        adj[airportindex[from]].push_back(flight(airportindex[to],d,c,t,dept,n,c1));
    }

    int time(string x, string y) {
        int hrx = stoi(x.substr(0,2));
        int mx = stoi(x.substr(3,2));
        int hry = stoi(y.substr(0,2));
        int my = stoi(y.substr(3,2));
        int start_minutes = hrx * 60 + mx;
        int end_minutes = hry * 60 + my;
        int diff = end_minutes - start_minutes;
        if (diff < 0) diff += 24 * 60;
        return diff;
    }

    string land(int t, string x) {
        int h=t/60, m=t%60, hr=stoi(x.substr(0,2)), min=stoi(x.substr(3,2));
        hr+=h; min+=m;
        hr += min / 60; min %= 60;
        hr %= 24;
        string hour=to_string(hr), minutes=to_string(min);
        if(hour.size()!=2) hour.insert(hour.begin(),'0'); 
        if(minutes.size()!=2) minutes.insert(minutes.begin(),'0'); 
        return hour+":"+minutes;
    }
    string land1(int t, string x) {
        int h=t/60, m=t%60, hr=stoi(x.substr(0,2)), min=stoi(x.substr(3,2));
        hr+=h; min+=m;
        hr += min / 60; min %= 60;
        string hour=to_string(hr), minutes=to_string(min);
        if(hour.size()!=2) hour.insert(hour.begin(),'0'); 
        if(minutes.size()!=2) minutes.insert(minutes.begin(),'0'); 
        return hour+":"+minutes;
    }

    Path dijkstra(int src, int dest, int b, int c, string st) {
        int n = indextoairport.size();
        priority_queue<pair<int, pair<string, int>>, vector<pair<int, pair<string, int>>>, greater<>> q;
        vector<int> dist(n, 1e9);
        vector<pair<int, int>> par(n, {-1, -1});
        vector<string> arr_time(n);
        vector<string> dep_time(n);
        dist[src] = 0;
        q.push({0, {st, src}});

        while(!q.empty()) {
            auto [d, p] = q.top(); q.pop();
            string t = p.first; int x = p.second;
            if(d > dist[x]) continue;
            for(auto &f : adj[x]) {
                int wait = time(t, f.departure);
                string arrival = land(f.time, f.departure);
                int y = time(t, "00:00");
                if(x == src) {
                    int flight_minutes = stoi(f.departure.substr(0,2)) * 60 + stoi(f.departure.substr(3,2));
                    int start_minutes = stoi(st.substr(0,2)) * 60 + stoi(st.substr(3,2));
                    if(flight_minutes < start_minutes) continue;
                    if(flight_minutes < start_minutes || flight_minutes >= 1440) continue;
                } 
                else {
                    if(wait < 120) continue;
                }
                int cost = f.cost * b;
                int travel = f.time * c;
                int lay = (x == src ? 0 : wait * c);
                int ex = (x == src ? wait * c : 0);
                int total = cost + travel + lay + d + ex;
                if(total < dist[f.to]) {
                    dist[f.to] = total;
                    par[f.to] = {x, flightindex[f.name]};
                    arr_time[f.to] = arrival;
                    dep_time[f.to] = f.departure;
                    q.push({total, {arrival, f.to}});
                }
            }
        }

        Path result;
        result.total_cost = dist[dest];
        if(dist[dest] == 1e9) return result;

        int temp = dest;
        string current_time = arr_time[temp];
        result.total_time = 0;
        vector<string> used_dep;
        while(temp != src) {
            result.flights_taken.push_back(par[temp].second);
            used_dep.push_back(dep_time[temp]);
            result.nodes.push_back(temp);
            temp = par[temp].first;
        }
        result.nodes.push_back(src);
        reverse(result.nodes.begin(), result.nodes.end());
        reverse(result.flights_taken.begin(), result.flights_taken.end());
        reverse(used_dep.begin(), used_dep.end());

        for(int i = 0; i < result.flights_taken.size(); ++i) {
            flight &f = indextoflight[result.flights_taken[i]];
            if(i > 0) {
                int lay = time(current_time, used_dep[i]);
                result.layovers.push_back(lay);
                result.total_time += lay;
            }
            result.total_time += f.time;
            current_time = land(f.time, used_dep[i]);
        }
        return result;
    }

    void k_shortestpaths(string from, string to, int b, int c, string st, int K=4) {
        int src = airportindex[from];
        int dest = airportindex[to];
        vector<Path> A;
        priority_queue<Path, vector<Path>, greater<>> B;

        Path first = dijkstra(src, dest, b, c, st);
        if(first.total_cost == 1e9){ cout << "No routes found\n"; return; }
        A.push_back(first);

        for(int k = 1; k < K; ++k) {
            for(int i = 0; i < A[k-1].nodes.size()-1; ++i) {
                int spur_node = A[k-1].nodes[i];
                set<pair<int, int>> removed;

                for(auto &p : A) {
                    if(p.nodes.size() > i+1 && equal(p.nodes.begin(), p.nodes.begin()+i+1, A[k-1].nodes.begin())) {
                        int u = p.nodes[i];
                        int v = p.nodes[i+1];
                        removed.insert({u, v});
                    }
                }

                vector<flight> backup = adj[spur_node];
                adj[spur_node].erase(remove_if(adj[spur_node].begin(), adj[spur_node].end(), [&](flight &f){
                    return removed.count({spur_node, f.to});
                }), adj[spur_node].end());

                Path spur = dijkstra(spur_node, dest, b, c, st);

                adj[spur_node] = backup;

                if(spur.total_cost == 1e9 || spur.nodes.empty() || spur.nodes[0] != spur_node) continue;
                Path total;
                total.nodes.insert(total.nodes.end(), A[k-1].nodes.begin(), A[k-1].nodes.begin()+i);
                total.flights_taken.insert(total.flights_taken.end(), A[k-1].flights_taken.begin(), A[k-1].flights_taken.begin()+i);
                total.nodes.insert(total.nodes.end(), spur.nodes.begin(), spur.nodes.end());
                total.flights_taken.insert(total.flights_taken.end(), spur.flights_taken.begin(), spur.flights_taken.end());

                total.total_cost = 0;
                total.total_time = 0;
                string prev_arrival = "";
                for(int j = 0; j < total.flights_taken.size(); ++j) {
                    flight &f = indextoflight[total.flights_taken[j]];
                    if(j > 0) {
                        int lay = time(prev_arrival, f.departure);
                        if(lay < 120) {
                            total.total_cost = 1e9;
                            break;
                        }
                        total.layovers.push_back(lay);
                        total.total_time += lay;
                    }
                    total.total_time += f.time;
                    total.total_cost += f.cost * b + f.time * c;
                    prev_arrival = land(f.time, f.departure);
                }
                if(total.total_cost != 1e9) B.push(total);
            }
            if(B.empty()) break;
            A.push_back(B.top()); B.pop();
        }
        for (Path &p : A) {
            p.total_cost = 0; 
            for (int i : p.flights_taken) {
                p.total_cost += indextoflight[i].cost;
            }
        }
        if(b>=c){
            sort(A.begin(), A.end(), [](const Path &a, const Path &b) {
                if(a.total_cost == b.total_cost) return a.total_time < b.total_time;
                return a.total_cost < b.total_cost;
            });
        }
        else{
            sort(A.begin(), A.end(), [](const Path &a, const Path &b) {
                if(a.total_time == b.total_time) return a.total_cost < b.total_cost;
                return a.total_time < b.total_time;
            });
        }
        for(int i = 0; i < A.size(); ++i) {
            cout << "\nOption " << (i+1) << ":\n";
            int cost = 0;
            for(int j = 0; j < A[i].flights_taken.size(); ++j) {
                if(j > 0) {
                    int lay = A[i].layovers[j-1];
                    cout <<"Layover Time: " << land(lay, "00:00") << endl;
                    cout<<endl;
                }
                flight &f = indextoflight[A[i].flights_taken[j]];
                string x = indextoairport[A[i].nodes[j]];
                string y = indextoairport[f.to];
                auto extract_code = [](const string& airport) {
                    size_t start = airport.find('('), end = airport.find(')');
                    if (start != string::npos && end != string::npos && end > start) return airport.substr(start + 1, end - start - 1);
                    return airport;
                };
                cout<<extract_code(x)<<" "<<"->"<<" "<<extract_code(y)<<endl;
                cout<<"Flight Code: "<<f.code<<endl;
                cout<< "Flight: " << f.name << endl;
                cout<<"Departure: " << f.departure <<endl;
                cout<<"Duration: " << f.time <<" min "<<endl;
                cout<<"Cost: " << f.cost << endl;
                cost += f.cost;
                cout << endl;
            }
            cout << "Total Cost of Trip: " << cost << endl;

            cout << "Total Time of Trip: " << land1(A[i].total_time, "00:00") << endl;
        }
        cout << "Hope you enjoy your flights!" << endl;
    }
};


int main(int argc, char* argv[]) {
    std::cout << "Program started" << std::endl << std::flush;

    std::cout << "argc = " << argc << std::endl;
    for (int i = 0; i < argc; ++i) {
        std::cout << "argv[" << i << "] = " << argv[i] << std::endl;
    }

    flightgraph fg;
    ifstream fin("flight.txt");
    if(!fin){
        cout<<"Error opening file"<<endl;
        return 1;
    }
    string from,to,name,dept,code;
    int d,c,time;
    while(fin>>from>>to>>time>>c>>d>>dept>>name>>code){
        replace(name.begin(),name.end(),'_',' ');
        fg.addflight(from, to, d, c,time, dept,name,code);
    }
    fin.close();

    if (argc < 2) {
        cout << "Error"<<endl;
        return 1;
    }

    string mode=argv[1];
    if(mode=="find"){
        if(argc<7){
            cout<<"Error"<<endl;
            return 1;
        }
        string from = argv[2];
        string to = argv[3];
        string time = argv[4];
        int b = stoi(argv[5]);
        int c = stoi(argv[6]);
        cout << "===== Flight Route Optimizer =====\n";
        cout << "\nFinding best route from " << from << " to " << to << " after " << time << "...\n\n";
        fg.k_shortestpaths(from, to, b, c, time);

    }

    else if(mode=="list"){
        ifstream fin("flight.txt");
        if(!fin){
            cout<<"Error opening file"<<endl;
            return 1;
        }
        string from,to,name,dept,code;
        int d,c,time;
        while(fin>>from>>to>>time>>c>>d>>dept>>name>>code){
            cout<<from<<" "<<to<<" "<<d<<" "<<c<<"  "<<time<<" "<<dept<<" "<<name<<" "<<code<<endl;
        }
    }
    return 0;
}


