#ifndef MAS_BVTREE_H
#define MAS_BVTREE_H

#include "mas/core/base.h"
#include "mas/concurrency/thread.h"
#include <vector>
#include <memory>

namespace mas {
namespace bvtree {

// things that can be bounded
class Boundable;

// Abstract bounding volume object
class BoundingSphere;
// Bounding Sphere
class BoundingBox;
// Abstract Bounding Box
class AABB;
// Axis-Aligned Bounding Box
class OBB;
// Oriented Bounding Box

// Definitions
// Objects that can be bounded
class Boundable {
public:
   // Update supplied BV using info from THIS
   template<typename BV>
   bool updateBV(BV& bv) const;

   virtual void getCentroid(Point3d& c) const = 0;

   virtual double distanceToPoint(const Point3d& pnt,
         Point3d& nearest) const = 0;
   virtual double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const = 0;
};

// Definitions
// objects that can be bounded
/**
 * Set of points to be bounded, points stored by value (does not allow points
 * to be shared between sets)
 */
template<typename Point3d>
class BoundablePointSet {
public:
   size_t idx;
   std::vector<Point3d> pnts;

public:
   BoundablePointSet(size_t idx);
   BoundablePointSet(std::vector<Point3d> pnts, size_t idx);

   size_t getIndex() const;
   void setIndex(size_t idx);

   void setPoints(std::vector<Point3d> pnts);
   void addPoint(Point3d pnt);

   template<typename BV>
   bool updateBV(BV& bv) const;

   void getCentroid(Point3d& c) const;
   void getCovariance(const Point3d& centre, Matrix3d& cov) const;

   // closest point
   double distanceToPoint(const Point3d& pnt, Point3d& nearest) const;

   // always inf
   double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const;
};

/**
 * Set of points to be bounded, points stored by pointer (allows points shared between sets)
 */
template<typename PointPtr>
class BoundablePointPtrSet {
public:
   size_t idx;
   std::vector<PointPtr> pnts;
public:
   BoundablePointPtrSet(size_t idx);
   BoundablePointPtrSet(std::vector<PointPtr> pnts, size_t idx);

   size_t getIndex() const;
   void setIndex(size_t idx);

   void setPoints(std::vector<PointPtr> pnts);

   void addPoint(PointPtr pnt);

   template<typename BV>
   bool updateBV(BV& bv) const;

   void getCentroid(Point3d& c) const;
   void getCovariance(const Point3d& centre, Matrix3d& cov) const;

   // closest point
   double distanceToPoint(const Point3d& pnt, Point3d& nearest) const;

   // always inf
   double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const;
};

// volumes
class BoundingVolume {
protected:
   double margin;
protected:
   BoundingVolume();
   BoundingVolume(double margin);

public:
   // margin to maintain around bounded volume
   virtual void setMargin(double m) = 0;
   virtual double getMargin() const;

   virtual bool intersectsPoint(const Point3d& p) const = 0;
   virtual bool intersectsSphere(const Point3d& c, double r) const = 0;
   virtual bool intersectsLine(const Point3d& p, const Vector3d& v) const = 0;
   virtual bool intersectsRay(const Point3d& p, const Vector3d& v) const = 0;
   virtual bool intersectsPlane(const Plane& p) const = 0;

   // Visitor pattern
   // Dispatch routines, if unknown pass on to bv.intersectsVisitor(*this)
   template<typename BV>
   bool intersects(const BV& bv) const;

   virtual double distanceToPoint(const Point3d& pnt,
         Point3d& nearest) const = 0;
   virtual double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const = 0;

   // Useful for adding default behaviour for new types
   // if not explicitly implemented
   virtual BoundingSphere getBoundingSphere() const = 0;
   virtual double getBoundingSphere(Point3d& centre) const = 0;

   // Update to include supplied info into this volume
   virtual bool updatePoint(const Point3d& p) = 0;
   virtual bool updateSphere(const Point3d& c, double r) = 0;

