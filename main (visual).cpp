#include <iostream>
#include "simpleUI.h"
#include <vector>
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
	Edge(int e, int l, float load, transportType t) : to(e), timeLength(l), lineLoad(load), type(t) {}
};

vector<vector<Edge>> vertexEdges[4];

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
void quickSort(vector<T>& arr, F f) {
	quickSort(arr.data(), arr.data() + arr.size(), f);
}

void DFS(Vertex z, vector<vector<Vertex>>& adjecency, vector<bool>& visited, vector<Vertex>& comp) {
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

int quantityOfVertices = 0;
bool dataExist = false;
vector<vector<Vertex>> adjecencyVertices[4];
vector<vector<Vertex>> componentsByType[4];

void initialInterface(Instance& StartInstance) {
	ImageLabel* background = new ImageLabel(&StartInstance);
	background->Size = { 1,1 };
	background->Position = { 0,0 };
	background->BackgroundColor = {85,80,80,255};
	background->Name = "Background";
	background->setImage("Textures/Background.png");
	background->ImageColor = { 85,80,80,200 };
	background->Overlay = STRETCH;

	Object2D* main = new Object2D(&StartInstance);
	main->Name = "Main";
	main->Size = { 1,1 };
	main->Position = { 0,0 };	
	main->BackgroundTransparency = 1;

	TextLabel* name = new TextLabel(main);
	name->Name = "Name";
	name->BackgroundTransparency = 1;
	name->font = "open";
	name->Size = { 0.65, 0.075 };
	name->Position = { 0.5, 0.01 };
	name->AnchorPosition = { 0.5, 0 };
	name->TextColor = { 255,178,103,255 };
	name->TextAnchor = TextAnchorEnum::N;
	name->Text = "Megapolis transport system";
	name->Roundness = 0.8;
	name->Segments = 10;
	name->Active = true;
	name->BackgroundColor = { 255,178,103,255 };
	name->SetMouseEnter([name]() { Animate::Create(&name->BackgroundTransparency, 0.15f, 0.9f); Animate::Create(&name->TextColor, 0.15f, { 255,255,111,255 }); });
	name->SetMouseLeave([name]() { Animate::Create(&name->BackgroundTransparency, 0.15f, 1.0f); Animate::Create(&name->TextColor, 0.15f, { 255,178,103,255 }); });

	TextLabel* create = new TextLabel(main);
	create->Name = "CreateNewWay";
	create->font = "open";
	create->Size = { 0.2, 0.08 };
	create->Position = { 0.05, 0.2 };
	create->TextColor = { 255,178,103,255 };
	create->TextAnchor = TextAnchorEnum::W;
	create->Text = " Create new     ";
	create->Active = true;
	create->BackgroundColor = { 65,60,60,255 };
	create->BorderColor = {35,30,30,255};
	create->BorderThickness = 4;
	create->Roundness = 0.15;
	create->SetMouseEnter([create]() { Animate::Create(&create->TextColor, 0.15f, { 255,230,111,255 }); Animate::Create(&create->Size.x, 0.15f, 0.23); });
	create->SetMouseLeave([create]() { Animate::Create(&create->TextColor, 0.15f, { 255,178,103,255 }); Animate::Create(&create->Size.x, 0.15f, 0.2); });

	TextLabel* history = new TextLabel(main);
	history->Name = "WaysHistory";
	history->font = "open";
	history->Size = { 0.12, 0.08 };
	history->Position = { 0.05, 0.3 };
	history->TextColor = { 255,178,103,255 };
	history->TextAnchor = TextAnchorEnum::W;
	history->Text = " History    ";
	history->Active = true;
	history->BackgroundColor = { 65,60,60,255 };
	history->BorderColor = { 35,30,30,255 };
	history->BorderThickness = 4;
	history->Roundness = 0.15;
	history->SetMouseEnter([history]() { Animate::Create(&history->TextColor, 0.15f, { 255,230,111,255 }); Animate::Create(&history->Size.x, 0.15f, 0.16); });
	history->SetMouseLeave([history]() { Animate::Create(&history->TextColor, 0.15f, { 255,178,103,255 }); Animate::Create(&history->Size.x, 0.15f, 0.12); });
	
	BoolValue* HistoryOpened = new BoolValue(history);
	HistoryOpened->Name = "HistoryStatus";
	HistoryOpened->Value = false;
	history->SetMouseClick1([HistoryOpened]() { HistoryOpened->Value = !HistoryOpened->Value; });

	ScrollFrame* historyScroll = new ScrollFrame(main);
	historyScroll->Position = {0.05, 0.38};
	historyScroll->Size = { 0.12,0 };
	historyScroll->BackgroundColor = { 65,60,60,255 };
	historyScroll->SliderColor = { 255,178,103,255 };
	historyScroll->SliderSize = 3;
	historyScroll->Visible = false;
	historyScroll->ZIndex = -1;

	new ChangedSignal(HistoryOpened->Value, [historyScroll, HistoryOpened]() {
		if (HistoryOpened->Value) {
			historyScroll->Visible = true;
			Animate::Create(&historyScroll->Size.y, 0.15f, 0.3, Animate::Quad);
		} else {
			Animate::Animation* s = Animate::Create(&historyScroll->Size.y, 0.15f, 0, Animate::Linear);
			s->Completed = [historyScroll]() { historyScroll->Visible = false; };
		}
	});

	Object2D* createBG = new Object2D(main);
	createBG->Position = { 0.67, 0.5 };
	createBG->AnchorPosition = { 0.5,0.5 };
	createBG->Size = { 0, 0 };
	createBG->BackgroundColor = { 95,90,90,255 };
	createBG->Name = "createBG1";
	createBG->Visible = false;
	for (int i = 0; i < 4; i++) {
		Object2D* corner = new Object2D(createBG);
		corner->BackgroundTransparency = 1;
		corner->Size = { 0.1, 0.1 };
		corner->Position = { 0.9f*(i == 1 or i == 3), 0.9f*(i==2 or i == 3)};
		corner->BorderThickness = 4;
		corner->BorderColor = { 255,178,103,255 };
	}
	Object2D* createBG2 = new Object2D(createBG);
	createBG2->Size = { 1,1 };
	createBG2->Position = { 0,0 };
	createBG2->BackgroundColor = { 95,90,90,255 };
	createBG2->Name = "createBG2";

	Object2D* createFrame = new Object2D(createBG2);
	createFrame->Visible = false;
	createFrame->Size = { 1,1 };
	createFrame->BackgroundTransparency = 1;
	createFrame->Name = "wayCreater";

	ScrollFrame* fillFrame = new ScrollFrame(createBG2);
	fillFrame->Visible = false;
	fillFrame->Size = { 1,1 };
	fillFrame->BackgroundTransparency = 1;
	fillFrame->Name = "dataFill";
	fillFrame->ScrollEnabled = false;
	fillFrame->SliderTransparency = 1;
	fillFrame->CanvasSize.y = 4;

	Object2D* firstFrame = new Object2D(fillFrame);
	firstFrame->Name = "firstFrame";
	firstFrame->Size = { 1,1 };
	firstFrame->Position = { 0,0 };
	firstFrame->BackgroundTransparency = 1;
	Object2D* secondFrame = Clone(firstFrame);
	secondFrame->Name = "secondFrame";
	secondFrame->Position.y = 1;
	Object2D* thirdFrame = Clone(firstFrame);
	thirdFrame->Name = "thirdFrame";
	thirdFrame->Position.y = 2;
	Object2D* fourthFrame = Clone(firstFrame);
	fourthFrame->Name = "fourthFrame";
	fourthFrame->Position.y = 3;

	TextLabel* fillName = new TextLabel(firstFrame);
	fillName->Name = "name";
	fillName->BackgroundTransparency = 1;
	fillName->TextColor = { 255,178,103,255 };
	fillName->font = "open";
	fillName->Text = "Fill data";
	fillName->Size = { 0.4, 0.075 };
	fillName->Position = { 0.5, 0 }; fillName->AnchorPosition = { 0.5, 0 };
	TextLabel* quantityOfVerticesLabel = new TextLabel(fillFrame);
	quantityOfVerticesLabel->BackgroundTransparency = 1;
	quantityOfVerticesLabel->Name = "quantityOfVerticesLabel";
	quantityOfVerticesLabel->Text = "Quantity of vertices:";
	quantityOfVerticesLabel->Size = { 0.7, 0.2 };
	quantityOfVerticesLabel->Position = { 0.03, 0.1 };
	quantityOfVerticesLabel->TextAnchor = TextAnchorEnum::W;
	quantityOfVerticesLabel->TextColor = { 255,178,103,255 };
	quantityOfVerticesLabel->font = "open";
	TextBox* quantityOfVerticesBox = new TextBox(fillFrame);
	quantityOfVerticesBox->Size = { 0.2, 0.1 };
	quantityOfVerticesBox->Position = { 0.75, 0.15 };
	quantityOfVerticesBox->BackgroundColor = { 75,70,70,255 };
	quantityOfVerticesBox->PlaceholderText = " ... ";
	quantityOfVerticesBox->TextSize = -1;
	quantityOfVerticesBox->PlaceholderTextColor = { 30,30,30,180 };
	quantityOfVerticesBox->TextColor = { 255,178,103,255 };
	quantityOfVerticesBox->Roundness = 0.3;
	quantityOfVerticesBox->font = "open";
	quantityOfVerticesBox->maxSymbols = 4;
	quantityOfVerticesBox->AllowedSymbols = "0123456789";
	quantityOfVerticesBox->CursorColor = { 255,178,103,255 };
	TextLabel* MustBeNumber = new TextLabel(firstFrame);
	MustBeNumber->BackgroundTransparency = 1;
	MustBeNumber->TextColor = { 255,0,0,255 };
	MustBeNumber->TextTransparency = 1;
	MustBeNumber->Position = { 0.5, 0.25 };
	MustBeNumber->Size = { 0.6, 0.1 };
	MustBeNumber->AnchorPosition = { 0.5,0 };
	MustBeNumber->Text = "Quantity must be a number";
	MustBeNumber->font = "open";
	TextLabel* EnterVertices = new TextLabel(firstFrame);
	EnterVertices->Name = "enter";
	EnterVertices->BackgroundColor = { 90,85,85,255 };
	EnterVertices->AnchorPosition = { 0.5,0.5 };
	EnterVertices->Position = { 0.5, 0.45 };
	EnterVertices->Text = " Enter ";
	EnterVertices->TextColor = { 255,178,103,255 };
	EnterVertices->BorderColor = { 15,15,15,255 };
	EnterVertices->BorderTransparency = 0.75;
	EnterVertices->BorderThickness = 4;
	EnterVertices->Roundness = 0.125;
	EnterVertices->Size = { 0.2, 0.1 };
	EnterVertices->font = "open";
	EnterVertices->Active = true;
	EnterVertices->SetMouseEnter([EnterVertices]() {Animate::Create(&EnterVertices->Size, 0.1, { 0.22, 0.11 }); Animate::Create(&EnterVertices->TextColor, 0.1, { 255,230,111,255 }); Animate::Create(&EnterVertices->BorderColor, 0.1, { 45,40,40,255 }); });
	EnterVertices->SetMouseLeave([EnterVertices]() {Animate::Create(&EnterVertices->Size, 0.1, { 0.2, 0.1 }); Animate::Create(&EnterVertices->TextColor, 0.1, { 255,178,103,255 }); Animate::Create(&EnterVertices->BorderColor, 0.1, { 15,15,15,255 }); });
	
	TextLabel* fillName2 = new TextLabel(secondFrame);
	fillName2->Name = "name2";
	fillName2->BackgroundTransparency = 1;
	fillName2->TextColor = { 255,178,103,255 };
	fillName2->font = "open";
	fillName2->Text = "Enter edges";
	fillName2->Size = { 0.4, 0.1 };
	fillName2->Position = { 0.5, 0 }; fillName2->AnchorPosition = { 0.5, 0 };

	ScrollFrame* edges = new ScrollFrame(secondFrame);
	edges->Name = "scrollEdges";
	edges->AnchorPosition = { 0.5,0 };
	edges->Size = { 0.6, 0.45 };
	edges->Position = {0.5, 0.1};
	edges->BackgroundColor = { 80,75,75,255 };
	edges->Roundness = 0.05;
	edges->SliderColor = { 255,178,103,255 };
	edges->SliderTransparency = 0.7;
	edges->ScrollSpeed = 0.2;

	Object2D* input = new Object2D(secondFrame);
	input->AnchorPosition = { 0.5,0 };
	input->Position = { 0.4, 0.7 };
	input->Size = { 0.6, 0.1 };
	input->BackgroundColor = { 80,75,75,255 };
	input->Roundness = 0.05;

	std::string texts[5] = {" from ", "  to  ", " time ", " type ", " load "};
	int maxSymbols[5] = {4,4,4,1,4};

	for (int i = 0; i < 5; i++) {
		TextBox* t = new TextBox(input);
		t->BackgroundTransparency = 1;
		t->AnchorPosition = { 0.5, 0 };
		t->Size = { 0.17, 1 };
		t->Position = { 0.1f + i * 0.2f, 0};
		t->PlaceholderTextColor = { 50,50,50,255 };
		t->font = "open";
		t->TextSize = -1;
		t->PlaceholderText = texts[i];
		if (i == 4) {
			t->AllowedSymbols = "0123456789.";
		} else {
			t->AllowedSymbols = "0123456789";
		}
		t->maxSymbols = maxSymbols[i];
		t->TextColor = { 255,178,103,255 };
	}

	Folder* objects = new Folder(edges);
	objects->Name = "edges";

	Object2D* edgeClone = new Object2D(edges);
	edgeClone->Name = "edgeClone";
	edgeClone->Position = { 0.025,0 };
	edgeClone->Size = { 0.95, 0.1 };
	edgeClone->BackgroundTransparency = 0.98;
	edgeClone->Visible = false;
	for (int i = 0; i < 5; i++) {
		TextLabel* t = new TextLabel(edgeClone);
		t->BackgroundTransparency = 1;
		t->AnchorPosition = { 0.5, 0 };
		t->Size = { 0.17, 1 };
		t->Position = { 0.1f + i * 0.2f, 0 };
		t->font = "open";
		t->TextSize = -1;
		t->Text = "test";
		t->TextColor = { 255,178,103,255 };
	}

	TextLabel* AddEdge = new TextLabel(secondFrame);
	AddEdge->Name = "enter2";
	AddEdge->BackgroundColor = { 90,85,85,255 };
	AddEdge->AnchorPosition = { 0.5,0.5 };
	AddEdge->Position = { 0.8, 0.75 };
	AddEdge->Text = "  Add  ";
	AddEdge->TextColor = { 255,178,103,255 };
	AddEdge->BorderColor = { 15,15,15,255 };
	AddEdge->BorderTransparency = 0.75;
	AddEdge->BorderThickness = 3;
	AddEdge->Roundness = 0.125;
	AddEdge->Size = { 0.15, 0.1 };
	AddEdge->font = "open";
	AddEdge->Active = true;
	AddEdge->SetMouseEnter([AddEdge]() {Animate::Create(&AddEdge->Size, 0.1, { 0.17, 0.11 }); Animate::Create(&AddEdge->TextColor, 0.1, { 255,230,111,255 }); Animate::Create(&AddEdge->BorderColor, 0.1, { 45,40,40,255 }); });
	AddEdge->SetMouseLeave([AddEdge]() {Animate::Create(&AddEdge->Size, 0.1, { 0.15, 0.1 }); Animate::Create(&AddEdge->TextColor, 0.1, { 255,178,103,255 }); Animate::Create(&AddEdge->BorderColor, 0.1, { 15,15,15,255 }); });
	AddEdge->SetMouse1HoldEnd([edges, edgeClone, objects, input]() {
		for (int i = 0; i < 5; i++) {
			if (dynamic_cast<TextBox*>(input->Children[i])->Text == "") return;
			float digit = TextToFloat(dynamic_cast<TextBox*>(input->Children[i])->Text.c_str());
			if ((i == 0 or i == 1) and (digit < 1 or digit > quantityOfVertices)) return;
			if (i == 3 and (digit < 1 or digit > 3)) return;
		}
		Object2D* clone = Clone(edgeClone);
		clone->setParent(objects);
		clone->Visible = true;
		for (int i = 0; i < 5; i++) {
			dynamic_cast<TextLabel*>(clone->Children[i])->Text = dynamic_cast<TextBox*>(input->Children[i])->Text;
			dynamic_cast<TextBox*>(input->Children[i])->Text = "";
		}
		clone->Position = {0.025, 0.1f*(objects->Children.size() - 1)};
		edges->CanvasSize.y = (objects->Children.size() > 10) ? (float)objects->Children.size() / 10 : 1;
		edges->CanvasPosition.y = edges->CanvasSize.y - 1;

		Vertex from = TextToInteger(dynamic_cast<TextLabel*>((objects->Children.back())->Children[0])->Text.c_str())-1;
		Vertex to = TextToInteger(dynamic_cast<TextLabel*>((objects->Children.back())->Children[1])->Text.c_str())-1;
		int time = TextToInteger(dynamic_cast<TextLabel*>((objects->Children.back())->Children[2])->Text.c_str());
		float load = TextToFloat(dynamic_cast<TextLabel*>((objects->Children.back())->Children[4])->Text.c_str());
		transportType transport = (transportType)(TextToInteger(dynamic_cast<TextLabel*>((objects->Children.back())->Children[3])->Text.c_str()) - 1);

		vertexEdges[3][from].push_back(Edge{ to, time, load, transport });
		vertexEdges[3][to].push_back(Edge{ from, time, load, transport });

		vertexEdges[transport][from].push_back(Edge{ to, time, load, transport });
		vertexEdges[transport][to].push_back(Edge{ from, time, load, transport });
	});
	
	TextLabel* mustHaveEdges = Clone(MustBeNumber);
	mustHaveEdges->Position.y = 1.55;
	mustHaveEdges->Name = "mustHaveEdges";
	mustHaveEdges->Text = "Edges quantity must be > 0";

	TextLabel* fillName3 = new TextLabel(thirdFrame);
	fillName3->Name = "name3";
	fillName3->BackgroundTransparency = 1;
	fillName3->TextColor = { 255,178,103,255 };
	fillName3->font = "open";
	fillName3->Text = "Enter other info";
	fillName3->Size = { 0.6, 0.1 };
	fillName3->Position = { 0.5, 0 }; fillName3->AnchorPosition = { 0.5, 0 };

	static int currentVertex = 0;

	TextLabel* verticesTransfer = new TextLabel(thirdFrame);
	verticesTransfer->Name = "verticesTransfer";
	verticesTransfer->Size = { 0.6, 0.1 };
	verticesTransfer->Position = { 0.09,0.15 };
	verticesTransfer->BackgroundColor = { 90,85,85,255 };
	verticesTransfer->TextColor = { 255,178,103,255 };
	verticesTransfer->font = "open";
	verticesTransfer->Text = " Enter transfer time for vertex 1: ";
	verticesTransfer->Roundness = 0.15;
	TextBox* transferBox = new TextBox(thirdFrame);
	transferBox->Name = "transferBox";
	transferBox->Size = { 0.2, 0.1 };
	transferBox->Position = { 0.71,0.15 };
	transferBox->BackgroundColor = { 90,85,85,255 };
	transferBox->TextColor = { 255,178,103,255 };
	transferBox->PlaceholderText = "0";
	transferBox->PlaceholderTextColor = { 30,30,30,180 };
	transferBox->font = "open";
	transferBox->Roundness = 0.15;
	transferBox->TextSize = -1;
	transferBox->AllowedSymbols = "0123456789";
	transferBox->maxSymbols = 4;
	transferBox->SetForTick([transferBox, verticesTransfer]() {
		if (currentVertex >= quantityOfVertices and quantityOfVertices != 0) {
			transferBox->SetForTick([]() {});
			verticesTransfer->Text = "Local transfers filled";
			transferBox->Active = false;
			transferBox->PlaceholderText = " - ";
			transferBox->Text = "";
			verticesTransfer->TextColor = { 170, 125, 90, 255 }; 
			if (transferBox == FocusedTextBox) { FocusedTextBox = nullptr; }
			return;
		}

		if (IsKeyPressed(KEY_ENTER) and FocusedTextBox == transferBox) {
			int digit = 0;
			for (int i = 0; transferBox->Text[i] != '\0'; i++) {
				if (transferBox->Text[i] < '0' or transferBox->Text[i] > '9') break;
				digit *= 10;
				digit += transferBox->Text[i] - '0';
			}

			localTransferTime[currentVertex++] = digit;
			if (currentVertex < quantityOfVertices) {
				std::ostringstream s;
				s << " Enter transfer time for vertex " << currentVertex + 1 << ": ";
				verticesTransfer->Text = s.str();
				transferBox->Text.clear();
			}
		}
	});

	static int currentTransport = 0;

	TextLabel* typeCoefficient = new TextLabel(thirdFrame);
	typeCoefficient->Name = "typeCoefficient";
	typeCoefficient->Size = { 0.6, 0.1 };
	typeCoefficient->Position = { 0.09,0.27 };
	typeCoefficient->BackgroundColor = { 90,85,85,255 };
	typeCoefficient->TextColor = { 255,178,103,255 };
	typeCoefficient->font = "open";
	typeCoefficient->Text = " Enter coefficient for transport 1: ";
	typeCoefficient->Roundness = 0.15;
	TextBox* coefficientBox = new TextBox(thirdFrame);
	coefficientBox->Name = "coefficientBox";
	coefficientBox->Size = { 0.2, 0.1 };
	coefficientBox->Position = { 0.71,0.27 };
	coefficientBox->BackgroundColor = { 90,85,85,255 };
	coefficientBox->TextColor = { 255,178,103,255 };
	coefficientBox->PlaceholderText = "1.0";
	coefficientBox->PlaceholderTextColor = { 30,30,30,180 };
	coefficientBox->font = "open";
	coefficientBox->Roundness = 0.15;
	coefficientBox->TextSize = -1;
	coefficientBox->AllowedSymbols = "0123456789.";
	coefficientBox->maxSymbols = 5;
	coefficientBox->SetForTick([coefficientBox, typeCoefficient]() {
		if (currentTransport >= 3) {
			coefficientBox->SetForTick([]() {});
			typeCoefficient->Text = "Coefficient filled";
			coefficientBox->Active = false;
			coefficientBox->PlaceholderText = " - ";
			typeCoefficient->TextColor = { 170, 125, 90, 255 };
			coefficientBox->Text = "";
			if (coefficientBox == FocusedTextBox) { FocusedTextBox = nullptr; }
			return;
		}

		if (IsKeyPressed(KEY_ENTER) and FocusedTextBox == coefficientBox) {
			float co = TextToFloat(coefficientBox->Text.c_str());
			if (co == 0) co = 1;
			coefficients[currentTransport++] = co;
			if (currentTransport < 3) {
				std::ostringstream s;
				s << " Enter coefficient for transport " << currentTransport + 1 << ": ";
				typeCoefficient->Text = s.str();
				coefficientBox->Text.clear();
			}
		}
	});

	static int currentFine = 0;
	static Vector2 fns[6] = {
		{1,2}, {1,3}, {2,1}, {2,3}, {3,1}, {3,2}
	};

	TextLabel* finesType = new TextLabel(thirdFrame);
	finesType->Name = "finesType";
	finesType->Size = { 0.6, 0.1 };
	finesType->Position = { 0.09,0.39 };
	finesType->BackgroundColor = { 90,85,85,255 };
	finesType->TextColor = { 255,178,103,255 };
	finesType->font = "open";
	finesType->Text = " Enter fine from 1 to 2: ";
	finesType->Roundness = 0.15;
	TextBox* finesBox = new TextBox(thirdFrame);
	finesBox->Name = "finesBox";
	finesBox->Size = { 0.2, 0.1 };
	finesBox->Position = { 0.71,0.39 };
	finesBox->BackgroundColor = { 90,85,85,255 };
	finesBox->TextColor = { 255,178,103,255 };
	finesBox->PlaceholderText = "0";
	finesBox->PlaceholderTextColor = { 30,30,30,180 };
	finesBox->font = "open";
	finesBox->Roundness = 0.15;
	finesBox->TextSize = -1;
	finesBox->AllowedSymbols = "0123456789";
	finesBox->maxSymbols = 5;
	finesBox->SetForTick([finesBox, finesType]() {
		if (currentFine >= 6) {
			finesBox->SetForTick([]() {});
			finesType->Text = "Fines filled";
			finesBox->Active = false;
			finesBox->PlaceholderText = " - ";
			finesBox->Text = "";
			finesType->TextColor = { 170, 125, 90, 255 };
			if (finesBox == FocusedTextBox) { FocusedTextBox = nullptr; }
			return;
		}

		if (IsKeyPressed(KEY_ENTER) and FocusedTextBox == finesBox) {
			int co = TextToInteger(finesBox->Text.c_str());
			fines[(int)fns[currentFine].x-1][(int)fns[currentFine].y-1] = co;
			currentFine++;
			if (currentFine < 6) {
				std::ostringstream s;
				s << " Enter fine from " << (int)fns[currentFine].x << " to " << (int)fns[currentFine].y <<": ";
				finesType->Text = s.str();
				finesBox->Text.clear();
			}
		}
	});

	TextLabel* EnterInfo = new TextLabel(thirdFrame);
	EnterInfo->Name = "enter3";
	EnterInfo->BackgroundColor = { 90,85,85,255 };
	EnterInfo->AnchorPosition = { 0.5,0.5 };
	EnterInfo->Position = { 0.5, 0.92 };
	EnterInfo->Text = " Next ";
	EnterInfo->TextColor = { 255,178,103,255 };
	EnterInfo->BorderColor = { 15,15,15,255 };
	EnterInfo->BorderTransparency = 0.75;
	EnterInfo->BorderThickness = 4;
	EnterInfo->Roundness = 0.125;
	EnterInfo->Size = { 0.2, 0.1 };
	EnterInfo->font = "open";
	EnterInfo->Active = true;
	EnterInfo->SetMouseEnter([EnterInfo]() {Animate::Create(&EnterInfo->Size, 0.3, { 0.4, 0.11 }); Animate::Create(&EnterInfo->TextColor, 0.3, { 255,230,111,255 }); Animate::Create(&EnterInfo->BorderColor, 0.1, { 45,40,40,255 }); });
	EnterInfo->SetMouseLeave([EnterInfo]() {Animate::Create(&EnterInfo->Size, 0.1, { 0.2, 0.1 }); Animate::Create(&EnterInfo->TextColor, 0.1, { 255,178,103,255 }); Animate::Create(&EnterInfo->BorderColor, 0.1, { 15,15,15,255 }); });
	EnterInfo->SetMouse1HoldEnd([EnterInfo, objects, fillFrame, thirdFrame, fourthFrame]() {
		Animate::Animation* sas = Animate::Create(&fillFrame->CanvasPosition.y, 0.7, 3, Animate::Cube, Animate::In);
		EnterInfo->Active = false;
		sas->Completed = [thirdFrame]() { thirdFrame->Visible = true; };

		/*********************
		*      Adjecency     *
		*********************/

		for (int i = 0; i < 4; i++) {
			vector<vector<Vertex>> a; a.resize(quantityOfVertices);
			adjecencyVertices[i] = a;
		}

		for (int type = 0; type < 4; type++) {
			for (int i = 0; i < quantityOfVertices; i++) {
				for (Edge e : vertexEdges[type][i]) {
					adjecencyVertices[type][i].push_back(e.to);
				}
			}
		}

		/***********************
		*      Components      *
		***********************/

		for (int i = 0; i < 4; i++) {
			componentsByType[i] = findComponents(quantityOfVertices, adjecencyVertices[i]);
			quickSort(componentsByType[i], [](vector<Vertex>& a, vector<Vertex>& b) { return a.size() > b.size();  });
		}

		static int currentComp = 0;

		for (int comp = 0; comp < 4; comp++) {
			Object2D* sc = new Object2D(fourthFrame);
			sc->Position = { comp * 1.0f, 0 };
			sc->Size = { 1, 1 };
			std::ostringstream s; s << "Object2D" << comp;
			sc->Name = s.str();
			sc->BackgroundColor = { 90,85,85,255 };

			TextLabel* name = new TextLabel(sc);
			name->Position = { 0.2, 0 };
			name->Size = { 0.6, 0.1 };
			name->font = "open";
			std::ostringstream s2; s2 << "Components of transport " << comp+1;
			name->Text = s2.str();
			name->TextColor = { 255,178,103,255 };
			name->BackgroundTransparency = 1;

			int index = -1;
			int quantity = 0;

			for (int i = 0; i < componentsByType[comp].size(); i++) {
				if (componentsByType[comp][i].size() > quantity) {
					index = i;
					quantity = componentsByType[comp][index].size();
				}
			}

			TextLabel* biggestL = new TextLabel(sc);
			biggestL->Name = "biggestText";
			biggestL->Position = {0.05, 0.15};
			biggestL->Size = {0.425, 0.1};
			biggestL->font = "open";
			biggestL->Text = "Biggest Component: ";
			biggestL->BackgroundTransparency = 0.9;
			biggestL->Roundness = 0.125;
			biggestL->TextColor = { 255,178,103,255 };
			ScrollFrame* biggest = new ScrollFrame(sc);
			biggest->Position = { 0.525, 0.15 };
			biggest->Size = { 0.425, 0.1 };
			biggest->BackgroundTransparency = 0.9;
			biggest->Roundness = 0.125;
			biggest->SliderColor = { 255,178,103,255 };
			biggest->SliderTransparency = 0.7;
			biggest->Direction = 'X';
			biggest->Name = "biggestScroll";
			ostringstream biggestData;
			for (int z = 0; z < componentsByType[comp][index].size(); z++) {
				biggestData << componentsByType[comp][index][z] + 1 << " ";
			}
			biggest->CanvasSize.x = (componentsByType[comp][index].size() > 8) ? ((float)componentsByType[comp][index].size() / 8) : 1;
			biggest->ScrollSpeed = 0.2;
			TextLabel* entireData = new TextLabel(biggest);
			entireData->Position = { 0, 0 };
			entireData->Size = { biggest->CanvasSize.x, 1 };
			entireData->font = "open";
			entireData->Text = (componentsByType[comp][index].size() == 1) ? " not exist " : biggestData.str();
			entireData->BackgroundTransparency = 1;
			entireData->TextAnchor = (componentsByType[comp][index].size() > 1) ? TextAnchorEnum::W : TextAnchorEnum::CENTER;
			entireData->TextColor = { 210,130,80,255 };
			entireData->Name = "biggestEntire";

			TextLabel* isolated = new TextLabel(sc);
			isolated->AnchorPosition = { 0.5,0 };
			isolated->Position = { 0.5, 0.3 };
			isolated->Size = { 0.7, 0.125 };
			isolated->Text = "No isolated components";
			isolated->font = "open";
			isolated->TextColor = { 153,153,255,255 };
			isolated->BackgroundTransparency = 1;
			isolated->Name = "isolated";

			dataExist = true;

			if (componentsByType[comp].size() > 1) {
				ScrollFrame* isolatedScroll = new ScrollFrame(sc);
				isolatedScroll->BackgroundTransparency = 0.9;
				isolatedScroll->BorderColor = { 70,60,60,255 };
				isolatedScroll->BorderThickness = 3;
				isolatedScroll->Direction = 'B';
				isolatedScroll->AnchorPosition = { 0.5,0 };
				isolatedScroll->Size = { 0.8, 0.5 };
				isolatedScroll->Position = { 0.5, 0.45 };
				isolatedScroll->SliderColor = { 255,178,103,255 };
				isolatedScroll->SliderTransparency = 0.5;
				isolatedScroll->ScrollSpeed = 0.2;
				isolatedScroll->Name = "isolatedScroll";
				isolatedScroll->CanvasSize.y = ((componentsByType[comp].size() - ((index != -1) ? 1 : 0)) > 5) ? ((float)(componentsByType[comp].size() - ((index != -1) ? 1 : 0)) / 5) : 1;
				isolated->Text = "Isolated components";
				isolated->TextColor = { 255,178,103,255 };
				for (int z = 0; z < componentsByType[comp].size(); z++) {
					if (z == index) continue;
					ostringstream sas;
					for (int i = 0; i < componentsByType[comp][z].size(); i++) {
						sas << componentsByType[comp][z][i] + 1 << " ";
					}
					TextLabel* c = new TextLabel(isolatedScroll);
					c->Size = { (float)componentsByType[comp][z].size() / 8, 0.2 };
					c->BackgroundTransparency = 1;
					c->Text = sas.str();
					c->font = "open";
					c->TextAnchor = TextAnchorEnum::W;
					c->Position = { 0, (isolatedScroll->Children.size() - 1) * 0.2f };
					c->TextColor = { 210,130,80,255 };
				}
			}
		}

		TextLabel* left = new TextLabel(fourthFrame);
		left->ZIndex = 1488;
		left->Active = true;
		left->BackgroundColor = { 85,80,80,255 };
		left->Text = "<";
		left->TextColor = { 100,95,95,255 };
		left->font = "rog";
		left->Size = { 0.07, 0.1 };
		left->Position = {0.01, 1.02};
		left->Roundness = 0.3;
		left->BorderColor = { 100,95,95,255 };
		left->BorderThickness = 3;
		left->SetMouseEnter([left]() {Animate::Create(&left->TextColor, 0.125f, { 115,110,110,255 }); Animate::Create(&left->BorderColor, 0.125f, { 115,110,110,255 }); Animate::Create(&left->Roundness, 0.125f, 0.5); });
		left->SetMouseLeave([left]() {Animate::Create(&left->TextColor, 0.125f, { 100,95,95,255 }); Animate::Create(&left->BorderColor, 0.125f, { 100,95,95,255 }); Animate::Create(&left->Roundness, 0.125f, 0.3); });
		left->SetMouse1HoldEnd([]() {currentComp = (currentComp != 0) ? currentComp - 1 : currentComp; });

		TextLabel* right = new TextLabel(fourthFrame);
		right->ZIndex = 1488;
		right->Active = true;
		right->BackgroundColor = { 85,80,80,255 };
		right->Text = ">";
		right->font = "rog";
		right->Size = { 0.07, 0.1 };
		right->Position = { 0.92, 0.88 };
		right->TextColor = { 100,95,95,255 };
		right->Roundness = 0.3;
		right->BorderColor = { 100,95,95,255 };
		right->BorderThickness = 3;
		right->SetMouseEnter([right]() {Animate::Create(&right->TextColor, 0.125f, { 115,110,110,255 }); Animate::Create(&right->BorderColor, 0.125f, { 115,110,110,255 }); Animate::Create(&right->Roundness, 0.125f, 0.5);  });
		right->SetMouseLeave([right]() {Animate::Create(&right->TextColor, 0.125f, { 100,95,95,255 }); Animate::Create(&right->BorderColor, 0.125f, { 100,95,95,255 }); Animate::Create(&right->Roundness, 0.125f, 0.3); });
		right->SetMouse1HoldEnd([]() {currentComp = (currentComp != 3) ? currentComp + 1 : currentComp; });

		new ChangedSignal(currentComp, [left, right, fourthFrame]() {
			if (currentComp == 0) {
				Animate::Create(&left->Position.y, 0.25, 1.03);
			} else {
				Animate::Create(&left->Position.y, 0.25, 0.88);
			}
			if (currentComp == 3) {
				Animate::Create(&right->Position.y, 0.25, 1.03);
			} else {
				Animate::Create(&right->Position.y, 0.25, 0.88);
			}

			static int lastComp = 0;
			ostringstream nml; nml << "Object2D" << lastComp;
			ostringstream nmc; nmc << "Object2D" << currentComp;
			Object2D* last = dynamic_cast<Object2D*>(fourthFrame->findChild(nml.str()));
			Object2D* current = dynamic_cast<Object2D*>(fourthFrame->findChild(nmc.str()));

			if (lastComp < currentComp) {
				last->Position = { 0,0 };
				current->Position = { 1,0 };
				Animate::Create(&last->Position.x, 0.5, -1, Animate::Quad, Animate::Out);
				Animate::Create(&current->Position.x, 0.5, 0, Animate::Quad, Animate::Out);
			} else {
				last->Position = { 0,0 };
				current->Position = { -1,0 };
				Animate::Create(&last->Position.x, 0.5, 1, Animate::Quad, Animate::Out);
				Animate::Create(&current->Position.x, 0.5, 0, Animate::Quad, Animate::Out);
			}

			lastComp = currentComp;
		});
	});

	TextLabel* EnterEdge = new TextLabel(secondFrame);
	EnterEdge->Name = "enter2";
	EnterEdge->BackgroundColor = { 90,85,85,255 };
	EnterEdge->AnchorPosition = { 0.5,0.5 };
	EnterEdge->Position = { 0.5, 0.92 };
	EnterEdge->Text = " Enter ";
	EnterEdge->TextColor = { 255,178,103,255 };
	EnterEdge->BorderColor = { 15,15,15,255 };
	EnterEdge->BorderTransparency = 0.75;
	EnterEdge->BorderThickness = 4;
	EnterEdge->Roundness = 0.125;
	EnterEdge->Size = { 0.2, 0.1 };
	EnterEdge->font = "open";
	EnterEdge->Active = true;
	EnterEdge->SetMouseEnter([EnterEdge]() {Animate::Create(&EnterEdge->Size, 0.1, { 0.22, 0.11 }); Animate::Create(&EnterEdge->TextColor, 0.1, { 255,230,111,255 }); Animate::Create(&EnterEdge->BorderColor, 0.1, { 45,40,40,255 }); });
	EnterEdge->SetMouseLeave([EnterEdge]() {Animate::Create(&EnterEdge->Size, 0.1, { 0.2, 0.1 }); Animate::Create(&EnterEdge->TextColor, 0.1, { 255,178,103,255 }); Animate::Create(&EnterEdge->BorderColor, 0.1, { 15,15,15,255 }); });
	EnterEdge->SetMouse1HoldEnd([objects, fillFrame, mustHaveEdges, EnterEdge, AddEdge, secondFrame]() {
		if (objects->Children.size() >= 1) {
			Animate::Animation* sas = Animate::Create(&fillFrame->CanvasPosition.y, 0.45, 2, Animate::Cube, Animate::In);
			EnterEdge->Active = false;
			AddEdge->Active = false;
			sas->Completed = [secondFrame]() { secondFrame->Visible = true; };
		}
		else {
			Animate::Animation* sas = Animate::Create(&mustHaveEdges->TextTransparency, 0.75, 0, Animate::Cube, Animate::Out);
			sas->Completed = [mustHaveEdges]() {
				Animate::Create(&mustHaveEdges->TextTransparency, 0.75, 1);
			};
		}
	});

	EnterVertices->SetMouse1HoldEnd([quantityOfVerticesBox, EnterVertices, fillFrame, MustBeNumber, firstFrame]() {
		bool digits = true; if (!quantityOfVerticesBox->Text.size()) digits = false;
		for (int i = 0; i < quantityOfVerticesBox->Text.size() and digits; i++) {
			if (quantityOfVerticesBox->Text[i] < '0' or quantityOfVerticesBox->Text[i] > '9' or (i == 0 and quantityOfVerticesBox->Text[i] == '0')) {
				digits = false;
				break;
			}
		}
		if (digits) {
			EnterVertices->Active = false;
			Animate::Animation* sas = Animate::Create(&fillFrame->CanvasPosition.y, 0.45, 1, Animate::Cube, Animate::In);
			sas->Completed = [firstFrame]() { firstFrame->Visible = false; };
			int quantity = 0;
			for (int i = 0; quantityOfVerticesBox->Text[i] != '\0'; i++) {
				quantity *= 10;
				quantity += quantityOfVerticesBox->Text[i] - '0';
			}

			quantityOfVertices = quantity;
			localTransferTime.resize(quantityOfVertices);
			for (int j = 0; j < 4; j++) {
				vertexEdges[j] = vector<vector<Edge>>(quantity);
			}

		} else {
			Animate::Animation* sas = Animate::Create(&MustBeNumber->TextTransparency, 0.75, 0, Animate::Cube, Animate::Out);
			sas->Completed = [MustBeNumber]() {
				Animate::Create(&MustBeNumber->TextTransparency, 0.75, 1);
			};
		}
	});

	static bool createOpened = false;
	static int currentTab = 0;
	static bool tabsInited = false;
	create->SetMouseClick1([]() { createOpened = !createOpened; });
	new ChangedSignal(dataExist, [createFrame, fillFrame, createBG, historyScroll]() {
		if (dataExist and not tabsInited) {
			tabsInited = true;
			TextLabel* tab0 = new TextLabel(createBG);
			tab0->Position = { 0.15, 0.02 };
			tab0->Size = { 0.185, 0.1 };
			tab0->Text = "   Data   ";
			tab0->Name = "tab0";
			tab0->font = "open";
			tab0->TextColor = { 255,178,103,255 };
			tab0->BackgroundColor = { 95,90,90,255 };
			tab0->BorderColor = { 255,178,103,255 };
			tab0->BorderThickness = 3;
			tab0->Roundness = 0.1;
			tab0->SetMouseEnter([tab0]() { Animate::Create(&tab0->Position.y, 0.1, -0.1); });
			tab0->SetMouseLeave([tab0]() { Animate::Create(&tab0->Position.y, 0.1, -0.09); });
			tab0->SetMouse1HoldEnd([]() {currentTab = 0; });
			tab0->ZIndex = -1;

			Animate::Animation* sas = Animate::Create(&tab0->Position.y, 0.2, -0.09);
			sas->Completed = [tab0]() {
				tab0->Active = true;
				};

			TextLabel* tab1 = new TextLabel(createBG);
			tab1->Position = { 0.35, 0.02 };
			tab1->Size = { 0.125, 0.1 };
			tab1->Text = "  Create  ";
			tab1->Name = "tab1";
			tab1->font = "open";
			tab1->TextColor = { 255, 255, 204, 255 };
			tab1->BackgroundColor = { 95,90,90,255 };
			tab1->BorderColor = { 255, 255, 204, 255 };
			tab1->BorderThickness = 3;
			tab1->Roundness = 0.1;
			tab1->SetMouseEnter([tab1]() { Animate::Create(&tab1->Position.y, 0.1, -0.1); });
			tab1->SetMouseLeave([tab1]() { Animate::Create(&tab1->Position.y, 0.1, -0.09); });
			tab1->SetMouse1HoldEnd([]() {currentTab = 1; });
			tab1->ZIndex = -1;

			Animate::Animation* an = Animate::Create(&tab1->Position.y, 0.2, -0.09);
			an->Completed = [tab1]() {
				tab1->Active = true;
			};

			TextLabel* createName = new TextLabel(createFrame);
			createName->Name = "createName";
			createName->Text = "Create a path";
			createName->AnchorPosition = { 0.5, 0 };
			createName->Size = { 0.6, 0.1 };
			createName->Position = { 0.5, 0 };
			createName->BackgroundTransparency = 1;
			createName->TextColor = { 255,178,103,255 };
			createName->font = "open";

			TextBox* startVertex = new TextBox(createFrame);
			startVertex->Name = "startVertex";
			startVertex->PlaceholderText = " Start ";
			startVertex->PlaceholderTextColor = { 30,30,30,255 };
			startVertex->TextColor = { 255,178,103,255 };
			startVertex->BorderColor = { 37,30,30,255 };
			startVertex->BorderThickness = 3;
			startVertex->BackgroundColor = {80,75,75,255};
			startVertex->font = "open";
			startVertex->maxSymbols = 4;
			startVertex->AllowedSymbols = "0123456789";
			startVertex->Position = { 0.1, 0.15 };
			startVertex->Size = { 0.3, 0.1 };
			startVertex->TextSize = -1;
			startVertex->Roundness = 0.1;

			TextBox* endVertex = new TextBox(createFrame);
			endVertex->Name = "endVertex";
			endVertex->PlaceholderText = " End ";
			endVertex->PlaceholderTextColor = { 30,30,30,255 };
			endVertex->TextColor = { 255,178,103,255 };
			endVertex->BorderColor = { 37,30,30,255 };
			endVertex->BorderThickness = 3;
			endVertex->BackgroundColor = {80,75,75,255};
			endVertex->font = "open";
			endVertex->maxSymbols = 4;
			endVertex->AllowedSymbols = "0123456789";
			endVertex->Position = { 0.6, 0.15 };
			endVertex->Size = { 0.3, 0.1 };
			endVertex->TextSize = -1;
			endVertex->Roundness = 0.1;

			TextLabel* convenienceName = new TextLabel(createFrame);
			convenienceName->Name = "convenienceName";
			convenienceName->Text = "Convenience coefficient: ";
			convenienceName->Size = { 0.6, 0.1 };
			convenienceName->Position = { 0.05, 0.27 };
			convenienceName->BackgroundTransparency = 1;
			convenienceName->TextColor = { 255,178,103,255 };
			convenienceName->font = "open";
			convenienceName->TextAnchor = TextAnchorEnum::N;

			TextBox* convenience = new TextBox(createFrame);
			convenience->Name = "convenience";
			convenience->PlaceholderText = " 0.0 ";
			convenience->PlaceholderTextColor = { 30,30,30,255 };
			convenience->TextColor = { 255,178,103,255 };
			convenience->BorderColor = { 37,30,30,255 };
			convenience->BorderThickness = 3;
			convenience->BackgroundColor = { 80,75,75,255 };
			convenience->font = "open";
			convenience->maxSymbols = 4;
			convenience->AllowedSymbols = "0123456789.";
			convenience->Position = { 0.65, 0.27 };
			convenience->Size = { 0.3, 0.1 };
			convenience->TextSize = -1;
			convenience->Roundness = 0.1;

			ImageLabel* point = new ImageLabel(createFrame);
			point->Name = "pointer";
			point->BackgroundTransparency = 1;
			point->ImageColor = { 255,178,103,255 };
			point->setImage("Textures/Pointer.png");
			point->AnchorPosition = { 0.5,0.5 };
			point->Position = { 0.5, 0.2 };
			point->Size = { 0.1, 0.1 };
			point->Active = true;
			point->SetMouseEnter([point]() {Animate::Create(&point->ImageColor, 0.1, { 255,255,204,255 }); Animate::Create(&point->Size, 0.1, { 0.12,0.12 }); });
			point->SetMouseLeave([point]() {Animate::Create(&point->ImageColor, 0.1, { 255,178,103,255 }); Animate::Create(&point->Size, 0.1, { 0.1,0.1 }); });

			auto getQuantityOnLayer = [](int l) { return (l == 1) ? 1 : l * 2 + 1; };

			TextLabel* res = new TextLabel(createFrame);
			res->Position = { 0.05, 0.9 };
			res->Size = { 0.9, 0.1 };
			res->font = "open";
			res->TextColor = { 255,178,103, 255 };
			res->Visible = false;
			res->BackgroundTransparency = 1;

			ScrollFrame* map = new ScrollFrame(createFrame);
			map->Direction = 'B';
			map->SliderColor = { 255,178,103,255 };
			map->SliderTransparency = 0.5;
			map->ScrollSpeed = 0.05;
			map->BackgroundTransparency = 0.9;
			map->Size = { 0.9, 0.38 };
			map->Position = { 0.05, 0.51 }; 
			map->Roundness = 0.1;
			int layers = std::ceil(std::sqrt(quantityOfVertices));
			map->CanvasSize.x = (getQuantityOnLayer(layers) > 12) ? (float)getQuantityOnLayer(layers)/12 : 1;
			map->CanvasSize.y = (layers > 7) ? (float)layers / 7 : 1;
			for (int i = 1; i < quantityOfVertices+1; i++) {
				int layer = std::ceil(std::sqrt(i));
				float y = layer * 0.125f;
				float startLayerPos = map->CanvasSize.x / 2 - 0.08f * (layer - 1);
				float x = startLayerPos + (i - std::pow(layer-1, 2)) * 0.08f;
				TextLabel* t = new TextLabel(map);
				t->AnchorPosition = { 0.5,0.5 };
				t->Size = { 0.05, 0.1 };
				t->Position = { x, y };
				t->BackgroundColor = {75,70,70,255};
				t->Text = std::to_string(i);
				t->TextColor = { 255,178,103,255 };
				t->Name = std::to_string(i);
				t->font = "open";
				if (vertexEdges[3][i - 1].size() == 0) {
					t->TextColor = { 40,35,35,255 };
				}
			}

			for (int i = 1; i < quantityOfVertices + 1; i++) {
				for (Edge e : vertexEdges[3][i-1]) {
					TextLabel* from = dynamic_cast<TextLabel*>(map->findChild(std::to_string(i)));
					TextLabel* to = dynamic_cast<TextLabel*>(map->findChild(std::to_string(e.to+1)));
					LineEx* l = new LineEx(map);
					l->ZIndex = -2;
					l->Position1 = { from->Position.x + from->AnchorPosition.x * from->Size.x - from->Size.x * 0.5f, from->Position.y + from->AnchorPosition.y * from->Size.y - from->Size.y * 0.5f };
					l->Position2 = { to->Position.x + to->AnchorPosition.x * to->Size.x - to->Size.x * 0.5f, to->Position.y + to->AnchorPosition.y * to->Size.y - to->Size.y * 0.5f };
					l->Thickness = 2;
					l->LineColor = { 40,35,35,255 };
					std::string sosya = from->Text; sosya += to->Text;
					l->Name = sosya;
				}
			}

			map->CanvasPosition.x = (map->CanvasSize.x > 1) ? map->CanvasSize.x / 2 : 0;

			TextLabel* EnterCreate = new TextLabel(createFrame);
			EnterCreate->Name = "enter4";
			EnterCreate->BackgroundColor = { 90,85,85,255 };
			EnterCreate->AnchorPosition = { 0.5,0.5 };
			EnterCreate->Position = { 0.5, 0.45 };
			EnterCreate->Text = " Create ";
			EnterCreate->TextColor = { 255,178,103,255 };
			EnterCreate->BorderColor = { 15,15,15,255 };
			EnterCreate->BorderTransparency = 0.75;
			EnterCreate->BorderThickness = 4;
			EnterCreate->Roundness = 0.125;
			EnterCreate->Size = { 0.2, 0.1 };
			EnterCreate->font = "open";
			EnterCreate->Active = true;
			EnterCreate->SetMouseEnter([EnterCreate]() {Animate::Create(&EnterCreate->Size, 0.1, { 0.22, 0.11 }); Animate::Create(&EnterCreate->TextColor, 0.3, { 255,230,111,255 }); Animate::Create(&EnterCreate->BorderColor, 0.1, { 45,40,40,255 }); });
			EnterCreate->SetMouseLeave([EnterCreate]() {Animate::Create(&EnterCreate->Size, 0.1, { 0.2, 0.1 }); Animate::Create(&EnterCreate->TextColor, 0.1, { 255,178,103,255 }); Animate::Create(&EnterCreate->BorderColor, 0.1, { 15,15,15,255 }); });
			
			static Color col[4] = {
				{ 255,204,204,255 },
				{ 204,229,255,255 },
				{ 204,255,204,255 },
				{ 255,178,103,255 }
			};
			std::string names[3] = {"Underground", "Autobus", "Railway"};

			for (int i = 0; i < 3; i++) {
				TextLabel* t = new TextLabel(createFrame);
				t->Position = { 0.05, 0.37f + i * 0.04f };
				t->Size = { 0.25, 0.04 };
				t->BackgroundTransparency = 1;
				t->font = "open";
				t->TextColor = col[i];
				t->Name = names[i];
				t->Text = names[i];
				t->TextAnchor = TextAnchorEnum::W;
			}

			EnterCreate->SetMouse1HoldEnd([startVertex, endVertex, convenience, map, res, historyScroll]() {
				if (startVertex->Text == "" or endVertex->Text == "") return;
				Vertex st = TextToInteger(startVertex->Text.c_str());
				Vertex target = TextToInteger(endVertex->Text.c_str());;
				if (st <= 0 or st > quantityOfVertices or target <= 0 or target > quantityOfVertices) return;
				float convenienceCoefficient = TextToFloat(convenience->Text.c_str());

				for (Instance* o : map->Children) {
					TextLabel* e = dynamic_cast<TextLabel*>(o);
					if (e) {
						e->BackgroundColor = { 75,70,70,255 };
						continue;
					}
					LineEx* s = dynamic_cast<LineEx*>(o);
					if (s) {
						s->LineColor = { 40,35,35,255 };
					}
				}

				vector<Result> results;

				vector<vector<pair<float, int>>> distantions(quantityOfVertices, vector<pair<float, int>>(3, { BILL, BILL }));
				prioritiedQueue heap;

				heap.push({ 0.0f, st - 1, Start });

				vector<vector<Vertex>> verticesWay(quantityOfVertices, vector<Vertex>(3));
				vector<vector<transportType>> transportWay(quantityOfVertices, vector<transportType>(3));

				while (!heap.empty()) {
					Node current = heap.pop();

					if (current.distantion > ((current.transport != Start) ? distantions[current.index][current.transport].first : 0)) continue;

					for (Edge e : vertexEdges[3][current.index]) {
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

				Vertex start = st;
				Vertex end = target - 1;

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

				results.push_back({ end + 1, (!(ans.first >= BILL and ans.second >= BILL)), ans.first, counterTransfers ,ans.first + convenienceCoefficient * counterTransfers, way });

				quickSort(results, [](const Result& a, const Result& b) {
					if (a.reachable != b.reachable) return a.reachable > b.reachable;
					if (!a.reachable and !b.reachable) return a.target < b.target;
					if (a.metric != b.metric) return a.metric < b.metric;
					if (a.time != b.time) return a.time < b.time;
					if (a.transfers != b.transfers) return a.transfers < b.transfers;
					return a.target < b.target;
					});

				ostringstream asas;
				res->Visible = true;

				Result& r = results[0]; // лучший результат
				if (!r.reachable) {
					asas << "Path not exist";
					res->Text = asas.str();
					res->TextColor = { 255,204,204,255 };
					return;
				}

				TextLabel* his = new TextLabel(historyScroll);
				his->Size = { 1, 0.1 };
				his->Position = { 0, (historyScroll->Children.size() - 1) * 0.1f };
				his->BackgroundTransparency = 1;
				his->Text = std::to_string(start);
				his->Text += " -> ";
				his->Text += std::to_string(r.target);
				his->TextColor = { 255,178,103,255 };
				his->font = "open";
				his->TextAnchor = TextAnchorEnum::W;
				historyScroll->CanvasSize.y = (historyScroll->Children.size() > 10) ? (float)historyScroll->Children.size() / 10 : 1;
				historyScroll->CanvasPosition.y = historyScroll->CanvasSize.y-1;

				asas << "Shortest time: " << r.time << ". Transfers quantity: " << r.transfers << ". Convenience metric: " << r.metric;

				res->Text = asas.str();
				res->TextColor = { 255,178,103,255 };

				for (int i = 0; i < r.way.size(); i++) {
					pair<int, transportType> obj = r.way[i];
					for (Instance* o : map->Children) {
						TextLabel* e = dynamic_cast<TextLabel*>(o);
						if (e) {
							if (e->Name == std::to_string(obj.first + 1) or e->Name == std::to_string(end + 1)) {
								e->BackgroundColor = { 100,95,95,255 };
							}
						}
					}

					if (i == 0) {
						continue;
					}

					std::string as = std::to_string(r.way[i].first+1); as += std::to_string(r.way[i-1].first+1);
					std::string as2 = std::to_string(r.way[i-1].first+1); as2 += std::to_string(r.way[i].first+1);

					for (Instance* o : map->Children) {
						LineEx* s = dynamic_cast<LineEx*>(o);
						if (s) {
							if (as == s->Name or as2 == s->Name) {
								s->LineColor = col[(obj.second == -1) ? 3 : obj.second];
							}
						}
					}
				}
			}
		);}
	});

	new ChangedSignal(currentTab, [createFrame, fillFrame, createBG]() {
		createFrame->Visible = currentTab == 1;
		fillFrame->Visible = currentTab == 0; 
		Color c0 = (currentTab == 0) ? Color{255, 178, 103, 255} : Color{ 255, 255, 204, 255 };
		Color c1 = (currentTab == 1) ? Color{255, 178, 103, 255} : Color{ 255, 255, 204, 255 };
		TextLabel* tab0 = dynamic_cast<TextLabel*>(createBG->findChild("tab0"));
		TextLabel* tab1 = dynamic_cast<TextLabel*>(createBG->findChild("tab1"));
		tab0->TextColor = c0;
		tab0->BorderColor = c0;
		tab1->TextColor = c1;
		tab1->BorderColor = c1;

		if (currentTab == 0) {
			Animate::Create(&tab1->Position.x, 0.1, 0.35);
			Animate::Create(&tab0->Size.x, 0.1, 0.185);
			Animate::Create(&tab1->Size.x, 0.1, 0.125);
		} else {
			Animate::Create(&tab1->Position.x, 0.1, 0.29);
			Animate::Create(&tab0->Size.x, 0.1, 0.125);
			Animate::Create(&tab1->Size.x, 0.1, 0.185);
		}
	});	

	new ChangedSignal(createOpened, [createBG, createFrame, fillFrame]() {
		createFrame->Visible = dataExist;
		fillFrame->Visible = !dataExist;
		currentTab = dataExist;

		if (createOpened) {
			createBG->Visible = true;
			Animate::Animation* sas = Animate::Create(&createBG->Size, 0.3f, { 0.6, 0.7 }, Animate::Circular);
			sas->Completed = [createBG]() {
				for (Instance* c : createBG->Children) {
					Object2D* child = dynamic_cast<Object2D*>(c);
					child->BorderTransparency = 1;
					Animate::Create(&child->BorderTransparency, 0.5f, 0);
				}
			};
		} else {
			Animate::Animation* s = Animate::Create(&createBG->Size, 0.15f, { 0, 0 });
			s->Completed = [createBG]() {createBG->Visible = false; };

			for (Instance* c : createBG->Children) {
				Object2D* child = dynamic_cast<Object2D*>(c);
				child->BorderTransparency = 0;

				Animate::Create(&child->BorderTransparency, 0.15f, 1);
			}
		}
	});
}

int WinMain() {
	Instance StartInstance(true);
	StartInstance.Name = "StartInstance";

	addFontToQueqe("open", "Fonts/open.ttf", 150);

	initialInterface(StartInstance);

	start(StartInstance, { 1400, 800, 144 }, "Megapolis transport system");
}