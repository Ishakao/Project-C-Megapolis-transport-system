#include <iostream>
#include <vector>
#include <functional>
using namespace std;

float fines[3][3]{};
float coefficients[3]{};

enum transportType {
	Underground = 0,
	Autobus,
	Railway,
	Start
};

using Vertex = int;
#define BILL 1e9
vector<float> localTransferTime;

class Edge {
public:
	Vertex to{};
	int timeLength{};
	float lineLoad{};
	transportType type{};

	inline float getTime(transportType lastTransport, int vertexIndex) {
		float time = timeLength * (1 + lineLoad * coefficients[type]);

		if (lastTransport != Start and lastTransport != type) {
			time += fines[lastTransport][type];
			time += localTransferTime[vertexIndex];
		}

		return time;
	}

	Edge() = delete;
	Edge(int e, int l, float load, transportType t) : to(e), timeLength(l), lineLoad(load), type(t) {  }
};

struct Result {
	int target;
	bool reachable;
	float time;
	int transfers;
	float metric;
	vector<pair<int, transportType>> way;
};

struct Node {
	float distantion;
	int index;
	transportType transport;
};

class prioritiedQueue {
	vector<Node> heap;

	inline int parent(int index) { return (index - 1) / 2; }
	inline int left(int index) { return index * 2 + 1; }
	inline int right(int index) { return index * 2 + 2; }

	void upward(int z) {
		while (z > 0 and heap[parent(z)].distantion > heap[z].distantion) {
			swap(heap[z], heap[parent(z)]);
			z = parent(z);
		}
	}

	void downward(int z) {
		int n = heap.size();

		while (true) {
			int l = left(z), r = right(z);
			int smallest = z;

			if (l < n and heap[l].distantion < heap[smallest].distantion) smallest = l;
			if (r < n and heap[r].distantion < heap[smallest].distantion) smallest = r;

			if (smallest == z) break;
			swap(heap[z], heap[smallest]);
			z = smallest;
		}
	}

public:
	void push(Node n) {
		heap.push_back(n);
		upward(heap.size() - 1);
	}

	Node pop() {
		Node a = heap[0];
		heap[0] = heap[heap.size() - 1];
		heap.pop_back();
		if (!empty()) downward(0);
		return a;
	}

	inline bool empty() { return heap.empty(); }
};

template<typename T, typename F>
T* partition(T* Begin, T* End, F f) {
	T* j = Begin - 1;
	for (T* i = Begin; i < End; i++) {
		bool result = f(*i, *(End - 1));
		if (result) {
			j++;
			swap(*i, *j);
		}
	}
	j++;
	swap(*j, *(End - 1));

	return j;
}

template <typename T, typename F>
void quickSort(T* Begin, T* End, F f) {
	if (End - Begin <= 1) return;
	T* pivot = partition(Begin, End, f);
	quickSort(Begin, pivot, f);
	quickSort(pivot + 1, End, f);
}
template <typename T, typename F>
void quickSort(vector<T>& arr, F f) { // quicksort который работает с векторами любых типов и с кастомным условием
	quickSort(arr.data(), arr.data()+arr.size(), f);
}

void DFS(int z, vector<vector<Vertex>>& adjecency, vector<bool>& visited, vector<Vertex>& comp) {
	visited[z] = true;
	comp.push_back(z);
	for (Vertex i : adjecency[z]) {
		if (!visited[i]) DFS(i, adjecency, visited, comp);
	}
}

vector<vector<Vertex>> findComponents(int n, vector<vector<Vertex>>& adjecency) {
	vector<bool> visited(n, false);
	vector<vector<Vertex>> components;
	for (int i = 0; i < n; i++) {
		if (visited[i]) continue;
		vector<Vertex> comp;
		DFS(i, adjecency, visited, comp);
		quickSort(comp, [](Vertex a, Vertex b) { return a < b; });
		components.push_back(comp);
	}
	return components;
}