   // Visitor pattern, if unknown boundable pass on to Boundable
   template<typename BoundablePtr>
   bool update(const BoundablePtr& b);

   // Bound a set of boundables, can use centroid and covariance
   template<typename BoundablePtr>
   void bound(const std::vector<BoundablePtr>& b); // shared boundables

   // Split into smaller groups for inserting into a tree
   template<typename BoundablePtr>
   bool split(std::vector<BoundablePtr> b, // move shared boundables
         std::vector<std::vector<BoundablePtr>> out) const;

};

class BoundingSphere {
private:
   double margin;
public:
   double r;
   Point3d c;

public:
   BoundingSphere(double margin = 0);
   BoundingSphere(const BoundingSphere& copyMe);
   BoundingSphere(const Point3d& c, double r, double margin = 0);

   void set(const Point3d& c, double r);
   void setRadius(double r);
   double getRadius() const;
   void setCentre(const Point3d& c);
   void getCentre(Point3d& c) const;

   void setMargin(double m);
   double getMargin();

   bool intersectsPoint(const Point3d& p) const;
   bool intersectsSphere(const Point3d& c, double r) const;
   bool intersectsLine(const Point3d& p, const Vector3d& v) const;
   bool intersectsRay(const Point3d& p, const Vector3d& v) const;
   bool intersectsPlane(const Plane& p) const;

   // Overrides to skip visitor (does intersectsSphere instead)
   template<typename BV>
   bool intersects(const BV& bv) const;

   double distanceToPoint(const Point3d& pnt, Point3d& nearest) const;
   double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const;

   BoundingSphere getBoundingSphere() const;
   double getBoundingSphere(Point3d& centre) const;

   bool updatePoint(const Point3d& p);
   bool updateSphere(const Point3d& c, double r);

   // Bound a set of boundables, can use centroid and covariance
   template<typename BoundablePtr>
   void bound(const std::vector<BoundablePtr>& b); // shared boundables

   // Split into smaller groups for inserting into a tree
   template<typename BoundablePtr>
   bool split(std::vector<BoundablePtr> b, // move shared boundables
         std::vector<std::vector<BoundablePtr>>& out) const;

};

class BoundingBox {
private:
   double margin;
   static int boxCorners[8][3];
public:
   Point3d c;
   Vector3d halfWidths;

public:
   BoundingBox(double margin = 0);
   BoundingBox(const BoundingBox& copyMe);
   BoundingBox(const Point3d& c, const Vector3d& hw, double margin = 0);

   void set(const Point3d& c, const Vector3d& hw);
   void setHalfWidths(const Vector3d& hw);
   void getHalfWidths(Vector3d& hw) const;
   void setCentre(const Point3d& c);
   void getCentre(Point3d& c) const;

   double getMargin();
   void setMargin(double m);

   // computes a corner, 0 <= idx < 8
   virtual void getCorner(int idx, Point3d& pnt) const;

   virtual void getLocalCoords(const Point3d& p, Point3d& out) const = 0;
   virtual void getLocalCoords(const Vector3d& v, Vector3d& out) const = 0;
   virtual void getWorldCoords(const Point3d& p, Point3d& out) const = 0;
   virtual void getWorldCoords(const Vector3d& p, Vector3d& out) const = 0;

   bool intersectsPoint(const Point3d& p) const;
   bool intersectsSphere(const Point3d& c, double r) const;
   bool intersectsLine(const Point3d& p, const Vector3d& v) const;
   bool intersectsRay(const Point3d& p, const Vector3d& v) const;
   bool intersectsPlane(const Plane& p) const;

   double distanceToPoint(const Point3d& pnt, Point3d& nearest) const;
   double distanceToPoint(const Point3d& pnt, const Vector3d& dir,
         Point3d& nearest) const;

   BoundingSphere getBoundingSphere() const;
   double getBoundingSphere(Point3d& centre) const;

