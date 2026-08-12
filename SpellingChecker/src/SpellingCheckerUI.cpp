#include "SpellingCheckerUI.h"
#include "imgui.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace bijoy::spelling {

SpellChecker SpellingCheckerUI::s_checker;
bool SpellingCheckerUI::s_dictionaryLoaded = false;
int SpellingCheckerUI::s_dictWordCount = 0;

char SpellingCheckerUI::s_inputBuffer[4096] = "";
std::string SpellingCheckerUI::s_lastProcessedText = "";

std::vector<SpellingCheckerUI::WordResult> SpellingCheckerUI::s_results;
std::vector<std::string> SpellingCheckerUI::s_liveSuggestions;
std::string SpellingCheckerUI::s_currentWord = "";
bool SpellingCheckerUI::s_showSuggestions = false;

int SpellingCheckerUI::s_totalChecked = 0;
int SpellingCheckerUI::s_totalCorrect = 0;
int SpellingCheckerUI::s_totalWrong = 0;
float SpellingCheckerUI::s_lastCheckTimeMs = 0.0f;

// Local state for tracking text cursor
static int s_cursorPos = 0;
static ImVec2 s_editorScreenPos;
static bool s_editorActive = false;

static int TextEditCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways) {
        s_cursorPos = data->CursorPos;
    }
    // Handle suggestion insertion if we want to programmatically insert suggestions
    return 0;
}

void SpellingCheckerUI::Initialize(const std::string& dictPath) {
    auto start = std::chrono::high_resolution_clock::now();
    s_dictWordCount = s_checker.LoadDictionary(dictPath);
    auto end = std::chrono::high_resolution_clock::now();
    
    if (s_dictWordCount >= 0) {
        s_dictionaryLoaded = true;
        std::cout << "[INFO] Loaded " << s_dictWordCount << " words in " 
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms\n";
    } else {
        std::cerr << "[ERROR] Failed to load dictionary from: " << dictPath << "\n";
    }
}

bool SpellingCheckerUI::Render() {
    bool shouldExit = false;

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    
    if (ImGui::Begin("Spelling Checker", nullptr, window_flags)) {
        if (!s_dictionaryLoaded) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Dictionary not loaded! Check dataset path.");
        }

        RenderToolbar();
        ImGui::Separator();
        
        // Editor area
        RenderTextEditor();
        
        // Split view for live results
        ImGui::Separator();
        ImGui::Text("Spell Check Results:");
        RenderResultsPanel();
        
        // Popup for Live Suggestions
        RenderSuggestionPopup();
        
        RenderStatusBar();
    }
    ImGui::End();

    return shouldExit;
}

void SpellingCheckerUI::Shutdown() {
    // Cleanup if needed
}

void SpellingCheckerUI::RenderToolbar() {
    if (ImGui::Button("Clear", ImVec2(80, 30))) {
        s_inputBuffer[0] = '\0';
        ProcessText();
    }
    ImGui::SameLine();
    ImGui::Text("Type Bangla text below. Misspelled words will be listed below.");
}

void SpellingCheckerUI::RenderTextEditor() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
    
    s_editorScreenPos = ImGui::GetCursorScreenPos();
    
    // Check if text changed
    bool textChanged = ImGui::InputTextMultiline("##Input", s_inputBuffer, sizeof(s_inputBuffer), 
        ImVec2(-1.0f, 200.0f), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_CallbackAlways, TextEditCallback);
    
    s_editorActive = ImGui::IsItemActive();
    
    if (textChanged || std::string(s_inputBuffer) != s_lastProcessedText) {
        ProcessText();
        s_lastProcessedText = s_inputBuffer;
    }
    
    // Handle live active word check
    if (s_editorActive) {
        std::string wordAtCursor = GetCurrentWordAtCursor();
        if (wordAtCursor != s_currentWord) {
            s_currentWord = wordAtCursor;
            UpdateLiveSuggestions();
        }
    } else {
        s_showSuggestions = false;
    }
    
    ImGui::PopStyleVar();
}

void SpellingCheckerUI::RenderResultsPanel() {
    ImGui::BeginChild("ResultsPanel", ImVec2(0, -30), true, ImGuiWindowFlags_HorizontalScrollbar);
    
    if (std::string(s_inputBuffer).empty()) {
        ImGui::TextDisabled("No text to check.");
    } else {
        if (s_totalWrong == 0 && s_totalChecked > 0) {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "All words are correctly spelled! (%d checked)", s_totalChecked);
        } else {
            for (auto& res : s_results) {
                if (res.isCorrect) continue; // Only show wrong words
                
                ImGui::PushID(&res);
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "❌ %s", res.word.c_str());
                ImGui::SameLine();
                
                if (res.suggestions.empty()) {
                    ImGui::TextDisabled("(No suggestions)");
                } else {
                    ImGui::Text(" Suggestions: ");
                    for (size_t i = 0; i < res.suggestions.size(); ++i) {
                        ImGui::SameLine();
                        if (ImGui::Button(res.suggestions[i].c_str())) {
                            // Replace word implementation could go here, 
                            // but requires careful string manipulation of s_inputBuffer
                        }
                    }
                }
                ImGui::PopID();
            }
        }
    }
    ImGui::EndChild();
}

