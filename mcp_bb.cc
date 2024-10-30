#include <iostream>
#include <vector>
#include <fstream>
#include <climits>
#include <queue>
#include <unordered_map>
#include <tuple>
#include <algorithm>

using namespace std;

struct Posicion {
    int x, y;

    bool operator==(const Posicion& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template <>
    struct hash<Posicion> {
        size_t operator()(const Posicion& p) const {
            return hash<int>()(p.x) ^ hash<int>()(p.y);
        }
    };
}

struct Nodo {
    int pasos;
    Posicion p;
    string ruta;

    bool operator<(const Nodo& otro) const {
        return pasos > otro.pasos;
    }
};

// Variables globales referentes al tamaño del laberinto
int altura = 0;
int anchura = 0;

// Variables globales referentes a los nodos
int visitados = 0;
int explorados = 0;
int hojas = 0;
int inviables = 0; // no cumplen la cota pesimista
int noPrometedores = 0; // no cumplen la cota optimista
int prometedoresDesc = 0; // nodos prometedores descartados (ya se ha encontrado un camino más corto)
int updateFromLeaf = 0;
int updatePesimistic = 0;
double nTime = 0;
string route = "";

vector<vector<int>> lecturaArchivo(const string& nArchivo) {
    ifstream archivo(nArchivo);

    if (!archivo.is_open()) {
        cerr << " ERROR : can’t open file '" << nArchivo << "'. " << endl;
        cerr << "Usage:" << endl;
        cerr << "maze_bt [-p] [--p2D] -f file" << endl;
        exit(EXIT_FAILURE);
    }

    archivo >> altura >> anchura;
    vector<vector<int>> nMaze(altura, vector<int>(anchura));

    for (int i = 0; i < altura; i++)
        for (int j = 0; j < anchura; j++)
            archivo >> nMaze[i][j];

    return nMaze;
}

// Originalmente sería el caso inicial, pero se ha modificado para que tan solo inicialice la matriz
// con los costes para llegar a una casilla en específico.
int mcp_it_matriz(const vector<vector<int>>& nMaze, vector<vector<int>>& res) {
    res[0][0] = nMaze[0][0];

    // Búsqueda de caminos para la primera fila 
    for (int j = 1; j < anchura; j++) {
        res[0][j] = res[0][j - 1] + nMaze[0][j];
    }

    // Búsqueda general
    for (int i = 1; i < altura; i++) {
        for (int j = 0; j < anchura; j++) {
            int y = res[i - 1][j];
            int x = (j > 0) ? res[i][j - 1] : INT_MAX;
            int xy = (j > 0) ? res[i - 1][j - 1] : INT_MAX;

            int comp = min({x, y, xy});
            res[i][j] = comp + nMaze[i][j];
        }
    }

    updatePesimistic++;
    return res[altura - 1][anchura - 1];
}



// Se usará como cota pesimista tanto inicial como para todos los nodos
int mcp_greedy(const vector<vector<int>>& nMaze, Posicion pFinal) {
    int pasosBajada = nMaze[pFinal.x][pFinal.y];
    Posicion nPos = pFinal;

    while (!(nPos.x == altura - 1 && nPos.y == anchura - 1)) {
        int x = (nPos.x < altura - 1) ? nMaze[nPos.x + 1][nPos.y] : INT_MAX;
        int y = (nPos.y < anchura - 1) ? nMaze[nPos.x][nPos.y + 1] : INT_MAX;
        int xy = (nPos.x < altura - 1 && nPos.y < anchura - 1) ? nMaze[nPos.x + 1][nPos.y + 1] : INT_MAX;

        if (xy <= x && xy <= y) {
            nPos.x++;
            nPos.y++;
            pasosBajada += xy;
        } else if (x <= y) {
            nPos.x++;
            pasosBajada += x;
        } else {
            nPos.y++;
            pasosBajada += y;
        }
    }

    return pasosBajada;
}

int caso_plausible(const vector<vector<int>>& maze, Posicion p) {
    return mcp_greedy(maze, p);
}

// Algoritmo voraz que asume que todas las casillas tendrán coste mínimo (de 1).
int noLimits_voraz(const Posicion& p) {
    return (altura - 1 - p.x) + (anchura - 1 - p.y);
}

bool es_prometedor(int pasos, const Posicion& p, int current_Best) {
    int cOptimista = noLimits_voraz(p);
    int nPrometedor = cOptimista + pasos;
    return current_Best > nPrometedor;
}


void mcp_it_parser(const vector<vector<int>>& res){
	
	//Posición final del tablero
	Posicion p;
	p.x=altura-1;
	p.y=anchura-1;

	string ruta = "";

	//abajo
	string xS = "5";
	//diagonal (abajo y derecha)
	string xyS = "4";
	//derecha
	string yS = "3";

	//Se comprueba si se ha llegado al final
	int result = res[p.x][p.y];
	
	//Vector que almacenará la ruta (0 = camino por el que no se ha pasado. 1 = camino por el que se ha pasado)
	//vector<vector<int>> salida( altura , vector < int >( anchura ));
	bool fin=false;

	if(result!=INT_MAX){
		/*
		//Se iniciza el vector salida
		for(int i=0; i<altura;i++){
			for(int j=0; j<anchura;j++){
				salida[i][j]=0;
			}
		}
		*/
		
		//salida[p.x][p.y]=1;
		
		while(fin==false){
			bool found=false;
			
			if(p.x!=0){
				//Si nMaze[p.x][p.y-1] y nMaze[p.x-1][p.y-1] son accesibles
				if(p.y != 0){	
					//Si res[p.x-1][p.y] es el menor (se iba a la derecha )
					if(res[p.x-1][p.y] < res[p.x][p.y-1] && res[p.x-1][p.y] < res[p.x-1][p.y-1]){
						p.x--;
						
						//cout<<"fola (x):"<<res[p.x][p.y]<<endl;
						//salida[p.x][p.y]=1;
						ruta = xS + ruta;
						found=true;
					}
				//en el caso que no (solo se puede ir a nMaze[p.x-1][p.y])
				}else{
					p.x--;
						
					//cout<<"fola (x):"<<res[p.x][p.y]<<endl;
					//salida[p.x][p.y]=1;
					ruta = xS + ruta;
					found=true;
					
				}
			}
			
			if(p.y!=0 && found==false){
				//Si nMaze[p.x][p.y-1] y nMaze[p.x-1][p.y-1] son accesibles
				if(p.x != 0){		
					//Si res[p.x][p.y-1] es el menor 
					if(res[p.x][p.y-1] < res[p.x-1][p.y] && res[p.x][p.y-1] < res[p.x-1][p.y-1]){
						p.y--;

						//cout<<"fola (y):"<<res[p.x][p.y]<<endl;
						//salida[p.x][p.y]=1;
						ruta = yS + ruta;
						found=true;
					}
				//en el caso que no (solo se puede ir a nMaze[p.x][p.y-1])
				}else{
					p.y--;
						
					//cout<<"fola (t):"<<res[p.x][p.y]<<endl;
					//salida[p.x][p.y]=1;
					ruta = yS + ruta;
					found=true;
				}
			}
			
			if(p.x!=0 && p.y!=0 && found==false){
				//Si res[p.x-1][p.y-1] es el menor (O igual al resto)
				if(res[p.x-1][p.y-1] <= res[p.x-1][p.y] && res[p.x-1][p.y-1] <= res[p.x][p.y-1]){
					p.x--;
					p.y--;

					//cout<<"fola (xy):"<<res[p.x][p.y]<<endl;
					//salida[p.x][p.y]=1;
					ruta = xyS + ruta;
					found=true;
				}
			}
			
			if(p.x==0 && p.y==0){
			
				//salida[p.x][p.y]=1;
				fin=true;
			}
		}
		
		/*3
		for(int i=0; i<altura;i++){
			for(int j=0; j<anchura;j++){
				if(salida[i][j]==1){
					cout<<'x';
				}else{
					cout<<'.';
				}
			}
			cout<<endl;
		}
		*/
	}else{
		cout<<"NO EXIT"<<endl;
	}

	//cout<<res[altura-1][anchura-1]<<endl;
	route = ruta;
}


int caso_plausible_inicial(const vector<vector<int>>& maze, vector<vector<int>>& vec){
	int pasos = 0;
	
	pasos = mcp_it_matriz(maze, vec);
	mcp_it_parser(vec);

	return pasos;
}  

void mcp_bb(const vector<vector<int>>& maze, vector<vector<int>>& vec, int& current_Best) {
    Nodo nNodo = {0, {0, 0}, ""};
    current_Best = caso_plausible_inicial(maze, vec);

    priority_queue<Nodo> q;
    q.push(nNodo);

    unordered_map<Posicion, int> coste;

    while (!q.empty()) {
        Nodo n = q.top();
        q.pop();

        if (n.p.x < altura && n.p.y < anchura && n.p.x >= 0 && n.p.y >= 0) {
            visitados++;
            n.pasos += maze[n.p.x][n.p.y];

            if (vec[n.p.x][n.p.y] >= n.pasos) {
                if (es_prometedor(n.pasos, n.p, current_Best)) {
                    if (caso_plausible(maze, n.p) + n.pasos > current_Best) {
                        vec[n.p.x][n.p.y] = n.pasos;
                        explorados++;

                        if (n.p.x == altura - 1 && n.p.y == anchura - 1) {
                            hojas++;
                            if (n.pasos < current_Best) {
                                route = n.ruta;
                                vec[n.p.x][n.p.y] = n.pasos;
                                current_Best = n.pasos;
                                updateFromLeaf++;
                            }
                            return;
                        }

                        static const vector<pair<int, int>> directions = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1}, {1, 0}, {1, -1}, {0, -1}, {-1, -1}};
                        static const vector<string> dirStrings = {"1", "2", "3", "4", "5", "6", "7", "8"};

                        for (int i = 0; i < directions.size(); ++i) {
                            Posicion newPos = {n.p.x + directions[i].first, n.p.y + directions[i].second};
                            Nodo newNodo = {n.pasos, newPos, n.ruta + dirStrings[i]};
                            q.push(newNodo);
                        }
                    } else {
                        prometedoresDesc++;
                    }
                } else {
                    noPrometedores++;
                }
            } else {
                inviables++;
            }
        } else {
            inviables++;
        }
    }
}

