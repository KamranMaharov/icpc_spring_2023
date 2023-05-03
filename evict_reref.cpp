#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
using namespace std;

int main() {

    /*if (make_tuple(3, 4) < make_tupxxxle(3, 1))
        cout << "yes" << endl;
    else
        cout << "no" << endl;*/

    /*int abc = 4 / 0;
    auto t1 = make_tuple(1, "abc");
    auto t2 = make_tuple(1, "abx");
    cout << (t1 == t2 ? "YES" : "NO") << endl;*/



    int N, Q, M;
    cin >> N >> Q >> M;


    int L[N + 1];
    for (int i=1; i<=N; i++) {
        cin >> L[i];
    } // TODO: ignoring for now

    int D[N + 1];
    for (int i=1; i<=N; i++) {
        cin >> D[i];
    } // TODO: ignoring for now

    int qmin[N + 1];
    int qbase[N + 1];
    int qmax[N + 1];

    for (int i=1; i<=N; i++) {
        cin >> qmin[i] >> qbase[i] >> qmax[i];
        // TODO: ignoring qbase for now
    }

    // SIMULATION STRUCTURES
    map<int, int> e_page_to_slot[N + 1];
    map<int, int> e_slot_to_page[N + 1];
    map<int, int> e_slot_to_time[N + 1];
    map<int, int> e_time_to_slot[N + 1];
    int e_buffer_size[N + 1];
    bool e_buffer_full[N + 1];
    int e_page_fault[N + 1];
    for (int i=1; i<=N; i++) {
        e_buffer_size[i] = 0;
        e_buffer_full[i] = false;
        e_page_fault[i] = 0;
    }
    int e_faultcount[N + 1][100100];
    for (int i=1; i<=N; i++) {
        for (int j=1; j<=D[i]; j++) {
            e_faultcount[i][j] = 0;
        }
    }

    // DECISION STRUCTURES
    //vector<tuple<int, int> > slots[N + 1]; // user -> (page, index)
    int bufferSize[N + 1];
    int tenantTime[N + 1];
    int pageFault[N + 1];
    for (int i=1; i<=N; i++) {
        bufferSize[i] = 0;
        tenantTime[i] = 0;
        pageFault[i] = 0;
    }
    map<tuple<int, int>, int> pageToSlot;
    map<int, tuple<int, int>> slotToPage;

    // maintain all tenant's assigned slots in global map
    /*map<int, int> slotToTime;
    map<int, int> timeToSlot;

    for (int tm = 1; tm <= Q; tm++) {
        slotToTime[tm] = tm;
        timeToSlot[tm] = tm;
    }*/

    // frequencies across tenants can be equal,
    // unlike recencies.
    map<int, tuple<int, int> > slotToHits[N + 1];
    map<tuple<int, int> , int> hitsToSlot[N + 1];

    // int time = Q;
    bool buffer_full = false;
    double md_factor = 0.5;
    double md2_factor = 1.0;
    int md_tenant = -1;

    for (int time=1; time<=M; time++) {

        int u, p;
        cin >> u >> p;


        // SIMULATION START
        { // simulate qbase - original LRU algorithm
            auto slotIt = e_page_to_slot[u].find(p);
            bool fallthrough = false;

            if (e_page_to_slot[u].size() != e_buffer_size[u])
                throw new std::invalid_argument("get out 1");
            if (e_slot_to_page[u].size() != e_buffer_size[u])
                throw new std::invalid_argument("get out 2");
            if (e_time_to_slot[u].size() != e_buffer_size[u])
                throw new std::invalid_argument("get out 2");
            if (e_slot_to_time[u].size() != e_buffer_size[u])
                throw new std::invalid_argument("get out 2");

            if (slotIt != e_page_to_slot[u].end() ) {
                int slot = slotIt -> second;

                int lastTime = e_slot_to_time[u][slot];
                e_slot_to_time[u].erase(slot);
                e_time_to_slot[u].erase(lastTime);

                e_slot_to_time[u][slot] = time;
                e_time_to_slot[u][time] = slot;
                fallthrough = true;
            } else {
                e_page_fault[u]++;
                e_faultcount[u][p]++;
            }

            if (!fallthrough && e_buffer_size[u] == qbase[u]) {
                //we need to evict u's own page
                int access_time = e_time_to_slot[u].begin() -> first;
                int slot = e_time_to_slot[u].begin() -> second;

                e_time_to_slot[u].erase(access_time); // we will remove this "access time"
                e_slot_to_time[u].erase(slot);
                e_time_to_slot[u][time] = slot;
                e_slot_to_time[u][slot] = time;

                int evictable = u;
                int page = e_slot_to_page[evictable][slot];
                if (evictable != u)
                    throw std::invalid_argument("no way!!!");

                e_buffer_size[u]--;
                e_page_to_slot[u].erase(page);
                
                e_page_to_slot[u][p] = slot;
                e_slot_to_page[u][slot] = p;
                e_buffer_size[u]++;
                fallthrough = true;
            }

            if (!fallthrough) {
                if (e_page_to_slot[u].size() < qbase[u]) {
                    if (e_buffer_full[u]) {
                        throw new std::invalid_argument("buffer is already full!!!");
                    }
                    int newSlot = e_page_to_slot[u].size() + 1;
                    e_page_to_slot[u][p] = newSlot;
                    e_slot_to_page[u][newSlot] = p;
                    e_buffer_size[u]++;

                    e_slot_to_time[u][newSlot] = time;
                    e_time_to_slot[u][time] = newSlot;

                    fallthrough = true;
                } else {
                    e_buffer_full[u] = true;
                }
            }


        } // SIMULATION END






        for (int tt=1; tt<=N; tt++) {
            if (bufferSize[tt] != slotToHits[tt].size())
                throw new std::invalid_argument("get out");
            if (bufferSize[tt] != hitsToSlot[tt].size())
                throw new std::invalid_argument("get out");
        }

        if (md_tenant != -1) {

            for (int kd=md_tenant; kd<=md_tenant; kd++) {
                map<int, tuple<int, int> > toHits;
                map<tuple<int, int> , int> toSlots;

                for (auto it = slotToHits[kd].begin(); it != slotToHits[kd].end(); it++) {
                    int slot = it->first;
                    tuple<int, int> hit_time = it->second;

                    tuple<int, int> hit_time_v2;

//if (get<1>(to_evict_hit_time) > tenantTime[evictable] - bufferSize[evictable] * md_factor) {


                    if (get<1>(hit_time) <= tenantTime[kd] - bufferSize[kd] * md2_factor) {
                        hit_time_v2 = make_tuple(std::max( get<0>(hit_time) / 2 , 0) , get<1>(hit_time));
                    } else {
                        hit_time_v2 = make_tuple( get<0>(hit_time) , get<1>(hit_time));
                    }

                    toHits[slot] = hit_time_v2;
                    toSlots[hit_time_v2] = slot;
                }
                slotToHits[kd].swap(toHits);
                hitsToSlot[kd].swap(toSlots);
            }

        }        

        
        // user u, page p
        auto up = make_tuple(u, p);
        tenantTime[u]++;
        md_tenant = -1;

        auto slotIt = pageToSlot.find(up);
        if (slotIt != pageToSlot.end()) {
            int slot = slotIt -> second;
            cout << slot << endl;

            tuple<int, int> hit_time = slotToHits[u][slot];
            hitsToSlot[u].erase(hit_time);

            auto n_hit_time = make_tuple(get<0>(hit_time) + 1, tenantTime[u]);
            hitsToSlot[u][n_hit_time] = slot;
            slotToHits[u][slot] = n_hit_time;
            continue;
        } else {
            pageFault[u]++;
        }

        if (bufferSize[u] == qmax[u]) {
            //we need to evict u's own page
            tuple<int, int> hit_time = hitsToSlot[u].begin() -> first;
            int slot = hitsToSlot[u].begin() -> second;
            hitsToSlot[u].erase(hit_time);

            hitsToSlot[u][make_tuple(0, tenantTime[u])] = slot;
            slotToHits[u][slot] = make_tuple(0, tenantTime[u]);

            int evictable, page;
            tie(evictable, page) = slotToPage[slot];
            if (evictable != u)
                throw std::invalid_argument("no way!!!");

            bufferSize[u]--;
            pageToSlot.erase( make_tuple(evictable, page ) );
            
            pageToSlot[up] = slot;
            slotToPage[slot] = up;
            bufferSize[u]++;
            cout << slot << endl;
            continue;
        }

        if (pageToSlot.size() < Q) {
            if (buffer_full) {
                throw new std::invalid_argument("buffer is already full!!!");
            }
            int newSlot = pageToSlot.size() + 1;
            pageToSlot[up] = newSlot;
            slotToPage[newSlot] = up;
            bufferSize[u]++;

            slotToHits[u][newSlot] = make_tuple(0, tenantTime[u]);
            hitsToSlot[u][make_tuple(0, tenantTime[u])] = newSlot;
            cout << newSlot << endl;
            continue;
        } else {
            buffer_full = true;
        }



        int evictable = -1;
        tuple<int, int> to_evict_hit_time;
        double min_tenant_cost = 1e9 * 1e9 * 1e9 * 1e9;
        for (int i=1; i<=N; i++) {
            if (hitsToSlot[i].empty())
                continue;

            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                double sla_rate = -1.0;

                if (e_page_fault[i] == 0) {
                    if (pageFault[i] == 0) {
                        sla_rate = 0;
                    } else {
                        sla_rate = pageFault[i];
                    }
                } else {
                    sla_rate = 1.0 * ( 1.0 * max(pageFault[i], e_page_fault[i]) - e_page_fault[i] ) /     (e_page_fault[i] );
                }

                double tenant_cost = L[i] * sla_rate * sla_rate;
                if (tenant_cost < min_tenant_cost) {
                    min_tenant_cost = tenant_cost;
                    evictable = i;
                    to_evict_hit_time = hitsToSlot[i].begin() -> first;
                }
            }
        }






        /*tuple<int, double> min_hit_time = make_tuple(100000000, 100000000.0);
        int evictable = -1;
        tuple<int, int> to_evict_hit_time = min_hit_time;

        for (int i : evictables) {
            if (hitsToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                tuple<int, int> hit_time = hitsToSlot[i].begin() -> first;

                tuple<int, int> adjusted_hit_time = make_tuple(
                    get<0>(hit_time),
                    -1.0 * (tenantTime[i] - get<1>(hit_time)) / bufferSize[i]
                );

                if (adjusted_hit_time < min_hit_time) {
                    min_hit_time = adjusted_hit_time;
                    to_evict_hit_time = hit_time;
                    evictable = i;
                }
            }
        }*/

        if (evictable == -1)
            throw new std::invalid_argument("no no no no!!!");

        int slot = hitsToSlot[evictable][to_evict_hit_time];
        hitsToSlot[evictable].erase(to_evict_hit_time);
        slotToHits[evictable].erase(slot);

        if (get<1>(to_evict_hit_time) < 1 || get<1>(to_evict_hit_time) > tenantTime[evictable]) {
            throw new std::invalid_argument("evict time is invalid");
        }

        if (get<1>(to_evict_hit_time) > tenantTime[evictable] - bufferSize[evictable] * md_factor) {
            md_tenant = evictable;
        }

        hitsToSlot[u][make_tuple(0, tenantTime[u])] = slot;
        slotToHits[u][slot] = make_tuple(0, tenantTime[u]);

        int eee, page;
        std::tie(eee, page) = slotToPage[slot];
        if (eee != evictable) 
            throw new std::invalid_argument("no no no !!!");

        bufferSize[evictable]--;
        pageToSlot.erase( make_tuple(evictable, page ) );



        pageToSlot[up] = slot;
        slotToPage[slot] = up;
        bufferSize[u]++;

        cout << slot << endl;

    }

    /*if (Q < totalDb) {
        if (Q * 2 < totalBase)
            throw new std::invalid_argument("no way!!!");
    }*/

}