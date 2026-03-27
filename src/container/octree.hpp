#ifndef OCTREE_CLASS_H

#define OCTREE_CLASS_H

#include <stdint.h>
#include "../sim.hpp"

/**
 * @brief Represents the Octree class to implement barnes-hut algorithm
 * @tparam T Represents a body that has a position and a mass
 *
 * Holds the pointer of each body then update the bodies motions using an technique.
 *
 * @important This class is not meant to be used for a long time. The typical implementation is that the tree is created then the nodes are updated, after this the tree is considered invalid. Then you need to recreate another tree to update the nodes again.For this reason it only uses pointer and doesn't own any memory so to avoid invalid pointer the tree should live as little as possible.
*/
template <typename T>
class Octree {
	static_assert(std::is_same<decltype(T::position), Vec>::value && (std::is_same< decltype(T::mu), float>::value || std::is_same< decltype(T::mu), double>::value) && (std::is_constructible_v<Vec, double> || std::is_constructible_v<Vec, float>));

	using Mass     = decltype(T::mu);
	typedef std::pair<Vec, Vec> Bounds;


	enum class NodePosition {
		UP_TOP_LEFT = 0,
		UP_TOP_RIGHT = 1,
		UP_BOTTOM_LEFT = 2,
		UP_BOTTOM_RIGHT = 3,
		BOTTOM_TOP_LEFT = 4,
		BOTTOM_TOP_RIGHT = 5,
		BOTTOM_BOTTOM_LEFT = 6,
		BOTTOM_BOTTOM_RIGHT = 7,
	};

	struct Node {
		// the total mass inside the node
		Mass mass = 0;
		// the center of mass of the node
		Vec center_of_mass = Vec(0.0);

		NodePosition position;

		Octree<T>* tree;
		// the current depth of this node how many parent does it haves
		size_t depth = 0;
		// The node parent
		Node* parent = nullptr;
		// The node childrens
		Node* children = nullptr;
		// The bodies that are contains in this code (Only for leaf nodes)
		std::vector<T*> bodies;


		void partition(const Bounds&);
		void partition();
		Bounds getBounds();
		void deleteChildren();
		void initChildren();

		private:
		static bool PointInBounds(const Vec& point, const Bounds& bounds);
		static Bounds GetSubBound(const NodePosition &position, const Bounds& bounds);
		bool isLeafNode();
		bool isRootNode();
	};

	// the number of bodies in this octree
	size_t size;
	// The root node Node* root = nullptr;
	Node* root;
	// maximum bodies per leaf nodes	
	unsigned int maxBodyPerNode = 5;

	// The current bounds of this tree
	Bounds bounds;
	public:
	~Octree();
	Octree(std::vector<T> &bodies);
	/**
	 * Rearrange the tree when it's internals node have been changed 
	 */
	void rearrange();
	/**
	 * @brief Addss the following bodies to the octree.
	 * This method manage the items
	 * @param bodies The bodies to add to the tree.
	 */
	
};

#endif
