#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>

using namespace std;

// Open file streams for writing
ofstream outputFile("DryRun.txt");
ofstream activeN("ActiveNodes.txt");

// class to represent an edge in the graph
class Edge
{
public:
    int capacity, flow; // Edge capacity and current flow
};

// class to represent a vertex in the graph
class Vertex
{
public:
    int height, excessFlow; // Vertex height and excess flow
};

// Node class to represent each element in the queue
class Node
{
public:
    int data;
    Node *next;

    // Constructor to initialize a node
    Node(int value) : data(value), next(nullptr) {}
};

// Queue class
class Queue
{
private:
    Node *front;
    Node *rear;

public:
    // Constructor to initialize an empty queue
    Queue() : front(nullptr), rear(nullptr) {}

    // Check if the queue is empty
    bool isEmpty()
    {
        return front == nullptr;
    }

    // Enqueue (insert) an element at the rear of the queue
    void enqueue(int value)
    {
        Node *newNode = new Node(value);
        if (isEmpty())
        {
            front = rear = newNode;
        }
        else
        {
            rear->next = newNode;
            rear = newNode;
        }
        activeN << "Enqueued: " << value << " | Front: " << front->data << " | Rear: " << rear->data << endl;
    }

    // Dequeue (remove) an element from the front of the queue
    int dequeue()
    {
        if (isEmpty())
        {
            activeN << "Queue is empty. Cannot dequeue." << endl;
            return 0;
        }

        Node *temp = front;
        front = front->next;
        if (front == nullptr)
        {
            // If the queue becomes empty after dequeue, update the rear
            rear = nullptr;
        }

        int data = temp->data;
        delete temp;

        activeN << "Dequeued: " << data << " | Front: " << (front ? front->data : -1) << " | Rear: " << (rear ? rear->data : -1) << endl;

        return data;
    }
};

// Global variables
const int INF = numeric_limits<int>::max();
vector<vector<Edge>> edges;
vector<Vertex> vertices;
int source, sink;

// Function to print the graph's structure
void printstuff()
{
    outputFile << "   node:::::::: (vertex capacity flow)\n";
    for (int u = 0; u < edges.size(); ++u)
    {
        outputFile << u << ":::::::: ";
        for (int v = 0; v < edges[u].size(); ++v)
            outputFile << v << ' ' << edges[u][v].capacity << ' ' << edges[u][v].flow << "  :  ";
        outputFile << endl;
    }

    outputFile << "   Vertices(vertex height excessflow):" << endl;
    int i = 0;
    for (const auto &v : vertices)
    {
        outputFile << i << ' ' << v.height << ' ' << v.excessFlow << "  ::  ";
        i++;
    }
    outputFile << endl;
}

// Function to push flow from u to v
void push(int u, int v, Queue &activeNodes)
{
    // Calculate the amount of flow that can be pushed from u to v
    int flow = min(edges[u][v].capacity - edges[u][v].flow, vertices[u].excessFlow);

    // Update flow values in the forward and backward edges
    edges[u][v].flow += flow;
    vertices[u].excessFlow -= flow;
    vertices[v].excessFlow += flow;
    edges[v][u].flow -= flow;

    // Add v to the active nodes if it is not the source or sink
    if (v != sink && v != source && flow)
    {
        activeNodes.enqueue(v);
    }
}

// Function to relabel the height of a vertex u
void relabel(int u)
{
    int minHeight = INF;

    // Find the minimum height of neighbors with available capacity
    for (int v = 0; v < vertices.size(); ++v)
    {
        const Edge &e = edges[u][v];
        if (e.flow < e.capacity)
        {
            minHeight = min(minHeight, vertices[v].height);
        }
    }

    // Set the height of u to one more than the minimum height
    vertices[u].height = minHeight + 1;
}

