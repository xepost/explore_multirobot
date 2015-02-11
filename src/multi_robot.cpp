/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2014, Zhi Yan.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Zhi Yan nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************/

#include <explore/multi_robot.h>

using namespace explore;

MultiRobot::MultiRobot(ros::NodeHandle &nh, ros::NodeHandle &private_nh) {
  tf_prefix_ = tf::getPrefixParam(private_nh);
  
  external_map_received_ = false;
  private_nh.param("external_map_topic", external_map_topic_, std::string("map"));
  external_map_subsriber_ = nh.subscribe(external_map_topic_, 1, &MultiRobot::externalMapCallback, this);
  
  prev_pose_known_ = false;
  distance_traveled_ = 0;
  actualcost_publisher_ = private_nh.advertise<std_msgs::Float64>("explore_actualcost", 1, true);
  
  estimatedcost_publisher_ = private_nh.advertise<std_msgs::Float64>("explore_estimatedcost", 1, true);
  
  private_nh.param("inscribed_scale", inscribed_scale_, 1.0);
}

MultiRobot::~MultiRobot() {
}

void MultiRobot::externalMapCallback(const nav_msgs::OccupancyGrid::ConstPtr &msg) {
  external_map_ = *msg;
  external_map_received_ = true;
}

void MultiRobot::updateMap(costmap_2d::Costmap2DROS &costmap) {
  if(external_map_received_) {
    costmap.updateStaticMap(external_map_);
    costmap.updateMap();
    external_map_received_ = false;
  }
}

void MultiRobot::actualCost(const geometry_msgs::PoseStamped &pose) {
  if(!prev_pose_known_) {
    prev_pose_ = pose;
    prev_pose_known_ = true;
  } else {
    double dx = prev_pose_.pose.position.x - pose.pose.position.x;
    double dy = prev_pose_.pose.position.y - pose.pose.position.y;
    double dist = sqrt(dx*dx+dy*dy);
    distance_traveled_ += dist;
    std_msgs::Float64 actual_cost;
    actual_cost.data = distance_traveled_;
    actualcost_publisher_.publish(actual_cost);
    prev_pose_ = pose;
  }
}

void MultiRobot::estimatedCost(const geometry_msgs::PoseStamped &goal, const geometry_msgs::PoseStamped &pose) {
  double dx = goal.pose.position.x - pose.pose.position.x;
  double dy = goal.pose.position.y - pose.pose.position.y;
  std_msgs::Float64 estimated_cost;
  estimated_cost.data = sqrt(dx*dx+dy*dy);
  estimatedcost_publisher_.publish(estimated_cost.data);
}