   bool updatePoint(const Point3d& p);
   bool updateSphere(const Point3d& c, double r);

};

class AABB: public BoundingBox {

public:
   AABB(double margin = 0);
   AABB(const AABB& copyMe);
   AABB(const Point3d& c, const Vector3d& hw, double margin = 0);

   virtual void getLocalCoords(const Point3d& p, Point3d& out) const;
   virtual void getLocalCoords(const Vector3d& v, Vector3d& out) const;
   virtual void getWorldCoords(const Point3d& p, Point3d& out) const;
   virtual void getWorldCoords(const Vector3d& p, Vector3d& out) const;

   template<typename BV>
   bool intersects(const BV& bv) const;

   // Bound a set of boundables, can use centroid and covariance
   template<typename BoundablePtr>
   void bound(const std::vector<BoundablePtr>& b); // shared boundables

   // Split into smaller groups for inserting into a tree
   // Split along longest axis
   template<typename BoundablePtr>
   bool split(std::vector<BoundablePtr> b, // move shared boundables
         std::vector<std::vector<BoundablePtr>>& out) const;

};

// explicit specializations
template<>
bool AABB::intersects(const BoundingSphere& bv) const;
template<>
bool AABB::intersects(const AABB& bv) const;
template<>
bool AABB::intersects(const OBB& bv) const;

class OBB: public BoundingBox {
public:
   RotationMatrix3d R;

public:
   OBB(double margin = 0);
   OBB(const OBB& copyMe);
   OBB(const AABB& copyMe);
   OBB(const Point3d& c, const RotationMatrix3d& R, const Vector3d& hw);
   OBB(const RigidTransform3d& trans, const Vector3d& hw);

   void set(const Point3d& c, const RotationMatrix3d& R, const Vector3d& hw);
   void set(const RigidTransform3d& trans, const Vector3d& hw);
   void setRotation(const RotationMatrix3d& R);
   void getRotation(RotationMatrix3d& R);

   virtual void getLocalCoords(const Point3d& p, Point3d& out) const;
   virtual void getLocalCoords(const Vector3d& v, Vector3d& out) const;
   virtual void getWorldCoords(const Point3d& p, Point3d& out) const;
   virtual void getWorldCoords(const Vector3d& p, Vector3d& out) const;

   template<typename BV>
   bool intersects(const BV& bv) const;

   // Bound a set of boundables, can use centroid and covariance
   template<typename BoundablePtr>
   void bound(const std::vector<BoundablePtr>& b); // shared boundables

   // Split into smaller groups for inserting into a tree
   // Split along longest axis
   template<typename BoundablePtr>
   bool split(std::vector<BoundablePtr> b, // move shared boundables
         std::vector<std::vector<BoundablePtr>>& out) const;

private:
   static bool boxesIntersect(const Vector3d& hw1, const Vector3d& hw2,
         const RotationMatrix3d& R1, const RotationMatrix3d& R2,
         const Vector3d& pd, const Vector3d& px);
   static bool boxesIntersect(const Vector3d& hw1, const Vector3d& hw2,
         const RotationMatrix3d& R21, const Vector3d& t21);
};

// explicit specializations
template<>
bool OBB::intersects(const BoundingSphere& bv) const;
template<>
bool OBB::intersects(const AABB& bv) const;
template<>
bool OBB::intersects(const OBB& bv) const;


// basic abstract binary node
template<typename BoundablePtr, typename BV>
class BVNode {
public:
    typedef std::shared_ptr<BVNode<BoundablePtr, BV>> SharedBVNode;
private:
   BVNode<BoundablePtr, BV>* parent; // raw parent node (so can observe strong or weak parent)

public:
   size_t idx;
   std::unique_ptr<BV> bv;
   std::vector<BoundablePtr> elems;
   std::vector<SharedBVNode> children;

private:
   BVNode(const BVNode<BoundablePtr, BV>& copyMe) = delete;
   BVNode& operator=(const BVNode<BoundablePtr, BV>& assignMe) = delete;

public:
   BVNode(double margin = 0);