// Function to discharge excess flow from a vertex u
void discharge(int u, Queue &activeNodes)
{
    // Continue until there is no excess flow at vertex u
    outputFile << u << " is being considered for discharge ::\nResidual graph at that time ::\n";
    printstuff();
    while (vertices[u].excessFlow > 0)
    {
        bool pushed = false;

        // Iterate over neighbors to push flow or relabel
        for (int v = 0; v < vertices.size(); ++v)
        {
            // Check if the height of the neighbor is one more than the height of u
            // and there is available capacity in the edge
            if (vertices[u].height == vertices[v].height + 1 &&
                edges[u][v].flow < edges[u][v].capacity)
            {
                // Push flow from u to v
                push(u, v, activeNodes);
                pushed = true;
                break;
            }
        }

        // If no push occurred, relabel u
        if (!pushed)
        {
            relabel(u);
        }
    }
    outputFile << endl
               << endl;
}

// Function to find the maximum flow in the network
int getMaxFlow()
{
    // Initialize the height of the source vertex
    vertices[source].height = vertices.size();

    // Vector to store active nodes during the algorithm
    Queue activeNodes;
    // Set the excess flow of the source to infinity
    vertices[source].excessFlow = INF;

    // Initialize the preflow by pushing flow from the source to all vertices
    for (int v = 0; v < vertices.size(); ++v)
    {
        push(source, v, activeNodes);
    }

    // Reset the excess flow of the source to 0
    vertices[source].excessFlow = 0;
    // Continue until there are active nodes
    while (!activeNodes.isEmpty())
    {
        // Get the last active node
        int u = activeNodes.dequeue();
        // Discharge the excess flow from the current node
        activeN << "  ~ " << u << " was considered for discharge \n";
        discharge(u, activeNodes);
        activeN << endl;
    }
    // Return the excess flow at the sink, which is the maximum flow
    return vertices[sink].excessFlow;
}

int main()
{
    // Read the input from a file
    ifstream file("sample1.txt");
    string line;
    int E = 0, V = 0;

    // Vector to store edge information directly
    vector<vector<int>> tempEdges;

    if (file.is_open())
    {
        // Parse the input file to populate tempEdges and find the number of vertices and edges
        while (getline(file, line))
        {
            istringstream iss(line);
            int u, v, c;
            if (iss >> u >> v >> c)
            {
                E++;
                cout << u << ' ' << v << ' ' << c << endl;
                if (V < v)
                    V = v;
                if (V < u)
                    V = u;
                // V = max({V, v, u});
                tempEdges.push_back({u, v, c});
            }
            else
            {
                cout << "Error reading integers from line: " << line << endl;
            }
        }
        file.close();
    }
    else
    {
        cout << "Unable to open sample file" << endl;
        return 1;
    }

    // Increment V once more to account for 0-based indexing
    V++;
    // Resize the matrix to V x V to represent the graph
    edges.resize(V, vector<Edge>(V, {0, 0})); // init edges matrix {capacity, flow}
    // Resize the vertices vector to V {excessFlow, height}
    vertices.resize(V, {0, 0});

    // Populate the edges matrix using values from the temporary vector
    for (const auto &edge : tempEdges)
    {
        int u = edge[0], v = edge[1], c = edge[2];
        edges[u][v] = {c, 0};
        edges[v][u] = {0, 0};
    }

    // Print the number of vertices and edges
    cout << "No. of vertices = " << V << " \nNo. of edges = " << E << endl;

    // Prompt the user to enter the source and sink vertices
    cout << "Enter the source and sink: ";
    cin >> source >> sink;
    // Check if the source and sink are valid
    if (sink >= V || source >= V || sink < 0 || source < 0)
    {
        cout << "Invalid input" << endl;
        return 0;
    }

    // Check if the file was opened successfully
    if (outputFile.is_open())
    {
        // output file opens perfectly
        // Calculate and print the maximum flow in the network
        cout << "Maximum flow is " << getMaxFlow() << endl;
        // Closing the output file
        if (!activeN.is_open())
        {
            cout << "Unable to open ActiveNodes file." << endl;
            return 1;
        }
        activeN.close();
        outputFile << "             **********Final**********\n";
        printstuff();
        outputFile.close();
    }
    else
    {
        cout << "Unable to open Dry Run file." << endl;
        return 1;
    }

    return 0;
}
