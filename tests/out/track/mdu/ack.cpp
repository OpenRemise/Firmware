#include <gtest/gtest.h>
#include <mdu/mdu.hpp>
#include "mn180.hpp"
#include "ms450.hpp"
#include "ms491.hpp"
#include "ms990.hpp"
#include "out/track/mdu/task_function.hpp"

using namespace out::track::mdu;

TEST(mdu, ack1) {
  {
    mdu_encoder_config_t encoder_config{
      .transfer_rate = 0u,
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };

    {
      auto [s, e]{ack1_indices(encoder_config, size(mn180.tf0_ch1))};
      EXPECT_EQ(s, 10817uz);
      EXPECT_EQ(e, 11267uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(mn180.tf0_ch2))};
      EXPECT_EQ(s, 10875uz);
      EXPECT_EQ(e, 11325uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(mn180.tf0_ch12))};
      EXPECT_EQ(s, 10417uz);
      EXPECT_EQ(e, 10867uz);
    }
  }

  {
    mdu_encoder_config_t encoder_config{
      .transfer_rate = 1u,
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };
    {
      auto [s, e]{ack1_indices(encoder_config, size(ms990.tf1_ch1))};
      EXPECT_EQ(s, 109uz);
      EXPECT_EQ(e, 116uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(ms990.tf1_ch2))};
      EXPECT_EQ(s, 112uz);
      EXPECT_EQ(e, 119uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(ms990.tf1_ch12))};
      EXPECT_EQ(s, 106uz);
      EXPECT_EQ(e, 113uz);
    }
  }

  {
    mdu_encoder_config_t encoder_config{
      .transfer_rate = 4u,
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };

    {
      auto [s, e]{ack1_indices(encoder_config, size(ms450.tf4_ch1))};
      EXPECT_EQ(s, 691uz);
      EXPECT_EQ(e, 719uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(ms450.tf4_ch2))};
      EXPECT_EQ(s, 696uz);
      EXPECT_EQ(e, 724uz);
    }

    {
      auto [s, e]{ack1_indices(encoder_config, size(ms450.tf4_ch12))};
      EXPECT_EQ(s, 667uz);
      EXPECT_EQ(e, 695uz);
    }
  }
}

TEST(mdu, ack2) {
  {
    mdu_encoder_config_t encoder_config{
      .transfer_rate = 0u,
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };

    {
      auto [s, e]{ack2_indices(encoder_config, size(mn180.tf0_ch1))};
      EXPECT_EQ(s, 11417uz);
      EXPECT_EQ(e, 11867uz);
    }

    {
      auto [s, e]{ack2_indices(encoder_config, size(mn180.tf0_ch2))};
      EXPECT_EQ(s, 11475uz);
      EXPECT_EQ(e, 11925uz);
    }

    {
      auto [s, e]{ack2_indices(encoder_config, size(mn180.tf0_ch12))};
      EXPECT_EQ(s, 11017uz);
      EXPECT_EQ(e, 11467uz);
    }
  }

  {
    mdu_encoder_config_t encoder_config{
      .transfer_rate = 3u,
      .num_preamble = 14u,
      .num_ackreq = 10u,
    };

    {
      auto [s, e]{ack2_indices(encoder_config, size(ms491.tf3_ch1))};
      EXPECT_EQ(s, 396uz);
      EXPECT_EQ(e, 411uz);
    }

    {
      auto [s, e]{ack2_indices(encoder_config, size(ms491.tf3_ch2))};
      EXPECT_EQ(s, 398uz);
      EXPECT_EQ(e, 413uz);
    }

    {
      auto [s, e]{ack2_indices(encoder_config, size(ms491.tf3_ch12))};
      EXPECT_EQ(s, 383uz);
      EXPECT_EQ(e, 398uz);
    }
  }
}