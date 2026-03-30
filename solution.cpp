#include <bits/stdc++.h>
using namespace std;

// Problem: count axis-aligned rectangles with bottom-left and top-right
// on given points, with no other points inside or on boundary.
// Points sorted by x. For pair (i,j) with i<j, pts[i].y < pts[j].y:
// valid iff no k in (i,j) has pts[i].y <= pts[k].y <= pts[j].y.
//
// CDQ divide and conquer approach:
// Split by x (already sorted). Count cross-half pairs where
// bottom-left is in left half, top-right is in right half.
//
// For cross pairs (i in L, j in R):
// - No point in L with index > i has y in [y_i, y_j]
//   => y_i must be the max y <= y_j among pts[i..m] restricted to y <= y_j
//      Actually: among pts[i+1..m], none has y in [y_i, y_j]
// - No point in R with index < j has y in [y_i, y_j]
//   => among pts[m+1..j-1], none has y in [y_i, y_j]
//
// For the left half: build a "suffix max below threshold" structure.
// For the right half: build a "prefix min above threshold" structure.
//
// Alternative approach: sweep from left to right within each half.

// Actually, let me use a different O(n log n) approach:
// 
// Key insight: For each point j (top-right), the valid bottom-lefts
// form the "visible" points looking left, only considering y < y_j.
// A point i is "visible" from j if it's the local max of y (< y_j)
// in suffix [i..j-1].
//
// Equivalently: among points to the left with y < y_j, point i is valid
// iff no point k with i < k < j has y_i <= y_k < y_j.
// This means y_i must be a "right-to-left record" in the filtered sequence.
//
// Use CDQ: divide points by x. For cross-half pairs:
// Left half sorted by some criterion, right half provides queries.

// Let me use a simpler approach: for each j, the valid bottom-lefts
// are exactly the "descending suffix maxima" of y-values to the left, 
// filtered to y < y_j.

// Approach using a modified merge / BIT:
// Process points left to right. Maintain a BIT on y-coordinates.
// For each point j, we need to count valid bottom-lefts.
// 
// A bottom-left i is valid for j iff:
// 1) y_i < y_j
// 2) The max y in (i, j) that is in [y_i, y_j] is < y_i (i.e., none exists)
//
// This is hard to query directly. Let me think differently.
//
// Observation: point i is a valid bottom-left for SOME top-right j
// iff the first point to its right with y > y_i forms a valid pair.
// Not quite...
//
// CDQ approach (cleaner):
// solve(l, r): count valid pairs within [l, r]
// = solve(l, m) + solve(m+1, r) + cross(l, m, m+1, r)
//
// cross: bottom-left in [l,m], top-right in [m+1,r]
// For such a pair (i, j):
// - y_i < y_j
// - No k in (i,j) with y_i <= y_k <= y_j
// = No k in (i+1, m) with y_i <= y_k <= y_j
//   AND no k in (m+1, j-1) with y_i <= y_k <= y_j
//
// Process right half from left to right (m+1 to r).
// Maintain: for each point in [m+1, j-1], track their y-values.
// For the left half, process from right to left (m down to l).
// Maintain: suffix max of y that is relevant.
//
// For right side: maintain "min_y_from_left" = min y >= 0 among [m+1..j-1]
// Wait, need to be more careful.
//
// For j in right half: define B_j = set of y-values of points in [m+1..j-1].
//   The blocking condition from right side: no y in B_j falls in [y_i, y_j].
//   Equivalently: the min value in B_j that is >= y_i must be > y_j.
//   Since y_i < y_j, this means: no value in B_j is in [y_i, y_j].
//
// For i in left half: define A_i = set of y-values of points in [i+1..m].
//   The blocking condition from left side: no y in A_i falls in [y_i, y_j].
//   Since we need this for variable y_j, this means: the min value in A_i 
//   that is >= y_i must be > y_j. Call this threshold_i = min{y in A_i : y >= y_i}.
//   If no such value, threshold_i = +inf.
//   Condition: threshold_i > y_j, i.e., y_j < threshold_i.
//
// Similarly for right side: for j, define threshold_j = min{y in B_j : y >= 0}.
//   Wait, we need: no y in B_j falls in [y_i, y_j].
//   This means min{y in B_j : y >= y_i} > y_j.
//   Call right_threshold(j, y_i) = min{y in B_j : y >= y_i}.
//   This depends on y_i, making it hard.
//
// Hmm, let me simplify. For the right side, as j increases,
// B_j grows. We can process j from left to right and maintain
// a sorted structure of B_j.
//
// For a fixed j, the valid bottom-left i must satisfy:
// 1) y_i < y_j
// 2) threshold_i > y_j (left blocking condition)
// 3) min{y in B_j : y >= y_i} > y_j (right blocking condition)
//    Since y_j > y_i and all y in B_j that are >= y_i must be > y_j,
//    this means: no y in B_j is in [y_i, y_j].
//    Equivalently: the largest y in B_j that is <= y_j must be < y_i.
//    Call this right_block_j = max{y in B_j : y <= y_j} (or -inf if none).
//    Condition: y_i > right_block_j.
//
// So conditions: y_i < y_j, y_j < threshold_i, y_i > right_block_j.
// Combined: right_block_j < y_i < y_j < threshold_i.
//
// For the left half: precompute threshold_i for each i in [l, m].
// threshold_i = min{pts[k].y : k in (i, m], pts[k].y >= pts[i].y}
// If no such k, threshold_i = +inf.
//
// For the right half: process j from m+1 to r. Maintain B_j as a sorted 
// set. right_block_j = max{y in B_j : y <= y_j}.
//
// For each j, count i in left half where:
// right_block_j < y_i < y_j AND y_j < threshold_i
// i.e., y_i in (right_block_j, y_j) AND threshold_i > y_j
//
// This is a 2D counting problem: given the left-half points as 
// (y_i, threshold_i) pairs, count those with y_i in (rb, y_j) 
// and threshold_i > y_j.
//
// This can be answered with an offline approach:
// Sort left-half by threshold_i, process queries in order of y_j,
// use a BIT on y-coordinates.
//
// But threshold_i depends on relative positions within left half,
// which are already fixed. We can precompute all threshold_i.
//
// Total complexity: O(n log^2 n) with this CDQ approach.
//
// This is getting complex. Let me try a different, simpler O(n log n) approach.

