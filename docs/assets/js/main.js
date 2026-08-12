/**
 * Omor Ekushe - Website Interactive Script
 */

document.addEventListener('DOMContentLoaded', () => {
  // Live Typing & Layout Simulator
  const layoutBtns = document.querySelectorAll('.layout-pill-btn');
  const simTextarea = document.getElementById('simulator-input');
  const currentLayoutLabel = document.getElementById('active-layout-name');
  const hotkeyBadge = document.getElementById('active-hotkey-badge');

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

  layoutBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      layoutBtns.forEach(b => b.classList.remove('active'));
      btn.classList.add('active');

      const layoutKey = btn.getAttribute('data-layout');
      if (currentLayoutLabel) {
        currentLayoutLabel.textContent = layoutNames[layoutKey] || 'Bangla';
      }
      if (simTextarea && sampleTexts[layoutKey]) {
        simTextarea.value = sampleTexts[layoutKey];
      }
    });
  });

  // Hotkey Simulator Keypress Simulation
  document.addEventListener('keydown', (e) => {
    // Detect Ctrl + Alt + B
    if (e.ctrlKey && e.altKey && (e.key === 'b' || e.key === 'B')) {
      e.preventDefault();
      
      // Toggle active layout button
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
        
        // Show visual feedback pulse on hotkey badge
        if (hotkeyBadge) {
          hotkeyBadge.classList.add('bg-warning', 'text-dark');
          setTimeout(() => hotkeyBadge.classList.remove('bg-warning', 'text-dark'), 600);
        }
      }
    }
  });

  // Copy Code Snippet Functionality
  const copyBtns = document.querySelectorAll('.btn-copy-code');
  copyBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      const targetId = btn.getAttribute('data-target');
      const codeElem = document.getElementById(targetId);
      if (codeElem) {
        const textToCopy = codeElem.innerText || codeElem.textContent;
        navigator.clipboard.writeText(textToCopy.trim()).then(() => {
          const originalText = btn.innerHTML;
          btn.innerHTML = '<i class="fas fa-check text-success"></i> Copied!';
          setTimeout(() => {
            btn.innerHTML = originalText;
          }, 2000);
        });
      }
    });
  });
});
