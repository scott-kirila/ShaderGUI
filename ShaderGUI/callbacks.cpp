#include "callbacks.h"

#include <iostream>
#include <string>

int text_resize_callback(ImGuiInputTextCallbackData* data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		auto user_data = (std::string*)data->UserData;
		IM_ASSERT(user_data->c_str() == data->Buf);
		user_data->resize(data->BufTextLen);
		data->Buf = (char*)user_data->c_str();
	}

	return 0;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}