void createWay(vector<vector<Edge>>& vertexEdges, vector<vector<Vertex>>& components) {
	int n = vertexEdges.size();

	cout << "Enter start vertex: ";
	Vertex st; cin >> st;

	cout << "Enter end vertices (-1 to end): ";
	vector<Vertex> targets;
	while (1) { Vertex v; cin >> v; if (v < 0 or v > n) break; targets.push_back(v); }

	float convenienceCoefficient;
	cout << "Enter convenience coefficient: ";
	cin >> convenienceCoefficient;

	vector<Result> results;

	/*********************
	*      Dijkstra      *
	*********************/

	vector<vector<pair<float, int>>> distantions(n, vector<pair<float, int>>(3, { BILL, BILL }));
	prioritiedQueue heap;

	heap.push({ 0.0f, st - 1, Start });

	vector<vector<Vertex>> verticesWay(n, vector<Vertex>(3));
	vector<vector<transportType>> transportWay(n, vector<transportType>(3));

	while (!heap.empty()) {
		Node current = heap.pop();

		if (current.distantion > ((current.transport != Start) ? distantions[current.index][current.transport].first : 0)) continue;

		for (Edge e : vertexEdges[current.index]) {
			float newDistantion = current.distantion + e.getTime(current.transport, ((current.distantion != 0) ? current.index : -1));

			int curTransfers = 0;
			if (current.transport != Start)
				curTransfers = distantions[current.index][current.transport].second;

			int add = (current.transport != Start && current.transport != e.type) ? 1 : 0;
			int newTransfers = curTransfers + add;

			auto& best = distantions[e.to][e.type];

			if (newDistantion < best.first or (newDistantion == best.first and newTransfers < best.second)) {
				best = { newDistantion, newTransfers };
				verticesWay[e.to][e.type] = current.index;
				transportWay[e.to][e.type] = current.transport;
				heap.push({ newDistantion, e.to, e.type });
			}
		}
	}

	for (int tar = 0; tar < targets.size(); tar++) {
		Vertex start = st;
		Vertex end = targets[tar]-1;

		vector<pair<int, transportType>> way;
		int counterTransfers = 0;

		pair<float, int> ans = { BILL, BILL };
		transportType bestType{};
		for (int i = 0; i < 3; i++) {
			if (distantions[end][i].first < ans.first or (distantions[end][i].first == ans.first and distantions[end][i].second < ans.second)) {
				ans = distantions[end][i];
				bestType = (transportType)i;
			}
		}

		if (!(ans.first >= BILL and ans.second >= BILL)) {
			int v = end;
			transportType t = bestType;

			while (v != start - 1) {
				way.push_back({ v, t });

				Vertex prevV = verticesWay[v][t]; if (prevV < 0) break;
				transportType prevT = transportWay[v][t];

				v = prevV;
				t = prevT;
			}

			way.push_back({ start - 1, t });
			reverse(way.begin(), way.end());

			for (int i = 1; i < way.size(); i++) {
				if (way[i].second != way[i - 1].second and way[i - 1].second != 3) {
					counterTransfers++;
				}
			}
		}

		results.push_back({ end+1, (!(ans.first >= BILL and ans.second >= BILL)), ans.first, counterTransfers ,ans.first + convenienceCoefficient * counterTransfers, way });
	}

	quickSort(results, [](const Result& a, const Result& b) {
		if (a.reachable != b.reachable) return a.reachable > b.reachable;
		if (!a.reachable and !b.reachable) return a.target < b.target;
		if (a.metric != b.metric) return a.metric < b.metric;
		if (a.time != b.time) return a.time < b.time;
		if (a.transfers != b.transfers) return a.transfers < b.transfers;
		return a.target < b.target;
	});

	for (Result& r : results) {
		if (!r.reachable) {
			cout << "Target " << r.target << ": Path not exist" << endl;
			continue;
		}

		cout << "Shortest time: " << r.time << ". Transfers quantity: " << r.transfers << ". Convenience metric: " << r.metric << endl;
		cout << "Shorted time way:" << endl;
		for (int i = 0; i < r.way.size(); i++) {
			pair<int, transportType> obj = r.way[i];
			cout << obj.first + 1 << " (" << (obj.second != 3 ? obj.second : -1) << ")" << ((i != r.way.size() - 1) ? " -> " : "\n");
		}
	}
}

