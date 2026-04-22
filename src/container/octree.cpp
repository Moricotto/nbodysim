#include "octree.hpp"
#include <algorithm>
#include <limits>
#include <numeric>
#include <iostream>
#include <cmath>
#include <glm/glm.hpp>
#include "../sim.hpp"
constexpr float EPSILON = 0.01;
constexpr float EPSILON_SQ = EPSILON * EPSILON;

template <typename T>
Octree<T>::Octree(std::vector<T> &bodies){
  Bounds xBound = calculateBounds(bodies, Coords::X);
  Bounds yBound = calculateBounds(bodies, Coords::Y);
  Bounds zBound = calculateBounds(bodies, Coords::Z);

  this->bounds.first = Vec(0.0);
  this->bounds.second = Vec(0.0);

	if (bodies.size() != 0){
		this->bounds.first = Vec(xBound.first.x, yBound.first.y, zBound.first.z);
		this->bounds.second = Vec(xBound.second.x, yBound.second.y, zBound.second.z);
	}
	// update the bounds
	this->root = new Node {.tree = this,.depth = 0, .bodies = std::vector<T*>() , };
  this->root->bodies.reserve(bodies.size());
  for (int i = 0; i < bodies.size(); i++){
    this->root->bodies.push_back(&bodies[i]);
  }
	this->rearrange();
}


template <typename T>
Octree<T>::~Octree() {
	if (!this->root)
		return;
	this->root->deleteChildren();
	delete this->root;
	this->root = nullptr;
};

template <typename T>
void Octree<T>::rearrange() {
	this->root->partition();
}

template <typename T>
void Octree<T>::update(const float G, const float dt){
  this->root->update(G, dt);
}

/**
 * @brief this function partition the bodies When we add new Data to each nodes
 * Subdivide 
*/
template <typename T>
void Octree<T>::Node::partition(const Bounds& bounds){
	if (this->bodies.size() <= tree->maxBodyPerNode){
		std::function<Vec(Vec, T*)> addFunc = [](Vec accum, T* body) {return accum + body->mass * body->position;};
		Mass total_mass = std::accumulate(this->bodies.begin(), this->bodies.end(), Mass(0), [](Mass accum, auto body) {return accum + body->mass;});

		Vec center_mass = std::accumulate(this->bodies.begin(), this->bodies.end(), Vec(0.0), addFunc) / total_mass;
		this->center_of_mass = center_mass;
		this->mass = total_mass;
		return;
	}

	if (isLeafNode()){ 
		this->initChildren();
	}

	int n = this->bodies.size();
	
	Bounds subBounds[8];
	for (int i = 0; i < 8; i++){
		NodePosition pos = static_cast<NodePosition>(i);
		subBounds[i] = Node::GetSubBound(pos, bounds);
	}
	for (int i = 0; i < n; i++){
		auto body = std::move(this->bodies.back());
		this->bodies.pop_back();
		bool found = false;

		for (int j = 0; j < 8; j++){
			if (!Node::PointInBounds(body->position, subBounds[j]))
				continue;
			this->children[j].bodies.push_back(body);
			found = true;
			break;
		}

		// just making sure that the thing found a parent else that'll be pretty bad.
		assert(found);
	}

	for (int i = 0; i < 8; i++){
		this->children[i].partition(subBounds[i]);
	}

	// update the mass and center of mass
	Mass m = 0.0;
	Vec center_of_mass = Vec(0.0f);

	for (int i = 0; i < 8; i++){
		center_of_mass += this->children[i].center_of_mass * this->children[i].mass;
		m += this->children[i].mass;
	}

	this->center_of_mass = center_of_mass / m;
	this->mass = m;
}

/**
 * @brief this function partition the bodies When we add new Data to each nodes
 * Subdivide 
*/
template <typename T>
void Octree<T>::Node::update(const float G, const float dt){
  if (this->mass == 0){
    return;
  }
  if (!isLeafNode()){
    for (int i = 0; i > 8; i++){
      this->children[i].update(G, dt);
    }
  }
  Node* root = this->tree->root;
  Bounds bounds = this->getBounds();
  float theta = 0.5f;
  
	
  for (T* body : bodies){
    body->acceleration = Vec(0.0);
    updateAcceleration(theta, G, dt, body, root, bounds);
  }
}

template <typename T>
void Octree<T>::Node::updateAcceleration(const float theta, const float G, const float dt, T* body, Node* node, Bounds bounds){

    auto distance = [](Vec a, Vec b){
      return std::sqrt((a.x - b.x) * (a.y - b.y) + (a.y - b.y) * (a.y - b.y) + (a.z - b.z) * (a.z - b.z));
    };
    float d = distance(body->position, node->center_of_mass);
    float s = node->getWidth(bounds);
    Vec r = node->center_of_mass - body->position; //vector from i to j
    // this represent the squared distance

    if (s / d < theta){
      float distance_sq = glm::dot(r, r);
      float denom = EPSILON_SQ + distance_sq;
      body->acceleration +=  (G * node->mass) / std::sqrt(denom * denom * denom) * r;
      return;
    }

    if (node->isLeafNode()){
      for (auto b : node->bodies){
        if (b == body) continue; 
          Vec r = b->position - body->position;
          float distance_sq = glm::dot(r, r);
          float denom = EPSILON_SQ + distance_sq;
          body->acceleration +=  (G * node->mass) / std::sqrt(denom * denom * denom) * r;
      }
      return;
    }
    for (int i = 0; i < 8; i++){
      NodePosition pos = static_cast<NodePosition>(i);
      Bounds childBound = Node::GetSubBound(pos, bounds);
      updateAcceleration(theta, G, dt, body, &node->children[i], childBound);
    }
}

