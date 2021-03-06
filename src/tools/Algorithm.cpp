#include "Algorithm.h"
#include "../game/Character.h"
#include "../common/Tile.h"
#include "../common/Common.h"

unsigned int node_count = 0;
unsigned int trials = 0;
unsigned int total_iter = 0;

bool operator<(Node n1, Node n2) {
	return n1.f() > n2.f();
}

inline float Node::getX() {return this->tile->x;}
inline float Node::getY() {return -this->tile->y;}

float h(Tile *tile,Tile *start, Tile *end,float bias) {
	float x = tile->x;
	float y = -tile->y;
	
	float sx = start->x;
	float sy = -start->y;
	
	float ex = end->x;
	float ey = -end->y;
	
	float dx1 = x - ex;
	float dy1 = y - ey;
	float dx2 = sx - ex;
	float dy2 = sy - ey;
	
	return 2*dist({x,y},{ex,ey})+bias*abs(dx1*dy2 - dx2*dy1);
}

bool characterCanReach(Character *c,float tx, float ty, bool adjacent) {
	return generatePath(c,tx,ty,adjacent).size() > 0;
}

std::vector<Tile*> generatePath(Character *c,float tx, float ty, bool adjacent) {
	
	Tile* start = c->location;
	Tile* end = TBAGame->gameWorld->getTileAt(tx,ty);
	
	//Don't bother pathing to impassable tile
	if(end->isPassable() == false and adjacent == false) {
		debug(c->getName()+" cannot generate path to impassable tile "+std::to_string((int)tx)+", "+std::to_string((int)ty));
		c->lastPathCheck = TBAGame->logicTicks;
		return {};
	}
	if(!start->isPassable()) {
		debug(c->getEntityName()+" is stuck at "+std::to_string(c->x)+", "+std::to_string(c->y));
		c->lastPathCheck = TBAGame->logicTicks;
		return {};
	}
	
	//Initialize starting node and known starting information
	Node start_node = Node(c->location,c->location,end);
	start_node.gScore = 0;

	//Contains open nodes
	std::priority_queue<Node> nodes;
	nodes.push(start_node);
	
	//Maps node ID's to the neighboring node with the shortest path to it
	std::map<int,Node> cameFrom;
	
	Node current;
	Node testNode;
	
	Tile *testTile;
	
	//Set for tracking members of node queue
	std::set<Node> n;
	std::set<Node>::iterator it;
	
	//Tentative gScore
	float tg;
	
	//Iteration estimation
	//Uses conservative estimate for iterations required
	// per unit of euclidean distance traveled
	//If exceeded, trial will be terminated and heuristic function coeffs will be changed
	float total_distance = dist({c->location->x,-c->location->y},{end->x,-end->y});
	int max_iter = total_distance*200;
	int iter=0;
	
	//Pathing bias
	float bias = .001;
	
	//Total trials made
	int t = 0;
	int max_trials = 3;
	
	//While there are still options to search
	while(!nodes.empty()) {
				
		current = nodes.top();


		if((char*)current.tile == (char*)end) break;
		if(adjacent and current.tile->adjacent(end)) break;
	
		nodes.pop();
		it = n.find(current);
	
		for(int i=-1;i<=1;i++) {
			for(int j=-1;j<=1;j++) {


				if(j == 0 and i == 0) continue;
				testTile = TBAGame->gameWorld->getTileAt(current.getX()+j,current.getY()+i);
				if(!testTile->isPassable()) continue;
				//Don't pass through corners or half corners
				if(TBAGame->gameWorld->isCorner(current.tile,testTile)) continue;

				testNode = Node(testTile,start,end);
				testNode.setBias(bias);
				iter++;

				if(iter >= max_iter) {
					c->lastPathCheck = TBAGame->logicTicks;
					// debug("Trial "+std::to_string(t)+" disqualified for exceeding max iterations, altering bias and restarting");
					current = start_node;
					n.clear();nodes = {};		
					bias *= 10;
					iter=0;
					if(++t >= max_trials) {
						// debug("Pathfinding disqualified for exceeding max trials, exiting");
						return {};
					}
				}
				//Use octile distance
				tg = current.gScore + ( (abs(i+j) == 1)? 1 : 1.41421356237);
				if(tg < testNode.gScore) {
					if(cameFrom.find(testNode.id) != cameFrom.end()) {
						cameFrom.erase(testNode.id);
					}
					cameFrom.insert({testNode.id,current});
					// debug("("+std::to_string(testNode.tile->x)+", "+std::to_string(testNode.tile->y)+") -> ("+std::to_string(current.tile->x)+", "+std::to_string(current.tile->y)+")");
					testNode.gScore = tg;
					// debug(n.size());
					if(n.find(testNode) == n.end()) {
						// debug("Adding new node: "+std::to_string(testNode.tile->x)+", "+std::to_string(testNode.tile->y)+" with fScore: "+std::to_string(testNode.f()));
						nodes.push(testNode);
						n.insert(testNode);
					}
				}
			}
		}
	}

	//Reconstruct path
	int d = current.id;
	std::vector<Tile*> path = {current.tile};
	while(cameFrom.find(d) != cameFrom.end()) {
		path.emplace(path.begin(),cameFrom.at(d).tile);
		d = cameFrom.at(d).id;
	}

	return path;

	//Debug info

	// debug("Done in "+std::to_string(k)+" iterations");
	// float ratio= k/dist({c->location->x,-c->location->y},{end->x,-end->y});
	// total_iter += ratio;
	// trials++;
	// debug("Average iterations per unit euclidian distance: "+std::to_string(total_iter/trials));
	// debug("Ratio for this trial: "+std::to_string(ratio));
}