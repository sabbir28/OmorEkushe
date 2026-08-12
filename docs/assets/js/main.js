---
  ---
  /**
   * Omor Ekushe - Website Interactive Engine
   * Responsive layout toggler, real-time Bangla typing simulator, copy-to-clipboard, and back-to-top service.
   */

  document.addEventListener('DOMContentLoaded', () => {
    // Live Typing & Layout Simulator
    const layoutBtns = document.querySelectorAll('.layout-pill-btn');
    const simTextarea = document.getElementById('simulator-input');
    const currentLayoutLabel = document.getElementById('active-layout-name');
    const hotkeyBadge = document.getElementById('active-hotkey-badge');
    const backToTopBtn = document.getElementById('backToTop');

    const sampleTexts = {
      'bijoy': 'আমার সোনার বাংলা, আমি তোমায় ভালোবাসি। অমর একুশে গ্রন্থমেলা।',
      'unijoy': 'মোদের গরব, মোদের আশা, আ-মরি বাংলা ভাষা! একুশে ফেব্রুয়ারি আন্তর্জাতিক মাতৃভাষা দিবস।',
      'english': 'Omor Ekushe Bengali Keyboard Layout Manager & Engine for Windows.'
    };

    const layoutNames = {
      'bijoy': 'Bijoy (বিজয়)',
      'unijoy': 'Unijoy (ইউনিজয়)',
      'english': 'English (Default)'
    };

    let activeLayout = 'bijoy';

    // Layout Button Selection
    layoutBtns.forEach(btn => {
      btn.addEventListener('click', () => {
        layoutBtns.forEach(b => b.classList.remove('active'));
        btn.classList.add('active');

        activeLayout = btn.getAttribute('data-layout') || 'bijoy';
        if (currentLayoutLabel) {
          currentLayoutLabel.textContent = layoutNames[activeLayout] || 'Bangla';
        }
        if (simTextarea && sampleTexts[activeLayout]) {
          simTextarea.value = sampleTexts[activeLayout];
        }
      });
    });

    // Hotkey Simulator Keypress Simulation (Ctrl + Alt + B)
    document.addEventListener('keydown', (e) => {
      if (e.ctrlKey && e.altKey && (e.key === 'b' || e.key === 'B')) {
        e.preventDefault();

        const activeBtn = document.querySelector('.layout-pill-btn.active');
        let nextBtn;
        if (!activeBtn || activeBtn.getAttribute('data-layout') === 'english') {
          nextBtn = document.querySelector('.layout-pill-btn[data-layout="bijoy"]');
        } else if (activeBtn.getAttribute('data-layout') === 'bijoy') {
          nextBtn = document.querySelector('.layout-pill-btn[data-layout="unijoy"]');
        } else {
          nextBtn = document.querySelector('.layout-pill-btn[data-layout="english"]');
        }
        if (nextBtn) {
          nextBtn.click();

          if (hotkeyBadge) {
            hotkeyBadge.classList.add('bg-warning', 'text-dark');
            setTimeout(() => hotkeyBadge.classList.remove('bg-warning', 'text-dark'), 600);
          }
        }
      }
    });

    // Copy Code Snippet Functionality with Fallback
    const copyBtns = document.querySelectorAll('.btn-copy-code');
    copyBtns.forEach(btn => {
      btn.addEventListener('click', () => {
        const targetId = btn.getAttribute('data-target');
        const codeElem = document.getElementById(targetId);
        if (codeElem) {
          const textToCopy = (codeElem.innerText || codeElem.textContent || '').trim();
          if (navigator.clipboard && window.isSecureContext) {
            navigator.clipboard.writeText(textToCopy).then(() => showCopiedFeedback(btn));
          } else {
            // Fallback for non-https local dev
            const textArea = document.createElement('textarea');
            textArea.value = textToCopy;
            textArea.style.position = 'fixed';
            textArea.style.opacity = '0';
            document.body.appendChild(textArea);
            textArea.focus();
            textArea.select();
            try {
              document.execCommand('copy');
              showCopiedFeedback(btn);
            } catch (err) {
              console.error('Copy fallback failed', err);
            }
            document.body.removeChild(textArea);
          }
        }
      });
    });

    function showCopiedFeedback(btn) {
      const originalText = btn.innerHTML;
      btn.innerHTML = '<i class="fas fa-check text-success me-1"></i> Copied!';
      setTimeout(() => {
        btn.innerHTML = originalText;
      }, 2000);
    }

    // Floating Back to Top Button Logic
    if (backToTopBtn) {
      window.addEventListener('scroll', () => {
        if (window.scrollY > 300) {
          backToTopBtn.classList.add('show');
        } else {
          backToTopBtn.classList.remove('show');
        }
      });

      backToTopBtn.addEventListener('click', () => {
        window.scrollTo({
          top: 0,
          behavior: 'smooth'
        });
      });
    }

    // ==========================================================================
    // GitHub Public REST API Integration Hub
    // Dynamic Repository Stats, Releases, Download Counters, Commits & Contributors
    // ==========================================================================
    const REPO_OWNER = 'sabbir28';
    const REPO_NAME = 'OmorEkushe';
    const API_BASE = `https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}`;

    // 1. Fetch Repository Details (Stars, Forks, Issues, Size, Language)
    async function fetchGitHubRepoDetails() {
      try {
        const res = await fetch(API_BASE);
        if (!res.ok) return;
        const repo = await res.json();

        // Stars Count
        const starElems = document.querySelectorAll('.github-stars-count');
        starElems.forEach(el => {
          el.textContent = repo.stargazers_count !== undefined ? repo.stargazers_count : '1+';
        });

        // Forks Count
        const forkElems = document.querySelectorAll('.github-forks-count');
        forkElems.forEach(el => {
          el.textContent = repo.forks_count !== undefined ? repo.forks_count : '0';
        });

        // Open Issues
        const issueElems = document.querySelectorAll('.github-issues-count');
        issueElems.forEach(el => {
          el.textContent = repo.open_issues_count !== undefined ? repo.open_issues_count : '0';
        });

        // Repo Language & Size
        const langElems = document.querySelectorAll('.github-primary-lang');
        langElems.forEach(el => {
          el.textContent = repo.language || 'C++';
        });

        const sizeElems = document.querySelectorAll('.github-repo-size');
        sizeElems.forEach(el => {
          if (repo.size) {
            const mbSize = (repo.size / 1024).toFixed(1);
            el.textContent = `${mbSize} MB`;
          }
        });
      } catch (err) {
        console.warn('GitHub Repo Details API fallback triggered:', err);
      }
    }

    // 2. Fetch Latest Release Data (Assets, Executable links, Download Count)
    async function fetchLatestGitHubRelease() {
      try {
        const response = await fetch(`${API_BASE}/releases/latest`);
        if (!response.ok) return;
        const releaseData = await response.json();

        // Release Tag
        const releaseBadges = document.querySelectorAll('.github-release-tag');
        releaseBadges.forEach(el => {
          el.textContent = releaseData.tag_name || 'v1.0.0';
        });

        // Published Date
        const releaseDates = document.querySelectorAll('.github-release-date');
        if (releaseData.published_at) {
          const pubDate = new Date(releaseData.published_at).toLocaleDateString(undefined, {
            year: 'numeric',
            month: 'short',
            day: 'numeric'
          });
          releaseDates.forEach(el => {
            el.textContent = pubDate;
          });
        }

        // Assets & Download Counts
        if (releaseData.assets && Array.isArray(releaseData.assets)) {
          let totalDownloads = 0;
          let setupAsset = releaseData.assets.find(a => a.name.toLowerCase().endsWith('.exe'));
          let portableAsset = releaseData.assets.find(a => a.name.toLowerCase().endsWith('.zip'));

          releaseData.assets.forEach(asset => {
            totalDownloads += asset.download_count || 0;
          });

          const dlCountElems = document.querySelectorAll('.github-total-downloads');
          dlCountElems.forEach(el => {
            el.textContent = totalDownloads > 0 ? `${totalDownloads} downloads` : 'Active Release';
          });

          if (setupAsset) {
            document.querySelectorAll('.github-download-setup').forEach(el => {
              el.href = setupAsset.browser_download_url;
              el.setAttribute('download', setupAsset.name);
            });
            document.querySelectorAll('.github-setup-size').forEach(el => {
              el.textContent = `${(setupAsset.size / (1024 * 1024)).toFixed(1)} MB`;
            });
            document.querySelectorAll('.github-setup-downloads').forEach(el => {
              el.textContent = `${setupAsset.download_count || 0} downloads`;
            });
          }

          if (portableAsset) {
            document.querySelectorAll('.github-download-portable').forEach(el => {
              el.href = portableAsset.browser_download_url;
              el.setAttribute('download', portableAsset.name);
            });
            document.querySelectorAll('.github-portable-size').forEach(el => {
              el.textContent = `${(portableAsset.size / (1024 * 1024)).toFixed(1)} MB`;
            });
            document.querySelectorAll('.github-portable-downloads').forEach(el => {
              el.textContent = `${portableAsset.download_count || 0} downloads`;
            });
          }
        }
      } catch (err) {
        console.warn('GitHub Release API fallback triggered:', err);
      }
    }

    // 3. Fetch Recent Commits Feed
    async function fetchRecentGitHubCommits() {
      const commitContainer = document.getElementById('github-commit-feed');
      if (!commitContainer) return;

      try {
        const res = await fetch(`${API_BASE}/commits?per_page=5`);
        if (!res.ok) return;
        const commits = await res.json();

        let html = '<div class="list-group list-group-flush rounded-3 border overflow-hidden">';
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
          <a href="${url}" target="_blank" class="list-group-item list-group-item-action d-flex align-items-center justify-content-between py-3 px-3">
            <div class="d-flex align-items-center gap-3">
              <img src="${avatar}" width="32" height="32" class="rounded-circle border" alt="${author}">
              <div>
                <div class="fw-semibold text-dark mb-0 text-truncate" style="max-width: 420px;">${msg}</div>
                <small class="text-muted"><i class="fas fa-user-edit me-1"></i> ${author} &bull; ${date}</small>
              </div>
            </div>
            <span class="badge bg-dark font-monospace text-success px-2 py-1">${sha}</span>
          </a>
        `;
        });
        html += '</div>';
        commitContainer.innerHTML = html;
      } catch (err) {
        console.warn('GitHub Commits API fallback triggered:', err);
      }
    }

    // 4. Fetch Contributors
    async function fetchGitHubContributors() {
      const contribContainer = document.getElementById('github-contributors-list');
      if (!contribContainer) return;

      try {
        const res = await fetch(`${API_BASE}/contributors`);
        if (!res.ok) return;
        const contributors = await res.json();

        let html = '<div class="d-flex flex-wrap gap-3 align-items-center">';
        contributors.forEach(user => {
          html += `
          <a href="${user.html_url}" target="_blank" class="text-decoration-none" title="${user.login} (${user.contributions} contributions)">
            <div class="d-flex align-items-center gap-2 bg-light border rounded-pill px-3 py-1 shadow-sm contributor-chip">
              <img src="${user.avatar_url}" width="28" height="28" class="rounded-circle" alt="${user.login}">
              <span class="fw-bold text-dark small">${user.login}</span>
              <span class="badge bg-success rounded-pill small">${user.contributions}</span>
            </div>
          </a>
        `;
        });
        html += '</div>';
        contribContainer.innerHTML = html;
      } catch (err) {
        console.warn('GitHub Contributors API fallback triggered:', err);
      }
    }

    // Trigger All API Calls concurrently
    fetchGitHubRepoDetails();
    fetchLatestGitHubRelease();
    fetchRecentGitHubCommits();
    fetchGitHubContributors();
  });


