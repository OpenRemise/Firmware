#include "dcc/loco.hpp"
#include "dcc_test.hpp"

TEST_F(DccTest, loco_to_base_to_json) {
  dcc::Loco loco;
  loco.name = "BR85";
  loco.speed_steps = z21::LocoInfo::DCC28;
  auto doc{static_cast<dcc::NvLocoBase&>(loco).toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);
  EXPECT_EQ(json, "{\"name\":\"BR85\",\"mode\":0,\"speed_steps\":2}");
}

TEST_F(DccTest, json_to_base_to_loco) {
  std::string json{"{\"name\":\"BR85\",\"speed_steps\":2}"};
  JsonDocument doc;
  deserializeJson(doc, json);
  dcc::NvLocoBase base;
  base.fromJsonDocument(doc);
  dcc::Loco loco;
  dynamic_cast<dcc::NvLocoBase&>(loco) = base;
  EXPECT_EQ(loco.name, "BR85");
  EXPECT_EQ(loco.speed_steps, z21::LocoInfo::DCC28);
}

TEST_F(DccTest, loco_to_json) {
  dcc::Loco loco;
  loco.name = "Reihe 2190";
  loco.rvvvvvvv = 1u << 7u | 42u;
  loco.f31_0 = 1u << 3u | 1u << 1u;
  auto doc{loco.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);
  EXPECT_EQ(
    json,
    "{\"name\":\"Reihe "
    "2190\",\"mode\":0,\"speed_steps\":4,\"rvvvvvvv\":170,\"f31_0\":10}");
}

TEST_F(DccTest, json_to_loco) {
  std::string json{
    "{\"name\":\"Reihe "
    "2190\",\"mode\":0,\"speed_steps\":4,\"rvvvvvvv\":170,\"f31_0\":10}"};
  JsonDocument doc;
  deserializeJson(doc, json);
  dcc::Loco loco;
  loco.fromJsonDocument(doc);
  EXPECT_EQ(loco.name, "Reihe 2190");
  EXPECT_EQ(loco.speed_steps, z21::LocoInfo::DCC128);
  EXPECT_EQ(loco.rvvvvvvv, 170u);
  EXPECT_EQ(loco.f31_0, 10u);
}