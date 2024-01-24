#include "callbacks.h"

#include "imgui_internal.h"

#include <algorithm>
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
    static int s_numLines{};
    static int cursorPos = data->CursorPos;

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

        if (s_candidateListOpen) {
            if (data->CursorPos == cursorPos + 1 || data->CursorPos == cursorPos - 1) {
                cursorPos = data->CursorPos;
            }
        }
        else {
            cursorPos = data->CursorPos;
        }

        if (data->Buf + cursorPos < s_wordStart || data->Buf + cursorPos > s_wordEnd) {
            s_candidatesList.clear();
            s_candidateListOpen = false;
        }

        if (s_candidatesList.size() >= 2) {

            s_numLines = 0;
            s_linePos = 0;
            bool newlineFound = false;
            int endPoint = (int)(s_wordEnd - data->Buf);
            for (int i = endPoint - 1; i >= 0; i--) {
                if (data->Buf[i] == '\n') {
                    s_numLines++;
                    newlineFound = true;
                }
                if (!newlineFound)
                    s_linePos++;
            }

            //char* tempLineEnd = data->Buf + data->CursorPos;
            //char* tempLineStart = tempLineEnd;
            //while (tempLineStart > data->Buf) {
            //    const char c = tempLineStart[-1];
            //    if (c == '\n') break;
            //    tempLineStart--;
            //}
            //s_linePos = (int)(tempLineEnd - tempLineStart);

            //s_numLines = 0;
            //tempLineEnd = data->Buf + data->CursorPos;
            //tempLineStart = tempLineEnd;
            //while (tempLineStart > data->Buf) {
            //    const char c = tempLineStart[-1];
            //    if (c == '\n') {
            //        s_numLines++;
            //    }
            //    tempLineStart--;
            //}
            //ImGui::SetNextWindowPos(
            //    ImVec2(ImGui::GetItemRectMax().x - 10 * ImGui::CalcTextSize(" ").x, ImGui::GetItemRectMin().y + ImGui::GetWindowHeight())
            //);
            std::cout << ImGui::CalcTextSize(" ").x << '\n';
            ImGui::SetNextWindowPos(ImVec2(
                    ImGui::GetItemRectMin().x + ImGui::CalcTextSize(" ").x * (s_linePos - s_length - 1),
                    ImGui::GetItemRectMax().y + ImGui::GetTextLineHeight() * (s_numLines + 1)
            ));
            if (ImGui::BeginTooltip()) {

                s_candidateListOpen = true;

                if  (data->CursorPos > cursorPos +1 || data->CursorPos < cursorPos - 1) {
                    data->CursorPos = cursorPos;
                }

                if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
                    s_currentItemIndex--;
                }
                else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
                    s_currentItemIndex++;
                }

                cursorPos = data->CursorPos;

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

            data->CursorPos = (int)(s_wordStart - data->Buf) + s_candidatesList[s_selectedCandidate].length() + 1;
            cursorPos = data->CursorPos;

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
    }

    return 0;
}

void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}
