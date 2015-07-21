
#pragma once

//
// スコア計算
//

#include "nn_gameenv.hpp"

namespace ngs {

const float score_konahen = 10.0f;  
const float score_just    = 2.0f; 

void ScoreKonahen(GameEnv& env)
{
  float score = score_konahen + static_cast<float>(env.just) * score_just;
  env.score += static_cast<int>(score);
}


struct RankInfo {
  picojson::object& params;
  int konahen;
  int just;
  int just_max;
  int miss;
  float tap_num;
  float time_total;
};

const float rank_max = 15.0f;

int ScoreToRank(const RankInfo& info)
{
  float rank = info.konahen * info.params["rank_konahen"].get<double>();
  DOUT << "Rank:" << rank;
  if (info.konahen > 0)
  {
    float just_rate = (float)info.just / (float)info.konahen;
    just_rate = just_rate * just_rate * info.params["rank_just"].get<double>();
    DOUT << " Just:" << just_rate;
    rank += just_rate;
    
    float just_combo_rate = (float)info.just_max / (float)info.konahen;
    just_combo_rate = just_combo_rate * just_combo_rate * info.params["rank_just_combo"].get<double>();
    DOUT << " JustCombo:" << just_combo_rate;
    rank += just_combo_rate;

    float miss_rate = info.miss * info.params["rank_miss"].get<double>();
    DOUT << " Miss:" << miss_rate;
    rank += miss_rate;
    
    float tap_avg = (info.tap_num / (float)info.konahen - 1.0f) * info.params["rank_tap"].get<double>();
    DOUT << " Tap:" << tap_avg;
    rank += tap_avg;
    
    float time_avg = (info.time_total / (float)info.konahen - 3.0f) * info.params["rank_time"].get<double>();
    DOUT << " Time:" << time_avg;
    rank += time_avg;
  }
  DOUT << " Rank:" << rank << std::endl;

  if (rank < 0.0f) rank = 0.0f;
  if (rank > rank_max) rank = rank_max;
  // 最大・最小チェック
  return rank;
}

}