   // move elements
   BVNode(std::vector<BoundablePtr> elems, double margin = 0);

   void setIndex(size_t idx);
   size_t getIndex();

   BVNode<BoundablePtr, BV>* getParent();
   void setParent(BVNode<BoundablePtr, BV>* parent);

   BoundingSphere getBoundingSphere() const;
   double getBoundingSphere(Point3d& centre) const;

   BV& getBoundingVolume();

   std::vector<BoundablePtr>& getElements() const;
   void setElements(std::vector<BoundablePtr> elems);

   size_t numElements() const;
   void clearElements();

   std::vector<std::shared_ptr<BVNode<BoundablePtr, BV>>>& getChildren();
   void setChildren(std::vector<std::shared_ptr<BVNode<BoundablePtr,BV>>>&& children);

   size_t numChildren() const;
   void clearChildren();
   void clear();

   bool isLeaf() const;
   bool isRoot() const;

   void setMargin(double margin);
   double getMargin() const;

   // split elements into more branches
   bool grow();
   bool growRecursively();

   void updateBounds();
   void updateBoundsUp(const BoundablePtr& b);

protected:
   BVNode<BoundablePtr,BV>* spawnChild(std::vector<BoundablePtr>&& elems);

};

// Abstract BINARY Bounding-volume tree
template<typename BoundablePtr, typename BV>
class BVTree {
public:
   typedef BVNode<BoundablePtr, BV> BVNodeType;

private:
   double margin;
   std::vector<std::shared_ptr<BVNodeType>> nodes;
   size_t leavesIdx;
   size_t nleaves;

private:
   BVTree(const BVTree<BoundablePtr, BV>& copyMe) = delete;
   BVTree<BoundablePtr, BV>& operator=(const BVTree<BoundablePtr, BV>& assignMe) = delete;

public:
   BVTree(double margin = 0);
   BVTree(std::vector<BoundablePtr> elems, double margin = 0);

   BVNodeType& getRoot() const;
   double getRadius() const;

   // margin around objects, for robustness
   void setMargin(double margin);
   double getMargin() const;

   void build(std::vector<BoundablePtr> elems, double margin = 0);

   void parallel_build(std::vector<BoundablePtr> elems, double margin = 0, size_t maxThreads = 0);