int mcp_bb(const vector<vector<int>>& maze, vector<vector<int>>& vec) {
    auto start = clock();
    int current_best = INT_MAX;
    mcp_bb(maze, vec, current_best);
    auto end = clock();
    nTime = nTime + (1000.0 * (end-start) / CLOCKS_PER_SEC);
    return current_best;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        cerr << "Usage: maze_bb [-p] [--p2D] -f file" << endl;
        return EXIT_FAILURE;
    }

    string fileName;
    bool flagP = false;
    bool flagP2D = false;

    for (int i = 1; i < argc; i++) {
        if (string(argv[i]) == "-f") {
            fileName = argv[++i];
        } else if (string(argv[i]) == "-p") {
            flagP = true;
        } else if (string(argv[i]) == "--p2D") {
            flagP2D = true;
        }
    }

    vector<vector<int>> maze = lecturaArchivo(fileName);
    vector<vector<int>> vec(altura, vector<int>(anchura, INT_MAX));

    int result = mcp_bb(maze, vec);

    if (flagP) {
        cout << "Final Best Cost: " << result << endl;
        cout << "Route: " << route << endl;
        cout << "Nodes Explored: " << explorados << endl;
        cout << "Nodes Visited: " << visitados << endl;
        cout << "Leaves: " << hojas << endl;
        cout << "Non-promising Nodes: " << noPrometedores << endl;
        cout << "Inviable Nodes: " << inviables << endl;
        cout << "Promising Nodes Discarded: " << prometedoresDesc << endl;
        cout << "Time: " << nTime << " miliseconds" << endl;
    }

    if (flagP2D) {
        ofstream output("output.txt");
        output << result << endl;
        for (const auto& row : vec) {
            for (const auto& cell : row) {
                output << cell << " ";
            }
            output << endl;
        }
        output.close();
    }

    return 0;
}