// SIMPLER APPROACH: Use a stack + BIT
// 
// Process points left to right. Maintain a stack of y-values.
// When processing point j with y_j:
// - For counting as top-right: iterate over the "visible" bottom-lefts.
//   The visible bottom-lefts have y-values that form a decreasing sequence
//   from the nearest (highest y < y_j) to the farthest.
// - When a new point is inserted, it might "block" some visibility.
//
// Actually, let's think about it as: each point can be a valid bottom-left
// for exactly one top-right. 
//
// Wait, is that true? Can a point be a valid bottom-left for multiple top-rights?
// Yes! Consider points at (0,0), (1,3), (2,1). Then (0,0) is valid bottom-left
// for both (1,3) and (2,1)... let me check.
// (0,0)-(1,3): no point in between (adjacent). Valid ✓
// (0,0)-(2,1): point (1,3) has y=3>1, outside. Valid ✓
// So yes, one bottom-left can pair with multiple top-rights.
//
// But can a point be a valid top-right for multiple bottom-lefts?
// Consider (0,0), (1,1), (2,3).
// (0,0)-(2,3): point (1,1) has y=1 in [0,3]. Not valid.
// (1,1)-(2,3): no point between. Valid.
// So (2,3) only pairs with (1,1), not (0,0).
// 
// In general, for a fixed top-right j, the valid bottom-lefts form
// a "staircase" going left, with decreasing y.

// OK let me just implement the CDQ approach properly.

int n;
vector<pair<int,int>> pts; // sorted by x
long long ans;

// Coordinate compress y-values
vector<int> ys;
int compress(int y) {
    return lower_bound(ys.begin(), ys.end(), y) - ys.begin();
}

// BIT for counting
struct BIT {
    vector<int> tree;
    int sz;
    void init(int n) { sz = n; tree.assign(n + 1, 0); }
    void update(int i, int v) { for (i++; i <= sz; i += i & (-i)) tree[i] += v; }
    int query(int i) { int s = 0; for (i++; i > 0; i -= i & (-i)) s += tree[i]; return s; }
    int query(int l, int r) { return l > r ? 0 : query(r) - (l > 0 ? query(l-1) : 0); }
};

BIT bit;

// For the left half, precompute threshold_i
// threshold_i = min{pts[k].y : k in (i, m], pts[k].y >= pts[i].y}
// Process from right to left using a set or stack

