#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
using namespace std;

int main() {

    /*vector<int> aa[1] = {vector<int>{1,2,3}};
    vector<int> bb[1] = {vector<int>{4, 5}};
    aa[0].swap(bb[0]);
    for (auto a : aa[0]) {cout << a << " ";}
    cout << endl;
    for (auto b : bb[0]) {cout << b << " ";}
    cout << endl;*/

    /*if (make_tuple(3, 4) < make_tuple(3, 1))
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
    int lastAccess[N + 1];
    for (int i=1; i<=N; i++) {
        bufferSize[i] = 0;
        lastAccess[i] = 0;
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

    bool launch_md = false;


    for (int time=1; time<=M; time++) {

        launch_md = false; // TODO: fix
        if ( launch_md ) {
            map<int, tuple<int, int> > toHits[N + 1];
            map<tuple<int, int> , int> toSlots[N + 1];
            for (int i=1; i<=N; i++) {
                int timeline = time - Q + Q * (maxL - L[i] + 1) / ( std::max(3, N) * maxL );
                //int timeline = time - md_factor * Q;

                for (auto it = slotToHits[i].begin(); it != slotToHits[i].end(); it++) {
                    int slot = it->first;
                    tuple<int, int> hit_time = it->second;

                    tuple<int, int> hit_time_v2;
                    if (get<1>(hit_time) <= timeline) {
                        hit_time_v2 = make_tuple(std::max( get<0>(hit_time) / 2, 0) , get<1>(hit_time));
                    } else {
                        hit_time_v2 = hit_time;
                    }

                    toHits[i][slot] = hit_time_v2;
                    toSlots[i][hit_time_v2] = slot;
                }
                slotToHits[i].swap(toHits[i]);
                hitsToSlot[i].swap(toSlots[i]);
            }
        }

        launch_md = false;

        int u, p;
        cin >> u >> p;
        // user u, page p
        auto up = make_tuple(u, p);

        auto slotIt = pageToSlot.find(up);
        if (slotIt != pageToSlot.end()) {
            int slot = slotIt -> second;
            cout << slot << endl;

            tuple<int, int> hit_time = slotToHits[u][slot];
            hitsToSlot[u].erase(hit_time);

            // TODO: hit_time + 1 to convert LRU to LFU.
            auto n_hit_time = make_tuple(get<0>(hit_time) , time);
            hitsToSlot[u][n_hit_time] = slot;
            slotToHits[u][slot] = n_hit_time;
            lastAccess[u] = time;
            continue;
        }

        if (bufferSize[u] == qmax[u]) {
            //we need to evict u's own page
            tuple<int, int> hit_time = hitsToSlot[u].begin() -> first;
            int slot = hitsToSlot[u].begin() -> second;
            hitsToSlot[u].erase(hit_time);

            // we are evicting "too recent" element, due to LFU affect
            int timeline = time - Q + Q * (maxL - L[u] + 1) / ( std::max(4, N) * maxL );

            if (get<1>(hit_time) >= timeline) {
                launch_md = true;
            }

            hitsToSlot[u][make_tuple(0, time)] = slot;
            slotToHits[u][slot] = make_tuple(0, time);

            int evictable, page;
            tie(evictable, page) = slotToPage[slot];
            if (evictable != u)
                throw std::invalid_argument("no way!!!");

            bufferSize[u]--;
            pageToSlot.erase( make_tuple(evictable, page ) );
            
            pageToSlot[up] = slot;
            slotToPage[slot] = up;
            bufferSize[u]++;
            lastAccess[u] = time;
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

            slotToHits[u][newSlot] = make_tuple(0, time);
            hitsToSlot[u][make_tuple(0, time)] = newSlot;
            cout << newSlot << endl;
            lastAccess[u] = time;
            continue;
        } else {
            buffer_full = true;
        }

        vector<int> distantTenants;

        if (N > 5) {
            int min_last_access = 100000000;
            int selected_tenant_1 = -1;
            for (int i=1; i<=N; i++) {

                if (i == u)
                    continue;

                if (bufferSize[i] > qmin[i] ||
                    (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                    if (lastAccess[i] < min_last_access) {
                        min_last_access = lastAccess[i];
                        selected_tenant_1 = i;
                    }
                }
            }

            /*min_last_access = 100000000;
            int selected_tenant_2 = -1;
            for (int i=1; i<=N; i++) {

                if (i == u)
                    continue;

                if (bufferSize[i] > qmin[i] ||
                    (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                    if (lastAccess[i] < min_last_access && i != selected_tenant_1) {
                        min_last_access = lastAccess[i];
                        selected_tenant_2 = i;
                    }
                }
            }

            min_last_access = 100000000;
            int selected_tenant_3 = -1;
            for (int i=1; i<=N; i++) {
                if (i == u)
                    continue;

                if (bufferSize[i] > qmin[i] ||
                    (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                    if (lastAccess[i] < min_last_access &&
                        i != selected_tenant_1 &&
                        i != selected_tenant_2) {
                        min_last_access = lastAccess[i];
                        selected_tenant_3 = i;
                    }
                }
            }*/

            distantTenants.push_back(selected_tenant_1);
            /*if (selected_tenant_2 != -1) {
                distantTenants.push_back(selected_tenant_2);
            }
            if (selected_tenant_3 != -1) {
                distantTenants.push_back(selected_tenant_3);
            }*/
        } else {
            for (int i=1; i<=N; i++) {
                distantTenants.push_back(i);
            }
        }

        tuple<int, int> min_hit_time = make_tuple(100000000, 100000000);
        int evictable = -1;
        tuple<int, int> to_evict_hit_time = min_hit_time;

        for (auto i : distantTenants) {
            if (hitsToSlot[i].empty())
                continue;
            if (bufferSize[i] > qmin[i] ||
                (i == u && bufferSize[u] >= qmin[u]) ) { //i is evictable
                tuple<int, int> hit_time = hitsToSlot[i].begin() -> first;

                tuple<int, int> projectedHitTime = make_tuple(
                    get<0>(hit_time),
                    -( 1.0 * (time - get<1>(hit_time)) * (L[i] + 1) / L[i] )
                );

                if (projectedHitTime < min_hit_time) {
                    min_hit_time = projectedHitTime;
                    to_evict_hit_time = hit_time;
                    evictable = i;
                }
            }
        }

        if (evictable == -1)
            throw new std::invalid_argument("no no no no!!!");

        int slot = hitsToSlot[evictable][to_evict_hit_time];
        hitsToSlot[evictable].erase(to_evict_hit_time);
        slotToHits[evictable].erase(slot);

        // we are evicting "too recent" element, due to LFU affect
        int timeline = time - Q + Q * (maxL - L[evictable] + 1) / ( std::max(4, N) * maxL );

        if (get<1>(to_evict_hit_time) >= timeline) {
            launch_md = true;
        }

        hitsToSlot[u][make_tuple(0, time)] = slot;
        slotToHits[u][slot] = make_tuple(0, time);

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
        lastAccess[u] = time;

    }

    /*if (Q < totalDb) {
        if (Q * 2 < totalBase)

            throw new std::invalid_argument("no way!!!");
    }*/

}