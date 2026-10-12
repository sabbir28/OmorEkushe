#pragma once

#include "core/layout.h"
#include <string>

namespace bijoy::core {

	struct StartupOptions {
		int defaultLayout = 0;
		int mainWindowLeft = 250;
		int mainWindowTop = 0;
		bool layoutActivationMode = false;
		bool trayMode = false;
		int applicationMode = 1;

		// New configuration options
		bool autoStart = false;            // Auto-start on Windows boot (HKCU Run key)
		int overlayOpacity = 100;          // Overlay opacity percentage (20 - 100)
		bool alwaysOnTop = true;           // Keep floating toolbar always on top
		bool snapToTop = true;             // Snap toolbar to screen top edge
		bool showSplash = true;            // Show splash screen at startup

		// Custom hotkey for Bangla Phonetic toggle (default: F12 = 0x7B)
		int phoneticToggleKey = 0x7B;
		bool phoneticToggleCtrl = false;
		bool phoneticToggleAlt = false;
		bool phoneticToggleShift = false;
	};

	StartupOptions LoadStartupOptions();
	bool SaveStartupOptions(const StartupOptions& options);

	// Auto-start system integration
	bool IsAutoStartEnabled();
	bool SetAutoStartEnabled(bool enable);

	// Custom layout shortcut persistence
	bool LoadCustomLayoutShortcut(const std::wstring& layoutName, LayoutShortcut& shortcut);
	bool SaveCustomLayoutShortcut(const std::wstring& layoutName, const LayoutShortcut& shortcut);
	bool DeleteCustomLayoutShortcut(const std::wstring& layoutName);
	void ResetCustomLayoutShortcuts();

} // namespace bijoy::core

