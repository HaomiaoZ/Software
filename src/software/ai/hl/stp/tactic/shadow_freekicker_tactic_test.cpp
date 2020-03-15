#include "software/ai/hl/stp/tactic/shadow_freekicker_tactic.h"

#include <gtest/gtest.h>

#include "shared/constants.h"
#include "software/ai/intent/move_intent.h"
#include "software/geom/util.h"
#include "software/new_geom/line.h"
#include "software/new_geom/util/distance.h"
#include "software/test_util/test_util.h"

class ShadowFreekickerTacticTest : public testing::Test
{
   protected:
    /**
     * The fixture function to help to create world with enemy robots and calculate distances needed for unit tests
     * @param ball_position: the position of the ball in Point format
     * @param enemy_robot_positions: a vector of Points to represent the position fo robots
     * @param left_or_right: a enum argument in the shadowFreekickerTactic to represent whether the robot is  blocking the freekick from left or right
     */
    void CreateSceneWithEnemyRobot(Point ball_position,
                                   std::vector<Point> enemy_robot_positions,
                                   ShadowFreekickerTactic::FreekickShadower left_or_right)
    {
        //set up world, robots, balls
        world = ::Test::TestUtil::createBlankTestingWorld();
        world = ::Test::TestUtil::setEnemyRobotPositions(world, enemy_robot_positions,
                                                         Timestamp::fromSeconds(0));

        world = ::Test::TestUtil::setBallPosition(world, ball_position,
                                                  Timestamp::fromSeconds(0));
        world = ::Test::TestUtil::setBallVelocity(world, Vector(0, 0),
                                                  Timestamp::fromSeconds(0));

        Robot friendly_robot(0, Point(0, 0), Vector(0, 0), Angle::zero(),
                             AngularVelocity::zero(), Timestamp::fromSeconds(0));
        world.mutableFriendlyTeam().updateRobots({friendly_robot});

        //setup tactic and acquire move action from it
        ShadowFreekickerTactic tactic = ShadowFreekickerTactic(
            left_or_right, world.enemyTeam(), world.ball(), world.field(), false);
        tactic.updateRobot(friendly_robot);

        auto action_ptr = tactic.getNextAction();
        ASSERT_TRUE(action_ptr);

        auto move_action = std::dynamic_pointer_cast<MoveAction>(action_ptr);
        ASSERT_NE(nullptr, move_action);

        // Calculate distance and angle for comparison
        ball_to_friendly_robot_distance =
            (world.ball().position() - move_action->getDestination()).length();

        Line enemy_to_ball_line = Line(world.enemyTeam().getAllRobots().at(0).position(),
                                       world.ball().position());

        middle_line_to_friendly_robot_distance =
            distance(enemy_to_ball_line, move_action->getDestination());

        // assume that only 1 enemy robot is on the field for angle calculation
        enemy_to_ball_angle = (world.ball().position() -
                               world.enemyTeam().getAllRobots().front().position())
                                  .orientation();
        enemy_to_dest_angle = (move_action->getDestination() -
                               world.enemyTeam().getAllRobots().front().position())
                                  .orientation();
    }

    /**
     * The fixture function to help to create world without enemy robots and calculate distances needed for unit tests
     * @param ball_position: the position of the ball in Point format
     * @param left_or_right: a enum argument in the shadowFreekickerTactic to represent whether the robot is  blocking the freekick from left or right
     */
    void CreateSceneWithoutEnemyRobot(
        Point ball_position, ShadowFreekickerTactic::FreekickShadower left_or_right)
    {
        //set up world, robot, balls
        World world = ::Test::TestUtil::createBlankTestingWorld();

        world = ::Test::TestUtil::setBallPosition(world, ball_position,
                                                  Timestamp::fromSeconds(0));
        world = ::Test::TestUtil::setBallVelocity(world, Vector(0, 0),
                                                  Timestamp::fromSeconds(0));
        Robot friendly_robot(0, Point(0, 0), Vector(0, 0), Angle::zero(),
                             AngularVelocity::zero(), Timestamp::fromSeconds(0));
        world.mutableFriendlyTeam().updateRobots({friendly_robot});

        //setup tactic and acquire move action from it
        ShadowFreekickerTactic tactic = ShadowFreekickerTactic(
            left_or_right, world.enemyTeam(), world.ball(), world.field(), false);
        tactic.updateRobot(friendly_robot);
        auto action_ptr = tactic.getNextAction();
        ASSERT_TRUE(action_ptr);
        auto move_action = std::dynamic_pointer_cast<MoveAction>(action_ptr);
        ASSERT_NE(nullptr, move_action);

        // Calculate distance and angle for comparison
        ball_to_friendly_robot_distance =
            (world.ball().position() - move_action->getDestination()).length();

        Line ball_to_goal_line =
            Line(world.ball().position(), world.field().friendlyGoal());

        middle_line_to_friendly_robot_distance =
            distance(ball_to_goal_line, move_action->getDestination());

        goal_to_ball_angle =
            (world.ball().position() - world.field().friendlyGoal()).orientation();
        goal_to_dest_angle =
            (move_action->getDestination() - world.field().friendlyGoal()).orientation();
    }

