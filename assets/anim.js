const cells = [];
const state = new Int8Array(N * N);          // 0 未访问 1 边界 2 已扩展 3 路径
const grid = document.getElementById('grid');
grid.style.gridTemplateColumns = 'repeat(' + N + ', 22px)';

for (let i = 0; i < N * N; i++) {
  const d = document.createElement('div');
  grid.appendChild(d);
  cells.push(d);
}

document.getElementById('meta').textContent =
  '启发函数 ' + HEURISTIC +
  '　规模 ' + N + '×' + N +
  '　起点 (' + Math.floor(START / N) + ', ' + (START % N) + ')' +
  '　终点 (' + Math.floor(END / N) + ', ' + (END % N) + ')' +
  '　搜索事件 ' + EV.length + ' 次　路径 ' + ROUTE.length + ' 格';
if (!ROUTE.length) document.getElementById('note').textContent = '（这张图没有通路，动画到搜索结束为止）';

let k = 0, cur = -1, prevCur = -1, last = -1, pathDrawn = 0, running = false, timer = null;

function paint(i) {
  if (i < 0) return;
  const d = cells[i];
  d.className = 'cell';
  if (MAP[i] === '0') d.classList.add('wall');
  d.textContent = (i === START) ? 'S' : (i === END) ? 'E' : '';
  if (i === START) d.classList.add('start');
  if (i === END) d.classList.add('end');
  if (state[i] === 1) d.classList.add('frontier');
  else if (state[i] === 2) d.classList.add('closed');
  else if (state[i] === 3) d.classList.add('path');
  if (i === cur) d.classList.add('current');
}

function hud() {
  let frontier = 0, closed = 0;
  for (let i = 0; i < N * N; i++) {
    if (state[i] === 1) frontier++;
    else if (state[i] === 2) closed++;
  }
  document.getElementById('hud').textContent =
    '事件 ' + k + '/' + EV.length + '　边界 ' + frontier + '　已扩展 ' + closed +
    (pathDrawn ? '　路径 ' + pathDrawn + '/' + ROUTE.length : '');
}

// 推进一帧：先把搜索事件放完，再从起点往终点逐格点亮路径
function applyOne() {
  if (k < EV.length) {
    const t = EV[k][0], i = +EV[k].slice(1);
    if (state[i] !== 2) state[i] = (t === 'P') ? 1 : 2;
    prevCur = cur; cur = i; last = i; k++;
    return true;
  }
  if (pathDrawn < ROUTE.length) {
    const i = ROUTE[pathDrawn++];
    prevCur = cur; cur = i; last = i;      // 路径逐格点亮，当前这一格高亮
    state[i] = 3;
    return true;
  }
  return false;
}

function step() {
  if (!applyOne()) return false;
  paint(prevCur); paint(last); hud();      // 画的是 last（本帧真正动过的那一格），不是 cur
  return true;
}

function repaintAll() { for (let i = 0; i < N * N; i++) paint(i); hud(); }

function seek(target) {
  pause();
  state.fill(0); k = 0; pathDrawn = 0; cur = -1; prevCur = -1; last = -1;
  while (k + pathDrawn < target && applyOne()) {}
  if (k + pathDrawn >= EV.length + ROUTE.length) cur = -1;   // 放完了就不留高亮
  repaintAll();
}

const speed = document.getElementById('speed');
speed.value = DELAY;
document.getElementById('spv').textContent = DELAY;
speed.oninput = () => { document.getElementById('spv').textContent = speed.value; };

function tick() {
  if (!running) return;
  if (!step()) { cur = -1; paint(last); hud(); pause(); return; }   // 收尾：去掉最后一格的高亮
  timer = setTimeout(tick, +speed.value);
}
function play() {
  if (running) return;
  if (k + pathDrawn >= EV.length + ROUTE.length) seek(0);
  running = true;
  document.getElementById('play').textContent = '暂停';
  tick();
}
function pause() {
  running = false;
  clearTimeout(timer);
  document.getElementById('play').textContent = '播放';
}

document.getElementById('play').onclick  = () => running ? pause() : play();
document.getElementById('step').onclick  = () => { pause(); step(); };
document.getElementById('back').onclick  = () => seek(k + pathDrawn - 1);
document.getElementById('reset').onclick = () => seek(0);
document.getElementById('jump').onclick  = () => seek(EV.length + ROUTE.length);

repaintAll();
setTimeout(play, 400);
