#ifndef OPENHD_WIFI_H
#define OPENHD_WIFI_H

#include <fstream>
#include <string>

#include "openhd_platform.h"
#include "openhd_util.h"
#include "openhd_util_filesystem.h"
#include "validate_settings_helper.h"
#include "wifi_channel.h"

// After discovery, the capabilities of a WiFi-Card are immutable !

enum class WiFiCardType {
  OPENHD_RTL_88X2AU,  // Supported
  OPENHD_RTL_88X2BU,  // Supported
  OPENHD_RTL_88X2CU,  // Supported
  // These are all unsupported, but might / might not work
  RTL_88X2AU,  // right card, but wrong driver
  RTL_88X2BU,  // right card, but wrong driver
  ATHEROS,     // not supported anymore
  MT_7921u,    // TODO add support
  RALINK,
  INTEL,
  BROADCOM,
  OPENHD_RTL_8852BU,  // testing phase
  OPENHD_EMULATED,
  AIC,
  UNKNOWN
};

static constexpr uint8_t WIFI_CARD_SUB_TYPE_UNKNOWN = 0;
static constexpr uint8_t WIFI_CARD_SUB_TYPE_RTL8812AU_ASUS = 1;
static constexpr uint8_t WIFI_CARD_SUB_TYPE_RTL8812AU_X20 = 2;

static std::string wifi_card_type_to_string(const WiFiCardType& card_type) {
  switch (card_type) {
    case WiFiCardType::OPENHD_RTL_88X2AU:
      return "OPENHD_RTL_88X2AU";
    case WiFiCardType::OPENHD_RTL_88X2BU:
      return "OPENHD_RTL_88X2BU";
    case WiFiCardType::OPENHD_RTL_88X2CU:
      return "OPENHD_RTL_88X2CU";
    case WiFiCardType::OPENHD_RTL_8852BU:
      return "OPENHD_RTL_8852BU";
    case WiFiCardType::RTL_88X2AU:
      return "RTL_88X2AU";
    case WiFiCardType::RTL_88X2BU:
      return "RTL_88X2BU";
    case WiFiCardType::ATHEROS:
      return "ATHEROS";
    case WiFiCardType::MT_7921u:
      return "MT_7921u";
    case WiFiCardType::RALINK:
      return "RALINK";
    case WiFiCardType::INTEL:
      return "INTEL";
    case WiFiCardType::BROADCOM:
      return "BROADCOM";
    case WiFiCardType::AIC:
      return "AIC";
    case WiFiCardType::UNKNOWN:
    default:
      return "UNKNOWN";
  }
}
static int wifi_card_type_to_int(const WiFiCardType& card_type) {
  return static_cast<int>(card_type);
}

static std::string wifi_card_sub_type_as_string(uint8_t sub_type) {
  if (sub_type == WIFI_CARD_SUB_TYPE_UNKNOWN) {
    return "UNKNOWN";
  }
  if (sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_ASUS) {
    return "ASUS";
  } else if (sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_X20) {
    return "X20";
  }
  return "ERROR";
}

struct WiFiCard {
  // These 3 are all (slightly different) identifiers of a card on linux.
  std::string device_name;
  std::string mac;
  // phy0, phy1,.., needed for iw commands that don't take the device name
  int phy80211_index = -1;
  // Name of the driver that runs this card.
  std::string driver_name;
  // Detected wifi card type, generated by checking known drivers.
  WiFiCardType type = WiFiCardType::UNKNOWN;
  // More info about a given wifi card - e.g. for rtl8812au, which
  // manufacturer produced the card (required for TX power levels)
  uint8_t sub_type = 0;
  [[nodiscard]] bool supports_2GHz() const {
    return !supported_frequencies_2G.empty();
  };
  [[nodiscard]] bool supports_5GHz() const {
    return !supported_frequencies_5G.empty();
  };
  // Returns true if the given card is supported by openhd (proper hardware &
  // proper driver)
  [[nodiscard]] bool supports_openhd_wifibroadcast() const {
    return type == WiFiCardType::OPENHD_RTL_88X2AU ||
           type == WiFiCardType::OPENHD_RTL_88X2BU ||
           type == WiFiCardType::OPENHD_RTL_88X2CU ||
           type == WiFiCardType::OPENHD_EMULATED;
  };
  // Returns true if the given card is exatly rtl8812au on x20 (custom HW) and
  // we know power levels ;)
  [[nodiscard]] bool is_openhd_rtl8812au_x20() const {
    return type == WiFiCardType::OPENHD_RTL_88X2AU &&
           sub_type == WIFI_CARD_SUB_TYPE_RTL8812AU_X20;
  };
  // supported 2G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_2G{};
  // supported 5G frequencies, in mhz
  std::vector<uint32_t> supported_frequencies_5G{};
  [[nodiscard]] std::vector<uint32_t> get_supported_frequencies_2G_5G() const {
    std::vector<uint32_t> ret{};
    OHDUtil::vec_append(ret, supported_frequencies_2G);
    OHDUtil::vec_append(ret, supported_frequencies_5G);
    return ret;
  };
};

// Only RTL8812au / BU / CU support changing the MCS index
static bool wifi_card_supports_variable_mcs(const WiFiCard& wifi_card) {
  if (wifi_card.type == WiFiCardType::OPENHD_EMULATED) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2BU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2CU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_8852BU) return true;
  return false;
}
// Only RTL8812au / BU so far support a 40Mhz channel width during injection
static bool wifi_card_supports_40Mhz_channel_width_injection(
    const WiFiCard& wifi_card) {
  if (wifi_card.type == WiFiCardType::OPENHD_EMULATED) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2AU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2BU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_88X2CU) return true;
  if (wifi_card.type == WiFiCardType::OPENHD_RTL_8852BU) return true;
  return false;
}

static bool wifi_card_supports_frequency(const WiFiCard& wifi_card,
                                         const uint32_t frequency) {
  const auto channel_opt = openhd::channel_from_frequency(frequency);
  if (!channel_opt.has_value()) {
    openhd::log::get_default()->debug("OpenHD doesn't know frequency {}",
                                      frequency);
    return false;
  }
  const auto& channel = channel_opt.value();
  for (const auto& supported_frequency :
       wifi_card.get_supported_frequencies_2G_5G()) {
    if (channel.frequency == supported_frequency) {
      return true;
    }
  }
  openhd::log::get_default()->debug("Card {} does not support frequency {}",
                                    wifi_card.device_name, frequency);
  return false;
}

static bool wifi_card_supports_frequency_channel_width(
    const WiFiCard& wifi_card, const int frequency, const int channel_width) {
  auto console = openhd::log::get_default();
  const auto channel_opt = openhd::channel_from_frequency(frequency);
  if (!channel_opt.has_value()) {
    console->debug("OpenHD doesn't know frequency {}", frequency);
    return false;
  }
  const auto& channel = channel_opt.value();
  // card (rtl8812au / bu) will crash otherwise anyways
  if (channel_width == 40 && !channel.is_legal_any_country_40Mhz) {
    console->debug("Card {} doesn't support 40Mhz on {}", wifi_card.device_name,
                   frequency);
    return false;
  }
  return wifi_card_supports_frequency(wifi_card, frequency);
}

static std::string debug_cards(const std::vector<WiFiCard>& cards) {
  std::stringstream ss;
  ss << "size:" << cards.size() << "{";
  for (const auto& card : cards) {
    ss << card.device_name << ",";
  }
  ss << "}";
  return ss.str();
}

void write_wificards_manifest(const std::vector<WiFiCard>& cards);

#endif