    World world;

    double ball_to_friendly_robot_distance;
    double ball_to_friendly_robot_distance_abs_error = 0.05;
    // To make the edge of the robot to be slightly more than 0.5m from the ball, the
    // center of the robot should be 0.62 m away from the ball
    double expected_all_to_friendly_robot_distance = 0.62;

    double middle_line_to_friendly_robot_distance;
    double middle_line_to_friendly_robot_distance__abs_error = 0.01;
    //The distance between robot and the middle line is 1.1 times of the robot radius(0.09m)
    double expected_middle_line_to_friendly_robot_distance   = 0.09 * 1.1;

    Angle enemy_to_ball_angle;
    Angle enemy_to_dest_angle;

    Angle goal_to_ball_angle;
    Angle goal_to_dest_angle;
};
//clang-format off
/**
 * The two tests immediate below has the scene shown in the graph below
 *                                                                      Enemy
 *                                                                        X
 *     +-----------------------------------------------------------------X+X-----------------------------------------------------------------+
 *     |                                                                  X                                                                  |
 *     |                                                         ball     |                                                                  |
 *     |                                           Left    +         O    |                                                                  |
 *     |                                                  + +             |                                                                  |
 *     |                                                   +              |                                                                  |
 *     |                                                           +      |                                                                  |
 *     |                                                          + +     |                                                                  |
 *     +--------------+                                  Right     +      |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +-------------------------------------------------------------------------------------------------------------------------------------+
 *
 */
//clang-format on

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_middle_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {
        Point(0, world.field().friendlyCornerPos().y())};
    Point ball_position(-0.09, world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_middle_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {
        Point(0, world.field().friendlyCornerPos().y())};
    Point ball_position(-0.09, world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *
 *
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +--------------+                                                   |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                    Left    +      |                                                   +--------------+
 *     |                                                          + +     |                                                                  |
 *     |                                                           +      |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                    +             |                                                                  |
 *     |                                                   + +   ball     |                                                                  |
 *     |                                          Right     +        O    +                                                                  |
 *     |                                                                  X                                                                  |
 *     +----------------------------------------------------------------+X+X+----------------------------------------------------------------+
 *                                                                        X  Enemy
 */
// clang-format on

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_middle_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {
        Point(0, world.field().friendlyCornerNeg().y())};
    Point ball_position(-0.09, world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_middle_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {
        Point(0, world.field().friendlyCornerNeg().y())};
    Point ball_position(-0.09, world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);
    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);


    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_top_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerPos().x(), world.field().friendlyCornerPos().y())};
    Point ball_position(world.field().friendlyCornerPos().x() - 0.09,
                        world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);


    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}

// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *                                                                                                                                         Enemy
 *                                                                                                                                           X
 *     +------------------------------------------------------------------+-----------------------------------------------------------------X+X
 *     |                                                                  |                                                       ball       X
 *     |                                                                  |                                       Left    +           O      |
 *     |                                                                  |                                              + +                 |
 *     |                                                                  |                                               +      +           |
 *     |                                                                  |                                                     + +          |
 *     |                                                                  |                                                      +           |
 *     |                                                                  |                                                Right             |
 *     +--------------+                                                   |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_top_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerPos().x(), world.field().friendlyCornerPos().y())};
    Point ball_position(world.field().friendlyCornerPos().x() - 0.09,
                        world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_top_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerNeg().x(), world.field().friendlyCornerPos().y())};
    Point ball_position(world.field().friendlyCornerNeg().x() + 0.09,
                        world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);


    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *
 *
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +--------------+                                                   |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                       Left    +   +--------------+
 *     |                                                                  |                                              + +                 |
 *     |                                                                  |                                               +                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                             +         ball       |
 *     |                                                                  |                                   Right    + +            O      |
 *     |                                                                  |                                             +                    |
 *     |                                                                  |                                                                  X   Enemy
 *     +------------------------------------------------------------------+-----------------------------------------------------------------X+X
 *                                                                                                                                           X
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_top_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerNeg().x(), world.field().friendlyCornerPos().y())};
    Point ball_position(world.field().friendlyCornerNeg().x() + 0.09,
                        world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);
    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_bottom_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerPos().x(), world.field().friendlyCornerNeg().y())};
    Point ball_position(world.field().friendlyCornerPos().x() - 0.09,
                        world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *
 *     X Enemy
 *    X+X-----------------------------------------------------------------+------------------------------------------------------------------+
 *     X       ball                                                       |                                                                  |
 *     |      O              +                                            |                                                                  |
 *     |                    + +   Right                                   |                                                                  |
 *     |                     +                                            |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                  +                                               |                                                                  |
 *     |                 + +                                              |                                                                  |
 *     +--------------+   +  Left                                         |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_bottom_left_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerPos().x(), world.field().friendlyCornerNeg().y())};
    Point ball_position(world.field().friendlyCornerPos().x() - 0.09,
                        world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *
 *
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +--------------+                                                   |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                              (0,0)+-------->                                          |              |
 *     |              |                                                                                                       |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                +                                                 |                                                                  |
 *     |               + +   Right                                        |                                                                  |
 *     |                +                                                 |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                    +                                             |                                                                  |
 *     |  ball             + +                                            |                                                                  |
 *     |         O          +  Left                                       |                                                                  |
 *     X                                                                  |                                                                  |
 *    X+X-----------------------------------------------------------------+------------------------------------------------------------------+
 *     X  Enemy
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_botom_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerNeg().x(), world.field().friendlyCornerNeg().y())};
    Point ball_position(world.field().friendlyCornerNeg().x() + 0.09,
                        world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_bottom_right_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(
        world.field().friendlyCornerNeg().x(), world.field().friendlyCornerNeg().y())};
    Point ball_position(world.field().friendlyCornerNeg().x() + 0.09,
                        world.field().friendlyCornerNeg().y() + 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below                                                                               |
 *                                                                                                                                                     X
 *                                                                                                                                                    X+X-------
 *     +------------------------------------------------------------------+------------------------------------------------------------------+         X  Enemy
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +--------------+                                                   |                                                   +--------------+
 *     |              |                                                   +                                                   |              |
 *     |              |                                                    y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                      Left      +          ball    |                                                   |              |
 *     |              |                               + +                 |  Enemy                                            |              |
 *     |              |                                +                  X       x                                           |              |
 *     |              |                                           O      X+X------->                                          |              |
 *     |              |                                +                  X (0,0)                                             |              |
 *     |              |                               + +                 +                                                   |              |
 *     |              |                   Right        +                  |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_with_enemy_robot_at_origin_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(0, 0)};
    Point ball_position(0, -0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(enemy_to_dest_angle, enemy_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_with_enemy_robot_at_origin_on_field)
{
    std::vector<Point> enemy_robot_positions = {Point(0, 0)};
    Point ball_position(0, -0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithEnemyRobot(ball_position, enemy_robot_positions, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the enemy robot and
    // the ball (from the POV of the enemy robot)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(enemy_to_dest_angle, enemy_to_ball_angle);
}
// clang-format off
/**
 *  The two tests immediate below has the scene shown in the graph below
 *
 *
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *     |                                                                  |                                                                  |
 *     |                                               +                  |                                                                  |
 *     |                                              + +           O     |                                                                  |
 *     |                                    Left       +                  |                                                                  |
 *     |                                                           ball   |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                  +               |                                                                  |
 *     +--------------+                                  + +              |                                                   +--------------+
 *     |              |                                   +               +                                                   |              |
 *     |              |                         Right                      y                                                  |              |
 *     |              |                                                   ^                                                   |              |
 *     |  Friendly    |                                                   |                                                   |   Enemy      |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |       x                                           |              |
 *     |              |                                                   +-------->                                          |              |
 *     |              |                                                     (0,0)                                             |              |
 *     |              |                                                   +                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     |              |                                                   |                                                   |              |
 *     +--------------+                                                   |                                                   +--------------+
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     |                                                                  |                                                                  |
 *     +------------------------------------------------------------------+------------------------------------------------------------------+
 *
 */
// clang-format on
TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_left_side_without_robot_in_enemy_team)
{
    Point ball_position(-0.09, world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right = ShadowFreekickerTactic::LEFT;

    CreateSceneWithoutEnemyRobot(ball_position, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the left of the line between the friendly net and
    // the ball (from the POV of the friendly net)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_GT(goal_to_dest_angle, goal_to_ball_angle);
}

TEST_F(ShadowFreekickerTacticTest,
       test_shadow_free_kicker_right_side_without_robot_in_enemy_team)
{
    Point ball_position(-0.09, world.field().friendlyCornerPos().y() - 0.09);
    ShadowFreekickerTactic::FreekickShadower left_or_right =
        ShadowFreekickerTactic::RIGHT;

    CreateSceneWithoutEnemyRobot(ball_position, left_or_right);

    // The edge of the robot should be slightly more than 0.5m from the ball
    EXPECT_NEAR(ball_to_friendly_robot_distance, expected_all_to_friendly_robot_distance,
                ball_to_friendly_robot_distance_abs_error);

    // The robot should be just to the right of the line between the friendly net and
    // the ball (from the POV of the friendly net)
    EXPECT_NEAR(middle_line_to_friendly_robot_distance,
                expected_middle_line_to_friendly_robot_distance,
                middle_line_to_friendly_robot_distance__abs_error);

    EXPECT_LT(goal_to_dest_angle, goal_to_ball_angle);
}