template <typename T>
void Octree<T>::Node::partition(){
	Bounds bounds = getBounds();
	this->partition(bounds);
}


template<typename T>
float Octree<T>::Node::getWidth(Bounds bounds){
  float xWidth = std::abs(bounds.first.x - bounds.second.x);
  float yWidth = std::abs(bounds.first.y - bounds.second.y);
  float zWidth = std::abs(bounds.first.z - bounds.second.z);

  return std::min(std::min(xWidth, yWidth), zWidth);
}


template <typename T>
Octree<T>::Bounds Octree<T>::Node::getBounds() {
	if (isRootNode()) {
		return this->tree->bounds;
	}
	Bounds parentBounds = this->parent->getBounds();
	return Node::GetSubBound(this->position, parentBounds);
}

// Create memory make sure it's deleted after
template <typename T> 
void Octree<T>::Node::initChildren(){
	if (this->children != nullptr)
		return;
	this->children = new Node[8];
	for (int i = 0; i < 8; i++){
		Node& child = this->children[i];
		child.position = static_cast<NodePosition>(i);
		child.parent = this;
    child.tree = this->tree;
	}
}

// Avoid memory leaks
template <typename T> 
void Octree<T>::Node::deleteChildren(){
	if (children == nullptr)
		return;
	for (int i = 0; i < 8; i++){
		Node child = this->children[i];
			child.deleteChildren();
	}
	delete[] this->children;
	this->children  = nullptr;
}
template <typename T>
bool Octree<T>::Node::PointInBounds(const Vec &point, const Bounds& bounds) {
	const auto  minPoint = bounds.first;
	const auto  maxPoint = bounds.second;
	return 
		minPoint.x <= point.x && minPoint.y <= point.y && minPoint.z <= point.z &&
		maxPoint.x >= point.x && maxPoint.y >= point.y && maxPoint.z >= point.z;
} 

template <typename T> 
Octree<T>::Bounds Octree<T>::Node::GetSubBound(
		const NodePosition &position,
		const Bounds& bounds
	) {
	
	Vec center = (bounds.first + bounds.second) / 2.0;
	double zMin, zMax, yMin, yMax, xMin, xMax;

	
	switch (position) {
		case NodePosition::UP_TOP_RIGHT:
		case NodePosition::UP_BOTTOM_RIGHT:
		case NodePosition::BOTTOM_TOP_RIGHT:
		case NodePosition::BOTTOM_BOTTOM_RIGHT:
		xMin = center.x;
		xMax = bounds.second.x;
		break;
		default:
		xMin = bounds.first.x;
		xMax = center.x;
	}

	switch (position) {
		case NodePosition::UP_TOP_LEFT:
		case NodePosition::UP_TOP_RIGHT:
		case NodePosition::BOTTOM_TOP_LEFT:
		case NodePosition::BOTTOM_TOP_RIGHT:
		yMin = center.y;
		yMax = bounds.second.y;
		break;
		default:
		yMin = bounds.first.y;
		yMax = center.y;
	}
	switch (position) {
		case NodePosition::UP_TOP_LEFT:
		case NodePosition::UP_TOP_RIGHT:
		case NodePosition::UP_BOTTOM_LEFT:
		case NodePosition::UP_BOTTOM_RIGHT:
		zMin = center.z;
		zMax = bounds.second.z;
		break;
		default:
		zMin = bounds.first.z;
		zMax = center.z;
	}

	return Bounds(Vec(xMin, yMin, zMin), Vec(xMax, yMax, zMax));
}


template <typename T>
bool Octree<T>::Node::isRootNode() {
	return this->parent == nullptr;
} 

template <typename T>
bool Octree<T>::Node::isLeafNode() {
	return this->children == nullptr;
}

template <typename T> 


Octree<T>::Bounds Octree<T>::calculateBounds(const std::vector<T>& bodies, Coords coords){
  float inf = std::numeric_limits<float>::max();
  float ninf = std::numeric_limits<float>::min();
  Bounds bound = {Vec(inf), Vec(ninf)};
  for (T body : bodies) {
    switch (coords) {
      case Coords::X:
        if (body.position.x < bound.first.x)
          bound.first.x = body.position.x;

        if (body.position.x > bound.second.x)
          bound.second.x = body.position.x;
        break;
      case Coords::Y:
        if (body.position.y < bound.first.y)
          bound.first.y = body.position.y;

        if (body.position.y > bound.second.y)
          bound.second.y = body.position.y;
        break;
      case Coords::Z:
        if (body.position.z < bound.first.z)
          bound.first.z = body.position.z;

        if (body.position.z > bound.second.z)
          bound.second.z = body.position.z;
        break;
    }
  }
  return bound;
}


template class Octree<Body>;
