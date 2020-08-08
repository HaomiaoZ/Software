#pragma once

#include <map>
#include <set>

#include "software/ai/navigator/path_planner/path_planner.h"

/**
 * ThetaStarPathPlanner uses the theta * algorithm to implement
 * the PathPlanner interface.
 * It is a grid-based graph algorithm, but it allows any two cells to be connected to each
 * other, as long as there's line of sight. It will optimize for the shortest total path
 * length. Theta Star (Theta*) is closely related to A Star (A*) (see
 * https://www.geeksforgeeks.org/a-search-algorithm/), but it will implicitly smooth paths
 * as it explores the graph.
 *
 * Read
 * https://web.archive.org/web/20190218161704/http://aigamedev.com/open/tutorial/theta-star-any-angle-paths/
 * for an explanation of how that works, including pseudocode and diagrams.
 */

class ThetaStarPathPlanner : public PathPlanner
{
   public:
    ThetaStarPathPlanner();

    /**
     * Returns a path that is an optimized path between start and end.
     *
     * @param start start point
     * @param end end point
     * @param navigable_area Rectangle representing the navigable area
     * @param obstacles obstacles to avoid
     *
     * @return a vector of points that is the optimal path avoiding obstacles
     *         if no valid path then return empty vector
     */
    std::optional<Path> findPath(const Point &start, const Point &end,
                                 const Rectangle &navigable_area,
                                 const std::vector<ObstaclePtr> &obstacles) override;


    class Coordinate
    {
       public:
        Coordinate(unsigned int row, unsigned int col): row_(row), col_(col)

        {
        }

        Coordinate():row_(0), col_ (0) {}

        unsigned int row(void) const
        {
            return row_;
        }

        unsigned int col(void) const
        {
            return col_;
        }

    private:
        unsigned int row_;
        unsigned int col_;

    };
private:
    class CellHeuristic
    {
       public:
        CellHeuristic()
            : parent_(0, 0),
              start_to_end_cost_estimate_(0),
              best_path_cost_(0),
              initialized_(false)
        {
        }

        /**
         * Updates CellHeuristics internal variables
         * Once updated, a CellHeuristic is considered intialized
         *
         * @param parent parent
         * @param start_to_end_cost_estimate The start to end_cost estimate
         * @param best_path_cost best_path_cost
         */
        void update(const Coordinate &parent, double start_to_end_cost_estimate,
                    double best_path_cost)
        {
            parent_                     = parent;
            start_to_end_cost_estimate_ = start_to_end_cost_estimate;
            best_path_cost_             = best_path_cost;
            initialized_                = true;
        }

        /**
         * Checks if this is initialized
         *
         * @return true if CellHeuristic is initialized
         */
        bool isInitialized(void) const
        {
            return initialized_;
        }

        /**
         * Gets start to end cost estimate, which is the sum of the best known real path
         * cost from the cell to start plus the Euclidean distance between the cell and
         * end
         *
         * @return start_to_end_cost_estimate
         */
        double pathCostAndEndDistHeuristic() const
        {
            return start_to_end_cost_estimate_;
        }

        /**
         * Gets best_path_cost, the best path cost from the cell to end
         *
         * @return the best path cost
         */
        double bestPathCost() const
        {
            return best_path_cost_;
        }

        /**
         * Gets parent Coordinate
         *
         * @return parent
         */
        Coordinate parent() const
        {
            return parent_;
        }

       private:
        Coordinate parent_;
        double start_to_end_cost_estimate_;
        double best_path_cost_;
        bool initialized_;
    };

    /**
     * Returns whether or not a cell is within bounds of grid
     *
     * @param coord Coordinate to consider
     *
     * @return true if cell is navigable
     */
    bool isCoordNavigable(const Coordinate &coord) const;

    /**
     * Returns whether or not a cell is unblocked
     *
     * @param coord Coordinate to consider
     *
     * @return true if cell is unblocked
     */
    bool isUnblocked(const Coordinate &coord);

    /**
     * Computes Euclidean distance from coord1 to coord2
     *
     * @param coord1 The first Coordinate
     * @param coord2 The second Coordinate
     *
     * @return distance between coord1 to coord2
     */
    double coordDistance(const Coordinate &coord1, const Coordinate &coord2) const;

    /**
     * Traces a path from the end back to the start
     * and populates a vector of points with that path
     *
     * @param end end cell
     *
     * @return vector of points with the path from start to end
     */
    std::vector<Point> tracePath(const Coordinate &end) const;

    /**
     * Updates the next cell's fields based on the current cell, end and the
     * distance to the next cell and checks if end is reached
     *
     * @param current The current cell
     * @param next    The next cell to be updated
     * @param end     The end cell
     *
     * @return true if next is the end
     */
    bool updateVertex(const Coordinate &current, const Coordinate &next,
                      const Coordinate &end);

    /**
     * Checks for line of sight between Coordinates
     *
     * @param coord1 The first Coordinate
     * @param coord2 The second Coordinate
     *
     * @return true if line of sight from coord1 to coord2
     */
    bool lineOfSight(const Coordinate &coord1, const Coordinate &coord2);

    /**
     * Finds closest unblocked cell to current_cell
     *
     * @param current_cell  current cell
     *
     * @return          closest unblocked cell to current_cell
     *                  if none found, return nullopt
     */
    std::optional<Coordinate> findClosestUnblockedCell(const Coordinate &current_cell);

    /**
     * Finds closest navigable point that's not in an obstacle to p
     *
     * @param p     a given point
     *
     * @return          closest free point to currCell
     *                  if not blocked then return p
     */
    Point findClosestFreePoint(const Point &p);

