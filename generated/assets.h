// 本文件由 tools/embed.py 自动生成，请不要手改。
// 内容来源：assets/anim.html、assets/anim.js
// 改过资源后重新运行：  python tools/embed.py
#pragma once

// assets/anim.html
inline constexpr char kAnimTemplate[] = R"ASSET(<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<title>A* 寻路过程</title>
<style>
  body{background:#161a1f;color:#d7dde5;margin:0;padding:24px;
       font:14px/1.6 system-ui,"Microsoft YaHei","PingFang SC",sans-serif}
  h1{font-size:17px;font-weight:600;margin:0 0 2px}
  .meta{color:#8b97a6;font-size:13px;margin-bottom:14px}
  .bar{display:flex;flex-wrap:wrap;gap:10px;align-items:center;margin-bottom:14px}
  button{background:#2b333d;color:#d7dde5;border:1px solid #3d4653;border-radius:7px;
         padding:6px 14px;font:inherit;cursor:pointer}
  button:hover{background:#36404c}
  input[type=range]{width:150px;vertical-align:middle}
  #hud{color:#8b97a6;font-size:13px;margin-left:auto}
  #grid{display:grid;gap:2px;width:max-content;padding:10px;background:#0f1317;border-radius:10px}
  .cell{width:22px;height:22px;border-radius:4px;background:#232a33;
        display:flex;align-items:center;justify-content:center;
        font-size:11px;font-weight:700;color:#0f1317;transition:background .07s}
  .wall{background:#39424e}
  .frontier{background:#e0a458}
  .closed{background:#3d7ebf}
  .path{background:#4ec9a0}
  .current{background:#ffe066;box-shadow:0 0 10px #ffe066}
  .start{outline:2px solid #7ee787;outline-offset:-2px}
  .end{outline:2px solid #ff7b72;outline-offset:-2px}
  .legend{display:flex;gap:16px;flex-wrap:wrap;margin-top:14px;color:#8b97a6;font-size:13px}
  .legend i{display:inline-block;width:12px;height:12px;border-radius:3px;margin-right:6px;
            vertical-align:-1px}
</style>
</head>
<body>
<h1>A* 寻路过程</h1>
<div class="meta" id="meta"></div>
<div class="bar">
  <button id="play">播放</button>
  <button id="step">单步</button>
  <button id="back">上一步</button>
  <button id="reset">重置</button>
  <button id="jump">跳到结束</button>
  <label>速度 <input type="range" id="speed" min="5" max="400" step="5"><span id="spv"></span> ms/帧</label>
  <span id="hud"></span>
</div>
<div id="grid"></div>
<div class="legend">
  <span><i style="background:#232a33"></i>未访问</span>
  <span><i style="background:#39424e"></i>墙</span>
  <span><i style="background:#e0a458"></i>边界（已发现，待扩展）</span>
  <span><i style="background:#3d7ebf"></i>已扩展</span>
  <span><i style="background:#ffe066"></i>当前扩展</span>
  <span><i style="background:#4ec9a0"></i>最终路径</span>
</div>
<div id="note" class="meta"></div>
<script>
@@DATA@@

@@SCRIPT@@
</script>
</body>
</html>
)ASSET";

// assets/anim.js
inline constexpr char kAnimScript[] = R"ASSET(const cells = [];
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
setTimeout(play, 400);)ASSET";
