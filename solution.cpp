#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    cin >> n;
    
    vector<pair<int,int>> pts(n);
    for (int i = 0; i < n; i++) {
        cin >> pts[i].first >> pts[i].second;
    }
    
    // Sort by x-coordinate
    sort(pts.begin(), pts.end());
    
    long long ans = 0;
    
    // For each j as top-right, sweep left to find valid bottom-lefts
    // A bottom-left i is valid if pts[i].y < pts[j].y and
    // no point k in (i,j) has pts[i].y <= pts[k].y <= pts[j].y
    // We track max_y_below = max y seen so far (< y_j) sweeping left from j
    // Point i is valid iff pts[i].y > max_y_below
    
    for (int j = 1; j < n; j++) {
        int yj = pts[j].second;
        int max_y_below = -1;
        for (int i = j - 1; i >= 0; i--) {
            int yi = pts[i].second;
            if (yi < yj) {
                if (yi > max_y_below) {
                    ans++;
                }
                max_y_below = max(max_y_below, yi);
            }
        }
    }
    
    cout << ans << "\n";
    return 0;
}
