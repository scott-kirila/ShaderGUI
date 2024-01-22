#include "callbacks.h"

#include <iostream>
#include <list>
#include <string>
#include <vector>

int text_edit_callback(ImGuiInputTextCallbackData* data) {

    const std::list<std::string> commandsList = {
            "vec2", "vec3", "vec4",
            "int", "float",
            "sin", "cos",
            "sampler2D", "texture", "uniform",
            "ViewportSize",
            "gl_FragCoord",
    };
    static std::vector<std::string> s_candidatesList{};

    static char* s_wordStart{ nullptr };
    static char* s_wordEnd{ nullptr };
    static int s_length{};

    static int s_currentItemIndex = 0;
    static int s_selectedCandidate = -1;

    static bool s_candidateListOpen = false;

    static bool s_autocomplete = false;

    static int s_linePos{};

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
        s_candidatesList.clear();

        // Locate beginning of current word
        s_wordEnd = data->Buf + data->CursorPos;
        s_wordStart = s_wordEnd;

        while (s_wordStart > data->Buf)
        {
            const char c = s_wordStart[-1];
            if (c == ' ' || c == '\t' || c == '\n' || c == ',' || c == ';')
                break;
            s_wordStart--;
        }

        s_length = (int)(s_wordEnd - s_wordStart);
        if (s_length == 0) break;

        // Build a list of candidates
        for (const auto& command : commandsList) {
            if (command.compare(0, s_length, s_wordStart, s_length) == 0)
                s_candidatesList.push_back(command);
        }

        std::cout << "Edit callback: " << s_candidatesList.size() << "\n";
        break;
    }
    case ImGuiInputTextFlags_CallbackCharFilter:
    {
        const char c = data->EventChar;

        if (s_candidatesList.size() >= 1) {
            if (c == '\t') {
                s_autocomplete = true;
                return 1;
            }
        }

        break;
    }
    case ImGuiInputTextFlags_CallbackAlways:
    {
        /*
        char* temp_line_end = data->Buf + data->CursorPos;
        char* temp_line_start = temp_line_end;
        while (temp_line_start > data->Buf) {
            const char c = temp_line_start[-1];
            if (c == '\n') break;
            temp_line_start--;
        }
        s_line_pos = (int)(temp_line_end - temp_line_start);
        */

        // ImGuiIO& io = ImGui::GetIO();
        const int cursorPos = data->CursorPos;

        if (s_candidatesList.size() >= 2) {

            ImGui::SetNextWindowPos(
                ImVec2(ImGui::GetItemRectMax().x - 10 * ImGui::CalcTextSize(" ").x, ImGui::GetItemRectMin().y + ImGui::GetWindowHeight())
            );
            //ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x + ImGui::CalcTextSize(" ").x * (s_line_pos - s_length), ImGui::GetItemRectMax().y));

            if (ImGui::BeginTooltip()) {
                s_candidateListOpen = true;

                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                    s_currentItemIndex--;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                    s_currentItemIndex++;
                }

                s_currentItemIndex %= s_candidatesList.size();

                for (int i = 0; i < s_candidatesList.size(); i++) {
                    const bool is_selected = (s_currentItemIndex == i);

                    if (ImGui::Selectable(s_candidatesList[i].c_str(), is_selected)) {
                        s_currentItemIndex = i;
                    }

                    if (is_selected) ImGui::SetItemDefaultFocus();
                }

                ImGui::EndTooltip();
            }

            s_selectedCandidate = s_currentItemIndex;
        }

        if (s_autocomplete) {
            if (s_candidatesList.size() >= 1 && s_wordEnd != nullptr && s_wordStart != nullptr)
            {
                data->DeleteChars((int)(s_wordStart - data->Buf), s_length);
                data->InsertChars(data->CursorPos, s_candidatesList[s_selectedCandidate].c_str());
                data->InsertChars(data->CursorPos, " ");
            }

            s_wordEnd = nullptr;
            s_wordStart = nullptr;
            s_length = 0;
            s_currentItemIndex = 0;
            s_candidateListOpen = false;
            s_candidatesList.clear();
            s_autocomplete = false;

            break;
        }

        break;
    }
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        if (s_candidatesList.size() >= 1 && s_wordEnd != nullptr && s_wordStart != nullptr)
        {
            data->DeleteChars((int)(s_wordStart - data->Buf), s_length);
            data->InsertChars(data->CursorPos, s_candidatesList[s_selectedCandidate].c_str());
            data->InsertChars(data->CursorPos, " ");
        }

        s_wordEnd = nullptr;
        s_wordStart = nullptr;
        s_length = 0;
        s_currentItemIndex = 0;
        s_candidateListOpen = false;
        s_candidatesList.clear();

        break;
    }
    }

    return 0;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
        //AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
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
        //AddLog("Possible matches:\n");
        //for (int i = 0; i < candidates.Size; i++)
        //    AddLog("- %s\n", candidates[i]);
    }

    return 0;
}
