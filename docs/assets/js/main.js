---
  ---
  /**
   * Omor Ekushe - Interactive JS & jQuery Web Engine
   * Features: Live Bangla Typing Simulator, Keyboard Layout Visualizer,
   * Copy Code Snippets to Clipboard, Back to Top Floating Service,
   * and GitHub Public REST API Integration.
   */

  $(document).ready(function () {
    // --------------------------------------------------------------------------
    // 1. Live Typing & Layout Simulator (jQuery Engine)
    // --------------------------------------------------------------------------
    const $layoutBtns = $('.layout-pill-btn');
    const $simTextarea = $('#simulator-input');
    const $activeLayoutLabel = $('#active-layout-name');
    const $activeHotkeyBadge = $('#active-hotkey-badge');
    const $backToTopBtn = $('#backToTop');

    const sampleTexts = {
      'bijoy': 'আমার সোনার বাংলা, আমি তোমায় ভালোবাসি। অমর একুশে গ্রন্থমেলা।',
      'unijoy': 'মোদের গরব, মোদের আশা, আ-মরি বাংলা ভাষা! একুশে ফেব্রুয়ারি আন্তর্জাতিক মাতৃভাষা দিবস।',
      'english': 'Omor Ekushe Bengali Keyboard Layout Manager & Engine for Windows 10/11.'
    };

    const layoutNames = {
      'bijoy': 'Bijoy (বিজয়)',
      'unijoy': 'Unijoy (ইউনিজয়)',
      'english': 'English (Default)'
    };

    let activeLayout = 'bijoy';

    // Layout Selector Pill Button Click
    $layoutBtns.on('click', function () {
      $layoutBtns.removeClass('active');
      $(this).addClass('active');

      activeLayout = $(this).attr('data-layout') || 'bijoy';
      if ($activeLayoutLabel.length) {
        $activeLayoutLabel.text(layoutNames[activeLayout] || 'Bangla');
      }
      if ($simTextarea.length && sampleTexts[activeLayout]) {
        $simTextarea.val(sampleTexts[activeLayout]);
      }
    });

    // Hotkey Simulator (Ctrl + Alt + B Keyboard Trigger)
    $(document).on('keydown', function (e) {
      if (e.ctrlKey && e.altKey && (e.key === 'b' || e.key === 'B')) {
        e.preventDefault();

        const $activeBtn = $('.layout-pill-btn.active');
        let $nextBtn;
        const current = $activeBtn.attr('data-layout');

        if (!current || current === 'english') {
          $nextBtn = $('.layout-pill-btn[data-layout="bijoy"]');
        } else if (current === 'bijoy') {
          $nextBtn = $('.layout-pill-btn[data-layout="unijoy"]');
        } else {
          $nextBtn = $('.layout-pill-btn[data-layout="english"]');
        }

        if ($nextBtn && $nextBtn.length) {
          $nextBtn.trigger('click');

          if ($activeHotkeyBadge.length) {
            $activeHotkeyBadge.addClass('bg-amber-400 text-slate-900').removeClass('bg-emerald-950 text-emerald-400');
            setTimeout(() => {
              $activeHotkeyBadge.removeClass('bg-amber-400 text-slate-900').addClass('bg-emerald-950 text-emerald-400');
            }, 600);
          }
        }
      }
    });

    // --------------------------------------------------------------------------
    // 2. Copy Code Snippet Handler with Toast Feedback
    // --------------------------------------------------------------------------
    $('.btn-copy-code').on('click', function () {
      const $btn = $(this);
      const targetId = $btn.attr('data-target');
      const $codeElem = $('#' + targetId);

      if ($codeElem.length) {
        const textToCopy = ($codeElem.text() || '').trim();

        if (navigator.clipboard && window.isSecureContext) {
          navigator.clipboard.writeText(textToCopy).then(() => showCopiedFeedback($btn));
        } else {
          // Fallback copy
          const $tempText = $('<textarea>');
          $('body').append($tempText);
          $tempText.val(textToCopy).select();
          try {
            document.execCommand('copy');
            showCopiedFeedback($btn);
          } catch (err) {
            console.error('Clipboard copy error:', err);
          }
          $tempText.remove();
        }
      }
    });

    function showCopiedFeedback($btn) {
      const originalHtml = $btn.html();
      $btn.html('<i class="fas fa-check text-emerald-400 me-1"></i> <span class="text-xs text-emerald-400 font-medium">Copied!</span>');
      setTimeout(() => {
        $btn.html(originalHtml);
      }, 2000);
    }

    // --------------------------------------------------------------------------
    // 3. Floating Back to Top Button Logic
    // --------------------------------------------------------------------------
    if ($backToTopBtn.length) {
      $(window).on('scroll', function () {
        if ($(this).scrollTop() > 300) {
          $backToTopBtn.addClass('show');
        } else {
          $backToTopBtn.removeClass('show');
        }
      });

      $backToTopBtn.on('click', function () {
        window.scrollTo({
          top: 0,
          behavior: 'smooth'
        });
      });
    }

    // --------------------------------------------------------------------------
    // 4. GitHub REST API Hub Integration
    // Repository Stats, Releases, Downloads, Commit Feed & Contributors
    // --------------------------------------------------------------------------
    const REPO_OWNER = 'sabbir28';
    const REPO_NAME = 'OmorEkushe';
    const API_BASE = `https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}`;

    // A. Fetch Repo Details (Stars, Forks, Open Issues, Lang)
    async function fetchRepoDetails() {
      try {
        const res = await fetch(API_BASE);
        if (!res.ok) return;
        const repo = await res.json();

        // Stars
        $('.github-stars-count').text(repo.stargazers_count !== undefined ? repo.stargazers_count : '1+');
        // Forks
        $('.github-forks-count').text(repo.forks_count !== undefined ? repo.forks_count : '0');
        // Open Issues
        $('.github-issues-count').text(repo.open_issues_count !== undefined ? repo.open_issues_count : '0');
        // Primary Language
        $('.github-primary-lang').text(repo.language || 'C++');

        if (repo.size) {
          const mbSize = (repo.size / 1024).toFixed(1);
          $('.github-repo-size').text(`${mbSize} MB`);
        }
      } catch (err) {
        console.warn('GitHub Repo API fallback:', err);
      }
    }

    // B. Fetch Latest Release (Setup Download, Version Tag, Date, Asset Stats)
    async function fetchLatestRelease() {
      try {
        const res = await fetch(`${API_BASE}/releases/latest`);
        if (!res.ok) return;
        const release = await res.json();

        // Release Tag
        $('.github-release-tag').text(release.tag_name || 'v1.0.0');

        // Published Date
        if (release.published_at) {
          const pubDate = new Date(release.published_at).toLocaleDateString(undefined, {
            year: 'numeric',
            month: 'short',
            day: 'numeric'
          });
          $('.github-release-date').text(pubDate);
        }

        // Assets & Download Metrics
        if (release.assets && Array.isArray(release.assets)) {
          let totalDl = 0;
          let setupAsset = release.assets.find(a => a.name.toLowerCase().endsWith('.exe'));
          let portableAsset = release.assets.find(a => a.name.toLowerCase().endsWith('.zip'));

          release.assets.forEach(asset => {
            totalDl += asset.download_count || 0;
          });

          $('.github-total-downloads').text(totalDl > 0 ? `${totalDl} downloads` : 'Active Release');

          if (setupAsset) {
            $('.github-download-setup').each(function () {
              $(this).attr('href', setupAsset.browser_download_url).attr('download', setupAsset.name);
            });
            $('.github-setup-size').text(`${(setupAsset.size / (1024 * 1024)).toFixed(1)} MB`);
            $('.github-setup-downloads').text(`${setupAsset.download_count || 0} downloads`);
          }

          if (portableAsset) {
            $('.github-download-portable').each(function () {
              $(this).attr('href', portableAsset.browser_download_url).attr('download', portableAsset.name);
            });
            $('.github-portable-size').text(`${(portableAsset.size / (1024 * 1024)).toFixed(1)} MB`);
            $('.github-portable-downloads').text(`${portableAsset.download_count || 0} downloads`);
          }
        }
      } catch (err) {
        console.warn('GitHub Release API fallback:', err);
      }
    }

    // C. Fetch Commit Feed
    async function fetchRecentCommits() {
      const $feedContainer = $('#github-commit-feed');
      if (!$feedContainer.length) return;

      try {
        const res = await fetch(`${API_BASE}/commits?per_page=5`);
        if (!res.ok) return;
        const commits = await res.json();

        let html = '<div class="space-y-3">';
        commits.forEach(item => {
          const msg = item.commit.message.split('\n')[0];
          const author = item.commit.author ? item.commit.author.name : 'Contributor';
          const date = new Date(item.commit.author.date).toLocaleDateString(undefined, {
            month: 'short',
            day: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
          });
          const sha = item.sha.substring(0, 7);
          const url = item.html_url;
          const avatar = item.author ? item.author.avatar_url : 'https://github.githubassets.com/favicons/favicon.png';

          html += `
                <a href="${url}" target="_blank" class="block p-3 bg-slate-900/80 border border-emerald-500/20 rounded-xl hover:border-emerald-400 hover:bg-slate-900 transition-all text-decoration-none group">
                    <div class="flex items-center justify-between gap-3">
                        <div class="flex items-center gap-3 min-w-0">
                            <img src="${avatar}" width="34" height="34" class="rounded-full border border-emerald-500/30" alt="${author}">
                            <div class="min-w-0">
                                <div class="text-sm font-semibold text-slate-100 truncate group-hover:text-emerald-400 transition-colors">${msg}</div>
                                <div class="text-xs text-slate-400"><i class="fas fa-user-edit me-1 text-emerald-500"></i> ${author} &bull; ${date}</div>
                            </div>
                        </div>
                        <span class="px-2.5 py-1 text-xs font-mono text-emerald-400 bg-emerald-950 border border-emerald-500/30 rounded-lg shrink-0">${sha}</span>
                    </div>
                </a>
                `;
        });
        html += '</div>';
        $feedContainer.html(html);
      } catch (err) {
        console.warn('GitHub Commits API error:', err);
      }
    }

    // D. Fetch Contributors
    async function fetchContributors() {
      const $contribContainer = $('#github-contributors-list');
      if (!$contribContainer.length) return;

      try {
        const res = await fetch(`${API_BASE}/contributors`);
        if (!res.ok) return;
        const contributors = await res.json();

        let html = '<div class="flex flex-wrap gap-2.5 items-center">';
        contributors.forEach(user => {
          html += `
                <a href="${user.html_url}" target="_blank" class="text-decoration-none group" title="${user.login} (${user.contributions} contributions)">
                    <div class="flex items-center gap-2 bg-slate-900/80 border border-emerald-500/20 hover:border-emerald-400 rounded-full px-3 py-1.5 transition-all">
                        <img src="${user.avatar_url}" width="26" height="26" class="rounded-full" alt="${user.login}">
                        <span class="text-xs font-semibold text-slate-200 group-hover:text-emerald-400 transition-colors">${user.login}</span>
                        <span class="badge bg-emerald-500/20 text-emerald-400 border border-emerald-500/30 rounded-full text-[10px]">${user.contributions}</span>
                    </div>
                </a>
                `;
        });
        html += '</div>';
        $contribContainer.html(html);
      } catch (err) {
        console.warn('GitHub Contributors API error:', err);
      }
    }

    // Initialize GitHub API Calls
    fetchRepoDetails();
    fetchLatestRelease();
    fetchRecentCommits();
    fetchContributors();
  });
