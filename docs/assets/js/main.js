/**
 * Omor Ekushe - Interactive JS Engine
 * Features: Light/Dark Theme Switcher, Live Avro Phonetic & Layout Simulator,
 * GitHub REST API Integration, Clipboard Copy Toasts, Back to Top scroll.
 */

$(document.documentElement).ready(function () {
  // ----------------------------------------------------------------------
  // 1. Theme Engine Initialization (Light & Dark Mode Switcher)
  // ----------------------------------------------------------------------
  const themeToggleBtn = $('#theme-toggle-btn');

  function setTheme(theme) {
    document.documentElement.setAttribute('data-theme', theme);
    localStorage.setItem('omor_theme', theme);

    if (theme === 'dark') {
      themeToggleBtn.html('<i class="fas fa-sun text-amber-400"></i>');
      themeToggleBtn.attr('title', 'Switch to Light Mode');
    } else {
      themeToggleBtn.html('<i class="fas fa-moon text-emerald-700"></i>');
      themeToggleBtn.attr('title', 'Switch to Dark Mode');
    }
  }

  // Determine initial theme
  const savedTheme = localStorage.getItem('omor_theme');
  if (savedTheme) {
    setTheme(savedTheme);
  } else {
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    setTheme(prefersDark ? 'dark' : 'light');
  }

  // Toggle event listener
  themeToggleBtn.on('click', function () {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const nextTheme = currentTheme === 'dark' ? 'light' : 'dark';
    setTheme(nextTheme);
  });

  // ----------------------------------------------------------------------
  // 2. Dynamic GitHub REST API Integration
  // ----------------------------------------------------------------------
  const GITHUB_REPO = 'sabbir28/OmorEkushe';
  const API_BASE = `https://api.github.com/repos/${GITHUB_REPO}`;

  function fetchGitHubRepoMetrics() {
    $.ajax({
      url: API_BASE,
      method: 'GET',
      dataType: 'json',
      success: function (data) {
        if (data.stargazers_count !== undefined) {
          $('.github-stars-count').text(data.stargazers_count);
        }
        if (data.forks_count !== undefined) {
          $('.github-forks-count').text(data.forks_count);
        }
        if (data.open_issues_count !== undefined) {
          $('.github-issues-count').text(data.open_issues_count);
        }
        if (data.language) {
          $('.github-primary-lang').text(data.language);
        }
        if (data.size) {
          const sizeMB = (data.size / 1024).toFixed(1);
          $('.github-repo-size').text(`~${sizeMB} MB`);
        }
      },
      error: function () {
        console.log('GitHub repo info fetch fallback active.');
      }
    });

    // Fetch Latest Release Assets
    $.ajax({
      url: `${API_BASE}/releases/latest`,
      method: 'GET',
      dataType: 'json',
      success: function (release) {
        if (release.tag_name) {
          $('.github-release-tag').text(release.tag_name);
        }
        if (release.assets && release.assets.length > 0) {
          let totalDownloads = 0;
          release.assets.forEach(asset => {
            totalDownloads += asset.download_count || 0;
            if (asset.name.toLowerCase().endsWith('.exe')) {
              $('.github-download-setup').attr('href', asset.browser_download_url);
              const sizeMB = (asset.size / (1024 * 1024)).toFixed(1);
              $('.github-setup-size').text(`(${sizeMB} MB)`);
            }
            if (asset.name.toLowerCase().endsWith('.zip')) {
              $('.github-download-portable').attr('href', asset.browser_download_url);
            }
          });
          $('.github-setup-downloads').html(`<i class="fas fa-download me-1"></i> ${totalDownloads} Downloads`);
        }
      },
      error: function () {
        console.log('GitHub release assets fallback active.');
      }
    });

    // Fetch Recent Commits Feed
    if ($('#github-commit-feed').length) {
      $.ajax({
        url: `${API_BASE}/commits?per_page=5`,
        method: 'GET',
        dataType: 'json',
        success: function (commits) {
          let html = '<div class="divide-y divide-emerald-500/10">';
          commits.forEach(item => {
            const message = item.commit.message.split('\n')[0];
            const authorName = item.commit.author.name;
            const authorAvatar = item.author ? item.author.avatar_url : 'https://github.com/github.png';
            const commitUrl = item.html_url;
            const commitSha = item.sha.substring(0, 7);
            const commitDate = new Date(item.commit.author.date).toLocaleDateString();

            html += `
              <div class="py-3 d-flex align-items-center justify-content-between gap-3">
                <div class="d-flex align-items-center gap-2 overflow-hidden">
                  <img src="${authorAvatar}" width="28" height="28" class="rounded-circle border border-emerald-500/30" alt="${authorName}">
                  <div class="text-truncate">
                    <a href="${commitUrl}" target="_blank" class="text-sm font-semibold hover:text-emerald-400 text-decoration-none" style="color: var(--text-heading);">${message}</a>
                    <div class="text-xs text-slate-400 font-mono">${authorName} • ${commitDate}</div>
                  </div>
                </div>
                <a href="${commitUrl}" target="_blank" class="badge bg-emerald-500/10 text-emerald-400 border border-emerald-500/20 font-mono text-xs text-decoration-none">${commitSha}</a>
              </div>
            `;
          });
          html += '</div>';
          $('#github-commit-feed').html(html);
        },
        error: function () {
          $('#github-commit-feed').html('<p class="text-xs text-slate-400 mb-0 font-mono"><i class="fas fa-info-circle me-1"></i> Public API rate limited or offline.</p>');
        }
      });
    }

    // Fetch Contributors Wall
    if ($('#github-contributors-list').length) {
      $.ajax({
        url: `${API_BASE}/contributors`,
        method: 'GET',
        dataType: 'json',
        success: function (contributors) {
          let html = '<div class="d-flex flex-wrap gap-2">';
          contributors.forEach(user => {
            html += `
              <a href="${user.html_url}" target="_blank" class="d-flex align-items-center gap-2 p-2 rounded-xl border border-emerald-500/20 text-decoration-none hover:border-emerald-400 transition-all" style="background: var(--bg-surface-alt);">
                <img src="${user.avatar_url}" width="32" height="32" class="rounded-circle" alt="${user.login}">
                <div class="pe-1">
                  <div class="text-xs font-bold" style="color: var(--text-heading);">${user.login}</div>
                  <div class="text-[10px] text-amber-500 font-mono">${user.contributions} commits</div>
                </div>
              </a>
            `;
          });
          html += '</div>';
          $('#github-contributors-list').html(html);
        },
        error: function () {
          $('#github-contributors-list').html('<p class="text-xs text-slate-400 mb-0">Contributors list unavailable.</p>');
        }
      });
    }
  }

  fetchGitHubRepoMetrics();

  // ----------------------------------------------------------------------
  // 3. Live Interactive Typing & Avro Phonetic Transliteration Engine
  // ----------------------------------------------------------------------

  // Lightweight JavaScript Avro Transliteration Parser
  function parseAvroWord(word) {
    if (!word) return "";
    
    // Quick dictionary overrides for common words
    const commonWords = {
      "ami": "আমি", "tumi": "তুমি", "amra": "আমরা", "tomra": "তোমরা",
      "bangla": "বাংলা", "bangladesh": "বাংলাদেশ", "dhaka": "ঢাকা",
      "omor": "অমর", "ekushe": "একুশে", "bhasha": "ভাষা", "gan": "গান",
      "gai": "গাই", "desh": "দেশ", "sonar": "সোনার", "shadhinota": "স্বাধীনতা",
      "bhalobashi": "ভালোবাসি", "dhonnobad": "ধন্যবাদ", "shagotom": "স্বাগতম",
      "kemon": "কেমন", "acho": "আছো", "achhi": "আছি", "valobasha": "ভালোবাসা"
    };
    const lower = word.toLowerCase();
    if (commonWords[lower]) {
      return commonWords[lower];
    }

    const conjunctRules = [
      ["kkh", "ক্ষ"], ["kkhN", "ক্ষ্ণ"], ["kkhm", "ক্ষ্ম"], ["gdh", "গ্ধ"],
      ["cch", "চ্ছ"], ["cchr", "চ্ছ্ব"], ["jng", "জ্ঞ"], ["jbh", "জ্ব"],
      ["TTh", "ট্ঠ"], ["DDh", "ড্ড"], ["ND", "ণ্ড"], ["NDh", "ণ্ঠ"], ["NN", "ণ্ণ"],
      ["tth", "ত্থ"], ["ttr", "ত্র"], ["tm", "ত্ম"], ["thb", "থ্ব"],
      ["ddh", "দ্ধ"], ["dgh", "দ্ঘ"], ["dbh", "দ্ব"], ["dhm", "ধ্ম"],
      ["nt", "ন্ত"], ["nth", "ন্থ"], ["nd", "ন্দ"], ["ndh", "ন্ধ"], ["nn", "ন্ন"],
      ["pl", "প্ল"], ["ps", "প্স"], ["bd", "ব্দ"], ["bdh", "ব্ধ"], ["bb", "ব্ব"],
      ["mps", "ম্প"], ["mph", "ম্ফ"], ["mb", "ম্ব"], ["mbh", "ম্ভ"], ["mm", "ম্ম"],
      ["shc", "শ্চ"], ["shch", "শ্ছ"], ["sht", "শ্ত"], ["shn", "শ্ন"], ["shb", "শ্ব"], ["shm", "শ্ম"], ["shl", "শ্ল"],
      ["Shk", "ষ্ক"], ["Shkh", "ষ্খ"], ["ShT", "ষ্ট"], ["ShTh", "ষ্ঠ"], ["ShN", "ষ্ণ"], ["Shp", "ষ্প"], ["Shph", "ষ্ফ"], ["Shm", "ষ্ম"],
      ["sk", "স্ক"], ["skh", "স্খ"], ["st", "স্ত"], ["sth", "স্থ"], ["sn", "স্ন"], ["sp", "স্প"], ["sph", "স্ফ"], ["sb", "স্ব"], ["sm", "স্ম"], ["sl", "স্ল"],
      ["hN", "হ্ণ"], ["hn", "হ্ন"], ["hb", "হ্ব"], ["hm", "হ্ম"], ["hl", "হ্ল"],
      ["k", "ক"], ["kh", "খ"], ["g", "গ"], ["gh", "ঘ"], ["Ng", "ঙ"],
      ["c", "চ"], ["ch", "ছ"], ["j", "জ"], ["jh", "ঝ"], ["NG", "ঞ"],
      ["T", "ট"], ["Th", "ঠ"], ["D", "ড"], ["Dh", "ঢ"], ["N", "ণ"],
      ["t", "ত"], ["th", "থ"], ["d", "দ"], ["dh", "ধ"], ["n", "ন"],
      ["p", "প"], ["ph", "ফ"], ["f", "ফ"], ["b", "ব"], ["bh", "ভ"], ["v", "ভ"], ["m", "ম"],
      ["z", "য"], ["r", "র"], ["l", "ল"], ["sh", "শ"], ["Sh", "ষ"], ["s", "স"], ["h", "হ"],
      ["R", "ড়"], ["Rh", "ঢ়"], ["y", "য়"], ["Y", "য়"]
    ];

    const vowelFull = {
      "aa": "আ", "a": "অ", "A": "আ", "i": "ই", "I": "ঈ", "ee": "ঈ",
      "u": "উ", "U": "ঊ", "oo": "ঊ", "e": "এ", "E": "এ", "oi": "ঐ",
      "OI": "ঐ", "o": "ও", "O": "ও", "ou": "ঔ", "OU": "ঔ", "rri": "ঋ"
    };

    const vowelKar = {
      "aa": "া", "a": "", "A": "া", "i": "ি", "I": "ী", "ee": "ী",
      "u": "ু", "U": "ূ", "oo": "ূ", "e": "ে", "E": "ে", "oi": "ৈ",
      "OI": "ৈ", "o": "ো", "O": "ো", "ou": "ৌ", "OU": "ৌ", "rri": "ৃ"
    };

    let result = "";
    let i = 0;
    let prevWasConsonant = false;

    while (i < word.length) {
      if (word[i] === '.') {
        result += "।";
        prevWasConsonant = false;
        i++;
        continue;
      }

      // Check vowels
      let matchedVowel = false;
      const two = word.substring(i, i + 2);
      const three = word.substring(i, i + 3);
      const one = word.substring(i, i + 1);

      if (three === "rri") {
        result += prevWasConsonant ? vowelKar["rri"] : vowelFull["rri"];
        i += 3;
        prevWasConsonant = false;
        continue;
      }
      if (vowelFull[two]) {
        result += prevWasConsonant ? vowelKar[two] : vowelFull[two];
        i += 2;
        prevWasConsonant = false;
        continue;
      }
      if (vowelFull[one]) {
        if (one === 'a' && prevWasConsonant) {
          // Implicit 'o' or skip kar
        } else {
          result += prevWasConsonant ? vowelKar[one] : vowelFull[one];
        }
        i += 1;
        prevWasConsonant = false;
        continue;
      }

      // Check consonants and conjuncts
      let matchedCons = false;
      for (const [pattern, bengali] of conjunctRules) {
        if (word.startsWith(pattern, i)) {
          result += bengali;
          i += pattern.length;
          prevWasConsonant = true;
          matchedCons = true;
          break;
        }
      }

      if (!matchedCons) {
        result += word[i];
        prevWasConsonant = false;
        i++;
      }
    }

    return result;
  }

  function transliterateText(input) {
    return input.replace(/[a-zA-Z0-9\.]+/g, function (token) {
      return parseAvroWord(token);
    });
  }

  const layoutMaps = {
    phonetic: {
      name: "Phonetic (Avro • F12)",
      sample: "ami banglay gan gai, ami banglar gan gai. omor ekushe amader gorbo!",
      isPhonetic: true
    },
    bijoy: {
      name: "Bijoy (বিজয়)",
      sample: "আমার সোনার বাংলা, আমি তোমায় ভালোবাসি। অমর একুশে গ্রন্থমেলা।",
      isPhonetic: false,
      keymap: { 'a': 'া', 'b': 'ন', 'c': 'চ', 'd': 'ড', 'e': 'ে', 'f': 'ফ', 'g': 'গ', 'h': 'হ', 'i': 'ি', 'j': 'জ', 'k': 'ক', 'l': 'ল', 'm': 'ম', 'n': 'স', 'o': 'গ', 'p': 'প', 'q': 'ং', 'r': 'র', 's': 'ু', 't': 'ট', 'u': 'ু', 'v': 'র', 'w': 'য', 'x': 'হ', 'y': 'থ', 'z': 'ধ' }
    },
    unijoy: {
      name: "Unijoy (ইউনিজয়)",
      sample: "মোদের গরব, মোদের আশা, আ-মরি বাংলা ভাষা!",
      isPhonetic: false,
      keymap: { 'a': 'ো', 'b': 'ব', 'c': 'চ', 'd': 'দ', 'e': 'ে', 'f': 'ফ', 'g': 'গ', 'h': 'হ', 'i': 'ি', 'j': 'জ', 'k': 'ক', 'l': 'ল', 'm': 'ম', 'n': 'ন', 'o': 'গ', 'p': 'প', 'r': 'র', 's': 'স', 't': 'ত', 'u': 'ু', 'v': 'ভ', 'w': 'ও', 'y': 'য়' }
    },
    english: {
      name: "English",
      sample: "Omor Ekushe Win32 Native Bengali Keyboard Engine for Windows.",
      isPhonetic: false,
      keymap: {}
    }
  };

  let activeLayout = 'phonetic';

  function activateLayout(layoutKey) {
    if (!layoutMaps[layoutKey]) return;
    activeLayout = layoutKey;
    $('.layout-pill-btn').removeClass('active');
    $(`.layout-pill-btn[data-layout="${layoutKey}"]`).addClass('active');
    $('#active-layout-name').text(layoutMaps[layoutKey].name);
    
    if (layoutKey === 'phonetic') {
      $('#simulator-input').val("আমি বাংলায় গান গাই, আমি বাংলার গান গাই। অমর একুশে আমাদের গর্ব!");
      $('#active-hotkey-badge').html('<i class="fas fa-bolt me-1 text-amber"></i> Press F12 or Ctrl+Alt+B');
    } else {
      $('#simulator-input').val(layoutMaps[layoutKey].sample);
      $('#active-hotkey-badge').html('<i class="fas fa-keyboard me-1 text-emerald"></i> Press Ctrl+Alt+B');
    }
  }

  $('.layout-pill-btn').on('click', function () {
    const layout = $(this).data('layout');
    activateLayout(layout);
  });

  // Real-time phonetic input conversion on keyup/input in simulator
  let isComposing = false;
  $('#simulator-input').on('input', function () {
    if (activeLayout === 'phonetic' && !isComposing) {
      const currentVal = $(this).val();
      // If user typed Latin letters, transliterate words ending with space or punctuation
      const words = currentVal.split(/(\s+)/);
      let changed = false;
      const convertedWords = words.map(chunk => {
        if (/^[a-zA-Z\.]+$/.test(chunk)) {
          changed = true;
          return parseAvroWord(chunk);
        }
        return chunk;
      });

      if (changed) {
        const caret = this.selectionStart;
        $(this).val(convertedWords.join(''));
        this.setSelectionRange(caret, caret);
      }
    }
  });

  $('#simulator-input').on('keydown', function (e) {
    // F12 Toggle: Instant Phonetic <-> English switch
    if (e.key === 'F12') {
      e.preventDefault();
      const next = activeLayout === 'phonetic' ? 'english' : 'phonetic';
      activateLayout(next);

      $('#active-hotkey-badge').addClass('bg-red-500 text-white').removeClass('bg-emerald-950 text-emerald-400');
      setTimeout(() => {
        $('#active-hotkey-badge').removeClass('bg-red-500 text-white').addClass('bg-emerald-950 text-emerald-400');
      }, 500);
      return;
    }

    // Ctrl+Alt+B layout cycle
    if (e.ctrlKey && e.altKey && e.key.toLowerCase() === 'b') {
      e.preventDefault();
      const layouts = ['phonetic', 'bijoy', 'unijoy', 'english'];
      let nextIndex = (layouts.indexOf(activeLayout) + 1) % layouts.length;
      activateLayout(layouts[nextIndex]);

      $('#active-hotkey-badge').addClass('bg-amber-500 text-black').removeClass('bg-emerald-950 text-emerald-400');
      setTimeout(() => {
        $('#active-hotkey-badge').removeClass('bg-amber-500 text-black').addClass('bg-emerald-950 text-emerald-400');
      }, 500);
    }
  });

  // ----------------------------------------------------------------------
  // 4. Code Copy to Clipboard Toast
  // ----------------------------------------------------------------------
  $('.btn-copy-code').on('click', function () {
    const targetId = $(this).data('target');
    const codeText = $('#' + targetId).text();
    const btn = $(this);

    navigator.clipboard.writeText(codeText).then(function () {
      const originalHtml = btn.html();
      btn.html('<i class="fas fa-check text-emerald-400 me-1"></i> Copied!').addClass('text-emerald-400');
      setTimeout(function () {
        btn.html(originalHtml).removeClass('text-emerald-400');
      }, 2000);
    }).catch(function (err) {
      console.error('Copy failed: ', err);
    });
  });

  // ----------------------------------------------------------------------
  // 5. Back to Top Scroll Button
  // ----------------------------------------------------------------------
  const backToTopBtn = $('#btn-back-to-top');

  $(window).on('scroll', function () {
    if ($(this).scrollTop() > 300) {
      backToTopBtn.addClass('show');
    } else {
      backToTopBtn.removeClass('show');
    }
  });

  backToTopBtn.on('click', function () {
    $('html, body').animate({ scrollTop: 0 }, 400);
  });
});
