"""Offline S1 evidence from the native CSV; stage B only reads frozen directions."""
import argparse
import csv
import hashlib
import math
from pathlib import Path
from statistics import median

SIGNALS = [('axis', 'axis_deflection_deg'), ('u=0', 'u0_deflection_deg'),
           ('u=0.5', 'u05_deflection_deg'), ('u=1', 'u1_deflection_deg')]
ANCHORS = [436, 441, 444, 448, 455, 460, 461, 462]


def unwrap(values):
    out = []
    for v in values:
        out.append(v if not out else out[-1] + math.remainder(v-out[-1], 360))
    return out


def statistics(rows, key):
    values = unwrap([r[key] for r in rows])
    steps = [b-a for a, b in zip(values, values[1:])]
    inc = [int(rows[i]['commit_tick']) for i, d in enumerate(steps) if d > 1e-8]
    dec = [int(rows[i]['commit_tick']) for i, d in enumerate(steps) if d < -1e-8]
    flat = [int(rows[i]['commit_tick']) for i, d in enumerate(steps) if abs(d) <= 1e-8]
    largest = max(range(len(steps)), key=lambda i: abs(steps[i]))
    return (f'{min(values):.6f}…{max(values):.6f} | {max(values)-min(values):.6f} | '
            f'{len(inc)}/{len(dec)}/{len(flat)} | {abs(steps[largest]):.6f} '
            f'({int(rows[largest]["commit_tick"])}→{int(rows[largest+1]["commit_tick"])}) | '
            f'增加起點 {inc}；持平起點 {flat}')


def plot(rows, curves, output, field=False):
    # Concrete offline timing plot for this study, with no game/runtime dependency.
    colors = ['#1565c0', '#b45400', '#00856a', '#9227a8', '#444444']
    parts = ['<svg xmlns="http://www.w3.org/2000/svg" width="1100" height="650" viewBox="0 0 1100 650">',
             '<rect width="1100" height="650" fill="white"/>',
             '<g font-family="sans-serif" font-size="14" fill="#222">']
    x = lambda tick: 85+(tick-rows[0]['commit_tick'])/(rows[-1]['commit_tick']-rows[0]['commit_tick'])*960
    y = lambda angle: 540-(angle+130)/220*440
    title = 'Stage B: frozen world directions / geometric sector' if field else 'Stage A: normal-free signed deflection / incoming return reference'
    parts.append(f'<text x="85" y="30" font-size="20">{title}</text>')
    if field:
        parts.append(f'<rect x="85" y="{y(45)}" width="960" height="{y(-45)-y(45)}" fill="#fff4c2"/>')
    for angle in range(-120, 81, 20):
        parts.append(f'<path d="M85 {y(angle)}H1045" stroke="#dddddd"/><text x="30" y="{y(angle)+5}">{angle}</text>')
    for tick in ANCHORS:
        label_y = 580 if tick == 461 else 565
        parts.append(f'<path d="M{x(tick)} 100V540" stroke="#dddddd"/><text x="{x(tick)}" y="{label_y}" text-anchor="middle">{tick}</text>')
    if field:
        for angle in [-45, 45]:
            parts.append(f'<path d="M85 {y(angle)}H1045" stroke="#888" stroke-dasharray="6 4"/><text x="90" y="{y(angle)-5}">{angle} deg</text>')
    for index, (name, key) in enumerate(curves):
        points = ' '.join(f'{x(r["commit_tick"]):.3f},{y(r[key]):.3f}' for r in rows)
        color = colors[index]
        parts.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="2"/>')
        parts.append(f'<text x="{85+index*180}" y="70" fill="{color}">{name}</text>')
    parts.extend(['<text x="500" y="610">Accepted commit tick (240 Hz)</text>',
                  '<text x="85" y="640">Degrees; samples joined for inspection, not a proof of continuous motion</text>', '</g></svg>'])
    output.write_text('\n'.join(parts), encoding='utf-8')


