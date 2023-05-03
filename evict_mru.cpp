#include <iostream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
using namespace std;

int main() {


    /*auto t1 = make_tuple(1, "abc");
    auto t2 = make_tuple(1, "abx");
    cout << (t1 == t2 ? "YES" : "NO") << endl;*/



    int N, Q, M;
    cin >> N >> Q >> M;
    tuple<int, int> buffer[Q + 1];
    int atime[Q + 1];

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

    vector<tuple<int, int> > slots[N + 1]; // user -> (page, index)
    map<tuple<int, int>, int> pageToSlot;
    set<int> evictables;

    for (int op=1; op<=M; op++) {
        int u, p;
        cin >> u >> p;
        // user u, page p
        auto up = make_tuple(u, p);

        auto slotIt = pageToSlot.find(up);
        if (slotIt != pageToSlot.end()) {
            cout << slotIt -> second << endl;
            continue;
        }

        if (slots[u].size() == qmax[u]) {
            int page, slot;
            std::tie(page, slot) = slots[u].back();
            slots[u].pop_back();
            pageToSlot.erase( make_tuple(u, page ) );

            pageToSlot[up] = slot;
            slots[u].push_back( make_tuple(p, slot ) );
            cout << slot << endl;
            continue;
        }

        if (pageToSlot.size() < Q) {
            int newSlot = pageToSlot.size() + 1;
            pageToSlot[up] = newSlot;
            slots[u].push_back( make_tuple(p, newSlot) );
            if (slots[u].size() > qmin[u]) {
                evictables.insert(u);
            }
            cout << newSlot << endl;
            continue;
        }

        int evictable = -1;
        if (evictables.empty() || (evictables.size() == 1 && evictables.count(u) == 1)) {
            evictable = u;
        } else {
            auto it = evictables.end();
            it--;
            if (*it == u) {
                it--;
            }
            evictable = *it;
        }
        

        int page, slot;
        std::tie(page, slot) = slots[evictable].back();
        slots[evictable].pop_back();
        pageToSlot.erase( make_tuple(evictable, page ) );

        if (slots[evictable].size() == qmin[evictable]) {
            evictables.erase(evictable);
        }

        pageToSlot[up] = slot;
        slots[u].push_back( make_tuple(p, slot ) );
        cout << slot << endl;
        if (slots[u].size() > qmin[u]) {
            evictables.insert(u);
        }
    }



}