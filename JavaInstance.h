#pragma once

struct JavaInstance {
	JavaClass* class_info;
	JavaInstance(JavaClass* p_class_info): class_info(p_class_info) {}
};
