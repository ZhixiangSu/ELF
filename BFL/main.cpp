#include <sys/time.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iterator>
#include <utility>
#include <vector>
#include <iostream>

#ifndef K
#define K 5
#endif
#ifndef D
#define D (320 * K)
#endif

namespace bs {

using namespace std;

struct node {
  int N_O_SZ, N_I_SZ;
  int *N_O, *N_I;
  /// 
  int indegree;
  int max_out;
  pair<int, int>Level;
  /// 
  int vis;
  union {
    int L_in[K];
#if K > 8
    unsigned int h_in;
#else
    unsigned char h_in;
#endif
  };
  union {
    int L_out[K];
#if K > 8
    unsigned int h_out;
#else
    unsigned char h_out;
#endif
  };
  pair<int, int> L_interval;
};
vector<node> nodes;
int vis_cur, cur;

void read_graph(const char *filename) {
  timeval start_at, end_at;
  gettimeofday(&start_at, 0);
  FILE *file = fopen(filename, "r");
  char header[] = "graph_for_greach";
  fscanf(file, "%s", header);
  int n;
  fscanf(file, "%d", &n);
  nodes.resize(n);
  vector<vector<int>> N_O(n), N_I(n);
  for (;;) {
    int u, v;
    if (feof(file) || fscanf(file, "%d", &u) != 1) {
      break;
    }
    fgetc(file);
    while (!feof(file) && fscanf(file, "%d", &v) == 1) {
      N_O[u].push_back(v);
      N_I[v].push_back(u);
    }
    fgetc(file);
  }
  fclose(file);
  for (int u = 0; u < n; u++) {
    nodes[u].N_O_SZ = N_O[u].size();
    nodes[u].N_O = new int[N_O[u].size()];
    copy(N_O[u].begin(), N_O[u].end(), nodes[u].N_O);
    nodes[u].N_I_SZ = N_I[u].size();
    nodes[u].N_I = new int[N_I[u].size()];
    copy(N_I[u].begin(), N_I[u].end(), nodes[u].N_I);
    ///
    nodes[u].indegree = nodes[u].N_I_SZ;
    /// 
  }
  gettimeofday(&end_at, 0);
  printf("read time(graph): %.3fs\n",
         end_at.tv_sec - start_at.tv_sec +
             double(end_at.tv_usec - start_at.tv_usec) / 1000000);
}

int h_in() {
  static int c = 0, r = rand();
  if (c >= (int)nodes.size() / D) {
    c = 0;
    r = rand();
  }
  c++;
  return r;
}

int h_out() {
  static int c = 0, r = rand();
  if (c >= (int)nodes.size() / D) {
    c = 0;
    r = rand();
  }
  c++;
  return r;
}

void dfs_in(node &u) {
  u.vis = vis_cur;

  if (u.N_I_SZ == 0) {
    u.h_in = h_in() % (K * 32);
  } else {
    for (int i = 0; i < K; i++) {
      u.L_in[i] = 0;
    }

    for (int i = 0; i < u.N_I_SZ; i++) {
      node &v = nodes[u.N_I[i]];
      if (v.vis != vis_cur) {
        dfs_in(v);
      }
      if (v.N_I_SZ == 0) {
        int hu = v.h_in;
        u.L_in[(hu >> 5) % K] |= 1 << (hu & 31);
      } else {
        for (int j = 0; j < K; j++) {
          u.L_in[j] |= v.L_in[j];
        }
      }
    }

    int hu = h_in();
    u.L_in[(hu >> 5) % K] |= 1 << (hu & 31);
  }
}

void dfs_out(node &u) {
  u.vis = vis_cur;

  u.L_interval.first = cur++;

  if (u.N_O_SZ == 0) {
    u.h_out = h_out() % (K * 32);
  } else {
    for (int i = 0; i < K; i++) {
      u.L_out[i] = 0;
    }

    for (int i = 0; i < u.N_O_SZ; i++) {
      node &v = nodes[u.N_O[i]];
      if (v.vis != vis_cur) {
        dfs_out(v);
      }
      if (v.N_O_SZ == 0) {
        int hu = v.h_out;
        u.L_out[(hu >> 5) % K] |= 1 << (hu & 31);
      } else {
        for (int j = 0; j < K; j++) {
          u.L_out[j] |= v.L_out[j];
        }
      }
    }

    int hu = h_out();
    u.L_out[(hu >> 5) % K] |= 1 << (hu & 31);
  }

  u.L_interval.second = cur;
}
/// 
vector<int> TopoOrder;
bool in_out_choice;
int level_count[2];
/// 
/// calculate filter label
void level_filter_construction()
{
    for (int i = 0; i < TopoOrder.size(); i++) {
        int u = TopoOrder[i];
        int tmpLdown = 0;
        for (int j = 0; j < nodes[u].N_I_SZ; j++) {
            int addr = nodes[u].N_I[j];
            if (nodes[addr].Level.second > tmpLdown) {
                tmpLdown = nodes[addr].Level.second;
            }
        }
        nodes[u].Level.second = tmpLdown + 1;
    }
    for (int i = TopoOrder.size() - 1; i >= 0; i--) {
        int u = TopoOrder[i];
        int tmpLup = 0;
        for (int j = 0; j < nodes[u].N_O_SZ; j++) {
            int addr = nodes[u].N_O[j];
            if (nodes[addr].Level.first > tmpLup) {
                tmpLup = nodes[addr].Level.first;
            }
            
        }
        nodes[u].Level.first = tmpLup + 1;
        level_count[(bool)tmpLup]++;
    }
    if (level_count[1] > level_count[0] * 0.1)
    {
        in_out_choice = false;
    }
    else
    {
        in_out_choice = true;
    }
}
void ELF_construction()
{
    if (in_out_choice)
    {
        for (int u = 0; u < nodes.size(); u++) {
            nodes[u].max_out = -nodes[u].N_I_SZ * nodes.size() - u;
        }
    }
    else
    {
        for (int u = 0; u < nodes.size(); u++) {
            nodes[u].max_out = -nodes[u].N_O_SZ * nodes.size() + u;
        }
    }
    for (int i = TopoOrder.size() - 1; i >= 0; i--) {
        int u = TopoOrder[i];
        int tmp_max_out = nodes[u].max_out;
        for (int j = 0; j < nodes[u].N_O_SZ; j++) {
            int addr = nodes[u].N_O[j];
            if (nodes[addr].max_out > tmp_max_out) {
                tmp_max_out = nodes[addr].max_out;
            }
        }
        nodes[u].max_out = tmp_max_out;
    }
}
/// 
void index_construction() {
  timeval start_at, end_at;
  gettimeofday(&start_at, 0);

  vis_cur++;
  for (int u = 0; u < nodes.size(); u++) {
    if (nodes[u].N_O_SZ == 0) {
      dfs_in(nodes[u]);
    }
  }
  vis_cur++;
  cur = 0;
  for (int u = 0; u < nodes.size(); u++) {
    if (nodes[u].N_I_SZ == 0) {
      dfs_out(nodes[u]);
      ///
      TopoOrder.push_back(u);
      /// 
    }
    
  }
   
  int Topo_cur = 0;
  while (Topo_cur != TopoOrder.size())
  {
      int u = TopoOrder[Topo_cur];
      for (int i = 0; i < nodes[u].N_O_SZ; i++)
      {
          int v = nodes[u].N_O[i];
          nodes[v].indegree--;
          if (nodes[v].indegree == 0)
          {
              TopoOrder.push_back(v);
          }
      }
      Topo_cur++;
  }
  level_filter_construction();
  ELF_construction();
   
  gettimeofday(&end_at, 0);
  printf("index time: %.3fms\n",
      (end_at.tv_sec - start_at.tv_sec) * 1000 +
      double(end_at.tv_usec - start_at.tv_usec) / 1000);
  long long index_size = 0;
  for (int u = 0; u < nodes.size(); u++) {
    index_size +=
        nodes[u].N_I_SZ == 0 ? sizeof(nodes[u].h_in) : sizeof(nodes[u].L_in);
    index_size +=
        nodes[u].N_O_SZ == 0 ? sizeof(nodes[u].h_out) : sizeof(nodes[u].L_out);
    index_size += sizeof(nodes[u].L_interval);
  }
  /// 
  index_size += nodes.size() * 3;
  /// 
  printf("index space: %.3fKB\n", double(index_size) / (1024));
}

vector<pair<pair<node, node>, int>> queries;

void read_queries(const char *filename) {
  timeval start_at, end_at;
  gettimeofday(&start_at, 0);
  FILE *file = fopen(filename, "r");
  int u, v, r;
  while (fscanf(file, "%d%d%d", &u, &v, &r) == 3) {
    queries.push_back(make_pair(make_pair(nodes[u], nodes[v]), r));
  }
  fclose(file);
  gettimeofday(&end_at, 0);
  printf("read time(query): %.3fs\n",
         end_at.tv_sec - start_at.tv_sec +
             double(end_at.tv_usec - start_at.tv_usec) / 1000000);
}
int filtered_num = 0;
bool reach(node &u, node &v) {
  if (u.N_I==v.N_I)
     return true;
  else if (
      ///
      u.Level.second >= v.Level.second ||
      u.Level.first <= v.Level.first||
      u.max_out<v.max_out ||
      /// 
      u.L_interval.second < v.L_interval.second) {
      filtered_num++;
    return false;
  } else if (u.L_interval.first <= v.L_interval.first) {
    return true;
  }

  if (v.N_I_SZ == 0) {
    return false;
  }
  if (u.N_O_SZ == 0) {
    return false;
  }
  if (v.N_O_SZ == 0) {
    if ((u.L_out[v.h_out >> 5] & (1 << (v.h_out & 31))) == 0) {
      return false;
    }
  } else {
    for (int i = 0; i < K; i++) {
      if ((u.L_out[i] & v.L_out[i]) != v.L_out[i]) {
        return false;
      }
    }
  }
  if (u.N_I_SZ == 0) {
    if ((v.L_in[u.h_in >> 5] & (1 << (u.h_in & 31))) == 0) {
      return false;
    }
  } else {
    for (int i = 0; i < K; i++) {
      if ((u.L_in[i] & v.L_in[i]) != u.L_in[i]) {
        return false;
      }
    }
  }

  for (int i = 0; i < u.N_O_SZ; i++) {
    if (nodes[u.N_O[i]].vis != vis_cur) {
      nodes[u.N_O[i]].vis = vis_cur;
      if (reach(nodes[u.N_O[i]], v)) {
        return true;
      }
    }
  }

  return false;
}

void run_queries() {
  timeval start_at, end_at;
  gettimeofday(&start_at, 0);
  for (vector<pair<pair<node, node>, int>>::iterator it = queries.begin();
       it != queries.end(); it++) {
    vis_cur++;
    int result = reach(it->first.first, it->first.second);
    if (it->second == -1 || it->second == result) {
      it->second = result;
    } else {
      fprintf(stderr, "ERROR!\n");
      exit(EXIT_FAILURE);
    }
  }
  gettimeofday(&end_at, 0);
  printf("query time: %.3fms\n",
         (end_at.tv_sec - start_at.tv_sec) * 1000 +
             double(end_at.tv_usec - start_at.tv_usec) / 1000);
}

void write_results() {
  int ncut = 0, pcut = 0, pos = 0;
  for (int i = 0; i < queries.size(); i++) {
    node &u = queries[i].first.first, &v = queries[i].first.second;
    bool pf = false, nf = false;

    if (u.L_interval.second < v.L_interval.second) {
      nf = true;
    } else if (u.L_interval.first <= v.L_interval.first) {
      pf = true;
    }

    if (v.N_I_SZ == 0) {
      nf = true;
    }
    if (u.N_O_SZ == 0) {
      nf = true;
    }
    if (v.N_O_SZ == 0) {
      if ((u.L_out[v.h_out >> 5] & (1 << (v.h_out & 31))) == 0) {
        nf = true;
      }
    } else {
      for (int i = 0; i < K; i++) {
        if ((u.L_out[i] & v.L_out[i]) != v.L_out[i]) {
          nf = true;
        }
      }
    }
    if (u.N_I_SZ == 0) {
      if ((v.L_in[u.h_in >> 5] & (1 << (u.h_in & 31))) == 0) {
        nf = true;
      }
    } else {
      for (int i = 0; i < K; i++) {
        if ((u.L_in[i] & v.L_in[i]) != u.L_in[i]) {
          nf = true;
        }
      }
    }

    if (queries[i].second) {
      pos++;
    }
    if (nf) {
      ncut++;
    }
    if (pf) {
      pcut++;
    }
  }
  printf("reachable: %d\n", pos);
  /// 
  cout << "filtered_num:" << filtered_num << endl;
  /// 
  printf("answered only by label: %d + %d = %d\n", ncut, pcut, ncut + pcut);
}
}

int main() {
  using namespace bs;
  string dataname = "agrocyc";
  string filename = "../../../data/" + dataname + "_dag_uniq.gra";
  //string filename = "../../../example_DAG.txt";
  string queryname = "../../../query/" + dataname + "/query.txt";
  read_graph(filename.c_str());
  index_construction();
  read_queries(queryname.c_str());
  run_queries();

  write_results();

  return 0;
}

