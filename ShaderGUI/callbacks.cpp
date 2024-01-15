#include "callbacks.h"

#include <iostream>
#include <string>

int text_edit_callback(ImGuiInputTextCallbackData* data) {
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
    case ImGuiInputTextFlags_CallbackCompletion:
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
