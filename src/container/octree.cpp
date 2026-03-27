#include "octree.hpp"
#include <algorithm>
#include <numeric>
#include "../sim.hpp"

template <typename T>
Octree<T>::Octree(std::vector<T> &bodies){
	auto xBound = std::minmax_element(bodies.begin(), bodies.end(), [](const T &a, const T &b) {return a.position.x < b.position.x;});
	auto yBound = std::minmax_element(bodies.begin(), bodies.end(), [](const T &a, const T &b) {return a.position.y < b.position.y;});
	auto zBound = std::minmax_element(bodies.begin(), bodies.end(), [](const T &a, const T &b) {return a.position.z < b.position.z;});

	if (xBound.first != bodies.end()){
		this->bounds.first = Vec(xBound.first->position.x, yBound.first->position.y, zBound.first->position.z);
		this->bounds.second = Vec(xBound.second->position.x, yBound.second->position.y, zBound.second->position.z);
	}
	else {
		this->bounds.first = Vec(0.0);
		this->bounds.second = Vec(0.0);
	}
	// update the bounds
	this->root = new Node {.tree = this,.depth = 0, .bodies = std::vector<T*>(bodies.size()) , };
	// this is fine to take the pointer of the objects because the container shouldn't live in the lifetime of the vector
	std::transform(bodies.begin(), bodies.end(), this->root->bodies.begin(), [](auto body) {return &body;});
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

/**
 * @brief this function partition the bodies When we add new Data to each nodes
 * Subdivide 
*/
template <typename T>
void Octree<T>::Node::partition(const Bounds& bounds){
	if (this->bodies.size() <= tree->maxBodyPerNode){
		std::function<Vec(Vec, T*)> addFunc = [](Vec accum, T* body) {return accum + body->mu * body->position;};
		Mass total_mass = std::accumulate(this->bodies.begin(), this->bodies.end(), Mass(0), [](Mass accum, auto body) {return accum + body->mu;});

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
		auto body = std::move(this->bodies.end());
		this->bodies.pop_back();
		bool found = false;

		for (int j = 0; j < 8; j++){
			if (!Node::PointInBounds((*body)->position, subBounds[i]))
				continue;
			this->children[i].bodies.push_back(*body);
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

template <typename T>
void Octree<T>::Node::partition(){
	Bounds bounds = getBounds();
	this->partition(bounds);
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
		Node child = this->children[i];
		child.position = static_cast<NodePosition>(i);
		child.parent = this;
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
		xMax = bounds.second.z;
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

template class Octree<Body>;
