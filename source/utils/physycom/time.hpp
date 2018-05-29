/* Copyright 2016-2017 - Alessandro Fabbri, Chiara Mizzi, Stefano Sinigardi */

// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt



#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <ctime>

#include <physycom/split.hpp>

// Convert date/time to unix time
size_t date_to_unix(const std::string &date) {
  std::vector<std::string> date_v;
  physycom::split(date_v, date, "- :", physycom::token_compress_off);
  size_t timestamp;
  try {
    struct tm time_now;
    time_now.tm_year = std::stoi(date_v[0]) - 1900;
    time_now.tm_mon = std::stoi(date_v[1]) - 1;
    time_now.tm_mday = std::stoi(date_v[2]);
    time_now.tm_hour = std::stoi(date_v[3]);
    time_now.tm_min = std::stoi(date_v[4]);
    time_now.tm_sec = std::stoi(date_v[5]);
    timestamp = (size_t)mktime(&time_now);
  }
  catch (...) {
    std::cerr << "Invalid date : " << date << std::endl;
    timestamp = 0;
  }
  return timestamp;
}

// Convert unix time to date/time
std::string unix_to_date(const size_t &t_unix)
{
  struct tm * t;
  t = std::localtime((time_t *)&t_unix);

  char tcs[100];
  std::strftime(tcs, sizeof(tcs), "%H:%M %d/%m/%Y", t);

  return std::string(tcs);
}


// Assign time slot
// 1) Auto mode, based on minutes
std::string find_slot_auto_ranges_minutes(const std::string &date, const std::string &time_min = "00:15",const std::string &time_max = "23:59", int dtmin = 30) {     // 3/10/2016 12:14:34
  // extract date components and convert hour to minutes since 00:00:00
  std::vector<std::string> tok;
  physycom::split(tok, date, "- :", physycom::token_compress_off);
  std::stringstream slot;
  slot << tok[0] << tok[1] << tok[2] << "_";
  size_t tmin_now = stoi(tok[3])*60+stoi(tok[4]);

  // convert time_min and time_max to minutes since 00:00:00
  physycom::split(tok, time_min, ":", physycom::token_compress_off);
  size_t tmin_min = stoi(tok[0])*60 + stoi(tok[1]);
  physycom::split(tok, time_max, ":", physycom::token_compress_off);
  size_t tmin_max = stoi(tok[0])*60 + stoi(tok[1]);

  // prepare the output string
  if ( tmin_now >= tmin_min && tmin_now < tmin_max ){
    size_t slot_index = (tmin_now - tmin_min) / dtmin;
    size_t tmin_slot = tmin_min + slot_index*dtmin;

    size_t hi = tmin_slot/60;
    size_t mi = tmin_slot - hi*60;
    size_t hf, mf;
    if (tmin_max > tmin_slot + dtmin) {
      hf = (tmin_slot + dtmin)/60;
      mf = tmin_slot + dtmin - hf*60;
    }
    else {
      hf = tmin_max / 60;
      mf = tmin_max - hf * 60;
    }

    slot << std::setw(2) << std::setfill('0') << hi << "."
      << std::setw(2) << std::setfill('0') << mi << "-"
      << std::setw(2) << std::setfill('0') << hf << "."
      << std::setw(2) << std::setfill('0') << mf;
  }
  else{
    slot << "residuals";
  }

  return slot.str();
}

// 2) Manual mode
std::string find_slot_manual_ranges(const std::string &date) {
  std::string slot;
  std::vector<std::string> date_v;
  physycom::split(date_v, date, "- :", physycom::token_compress_off);
  slot = date_v[0] + date_v[1] + date_v[2] + "_";
  int h = stoi(date_v[3]);
  if (h < 8 || h >= 23) {
    slot += "23-08";
  }
  else if (h >= 8 && h < 12) {
    slot += "08-12";
  }
  else if (h >= 12 && h < 16) {
    slot += "12-16";
  }
  else if (h >= 16 && h < 20) {
    slot += "16-20";
  }
  else if (h >= 20 && h < 23) {
    slot += "20-23";
  }
  else {
    slot += "residuals";     // you should not see this...
  }

  return slot;
}

// 3) Mini manual mode
std::string find_slot_mini_ranges(const std::string &date) {
  std::string slot;
  std::vector<std::string> date_v;
  physycom::split(date_v, date, "- :", physycom::token_compress_off);
  int h = stoi(date_v[3]);
  if (h >= 7 && h < 12) {
    slot = "07-12";
  }
  else if (h >= 12 && h < 17) {
    slot = "12-17";
  }
  else if (h >= 17 && h < 22) {
    slot = "17-22";
  }
  else {
    slot = "night";
  }

  return slot;
}

// Returns a vector of string with the given ranges
// E.G. "10.00-10.30" "10.30-11.00" ...
std::vector<std::string> get_slot_auto_ranges_minutes(const std::string &time_min = "00:15", const std::string &time_max = "23:59", int dtmin = 30)
{
  std::vector<std::string> tok;
  physycom::split(tok, time_min, ":", physycom::token_compress_off);
  int tmin_min = stoi(tok[0]) * 60 + stoi(tok[1]);
  physycom::split(tok, time_max, ":", physycom::token_compress_off);
  int tmin_max = stoi(tok[0]) * 60 + stoi(tok[1]);

  // prepare the output string
  std::vector<std::string> slots;
  int slot_num = (tmin_max - tmin_min) / dtmin;
  for (int i = 0; i<slot_num; ++i)
  {
    int tmin_slot = tmin_min + i*dtmin;
    int hi = tmin_slot / 60;
    int mi = tmin_slot - hi * 60;
    int hf, mf;
    if (tmin_max > tmin_slot + dtmin)
    {
      hf = (tmin_slot + dtmin) / 60;
      mf = tmin_slot + dtmin - hf * 60;
    }
    else
    {
      hf = tmin_max / 60;
      mf = tmin_max - hf * 60;
    }

    std::stringstream slot;
    slot
      << std::setw(2) << std::setfill('0') << hi << "."
      << std::setw(2) << std::setfill('0') << mi << "-"
      << std::setw(2) << std::setfill('0') << hf << "."
      << std::setw(2) << std::setfill('0') << mf;
    slots.push_back(slot.str());
  }

  return slots;
}
