#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
using namespace std;

int main() {
    /*int abc = 4 / 0;
    auto t1 = make_tuple(1, "abc");
    auto t2 = make_tuple(1, "abx");
    cout << (t1 == t2 ? "YES" : "NO") << endl;*/

    int N, Q, M;
    cin >> N >> Q >> M;

    int L[N + 1];
    int maxL = 0;
    for (int i=1; i<=N; i++) {
        cin >> L[i];
        maxL = std::max(maxL, L[i]);
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

    int total_access[N + 1];
    for (int i=1; i<=N; i++) {
        total_access[i] = 0;
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
    int bufferSize[N + 1];
    int pageFault[N + 1];
    for (int i=1; i<=N; i++) {
        bufferSize[i] = 0;
        pageFault[i] = 0;
    }
    map<tuple<int, int>, int> pageToSlot;
    map<int, tuple<int, int>> slotToPage;
    map<int, int> slotToTime[N + 1];
    map<int, int> timeToSlot[N + 1];
    int faultcount[N + 1][100100];
    for (int i=1; i<=N; i++) {
        for (int j=1; j<=D[i]; j++) {
            faultcount[i][j] = 0;
        }
    }

    bool buffer_full = false;

    for (int time=1; time<=M; time++) {
        int u, p;
        cin >> u >> p;

        total_access[u]++;

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

        // DECISION START
        // user u, page p
        auto up = make_tuple(u, p);

        auto slotIt = pageToSlot.find(up);
        if (slotIt != pageToSlot.end()) {
            int slot = slotIt -> second;
            cout << slot << endl;

            int lastTime = slotToTime[u][slot];
            timeToSlot[u].erase(lastTime);

            timeToSlot[u][time] = slot;
            slotToTime[u][slot] = time;
            continue;
        } else {
            pageFault[u]++;
            faultcount[u][p]++;
        }

        if (bufferSize[u] == qmax[u]) {
            //we need to evict u's own page
            int access_time = timeToSlot[u].begin() -> first;
            int slot = timeToSlot[u].begin() -> second;

            timeToSlot[u].erase(access_time); // we will remove this "access time"
            timeToSlot[u][time] = slot;
            slotToTime[u][slot] = time;

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

            slotToTime[u][newSlot] = time;
            timeToSlot[u][time] = newSlot;
            cout << newSlot << endl;

            continue;
        } else {
            buffer_full = true;
        }

        /*double max_ratio = 0.0;
        for (int i=1; i<=N; i++) {
            if (timeToSlot[i].empty())
                continue;

            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                if (1.0 * e_page_fault[i] / total_access[i] > max_ratio) {
                    max_ratio = 1.0 * e_page_fault[i] / total_access[i];
                }
            }
        }*/

        vector<int> evictables;
        for (int i=1; i<=N; i++) {
            if (i == u) {
                if (bufferSize[u] >= qmin[u] && bufferSize[u] >= qbase[u]) {
                    evictables.push_back(u);
                }
            } else {
                if (bufferSize[i] > qmin[i] && bufferSize[i] > qbase[i]) {
                    evictables.push_back(i);
                }
            }
        }
        if (evictables.empty()) {
            for (int i=1; i<=N; i++) {
                evictables.push_back(i);
            }
        }

        int evictable = -1;
        int access_time = -1;
        double min_tenant_cost = 1e9 * 1e9 * 1e9 * 1e9;
        for (int i : evictables) {
            if (timeToSlot[i].empty())
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
                    access_time = timeToSlot[i].begin() -> first;
                }
            }
        }


        double min_weight = 1e9 * 1e9;
        int cnt = 10;
        for (auto it = timeToSlot[evictable].begin(); it != timeToSlot[evictable].end(); it++) {
            --cnt;
            if (cnt == 0)
                break;
 
            int slot = it->second;
            tuple<int, int> user_page = slotToPage[slot];
            if (get<0>(user_page) != evictable) {
                throw new std::invalid_argument("get out !!!");
            }
            int page = get<1>(user_page);

            double weight = 0;
            if (e_faultcount[evictable][page] == 0) {
                if (faultcount[evictable][page] == 0) {
                    weight = 0;
                } else {
                    weight = faultcount[evictable][page];
                }
            } else {
                weight = 1.0 * (faultcount[evictable][page] - e_faultcount[evictable][page] ) / e_faultcount[evictable][page];
            }

            if (weight < min_weight) {
                min_weight = weight;
                access_time = it->first;
            }


        }

        /*cnt = 10;
        for (auto it = timeToSlot[evictable].rbegin(); it != timeToSlot[evictable].rend(); it++) {
            --cnt;
            if (cnt == 0)
                break;
 
            int slot = it->second;
            tuple<int, int> user_page = slotToPage[slot];
            if (get<0>(user_page) != evictable) {
                throw new std::invalid_argument("get out !!!");
            }
            int page = get<1>(user_page);
            int diff = faultcount[evictable][page] - e_faultcount[evictable][page];
            if (diff < 0) {
                access_time = it->first;
            }
        }*/

        /*int access_time = -1;
        int min_time = 100000000;
        int evictable = -1;

        int considered = 0;
        for (int i=1; i<=N; i++) {
            if (timeToSlot[i].empty())
                continue;
            
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                
                /*if (max_ratio >= 0.95 && 1.0 * e_page_fault[i] / total_access[i] < max_ratio - 1e-9) {
                    continue;
                }
                considered++;

                int lruTime = timeToSlot[i].begin() -> first;
                if (lruTime < min_time) {
                    min_time = lruTime;
                    access_time = lruTime;
                    evictable = i;
                }
            }
        }*/

        //if (considered > 1)
        //    throw new std::invalid_argument("come on !!!");

        if (evictable == -1)
            throw new std::invalid_argument("no no no no!!!");

        int slot = timeToSlot[evictable][access_time];
        timeToSlot[evictable].erase(access_time);
        slotToTime[evictable].erase(slot);

        timeToSlot[u][time] = slot;
        slotToTime[u][slot] = time;

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