// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <jni.h>
#include <iostream>

#include <string>
using std::string;


#include <vector>
using std::vector;

#include <memory>
using std::shared_ptr;
using std::make_shared;

#include <sstream>
#include <fstream>

#include <type_traits>


#include <boost/algorithm/string.hpp>

#include <boost/optional.hpp>
using boost::optional;

#include <set>
#include <map>

#include <filesystem>

namespace fs = std::experimental::filesystem;

