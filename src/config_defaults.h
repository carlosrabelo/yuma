#pragma once

namespace ConfigDefaults {
constexpr const char kPoolHost[] = "public-pool.io";
constexpr int kPoolPort = 21496;
constexpr const char kPoolUser[] = "bc1qw2raw7urfuu2032uyyx9k5pryan5gu6gmz6exm.yuna";
constexpr const char kPoolPass[] = "x";
constexpr int kDifficulty = 1024;
constexpr bool kVardiffEnabled = true;
constexpr int kVardiffTarget = 30;
constexpr int kVardiffMin = 256;
constexpr int kVardiffMax = 16384;
constexpr bool kUseStaticIp = false;
constexpr const char kStaticIp[] = "";
constexpr const char kStaticGateway[] = "";
constexpr const char kStaticSubnet[] = "";
constexpr const char kStaticDns[] = "";
} // namespace ConfigDefaults