def stage_a(rows, directory, digest):
    invalid = sum(bool(not r['axis_valid'] or r['any_point_invalid']) for r in rows)
    if invalid:
        raise ValueError(f'{invalid} invalid rows: stop continuous-sweep interpretation')
    lines = ['# Normal-Free Bat Direction Basis S1 — Stage A', '',
             f'CSV SHA-256: `{digest}`。32 rows，production-derived commit 431～462，ex=ey=0，Michael 75/85/3，Compact/Normal。', '',
             '本階段沒有場地。零度是 immutable incoming velocity 的水平反向；正號為 atan2(ref.z*v.x−ref.x*v.z, ref.x*v.x+ref.z*v.z)。'
             'Axis branch 固定為 cross(world_up, ordered barrel→tip)，不逐 row 翻面。Velocity 是固定 u、±0.0001 s 中央差分；不使用最近點或接觸 normal。', '',
             '| 訊號 | Unwrapped min…max ° | 全幅 ° | 增/減/平 | 最大相鄰角差 ° | 局部反轉／平台 |',
             '|---|---|---|---|---|---|']
    for name, key in [('bat-axis azimuth', 'axis_azimuth_deg')]+SIGNALS:
        lines.append(f'| {name} | {statistics(rows, key)} |')
    spreads = [r['point_spread_deg'] for r in rows]
    differences = [abs(r['axis_minus_mid_deg']) for r in rows]
    lines += ['', f'Fixed-point spread：max {max(spreads):.6f}°（462），median {median(spreads):.6f}°；'
              f'三點 deflection 同號 {sum(r["point_sign_agreement"] == 1 for r in rows)}/32；退化 {invalid}/32。',
              f'Axis 與 mid velocity 最短角差絕對值：min {min(differences):.6f}°、median {median(differences):.6f}°、max {max(differences):.6f}°（462）。'
              'Early 431～433差距約8～10°；最晚461／462擴大至約16／22°，不是兩端對稱。', '',
              '| tick | contact s | local phase ms | axis deflection ° | u0 ° | mid ° | u1 ° | spread ° | mid total/horizontal m/s |',
              '|---|---|---|---|---|---|---|---|---|']
    for r in rows:
        if int(r['commit_tick']) in ANCHORS:
            lines.append(f'| {int(r["commit_tick"])} | {r["contact_time_s"]:.15f} | {r["local_phase_ms"]:.9f} | '
                         f'{r["axis_deflection_deg"]:.6f} | {r["u0_deflection_deg"]:.6f} | {r["u05_deflection_deg"]:.6f} | '
                         f'{r["u1_deflection_deg"]:.6f} | {r["point_spread_deg"]:.6f} | {r["u05_speed"]:.6f}/{r["u05_horizontal_speed"]:.6f} |')
    lines += ['', '## 判讀', '',
              '1. Axis-derived direction 在此240 Hz sweep單調不增，沒有方向翻面；是較乾淨的未來候選 basis。最大相鄰角差約14.7°仍不小，有限取樣不能證明數學連續性。',
              '2. 固定點速度大部分接近且全數同號，但461／462的spread增至6.35／9.59°，選點開始有實質影響；不能把mid當唯一真值。',
              '3. Mid／tip單調不增，u0在461→462反轉約0.739°；velocity不是完全穩定的bat-level方向。',
              '4. Axis與velocity通常相差數度，最晚達22.22°。不混合、不加權，也不選擇好看的點。',
              '5. Late端mid總速由460的63.59降至461的34.96、462的25.38 m/s；不是接近零速，卻有明顯速度與方向分歧。'
              '這些local phase約85.77／81.60／77.43 ms，仍在protected swing內，不能誤稱為recovery／settling。'
              f'全域mid總速最小 {min(r["u05_speed"] for r in rows):.6f} m/s；三點水平速度最小 '
              f'{min(r[p+"_horizontal_speed"] for r in rows for p in ["u0", "u05", "u1"]):.6f} m/s；未建立任何gameplay速度cutoff。',
              '6. 436與444雖共享contact world time，但axis差約37.61°、mid velocity差約37.30°，確實保留不同玩家決策所對應的motion phase。',
              '7. Axis不需速度微分或選material point，單一ordered branch且本域單調，技術上較乾淨；仍不可直接promotion。'
              '446～450都在125 ms local phase，四訊號皆plateau；這是effective-time取樣政策的限制。'
              '只驗證一組pitch／profile／tempo，沒有驗證未來motion或pitch變化；尚未建立gameplay authority契約。Michael＋Julia review待決。']
    (directory/'stage-a-summary.md').write_text('\n'.join(lines)+'\n', encoding='utf-8')
    plot(rows, SIGNALS, directory/'stage-a-directions.svg')
    (directory/'stage-a.sha256').write_text(digest+'  direction-basis.csv\n', encoding='ascii')


