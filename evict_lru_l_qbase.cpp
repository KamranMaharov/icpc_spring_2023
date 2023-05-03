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
    tuple<int, int> buffer[Q + 1];
    int atime[Q + 1];

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

    //vector<tuple<int, int> > slots[N + 1]; // user -> (page, index)
    int bufferSize[N + 1];
    for (int i=1; i<=N; i++) {
        bufferSize[i] = 0;
    }
    map<tuple<int, int>, int> pageToSlot;
    map<int, tuple<int, int>> slotToPage;

    map<int, int> slotToFreq;

    // maintain all tenant's assigned slots in global map
    /*map<int, int> slotToTime;
    map<int, int> timeToSlot;

    for (int tm = 1; tm <= Q; tm++) {
        slotToTime[tm] = tm;
        timeToSlot[tm] = tm;
    }*/

    map<int, int> slotToTime[N + 1];
    map<int, int> timeToSlot[N + 1];

    // int time = Q;
    bool buffer_full = false;

    for (int time=1; time<=M; time++) {
        int u, p;
        cin >> u >> p;
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

            slotToFreq[slot]++;
            continue;
        }

        if (bufferSize[u] == qmax[u]) {
            //we need to evict u's own page
            int access_time = timeToSlot[u].begin() -> first;
            int slot = timeToSlot[u].begin() -> second;
            slotToFreq[slot] = 0;

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

            slotToFreq[newSlot] = 0;
            continue;
        } else {
            buffer_full = true;
        }


        vector<int> evictables;
        for (int i=1; i<=N; i++) {
            if (i == u) {
                if (bufferSize[u] >= qmin[u] && bufferSize[u] >= qbase[u]) {
                    evictables.push_back(u);
                    continue;
                }
            } else {
                if (bufferSize[i] > qmin[i] && bufferSize[i] >  qbase[i]) {
                    evictables.push_back(i);
                    continue;
                }
            }
        }

        

        if (evictables.empty()) {
            // TODO: throw exception to check
            for (int i=1; i<=N; i++) {
                evictables.push_back(i);
            }
        }

        /*int min_time = 100000000;
        int evictable = -1;
        int access_time = -1;

        for (auto i : evictables) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                int lruTime = timeToSlot[i].begin() -> first;
                
                if (lruTime < min_time) {
                    evictable = i;
                    min_time = lruTime;
                    access_time = lruTime;
                }
            }
        }*/

        int min_time = 100000000;

        for (auto i : evictables) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                int lruTime = timeToSlot[i].begin() -> first;
                
                if (lruTime < min_time) {
                    min_time = lruTime;
                }
            }
        }



        int evictable = -1;
        int access_time = -1;
        int lru_time = 100000000;
        int minFreq = 100000000;

        for (auto i : evictables) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                
                int cnt = 0;
                for (auto it = timeToSlot[i].begin(); it != timeToSlot[i].end(); it++) {
                    cnt++;
                    if (cnt == 51)
                        break;
                    int current_time = it->first;
                    if (current_time > min_time + 2000)
                        continue;

                    int current_slot = it->second;
                    int freq = slotToFreq[current_slot];
                    if (freq < minFreq) {
                        minFreq = freq;
                        evictable = i;
                        access_time = current_time;
                        lru_time = current_time;
                    } else if (freq == minFreq && current_time < lru_time) {
                        evictable = i;
                        access_time = current_time;
                        lru_time = current_time;
                    }
                }
            }
        }
        
        /*int min_time = 100000000;
        int max_time = -100000000;
        int evictable = -1;
        int access_time = -1;

        for (auto i : evictables) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                int lruTime = timeToSlot[i].begin() -> first;
                int projectedTime = 1.0 * (time - lruTime) * (L[i] + 1) / L[i];
                if (projectedTime > max_time) {
                    max_time = projectedTime;
                    evictable = i;
                    access_time = lruTime;
                }
            }
        }*/

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

        slotToFreq[slot] = 0;
        cout << slot << endl;

    }

    /*if (Q < totalDb) {
        if (Q * 2 < totalBase)
            throw new std::invalid_argument("no way!!!");
    }*/

}