   // intersection, return number of leaves
   size_t intersectPoint(const Point3d& p,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   size_t intersectPoint(const Point3d& p, std::vector<BVNodeType*>& out) const;

   size_t intersectSphere(const Point3d& c, double r,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   size_t intersectSphere(const Point3d& c, double r,
         std::vector<BVNodeType*>& out) const;

   size_t intersectLine(const Point3d& p, const Vector3d& dir,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   size_t intersectLine(const Point3d& p, const Vector3d& dir,
         std::vector<BVNodeType*>& out) const;

   size_t intersectRay(const Point3d& p, const Vector3d& dir,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   size_t intersectRay(const Point3d& p, const Vector3d& dir,
         std::vector<BVNodeType*>& out) const;

   size_t intersectPlane(const Plane& plane,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   size_t intersectPlane(const Plane& plane,
         std::vector<BVNodeType*>& out) const;

   template<typename BV2>
   size_t intersectBV(const BV2& bv,
         std::vector<std::shared_ptr<BVNodeType>>& out) const;
   template<typename BV2>
   size_t intersectBV(const BV2& bv, std::vector<BVNodeType*>& out) const;

   template<typename BoundablePtr2, typename BV2>
   size_t intersectTree(const BVTree<BoundablePtr2, BV2>& tree,
         std::vector<std::shared_ptr<BVNodeType>>& mine,
         std::vector<std::shared_ptr<BVNode<BoundablePtr2, BV2>>>& hers) const;

   template<typename BoundablePtr2, typename BV2>
   size_t intersectTree(const BVTree<BoundablePtr2,BV2>& tree,
   std::vector<BVNodeType*>& mine,
   std::vector<BVNode<BoundablePtr2,BV2>*>& hers) const;

   size_t getLeaves(std::vector<std::shared_ptr<BVNodeType>>& leaves);
   size_t getLeaves(std::vector<BVNodeType*>& leaves);

   size_t numLeaves();
   BVNodeType& getLeaf(size_t leafIdx);

   size_t numNodes();
   BVNodeType& getNode(size_t nodeIdx);

   void update();
   void parallel_update();

protected:


   std::shared_ptr<BVNodeType> recursive_build(
           size_t& nextNodeIdx, size_t& nextLeafIdx, std::vector<BoundablePtr>&& elems);
   std::shared_ptr<BVNodeType> parallel_recursive_build(
              std::atomic<size_t>& nextNodeIdx, std::atomic<size_t>& nextLeafIdx, std::vector<BoundablePtr>&& elems,
              mas::concurrency::async_thread_pool &pool);

   // intersection, return number of leaves
   void intersectPointRecursively(const Point3d& p,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   void intersectPointRecursively(const Point3d& p,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   void intersectSphereRecursively(const Point3d& c, double r,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   void intersectSphereRecursively(const Point3d& c, double r,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   void intersectLineRecursively(const Point3d& p, const Vector3d& dir,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   void intersectLineRecursively(const Point3d& p, const Vector3d& dir,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   void intersectRayRecursively(const Point3d& p, const Vector3d& dir,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   void intersectRayRecursively(const Point3d& p, const Vector3d& dir,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   void intersectPlaneRecursively(const Plane& plane,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   void intersectPlaneRecursively(const Plane& plane,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   template<typename BV2>
   void intersectBVRecursively(const BV2& bv,
   std::vector<std::shared_ptr<BVNodeType>>& out, const std::shared_ptr<BVNodeType>& node) const;
   template<typename BV2>
   void intersectBVRecursively(const BV2& bv,
   std::vector<BVNodeType*>& out, BVNodeType* node) const;

   template<typename BoundablePtr2, typename BV2>
   void intersectTreeRecursively(const std::shared_ptr<BVNodeType>& me,
   const std::shared_ptr<BVNode<BoundablePtr2,BV2>>& her, std::vector<std::shared_ptr<BVNodeType>>& mine,
   std::vector<std::shared_ptr<BVNode<BoundablePtr2,BV2>>>& hers) const;
   template<typename BoundablePtr2, typename BV2>
   void intersectTreeRecursively(BVNodeType* me, BVNode<BoundablePtr2,BV2>* her,
   std::vector<BVNodeType*>& mine,
   std::vector<BVNode<BoundablePtr2,BV2>*>& hers) const;

   void getLeavesRecursively(std::vector<std::shared_ptr<BVNodeType>>& leaves,
   const std::shared_ptr<BVNodeType>& node) const;
   void getLeavesRecursively(std::vector<BVNodeType*>& leaves,
   BVNodeType* node) const;

};

template <typename BoundablePtr, typename BV>
class BVTreeUpdater {
public:
	static void update(BVTree<BoundablePtr,BV>* tree);
	static void parallel_update(BVTree<BoundablePtr,BV>* tree);
};

// static routines
template<typename BoundablePtr, typename BV>
BoundablePtr nearest_boundable(const BVTree<BoundablePtr, BV>& bvh,
      const Point3d& p, Point3d& nearestPoint);

template<typename BoundablePtr, typename BV>
BoundablePtr nearest_boundable(const BVTree<BoundablePtr, BV>& bvh,
      const Point3d& p, const Vector3d& dir, Point3d& nearestPoint);

}
}

#include "mas/bvtree/bvtree.hpp"

#endif

