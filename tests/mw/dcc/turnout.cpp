#include "dcc_test.hpp"
#include "mw/dcc/loco.hpp"

TEST_F(DccTest, turnout_to_json) {
  mw::dcc::Turnout turnout;
  turnout.name = "North";
  turnout.position = z21::TurnoutInfo::Position::P1;
  turnout.group = {
    .addresses = {13u},
    .states = {{z21::TurnoutInfo::Position::P0, z21::TurnoutInfo::Position::P0},
               {z21::TurnoutInfo::Position::P0, z21::TurnoutInfo::Position::P1},
               {z21::TurnoutInfo::Position::P1, z21::TurnoutInfo::Position::P0},
               {z21::TurnoutInfo::Position::P1,
                z21::TurnoutInfo::Position::P1}},
  };
  auto doc{turnout.toJsonDocument()};
  std::string json;
  json.reserve(1024uz);
  serializeJson(doc, json);
  std::cout << json << "\n";
  EXPECT_EQ(
    json,
    R"({"name":"North","mode":0,"position":2,"type":0,"group":{"addresses":[13],"states":[[1,1],[1,2],[2,1],[2,2]]}})");
}

TEST_F(DccTest, json_to_turnout) {
  std::string json{
    R"({"name":"North","mode":0,"position":2,"type":0,"group":{"addresses":[13],"states":[[1,1],[1,2],[2,1],[2,2]]}})"};
  JsonDocument doc;
  deserializeJson(doc, json);
  mw::dcc::Turnout turnout{doc};
  EXPECT_EQ(turnout.name, "North");
  EXPECT_EQ(turnout.group.addresses, decltype(turnout.group.addresses){13u});
  EXPECT_EQ(
    turnout.group.states,
    (decltype(turnout.group.states){
      {z21::TurnoutInfo::Position::P0, z21::TurnoutInfo::Position::P0},
      {z21::TurnoutInfo::Position::P0, z21::TurnoutInfo::Position::P1},
      {z21::TurnoutInfo::Position::P1, z21::TurnoutInfo::Position::P0},
      {z21::TurnoutInfo::Position::P1, z21::TurnoutInfo::Position::P1}}));
}
