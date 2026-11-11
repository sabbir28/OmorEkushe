#pragma once

#include "SpellingChecker.h"
#include <string>
#include <vector>
#include <chrono>

namespace bijoy::spelling {

// -------------------------------------------------------------------------
// SpellingCheckerUI
// ImGui-based live spell checking interface with real-time suggestions
// -------------------------------------------------------------------------
class SpellingCheckerUI {
public:
    // Initialize — loads the dictionary
    static void Initialize(const std::string& dictPath);

    // Render the full UI. Returns true if user wants to quit
    static bool Render();

    // Cleanup
    static void Shutdown();

private:
    // Core spell checker engine
    static SpellChecker s_checker;
    static bool s_dictionaryLoaded;
    static int s_dictWordCount;

    // Text input buffer
    static char s_inputBuffer[4096];
    static std::string s_lastProcessedText;

    // Per-word result for live display
    struct WordResult {
        std::string word;
        bool isCorrect;
        std::vector<std::string> suggestions;
        int startByte;  // byte offset in original text
        int endByte;
    };

    static std::vector<WordResult> s_results;

    // Live suggestion state
    static std::vector<std::string> s_liveSuggestions;
    static std::string s_currentWord;
    static bool s_showSuggestions;

    // Stats
    static int s_totalChecked;
    static int s_totalCorrect;
    static int s_totalWrong;

    // Timing
    static float s_lastCheckTimeMs;

    // Internal helpers
    static void ProcessText();
    static void UpdateLiveSuggestions();
    static std::string GetCurrentWordAtCursor();

    // UI rendering sub-sections
    static void RenderToolbar();
    static void RenderTextEditor();
    static void RenderResultsPanel();
    static void RenderSuggestionPopup();
    static void RenderStatusBar();
};

} // namespace bijoy::spelling
