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
            continue;
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

        int access_time = 100000000;
        int evictable = -1;
        int min_time = 100000000;
        double max_time = -100000000;
        int min_lru_time = 100000000;

        for (int i=1; i<=N; i++) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                int lruTime = timeToSlot[i].begin() -> first;
                if (lruTime < min_lru_time) {
                    min_lru_time = lruTime;
                }
            }
        }

        int threshold = min_lru_time + Q / N;

        for (int i=1; i<=N; i++) {
            if (timeToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                int lruTime = timeToSlot[i].begin() -> first;
                if (lruTime > threshold) {
                    continue;
                }

                double adjustedTime = 1.0 * (threshold - lruTime) * (L[i] + 1) / L[i];

                if (adjustedTime > max_time) {
                    max_time = adjustedTime;
                    access_time = lruTime;
                    evictable = i;
                }

                /*if (L[i] < min_l) {
                    min_l = L[i];
                    access_time = lruTime;
                    evictable = i;
                } else if (L[i] == min_l && lruTime < access_time) {
                    access_time = lruTime;
                    evictable = i;
                }*/
                //double adjustedTime = 1.0 * (time - lruTime) * (2 * L[i] + 1) / L[i];
                
                //double adjustedTime = 1.0 * (time - lruTime) * ( (maxL - 1) * (maxL - 1) + (maxL - L[i]) ) / (maxL * (maxL - 1));

                /*double adjustedTime = 1.0 * (time - lruTime) * maxL / L[i];

                if (adjustedTime > max_time) {
                    max_time = adjustedTime;
                    access_time = lruTime;
                    evictable = i;
                }*/
            }
        }

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

    int totalBase = 0;
    for (int i=1; i<=N; i++) {
        totalBase += qbase[i];
    }

    if (Q < totalBase) {
        if (Q * 2 < totalBase)
            throw new std::invalid_argument("no way!!!");
    }
}