def stage_b(rows, directory, digest):
    assert (directory/'stage-a.sha256').read_text().split()[0] == digest, 'Frozen Stage A hash changed'
    # Only stored vectors/angles are read. No native sampling or formula selection here.
    for r in rows:
        r['axis_world'] = math.degrees(math.atan2(r['axis_direction_x'], r['axis_direction_z']))
        for prefix in ['u0', 'u05', 'u1']:
            r[prefix+'_world'] = math.degrees(math.atan2(r[prefix+'_vx'], r[prefix+'_vz']))
    curves = [('axis', 'axis_world'), ('u=0', 'u0_world'), ('u=0.5', 'u05_world'), ('u=1', 'u1_world')]
    lines = ['# Stage B — frozen direction projection', '', f'Read-only Stage A SHA-256: `{digest}`。', '',
             '以下才使用field-center +Z與±45°幾何sector；沒有回饋Stage A，沒有fair/foul gameplay規則。', '',
             '| 訊號 | World angle min…max ° | inside/outside | boundary crossings | production同號/異號 | 異號ticks |',
             '|---|---|---|---|---|---|']
    for name, key in curves:
        inside = [abs(r[key]) <= 45 for r in rows]
        crossing = [f'{int(rows[i-1]["commit_tick"])}→{int(rows[i]["commit_tick"])}' for i in range(1, len(rows)) if inside[i] != inside[i-1]]
        disagree = [int(r['commit_tick']) for r in rows if (r[key] > 0) != (r['production_spray_deg'] > 0)]
        lines.append(f'| {name} | {min(r[key] for r in rows):.6f}…{max(r[key] for r in rows):.6f} | '
                     f'{sum(inside)}/{len(rows)-sum(inside)} | {", ".join(crossing)} | {len(rows)-len(disagree)}/{len(disagree)} | {disagree} |')
    lines += ['', '## Current production comparison', '',
              '| 訊號 | min…max ° | 全幅 ° | 增/減/平 | 最大相鄰角差 ° | 平台 |', '|---|---|---|---|---|---|',
              f'| production spray | {statistics(rows, "production_spray_deg")} |', '',
              'Production全32 rows在sector內；431／432為+35°clamp，kinematics仍改變。'
              'Kinematics的446～450 phase plateau則不是production plateau。'
              'Axis／velocity的world angular range遠大於production，均跨過兩側sector邊界；這是觀察，不是改善fair/foul比例的理由。'
              'Stage A以incoming return為零；本表以+Z為零，因此近零處符號比較必須在同一world frame做。', '',
              'Axis的技術優點來自point-independent、單一branch和本域單調性，與sector比例無關。'
              '未實作runtime alternate mode或新BallResponse；仍等待Michael＋Julia review。']
    (directory/'stage-b-summary.md').write_text('\n'.join(lines)+'\n', encoding='utf-8')
    with (directory/'stage-b-projection.csv').open('w', newline='', encoding='utf-8') as out:
        writer = csv.writer(out)
        writer.writerow(['commit_tick']+[name for _, key in curves for name in [key, key+'_inside']]+['production_spray_deg'])
        for r in rows:
            writer.writerow([int(r['commit_tick'])]+[v for _, key in curves for v in [format(r[key], '.17g'), abs(r[key]) <= 45]]+[r['production_spray_deg']])
    plot(rows, curves+[('production', 'production_spray_deg')], directory/'stage-b-projection.svg', True)
    assert hashlib.sha256((directory/'direction-basis.csv').read_bytes()).hexdigest() == digest


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('stage', choices=['a', 'b'])
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    path = args.directory/'direction-basis.csv'
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    with path.open(newline='', encoding='utf-8') as source:
        rows = [{k: float(v) for k, v in r.items()} for r in csv.DictReader(source)]
    assert [int(r['commit_tick']) for r in rows] == list(range(431, 463)), 'Unexpected native-derived domain'
    if args.stage == 'a':
        if (args.directory/'stage-a.sha256').exists():
            assert (args.directory/'stage-a.sha256').read_text().split()[0] == digest, 'Refusing to overwrite frozen evidence'
        stage_a(rows, args.directory, digest)
    else:
        stage_b(rows, args.directory, digest)
    print(f'Stage {args.stage}: {len(rows)} rows; SHA-256 {digest}')
