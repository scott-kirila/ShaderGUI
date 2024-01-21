#include "callbacks.h"

#include <iostream>
#include <list>
#include <string>

int text_edit_callback(ImGuiInputTextCallbackData* data) {

    static std::list<std::string> commands_list = {
            "vec2", "vec3", "vec4",
            "int", "float",
            "sin", "cos",
            "sampler2D", "texture", "uniform",
            "ViewportSize",
            "gl_FragCoord",
    };
    static std::list<std::string> candidates_list{};

    static char* s_word_start{ nullptr };
    static char* s_word_end{ nullptr };
    static int s_length{};

    switch (data->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackResize:
    {
        auto user_data = (std::string*)data->UserData;
        IM_ASSERT(user_data->c_str() == data->Buf);
        user_data->resize(data->BufTextLen);
        data->Buf = (char*)user_data->c_str();
        break;
    }
    case ImGuiInputTextFlags_CallbackEdit:
    {
        candidates_list.clear();

        // Locate beginning of current word
        s_word_end = data->Buf + data->CursorPos;
        s_word_start = s_word_end;

        while (s_word_start > data->Buf)
        {
            const char c = s_word_start[-1];
            if (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == ';')
                break;
            s_word_start--;
        }

        s_length = (int)(s_word_end - s_word_start);
        std::cout << s_length << "\n";
        if (s_length == 0) break;

        // Build a list of candidates
        for (const auto& command : commands_list) {
            if (command.compare(0, s_length, s_word_start, s_length) == 0)
                candidates_list.push_back(command);
        }

        std::cout << "Edit callback: " << candidates_list.size() << "\n";

        break;
    }
    //case ImGuiInputTextFlags_CallbackAlways:
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
            std::cout << "Callback triggered!\n";
            if (candidates_list.size() != 0)
                std::cout << candidates_list.front() << "\n";
        }


        /*
        std::list<const char*> commands = {
            "vec2", "vec3", "vec4",
            "int", "float",
            "sin", "cos",
            "sampler2D", "texture", "uniform",
            "ViewportSize",
            "gl_FragCoord",
        };

        ImVector<const char*> Commands;
        
        for (const auto& command : commands)
            Commands.push_back(command);

        // Locate beginning of current word
        const char* word_end = data->Buf + data->CursorPos;
        const char* word_start = word_end;
        while (word_start > data->Buf)
        {
            const char c = word_start[-1];
            if (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == ';')
                break;
            word_start--;
        }

        // Build a list of candidates
        ImVector<const char*> candidates;
        for (int i = 0; i < Commands.Size; i++)
            if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                candidates.push_back(Commands[i]);
        */
        if (candidates_list.size() == 0)
        {
            // No match
            //AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
        }
        else if (candidates_list.size() == 1 && s_word_end != nullptr && s_word_start != nullptr)
        {
            // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
            data->DeleteChars((int)(s_word_start - data->Buf), s_length);
            data->InsertChars(data->CursorPos, candidates_list.front().c_str());
            data->InsertChars(data->CursorPos, " ");

            s_word_end = nullptr;
            s_word_start = nullptr;
            s_length = 0;
        }
        else
        {
            std::cout << candidates_list.size() << "\n";
            // Multiple matches. Complete as much as we can..
            // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
            /*
            int match_len = (int)(s_word_end - s_word_start);
            for (;;)
            {
                int c = 0;
                bool all_candidates_matches = true;
                for (int i = 0; i < candidates_list.size() && all_candidates_matches; i++)
                    if (i == 0)
                        c = toupper(candidates[i][match_len]);
                    else if (c == 0 || c != toupper(candidates[i][match_len]))
                        all_candidates_matches = false;
                if (!all_candidates_matches)
                    break;
                match_len++;
            }

            if (match_len > 0)
            {
                data->DeleteChars((int)(s_word_start - data->Buf), (int)(s_word_end - s_word_start));
                data->InsertChars(data->CursorPos, candidates_list.front().c_str(), candidates_list.front().c_str() + match_len);
            }
            */

            // List matches
            //AddLog("Possible matches:\n");
            //for (int i = 0; i < candidates.Size; i++)
            //    AddLog("- %s\n", candidates[i]);

            //ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y));
            //ImGui::SetNextWindowSize(ImVec2(200, 400));
            ImGui::SetTooltip("Suggestions");
            if (ImGui::BeginTooltip())
            {
                for (const auto& candidate : candidates_list)
                    ImGui::TextUnformatted(candidate.c_str());
                ImGui::EndTooltip();
            }
        }
        break;
    }
    }

    return 0;
}

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

void AddLog(const char* fmt, ...) IM_FMTARGS(2)
{
    ImVector<char*> Items;

    // FIXME-OPT
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    Items.push_back(Strdup(buf));
}

int text_completion_callback(ImGuiInputTextCallbackData* data)
{
    std::list<const char*> commands = {
        "vec2", "vec3", "vec4",
        "int", "float",
        "sin", "cos",
        "texture",
    };

    ImVector<const char*> Commands;

    for (const auto& command : commands)
        Commands.push_back(command);

    // Locate beginning of current word
    const char* word_end = data->Buf + data->CursorPos;
    const char* word_start = word_end;
    while (word_start > data->Buf)
    {
        const char c = word_start[-1];
        if (c == ' ' || c == '\t' || c == ',' || c == ';')
            break;
        word_start--;
    }

    // Build a list of candidates
    ImVector<const char*> candidates;
    for (int i = 0; i < Commands.Size; i++)
        if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
            candidates.push_back(Commands[i]);

    if (candidates.Size == 0)
    {
        // No match
        AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
    }
    else if (candidates.Size == 1)
    {
        // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
        data->InsertChars(data->CursorPos, candidates[0]);
        data->InsertChars(data->CursorPos, " ");
    }
    else
    {
        // Multiple matches. Complete as much as we can..
        // So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
        int match_len = (int)(word_end - word_start);
        for (;;)
        {
            int c = 0;
            bool all_candidates_matches = true;
            for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                if (i == 0)
                    c = toupper(candidates[i][match_len]);
                else if (c == 0 || c != toupper(candidates[i][match_len]))
                    all_candidates_matches = false;
            if (!all_candidates_matches)
                break;
            match_len++;
        }

        if (match_len > 0)
        {
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
        }

        // List matches
        AddLog("Possible matches:\n");
        for (int i = 0; i < candidates.Size; i++)
            AddLog("- %s\n", candidates[i]);
    }

    return 0;
}
