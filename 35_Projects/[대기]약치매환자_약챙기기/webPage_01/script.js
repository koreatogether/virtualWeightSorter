const DAYS = ["월요일", "화요일", "수요일", "목요일", "금요일", "토요일", "일요일"];
const SLOTS = ["아침", "점심", "저녁"];

const weekRing = document.getElementById("weekRing");
const dayRing = document.getElementById("dayRing");
const dayLabel = document.getElementById("dayLabel");
const slotLabel = document.getElementById("slotLabel");
const stepLabel = document.getElementById("stepLabel");
const playBtn = document.getElementById("playBtn");
const stepBtn = document.getElementById("stepBtn");
const resetBtn = document.getElementById("resetBtn");

let dayIndex = 0;
let slotIndex = 0;
let dayTurns = 0;
let slotTurns = 0;
let timer = null;

function placeSegment(el, angleDeg, radius) {
  const rad = (angleDeg * Math.PI) / 180;
  const x = Math.cos(rad) * radius;
  const y = Math.sin(rad) * radius;
  el.style.transform = `translate(-50%, -50%) translate(${x}px, ${y}px)`;
}

function createSegments() {
  const weekRadius = 150;
  const dayRadius = 120;

  DAYS.forEach((name, i) => {
    const angle = (360 / DAYS.length) * i - 90;
    const seg = document.createElement("div");

    seg.className = "week-segment";
    seg.dataset.index = i.toString();
    seg.dataset.baseAngle = angle.toString();
    seg.dataset.radius = weekRadius.toString();
    seg.textContent = name.replace("요일", "");
    placeSegment(seg, angle, weekRadius);

    weekRing.appendChild(seg);
  });

  SLOTS.forEach((name, i) => {
    const angle = (360 / SLOTS.length) * i - 90;
    const seg = document.createElement("div");

    seg.className = "day-segment";
    seg.dataset.index = i.toString();
    seg.dataset.baseAngle = angle.toString();
    seg.dataset.radius = dayRadius.toString();
    seg.textContent = name;
    placeSegment(seg, angle, dayRadius);

    dayRing.appendChild(seg);
  });
}

function refreshLabels() {
  dayLabel.textContent = DAYS[dayIndex];
  slotLabel.textContent = SLOTS[slotIndex];
  stepLabel.textContent = `${dayIndex * 3 + slotIndex + 1} / 21`;
}

function refreshActive() {
  document.querySelectorAll(".week-segment").forEach((el) => {
    el.classList.toggle("active", Number(el.dataset.index) === dayIndex);
  });

  document.querySelectorAll(".day-segment").forEach((el) => {
    el.classList.toggle("active", Number(el.dataset.index) === slotIndex);
  });
}

function refreshRotation() {
  const weekAngle = -(360 / DAYS.length) * dayTurns;
  const slotAngle = -(360 / SLOTS.length) * slotTurns;

  document.querySelectorAll(".week-segment").forEach((el) => {
    const base = Number(el.dataset.baseAngle);
    const radius = Number(el.dataset.radius);
    placeSegment(el, base + weekAngle, radius);
  });

  document.querySelectorAll(".day-segment").forEach((el) => {
    const base = Number(el.dataset.baseAngle);
    const radius = Number(el.dataset.radius);
    placeSegment(el, base + slotAngle, radius);
  });
}

function render() {
  refreshLabels();
  refreshActive();
  refreshRotation();
}

function stepForward() {
  slotTurns += 1;
  slotIndex = (slotIndex + 1) % SLOTS.length;

  if (slotIndex === 0) {
    dayTurns += 1;
    dayIndex = (dayIndex + 1) % DAYS.length;
  }

  render();
}

function togglePlay() {
  if (timer) {
    clearInterval(timer);
    timer = null;
    playBtn.textContent = "자동 재생";
    return;
  }

  playBtn.textContent = "일시정지";
  timer = setInterval(stepForward, 1200);
}

function resetAll() {
  if (timer) {
    clearInterval(timer);
    timer = null;
    playBtn.textContent = "자동 재생";
  }

  dayIndex = 0;
  slotIndex = 0;
  dayTurns = 0;
  slotTurns = 0;
  render();
}

playBtn.addEventListener("click", togglePlay);
stepBtn.addEventListener("click", stepForward);
resetBtn.addEventListener("click", resetAll);

createSegments();
render();
