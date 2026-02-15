const TRAY_COUNT = 12;
const MIN_W = 600;
const MAX_W = 700;
const DEFAULT_TARGET_G = 2000;
const DEFAULT_TOLERANCE_G = 90;
const BOX_LIMIT = 4;

const runBtn = document.getElementById("runBtn");
const startBtn = document.getElementById("startBtn");
const trayGrid = document.getElementById("trayGrid");
const resultList = document.getElementById("resultList");
const targetKgInput = document.getElementById("targetKgInput");
const itemCountInput = document.getElementById("itemCountInput");
const toleranceInput = document.getElementById("toleranceInput");
const weightMinInput = document.getElementById("weightMinInput");
const weightMaxInput = document.getElementById("weightMaxInput");

let weights = [];
let boxes = [];

let xorState = 2463534242;

function setSeed(seed) {
  const normalized = seed >>> 0;
  xorState = normalized === 0 ? 2463534242 : normalized;
}

function xorshift32() {
  xorState ^= xorState << 13;
  xorState ^= xorState >>> 17;
  xorState ^= xorState << 5;
  xorState >>>= 0;
  return xorState / 4294967296;
}

function randInt(min, max) {
  return Math.floor(xorshift32() * (max - min + 1)) + min;
}

function createWeights(weightMin, weightMax) {
  return Array.from({ length: TRAY_COUNT }, () => randInt(weightMin, weightMax));
}

function parseSettings() {
  const targetKgRaw = targetKgInput.value.trim();
  const itemCountRaw = itemCountInput.value.trim();
  const toleranceRaw = toleranceInput.value.trim();
  const weightMinRaw = weightMinInput.value.trim();
  const weightMaxRaw = weightMaxInput.value.trim();

  const targetKg = targetKgRaw === "" ? 2 : Number(targetKgRaw);
  const targetG = Number.isFinite(targetKg) && targetKg > 0 ? Math.round(targetKg * 1000) : DEFAULT_TARGET_G;

  const requestedCount = itemCountRaw === "" ? null : Number(itemCountRaw);
  const itemCount =
    Number.isInteger(requestedCount) && requestedCount >= 1 && requestedCount <= TRAY_COUNT
      ? requestedCount
      : null;

  const toleranceVal = toleranceRaw === "" ? DEFAULT_TOLERANCE_G : Number(toleranceRaw);
  const toleranceG = Number.isFinite(toleranceVal) && toleranceVal >= 0 ? Math.round(toleranceVal) : DEFAULT_TOLERANCE_G;

  const minVal = weightMinRaw === "" ? MIN_W : Number(weightMinRaw);
  const maxVal = weightMaxRaw === "" ? MAX_W : Number(weightMaxRaw);
  const safeMin = Number.isFinite(minVal) && minVal >= 1 ? Math.round(minVal) : MIN_W;
  const safeMax = Number.isFinite(maxVal) && maxVal >= 1 ? Math.round(maxVal) : MAX_W;
  const weightMin = Math.min(safeMin, safeMax);
  const weightMax = Math.max(safeMin, safeMax);

  return {
    targetG,
    toleranceG,
    itemCount,
    weightMin,
    weightMax,
  };
}

function findCandidates(currentWeights, settings) {
  const candidates = [];
  const maxMask = 1 << currentWeights.length;

  for (let mask = 1; mask < maxMask; mask += 1) {
    let sum = 0;
    const indices = [];

    for (let i = 0; i < currentWeights.length; i += 1) {
      if (mask & (1 << i)) {
        sum += currentWeights[i];
        indices.push(i);
      }
    }

    if (settings.itemCount !== null && indices.length !== settings.itemCount) {
      continue;
    }

    const diff = Math.abs(sum - settings.targetG);
    if (diff <= settings.toleranceG) {
      candidates.push({
        mask,
        indices,
        sum,
        diff,
      });
    }
  }

  candidates.sort((a, b) => a.diff - b.diff || a.indices.length - b.indices.length || a.sum - b.sum);
  return candidates;
}

function pickNonOverlapping(candidates, limit = BOX_LIMIT) {
  const selected = [];
  let usedMask = 0;

  for (const c of candidates) {
    if ((usedMask & c.mask) === 0) {
      selected.push(c);
      usedMask |= c.mask;
      if (selected.length >= limit) {
        break;
      }
    }
  }

  return selected;
}

function renderTrays() {
  trayGrid.innerHTML = "";

  for (let i = 0; i < TRAY_COUNT; i += 1) {
    const tray = document.createElement("article");
    tray.className = "tray";
    tray.dataset.index = String(i);

    const head = document.createElement("div");
    head.className = "tray-head";
    head.innerHTML = `<span>접시 ${i + 1}</span><span>#${i + 1}</span>`;

    const grape = document.createElement("div");
    grape.className = "grape";
    grape.textContent = weights.length ? `${weights[i]}g` : "-";

    tray.append(head, grape);
    trayGrid.appendChild(tray);
  }
}

function clearTrayColors() {
  const trays = trayGrid.querySelectorAll(".tray");
  trays.forEach((tray) => {
    tray.dataset.box = "";
    tray.classList.remove("active");
  });
}

function colorBoxes(selectedBoxes) {
  clearTrayColors();
  selectedBoxes.forEach((box, boxIdx) => {
    const boxNo = boxIdx + 1;
    box.indices.forEach((idx) => {
      const tray = trayGrid.querySelector(`.tray[data-index="${idx}"]`);
      if (tray) {
        tray.dataset.box = String(boxNo);
        tray.classList.add("active");
      }
    });
  });
}

function renderResults(selectedBoxes, settings) {
  resultList.innerHTML = "";

  if (!selectedBoxes.length) {
    const countText = settings.itemCount === null ? "개수 제한 없음" : `${settings.itemCount}개 고정`;
    resultList.textContent = `조건(${settings.targetG}g ±${settings.toleranceG}g, ${countText})을 만족하는 조합을 찾지 못했습니다.`;
    return;
  }

  selectedBoxes.forEach((box, idx) => {
    const el = document.createElement("div");
    el.className = `result-item box-${idx + 1}`;

    const indexText = box.indices.map((i) => i + 1).join(", ");
    el.innerHTML = `
      <strong>Box ${idx + 1}</strong>
      <div>총무게: ${box.sum}g</div>
      <div>오차: ${box.diff}g</div>
      <div>개수: ${box.indices.length}개</div>
      <div>선택 인덱스: [${indexText}]</div>
    `;

    resultList.appendChild(el);
  });
}

runBtn.addEventListener("click", () => {
  const seed = (Date.now() ^ (performance.now() * 1000)) >>> 0;
  setSeed(seed);

  const settings = parseSettings();
  weights = createWeights(settings.weightMin, settings.weightMax);

  boxes = [];
  renderTrays();
  clearTrayColors();
  resultList.textContent = `포도 무게 생성 범위: ${settings.weightMin}g ~ ${settings.weightMax}g. 조합 시작 버튼을 눌러 최적 조합을 탐색하세요.`;
  startBtn.disabled = false;
});

startBtn.addEventListener("click", () => {
  if (!weights.length) {
    return;
  }
  const settings = parseSettings();
  const candidates = findCandidates(weights, settings);
  boxes = pickNonOverlapping(candidates, BOX_LIMIT);
  colorBoxes(boxes);
  renderResults(boxes, settings);
});

renderTrays();
