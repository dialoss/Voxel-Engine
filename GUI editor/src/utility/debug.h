#pragma once
#include <iostream>
#include "glm/glm.hpp"

namespace Debug {
	static int cnt = 0;
	static double maxTime = 0;
	static int div = 0;
	template <class T>
	static void log(T value) {
		std::cout << value << '\n';
	}
	static void vec(glm::vec3 &a) {
		std::cout << a.x << ' ' << a.y << ' ' << a.z << '\n';
	}
	static int count() {
		cnt++;
		return cnt;
	}
	static double update(double t) {
		div++;
		maxTime += t;
		return maxTime / div;
	}
}
