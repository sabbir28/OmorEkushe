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

  // GitHub Public API Dynamic Release Integration
  async function fetchLatestGitHubRelease() {
    const repoOwner = 'sabbir28';
    const repoName = 'OmorEkushe';
    const apiUrl = `https://api.github.com/repos/${repoOwner}/${repoName}/releases/latest`;

    try {
      const response = await fetch(apiUrl);
      if (!response.ok) {
        console.warn('GitHub API response not ok, falling back to release page.');
        return;
      }
      const releaseData = await response.json();

      // Update release version badges
      const releaseBadges = document.querySelectorAll('.github-release-tag');
      releaseBadges.forEach(el => {
        el.textContent = releaseData.tag_name || 'latest';
      });

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

      // Parse assets
      if (releaseData.assets && Array.isArray(releaseData.assets)) {
        let setupAsset = releaseData.assets.find(a => a.name.toLowerCase().endsWith('.exe'));
        let portableAsset = releaseData.assets.find(a => a.name.toLowerCase().endsWith('.zip'));

        if (setupAsset) {
          document.querySelectorAll('.github-download-setup').forEach(el => {
            el.href = setupAsset.browser_download_url;
            el.setAttribute('download', setupAsset.name);
          });
          document.querySelectorAll('.github-setup-size').forEach(el => {
            el.textContent = (setupAsset.size / (1024 * 1024)).toFixed(1) + ' MB';
          });
        }

        if (portableAsset) {
          document.querySelectorAll('.github-download-portable').forEach(el => {
            el.href = portableAsset.browser_download_url;
            el.setAttribute('download', portableAsset.name);
          });
          document.querySelectorAll('.github-portable-size').forEach(el => {
            el.textContent = (portableAsset.size / (1024 * 1024)).toFixed(1) + ' MB';
          });
        }
      }
    } catch (err) {
      console.error('Error fetching GitHub releases:', err);
    }
  }

  fetchLatestGitHubRelease();
});

