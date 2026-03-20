#include "octree.hpp"
#include <algorithm>
#include <numeric>

// template <typename T>
// class Octree {
// 	static_assert(std::is_same<decltype(T::position), Vec>::value && (std::is_same< decltype(T::mass), float>::value || std::is_same< decltype(T::mass), double>::value));
//
//
// 	class Node {
// 		decltype(T::mass) mass;
// 		size_t depth;
// 		Node parent;
// 		Node childrens[8];
// 		std::vector<T> bodies;
// 	};
//
// 	public:
// 	Octree();
// 	void rearrange();
// 	void addBodies(T bodies...);
// };



template <typename T>
Octree<T>::Octree(std::vector<T> &bodies){
	std::pair<double, double> xBound = std::minmax(bodies, [](const T &a, const T &b) {return a.position.x < b.position.x;});
	std::pair<double, double> yBound = std::minmax(bodies, [](const T &a, const T &b) {return a.position.y < b.position.y;});
	std::pair<double, double> zBound = std::minmax(bodies, [](const T &a, const T &b) {return a.position.z < b.position.z;});

	// update the bounds
	this->bounds.first = Vec(xBound.first, yBound.first, zBound.first);
	this->bounds.second = Vec(xBound.second, yBound.second, zBound.second);
	this->root = Node {.bodies = std::vector<T*>(bodies.size()) , .depth = 0, .tree = this};
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
	if (this->bodies->size() <= tree->maxBodyPerNode){

		auto begin = this->bodies.first();
		auto end = this->bodies.end();


		Mass total_mass = std::accumulate(begin, end, Mass(0), [](Mass accum, auto body) {return accum + body->mass;});
		Vec center_mass = std::accumulate(begin, end, Vec(0),
				[](auto body, Vec accum) {return accum + body->mass * body->position;}) / (this->bodies->size());
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
			if (!Node::PointInBounds(body->position, subBounds[i]))
				continue;
			this->children[i].bodies.push_back(body);
			found = true;
			break;
		}

		// just making sure that the thing found a parent else that'll be pretty bad.
		assert(found);
	}

	for (int i = 0; i < 8; i++){
		this->children[i].partition(subBounds[i]);
	}



	

}

template <typename T>
void Octree<T>::Node::partition(){
	partition(getBounds());
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
	if (this->children == nullptr)
		return;
	for (int i = 0; i < 8; i++){
		Node child = this->children[i];
		child.deleteChildren();
	}

	delete []children;
	this->children = nullptr;
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