void solve(int l, int r) {
    if (r - l <= 0) return;
    if (r - l == 1) {
        // single pair
        int i = l, j = l + 1;
        if (pts[i].second < pts[j].second) ans++;
        return;
    }
    
    int m = (l + r) / 2;
    solve(l, m);
    solve(m + 1, r);
    
    // Count cross pairs: bottom-left in [l, m], top-right in [m+1, r]
    
    // Precompute threshold_i for left half
    // threshold_i = min y >= y_i among points in (i, m] (inclusive of m)
    // Use a multiset scanning from right to left
    int leftLen = m - l + 1;
    vector<int> threshold(leftLen, INT_MAX);
    {
        // We need: for each i in [l, m], find the minimum y among pts[k].y 
        // for k in (i, m] where pts[k].y >= pts[i].y.
        // Scan from right (m) to left (l).
        // Maintain a sorted set. When at position i, the set contains 
        // y-values of pts[i+1..m].
        set<int> S;
        for (int i = m; i >= l; i--) {
            // threshold for i: min y in S that is >= pts[i].second
            auto it = S.lower_bound(pts[i].second);
            if (it != S.end()) {
                threshold[i - l] = *it;
            }
            S.insert(pts[i].second);
        }
    }
    
    // For right half, process j from m+1 to r.
    // Maintain sorted set B_j of y-values in [m+1, j-1].
    // right_block_j = max y in B_j that is <= y_j (or -1 if none).
    //
    // For each j, count i in [l, m] where:
    //   right_block_j < y_i < y_j AND threshold_i > y_j
    //
    // Offline approach:
    // - Put all left-half points as (y_i, threshold_i) pairs
    // - For each j, query: count of (y_i, threshold_i) with 
    //   y_i in (right_block_j, y_j) and threshold_i > y_j
    //
    // We can sort queries by y_j and use a BIT.
    // 
    // But threshold_i > y_j means we need to process in order of 
    // decreasing y_j (or add points with threshold_i >= current y_j).
    //
    // Alternative: for each j, the constraint threshold_i > y_j can be 
    // handled by only including left-half points with threshold_i > y_j.
    //
    // Since different j's have different y_j, this requires careful ordering.
    //
    // Sort right-half points by y_j descending. Sort left-half points by 
    // threshold_i descending. Use two pointers: for each j (descending y_j),
    // add all left-half points with threshold_i > y_j to BIT (keyed by y_i).
    // Then query BIT for count of y_i in (right_block_j, y_j).
    
    // Prepare left-half entries sorted by threshold descending
    struct LeftEntry { int yi; int thresh; };
    vector<LeftEntry> leftEntries;
    for (int i = l; i <= m; i++) {
        leftEntries.push_back({pts[i].second, threshold[i - l]});
    }
    sort(leftEntries.begin(), leftEntries.end(), [](const LeftEntry& a, const LeftEntry& b) {
        return a.thresh > b.thresh;
    });
    
    // Prepare right-half queries sorted by y_j descending
    // Also compute right_block_j for each j
    struct RightQuery { int yj; int right_block; int idx; };
    vector<RightQuery> rightQueries;
    {
        set<int> B;
        for (int j = m + 1; j <= r; j++) {
            int yj = pts[j].second;
            int rb = -1;
            if (!B.empty()) {
                auto it = B.upper_bound(yj);
                if (it != B.begin()) {
                    --it;
                    rb = *it;
                }
            }
            rightQueries.push_back({yj, rb, j});
            B.insert(yj);
        }
    }
    sort(rightQueries.begin(), rightQueries.end(), [](const RightQuery& a, const RightQuery& b) {
        return a.yj > b.yj;
    });
    
    // Two pointer: add left entries, query for right
    int li = 0;
    for (auto& q : rightQueries) {
        // Add all left entries with threshold > q.yj
        while (li < (int)leftEntries.size() && leftEntries[li].thresh > q.yj) {
            bit.update(compress(leftEntries[li].yi), 1);
            li++;
        }
        // Query: count y_i in (q.right_block, q.yj)
        int lo = compress(q.right_block + 1); // first y > right_block
        int hi = compress(q.yj) - 1;          // last y < yj
        // But we need y_i strictly > right_block and strictly < yj
        // Since all y-values are distinct integers, right_block+1 and yj-1 
        // might not be actual y-values. We need to find the compressed range.
        // lo = first compressed index >= right_block+1
        // hi = last compressed index <= yj-1
        int lo_idx = lower_bound(ys.begin(), ys.end(), q.right_block + 1) - ys.begin();
        int hi_idx = (lower_bound(ys.begin(), ys.end(), q.yj) - ys.begin()) - 1;
        if (lo_idx <= hi_idx) {
            ans += bit.query(lo_idx, hi_idx);
        }
    }
    
    // Clean up BIT
    for (int i = 0; i < li; i++) {
        bit.update(compress(leftEntries[i].yi), -1);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    cin >> n;
    pts.resize(n);
    for (int i = 0; i < n; i++) {
        cin >> pts[i].first >> pts[i].second;
    }
    
    sort(pts.begin(), pts.end());
    
    // Coordinate compress y
    ys.resize(n);
    for (int i = 0; i < n; i++) ys[i] = pts[i].second;
    sort(ys.begin(), ys.end());
    ys.erase(unique(ys.begin(), ys.end()), ys.end());
    
    bit.init(ys.size());
    ans = 0;
    
    if (n >= 2) {
        solve(0, n - 1);
    }
    
    cout << ans << "\n";
    return 0;
}