    /**
     * Checks if a point is navigable and doesn't exist in any obstacles
     *
     * @param p     a given point
     *
     * @return      if p is navigable and isn't in an obstacle
     * */
    bool isPointNavigableAndFreeOfObstacles(const Point &p);

    /**
     * Checks if a point is navigable
     *
     * @param p     a given point
     *
     * @return      if p is navigable
     * */
    bool isPointNavigable(const Point &p) const;

    /**
     * Converts a cell in grid to a point on navigable area
     *
     * @param coord Coordinate to convert
     *
     * @return Point on navigable area
     */
    Point convertCoordToPoint(const Coordinate &coord) const;

    /**
     * Converts a point on navigable area to a cell in grid
     *
     * @param p point on navigable area
     *
     * @return cell in grid
     */
    Coordinate convertPointToCoord(const Point &p) const;

    /**
     * Try to find a path to end and leave
     * trail markers along the way
     *
     * @param end_coord end coordinates
     *
     * @return true if path to end was found
     */
    bool findPathToEnd(const Coordinate &end_coord);

    /**
     * Update vertex for all neighbours (all 8 directions) of current_coord
     *
     * @param current_coord The current coordinate
     * @param end_coord end coordinates
     *
     * @return true if path to end was found among successors
     */
    bool visitNeighbours(const Coordinate &current_coord, const Coordinate &end_coord);

    /**
     * Adjusts start and end points for navigability and determines if a path cannot be
     * found
     *
     * If start or end are non-navigable, then no path exists
     *
     * If start or end are blocked by obstacle(s), then try to find a nearby point that
     * isn't blocked
     * - If a point can be found then set the endpoint to that nearest point
     * - If no nearby points are unblocked, then no path exists
     *
     * @param [in/out] start_coord source coordinate
     * @param [in/out] end_coord end coordinate
     *
     * @return true if there is no path to end
     */
    bool adjustEndPointsAndCheckForNoPath(Coordinate &start_coord, Coordinate &end_coord);

    /**
     * Resets and initializes member variables to prepare for planning a new path
     *
     * @param navigable_area Rectangle representing the navigable area
     * @param obstacles obstacles to avoid
     */
    void resetAndInitializeMemberVariables(const Rectangle &navigable_area,
                                           const std::vector<ObstaclePtr> &obstacles);

    /**
     * Computes a key from the given Coordinate for collision-free access to
     * unblocked_grid
     *
     * @param coord Coordinate to compute
     *
     * @return key for the Coordinate
     */
    unsigned long computeMapKey(const Coordinate &coord) const;

    /**
     * Computes a key from the given pair of Coordinates for collision-free access to
     * line_of_sight_cache
     * @param coord1 Coordinate 1 of pair to hash
     * @param coord2 Coordinate 2 of pair to hash
     *
     * @return key for the Coordinate pair
     */
    unsigned long computeMapKey(const ThetaStarPathPlanner::Coordinate &coord1,
                                const ThetaStarPathPlanner::Coordinate &coord2) const;

    // if close to end then return direct path to end point
    static constexpr double CLOSE_TO_END_THRESHOLD = 0.01;  // in metres

    // TODO(Issue #1448): Test and revise this value and the associated approach
    // increase in threshold to reduce oscillation for when the end of the path is blocked
    static constexpr unsigned int BLOCKED_END_OSCILLATION_MITIGATION =
        2;  // multiples of CLOSE_TO_END_THRESHOLD to ignore to control oscillation

    // resolution for searching for unblocked point around a blocked end
    static constexpr double BLOCKED_END_SEARCH_RESOLUTION =
        50.0;  // number of fractions to divide 1m

    const double SIZE_OF_GRID_CELL_IN_METERS =
        ROBOT_MAX_RADIUS_METERS;  // this is the n in the O(n^2) algorithm :p

    std::vector<ObstaclePtr> obstacles;
    unsigned int num_grid_rows;
    unsigned int num_grid_cols;
    double max_navigable_x_coord;
    double max_navigable_y_coord;

    // open_list represents Coordinates that we'd like to visit. Elements are pairs of
    // start_to_end_cost_estimate and Coordinate, so the set is implicitly ordered by
    // start_to_end_cost_estimate (and then by Coordinate to break ties). This ensures
    // that open_list.begin() is the Coordinate with the lowest start_to_end_cost_estimate
    std::set<std::pair<double, Coordinate>> open_list;

    // closed_list represent coords we've already visited so
    // it contains coords for which we calculated the CellHeuristic
    std::set<Coordinate> closed_list;

    // Declare a 2D array of structure to hold the details of that CellHeuristic
    std::vector<std::vector<CellHeuristic>> cell_heuristics;

    // The following data structures improve performance by caching the results of
    // isUnblocked and lineOfSight.
    // They are indexed with an unsigned long key for performance
    // Description of the Grid-
    // true --> The cell is not blocked
    // false --> The cell is blocked
    // We update this as we go to avoid updating cells we don't use
    std::map<unsigned long, bool> unblocked_grid;
    // Cache of line of sight that maps a unsigned long key computed from a pair of
    // coordinates to whether those two Coordinates have line of sight between them
    std::map<unsigned long, bool> line_of_sight_cache;
};

bool operator ==(const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return coord1.row()==coord2.row() && coord1.col()== coord2.col();
}

bool operator > (const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return coord1.col()>coord2.col()||(coord1.col()==coord2.col()&& coord1.row()>coord2.row());
}

bool operator < (const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return !(coord1>coord2||coord1==coord2);
}

bool operator !=(const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return !(coord1==coord2);
}

bool operator >= (const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return !(coord1<coord2);
}

bool operator <= (const ThetaStarPathPlanner::Coordinate &coord1, const ThetaStarPathPlanner::Coordinate &coord2)
{
    return !(coord1>coord2);
}

