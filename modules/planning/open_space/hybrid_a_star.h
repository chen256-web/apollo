/******************************************************************************
 * Copyright 2018 The Apollo Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

/*
 * @file
 */

/*
 * Inspired by
 *
 * Dolgov, Dmitri, et al. “Path Planning for Autonomous Vehicles in Unknown
 * Semi-Structured Environments.” The International Journal of Robotics
 * Research, vol. 29, no. 5, 2010, pp. 485–501., doi:10.1177/0278364909359210.
 *
 * Xiaojing, et al. "Optimization-Based Collision Avoidance" (arXiv:1711.03449)
 * and its implementation (https://github.com/XiaojingGeorgeZhang/H-OBCA).
 */

#pragma once

#include <map>
#include <memory>
#include <queue>
#include <unordered_map>
#include <utility>
#include <vector>

#include "modules/planning/open_space/node3d.h"
#include "modules/planning/open_space/reeds_shepp_path.h"

#include "cyber/common/log.h"
#include "cyber/common/macros.h"
#include "modules/common/configs/proto/vehicle_config.pb.h"
#include "modules/common/configs/vehicle_config_helper.h"
#include "modules/common/math/math_utils.h"
#include "modules/common/time/time.h"
#include "modules/planning/common/obstacle.h"
#include "modules/planning/common/planning_gflags.h"
#include "modules/planning/proto/planner_open_space_config.pb.h"

namespace apollo {
namespace planning {

using apollo::common::Status;

struct HybridAStartResult {
  std::vector<double> x;
  std::vector<double> y;
  std::vector<double> phi;
  std::vector<double> v;
  std::vector<double> a;
  std::vector<double> steer;
};

class HybridAStar {
 public:
  explicit HybridAStar(const PlannerOpenSpaceConfig& open_space_conf);
  virtual ~HybridAStar() = default;
  bool Plan(double sx, double sy, double sphi, double ex, double ey,
            double ephi, const std::vector<double>& XYbounds,
            const std::vector<std::vector<common::math::Vec2d>>&
                obstacles_vertices_vec,
            HybridAStartResult* result);

 private:
  bool AnalyticExpansion(std::shared_ptr<Node3d> current_node,
                         const std::vector<std::vector<common::math::Vec2d>>&
                             obstacles_vertices_vec);
  bool ReedSheppHeuristic(std::shared_ptr<Node3d> current_node,
                          std::shared_ptr<ReedSheppPath> reeds_shepp_to_end);
  // check collision and validity
  bool ValidityCheck(std::shared_ptr<Node3d> node,
                     const std::vector<std::vector<common::math::Vec2d>>&
                         obstacles_vertices_vec);
  // check Reeds Shepp path collision and validity
  bool RSPCheck(const std::shared_ptr<ReedSheppPath> reeds_shepp_to_end,
                const std::vector<std::vector<common::math::Vec2d>>&
                    obstacles_vertices_vec);
  // load the whole RSP as nodes and add to the close set
  std::shared_ptr<Node3d> LoadRSPinCS(
      const std::shared_ptr<ReedSheppPath> reeds_shepp_to_end,
      std::shared_ptr<Node3d> current_node);
  std::shared_ptr<Node3d> Next_node_generator(
      std::shared_ptr<Node3d> current_node, size_t next_node_index);
  void CalculateNodeCost(
      std::shared_ptr<Node3d> current_node, std::shared_ptr<Node3d> next_node,
      const std::shared_ptr<ReedSheppPath> reeds_shepp_to_end);
  double HeuristicCost();
  double HoloObstacleHeuristic();
  double NonHoloNoObstacleHeuristic(
      const std::shared_ptr<ReedSheppPath> reeds_shepp_to_end);
  double CalculateRSPCost(
      const std::shared_ptr<ReedSheppPath> reeds_shepp_to_end);
  bool GetResult(HybridAStartResult* result);
  bool GenerateSpeedAcceleration(HybridAStartResult* result);

 private:
  PlannerOpenSpaceConfig planner_open_space_config_;
  common::VehicleParam vehicle_param_ =
      common::VehicleConfigHelper::GetConfig().vehicle_param();
  size_t next_node_num_ = 0;
  double max_steer_ = 0.0;
  double step_size_ = 0.0;
  double xy_grid_resolution_ = 0.0;
  double back_penalty_ = 0.0;
  double gear_switch_penalty_ = 0.0;
  double steer_penalty_ = 0.0;
  double steer_change_penalty_ = 0.0;
  double delta_t_ = 0.0;
  std::vector<double> XYbounds_;
  std::shared_ptr<Node3d> start_node_;
  std::shared_ptr<Node3d> end_node_;
  std::shared_ptr<Node3d> final_node_;
  struct cmp {
    bool operator()(const std::pair<size_t, double>& left,
                    const std::pair<size_t, double>& right) const {
      return left.second >= right.second;
    }
  };
  std::priority_queue<std::pair<size_t, double>,
                      std::vector<std::pair<size_t, double>>, cmp>
      open_pq_;
  std::unordered_map<size_t, std::shared_ptr<Node3d>> open_set_;
  std::unordered_map<size_t, std::shared_ptr<Node3d>> close_set_;
  std::unordered_map<size_t, std::shared_ptr<ReedSheppPath>>
      ReedSheppPath_cache_;
  std::unique_ptr<ReedShepp> reed_shepp_generator_;
};

}  // namespace planning
}  // namespace apollo