void SpellingCheckerUI::RenderStatusBar() {
    ImGui::TextDisabled("Status: %s | Words: %d / %d (Errors: %d) | Dictionary: %d words | Check latency: %.2f ms", 
        s_dictionaryLoaded ? "Ready" : "Error",
        s_totalCorrect, s_totalChecked, s_totalWrong,
        s_dictWordCount, s_lastCheckTimeMs);
}

std::string SpellingCheckerUI::GetCurrentWordAtCursor() {
    std::string text = s_inputBuffer;
    if (text.empty() || s_cursorPos < 0 || s_cursorPos > static_cast<int>(text.length())) return "";

    std::string delimiters = " \t\n\r,.;:!?()[]{}'\"";
    
    int start = s_cursorPos - 1;
    while (start >= 0 && delimiters.find(text[start]) == std::string::npos) {
        start--;
    }
    start++;
    
    int end = s_cursorPos;
    while (end < static_cast<int>(text.length()) && delimiters.find(text[end]) == std::string::npos) {
        end++;
    }
    
    if (start >= end) return "";
    return text.substr(start, end - start);
}

void SpellingCheckerUI::UpdateLiveSuggestions() {
    s_showSuggestions = false;
    s_liveSuggestions.clear();
    
    if (!s_dictionaryLoaded || s_currentWord.empty()) return;
    
    // Don't show numeric only suggestions or too small
    if (s_currentWord.length() < 2) return;
    
    bool ok = s_checker.Check(s_currentWord);
    if (!ok) {
        s_liveSuggestions = s_checker.Suggest(s_currentWord, 5);
        if (!s_liveSuggestions.empty()) {
            s_showSuggestions = true;
        }
    }
}

void SpellingCheckerUI::RenderSuggestionPopup() {
    if (!s_showSuggestions || !s_editorActive || s_liveSuggestions.empty()) return;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.15f, 0.15f, 0.18f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.4f, 0.4f, 0.5f, 1.0f));
    
    // Calculate caret position approximate
    std::string text = s_inputBuffer;
    std::string textToCursor = text.substr(0, s_cursorPos);
    
    // Find last newline
    size_t lastNewline = textToCursor.find_last_of('\n');
    std::string currentLineText = (lastNewline == std::string::npos) ? textToCursor : textToCursor.substr(lastNewline + 1);
    
    // Y offset based on number of newlines
    int newlines = static_cast<int>(std::count(textToCursor.begin(), textToCursor.end(), '\n'));
    
    ImVec2 caretOffset = ImGui::CalcTextSize(currentLineText.c_str());
    caretOffset.y = (newlines + 1) * ImGui::GetTextLineHeightWithSpacing();
    
    // Account for frame padding inside the input text
    caretOffset.x += 10.0f; // from PushStyleVar(ImGuiStyleVar_FramePadding)
    caretOffset.y += 10.0f;
    
    ImVec2 popupPos = ImVec2(s_editorScreenPos.x + caretOffset.x, s_editorScreenPos.y + caretOffset.y);
    ImGui::SetNextWindowPos(popupPos);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing;
    if (ImGui::Begin("LiveSuggestionsPopup", nullptr, flags)) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.5f, 1.0f), "❌ %s", s_currentWord.c_str());
        ImGui::Separator();
        for (const auto& suggestion : s_liveSuggestions) {
            if (ImGui::Selectable(suggestion.c_str())) {
                // To safely implement replacement, we need to modify the input buffer and rely on User callback.
                // For now, we will just show them. Fully interacting might steal focus.
            }
        }
    }
    ImGui::End();
    
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
}

void SpellingCheckerUI::ProcessText() {
    if (!s_dictionaryLoaded) return;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    s_results.clear();
    s_totalChecked = 0;
    s_totalCorrect = 0;
    s_totalWrong = 0;
    
    std::string text(s_inputBuffer);
    
    std::string delimiters = " \t\n\r,.;:!?()[]{}'\"";
    size_t pos = 0;
    
    while (pos < text.length()) {
        pos = text.find_first_not_of(delimiters, pos);
        if (pos == std::string::npos) break;
        
        size_t end = text.find_first_of(delimiters, pos);
        if (end == std::string::npos) end = text.length();
        
        std::string word = text.substr(pos, end - pos);
        
        // Keep English or numbers out of spell checker ideally, but for now check all
        // Only checking words with at least some length
        if (!word.empty()) {
            s_totalChecked++;
            bool ok = s_checker.Check(word);
            
            WordResult wr;
            wr.word = word;
            wr.isCorrect = ok;
            wr.startByte = static_cast<int>(pos);
            wr.endByte = static_cast<int>(end);
            
            if (ok) {
                s_totalCorrect++;
            } else {
                s_totalWrong++;
                wr.suggestions = s_checker.Suggest(word, 5); // top 5 suggestions
            }
            
            s_results.push_back(wr);
        }
        
        pos = end;
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    s_lastCheckTimeMs = std::chrono::duration<float, std::milli>(end_time - start).count();
}

} // namespace bijoy::spelling
