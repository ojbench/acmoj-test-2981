#include <cstdio>
#include <algorithm>
#include <cstring>
using namespace std;

const int MAXN = 200005;

int n;
pair<int,int> pts[MAXN];
int cy[MAXN]; // compressed y
int yvals[MAXN], ny;
long long ans;

// BIT01 for find_first_ge and find_last_le
struct BIT01 {
    int tree[MAXN], sz, LOG;
    int stk[MAXN * 20], top; // track modified tree positions
    
    void init(int n) {
        sz = n; top = 0; LOG = 0;
        while ((1 << (LOG+1)) <= n) LOG++;
    }
    
    void add(int i) { // 0-indexed, add 1
        for (int j = i + 1; j <= sz; j += j & -j) {
            tree[j]++;
            stk[top++] = j;
        }
    }
    
    int prefix(int i) { // sum [0..i]
        int s = 0;
        for (int j = i + 1; j > 0; j -= j & -j) s += tree[j];
        return s;
    }
    
    int find_kth(int k) { // 1-indexed k, returns 0-indexed position
        int pos = 0;
        for (int pw = 1 << LOG; pw; pw >>= 1)
            if (pos + pw <= sz && tree[pos + pw] < k) {
                pos += pw; k -= tree[pos];
            }
        return pos;
    }
    
    int find_first_ge(int lo) {
        int total = prefix(sz - 1);
        int before = lo > 0 ? prefix(lo - 1) : 0;
        if (before >= total) return -1;
        return find_kth(before + 1);
    }
    
    int find_last_le(int hi) {
        if (hi < 0) return -1;
        int cnt = prefix(hi);
        if (cnt == 0) return -1;
        return find_kth(cnt);
    }
    
    void clear() {
        while (top) tree[stk[--top]] = 0;
    }
} pres;

// Sum BIT for counting
struct SumBIT {
    int tree[MAXN], sz;
    
    void init(int n) { sz = n; }
    
    void add(int i, int v) {
        for (int j = i + 1; j <= sz; j += j & -j) tree[j] += v;
    }
    
    int prefix(int i) {
        int s = 0;
        for (int j = i + 1; j > 0; j -= j & -j) s += tree[j];
        return s;
    }
    
    int range(int l, int r) {
        return l > r ? 0 : prefix(r) - (l > 0 ? prefix(l - 1) : 0);
    }
} cntBIT;

int thresh[MAXN];
int rblock[MAXN];
int sortL[MAXN], sortR[MAXN];

void solve(int l, int r) {
    if (l >= r) return;
    int m = (l + r) / 2;
    solve(l, m);
    solve(m + 1, r);
    
    // Threshold: min compressed y >= cy[i] among [i+1..m]
    for (int i = m; i >= l; i--) {
        thresh[i] = pres.find_first_ge(cy[i]);
        if (thresh[i] == -1) thresh[i] = ny;
        pres.add(cy[i]);
    }
    pres.clear();
    
    // Right block: max compressed y <= cy[j] among [m+1..j-1]
    for (int j = m + 1; j <= r; j++) {
        rblock[j] = pres.find_last_le(cy[j]);
        pres.add(cy[j]);
    }
    pres.clear();
    
    // Count: rblock[j] < cy[i] < cy[j] AND thresh[i] > cy[j]
    int nL = 0, nR = 0;
    for (int i = l; i <= m; i++) sortL[nL++] = i;
    for (int j = m + 1; j <= r; j++) sortR[nR++] = j;
    
    sort(sortL, sortL + nL, [](int a, int b) { return thresh[a] > thresh[b]; });
    sort(sortR, sortR + nR, [](int a, int b) { return cy[a] > cy[b]; });
    
    int li = 0;
    for (int ri = 0; ri < nR; ri++) {
        int j = sortR[ri];
        int cyj = cy[j];
        while (li < nL && thresh[sortL[li]] > cyj)
            cntBIT.add(cy[sortL[li++]], 1);
        int lo = rblock[j] + 1, hi = cyj - 1;
        if (lo <= hi) ans += cntBIT.range(lo, hi);
    }
    for (int i = 0; i < li; i++)
        cntBIT.add(cy[sortL[i]], -1);
}

int main() {
    scanf("%d", &n);
    for (int i = 0; i < n; i++) scanf("%d%d", &pts[i].first, &pts[i].second);
    sort(pts, pts + n);
    
    for (int i = 0; i < n; i++) yvals[i] = pts[i].second;
    sort(yvals, yvals + n);
    ny = unique(yvals, yvals + n) - yvals;
    for (int i = 0; i < n; i++)
        cy[i] = lower_bound(yvals, yvals + ny, pts[i].second) - yvals;
    
    memset(pres.tree, 0, sizeof(pres.tree));
    memset(cntBIT.tree, 0, sizeof(cntBIT.tree));
    pres.init(ny);
    cntBIT.init(ny);
    ans = 0;
    
    solve(0, n - 1);
    printf("%lld\n", ans);
}