int main() {
	cout << "Enter quantity of vertices: ";
	int n; cin >> n;
	localTransferTime.resize(n);
	vector<vector<Edge>> vertexEdges[4] = { vector<vector<Edge>>(n), vector<vector<Edge>>(n), vector<vector<Edge>>(n), vector<vector<Edge>>(n) };

	for (int i = 0; i < n; i++) {
		cout << "Enter local transfer fine for vertex " << i << " (minutes): ";
		float m; cin >> m;
		localTransferTime[i] = m;
	}

	for (int i = 0; i < 3; i++) {
		cout << "Coefficient of type " << i << " (%): ";
		float move; cin >> move;
		coefficients[i] = move / 100.0f;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (i == j) { fines[i][j] = 0; continue; }
			cout << "Fine for transfer " << i << " -> " << j << " (minutes): ";
			float move; cin >> move;
			fines[i][j] = move;
		}
	}
	cout << endl;
	cout << "Enter all of edges: " << endl << "from  to  time  load  transport" << endl;
	while (1) {
		Vertex from, to; 
		int time, transport; 
		float load;
		cin >> from; 
		if (from < 1) break;
		cin >> to >> time >> load >> transport;
		from--; to--;
		vertexEdges[3][from].push_back(Edge{ to, time, load, (transportType)transport });
		vertexEdges[3][to].push_back(Edge{ from, time, load, (transportType)transport });

		vertexEdges[transport][from].push_back(Edge{ to, time, load, (transportType)transport });
		vertexEdges[transport][to].push_back(Edge{ from, time, load, (transportType)transport });
	}

	vector<vector<Vertex>> adjecencyVertices[4] = { vector<vector<Vertex>>(n), vector<vector<Vertex>>(n), vector<vector<Vertex>>(n), vector<vector<Vertex>>(n) };
	for (int type = 0; type < 4; type++) {
		for (int i = 0; i < n; i++) {
			for (Edge e : vertexEdges[type][i]) {
				adjecencyVertices[type][i].push_back(e.to);
			}
		}
	}

	vector<vector<Vertex>> componentsByType[4] = {findComponents(n, adjecencyVertices[0]), findComponents(n, adjecencyVertices[1]), findComponents(n, adjecencyVertices[2]), findComponents(n, adjecencyVertices[3]) };
	
	for (int comp = 0; comp < 4; comp++) {
		quickSort(componentsByType[comp], [](vector<Vertex>& a, vector<Vertex>& b) { return a.size() > b.size();  });
		int index = -1;
		int quantity = 0;

		for (int i = 0; i < componentsByType[comp].size(); i++) {
			if (componentsByType[comp][i].size() > quantity) {
				index = i;
				quantity = componentsByType[comp][index].size();
			}
		}

		cout << "Biggest component by " << comp << " type of transport: ";
		if (index != -1) {
			for (int z = 0; z < componentsByType[comp][index].size(); z++) {
				cout << componentsByType[comp][index][z]+1 << " ";
			} cout << "(" << quantity << ")" << endl;
		}
		else {
			cout << "not exist" << endl;
		}

		if (componentsByType[comp].size() > 1) {
			cout << "Isolated components by " << comp << " type of transport: " << endl;
			for (int z = 0; z < componentsByType[comp].size(); z++) {
				if (z == index) continue;
				cout << "| ";
				for (int i = 0; i < componentsByType[comp][z].size(); i++) {
					cout << componentsByType[comp][z][i] + 1 << " ";
				} cout << " |" << endl;
			}
		} else {
			cout << "No isolated components" << endl;
		}
	}

	createWay(vertexEdges[3], componentsByType[3]);
}
