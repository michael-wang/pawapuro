"""Offline interpretation of this one study CSV; never recalculates the probe."""
import argparse
import csv
import hashlib
import math
from pathlib import Path


def unwrap(values):
    result = []
    for value in values:
        result.append(value if not result else result[-1] + (value - result[-1] + 180) % 360 - 180)
    return result


def signal(rows, key):
    valid = [r for r in rows if math.isfinite(r[key])]
    angles = unwrap([r[key] for r in valid])
    # Do not infer continuity across invalid rows.
    steps = [angles[i] - angles[i - 1] for i in range(1, len(valid))
             if valid[i]['commit_tick'] == valid[i - 1]['commit_tick'] + 1]
    positive = sum(d > 1e-8 for d in steps)
    negative = sum(d < -1e-8 for d in steps)
    return (f"{min(angles):.6f}～{max(angles):.6f}°（unwrap 範圍 {max(angles)-min(angles):.6f}°）；"
            f"有效相鄰 tick 最大角差 {max(map(abs, steps), default=0):.6f}°；"
            f"增加／減少／持平={positive}/{negative}/{len(steps)-positive-negative}。")


def plot(rows, output, field=False):
    # A local direction diagram, not a runtime widget or plotting framework.
    lines = ['<svg xmlns="http://www.w3.org/2000/svg" width="760" height="760" viewBox="0 0 760 760">',
             '<rect width="760" height="760" fill="white"/>']
    title = ('Stage B: frozen rays; +Z up, +X right' if field else
             'Stage A: fieldless rays; straight return up')
    lines.append(f'<text x="30" y="35" font-family="sans-serif" font-size="22">{title}</text>')
    lines.append('<text x="30" y="65" font-family="sans-serif" font-size="16">436: no ray (non-closing). Color progresses with commit tick.</text>')
    if field:
        for angle in (-math.pi/4, math.pi/4):
            lines.append(f'<path d="M380 400 L{380+300*math.sin(angle)} {400-300*math.cos(angle)}" stroke="gray" stroke-dasharray="6 6"/>')
    for r in rows:
        if not r['direction_valid']:
            continue
        angle = (math.atan2(r['candidate_vx'], r['candidate_vz']) if field
                 else math.radians(r['fieldless_deflection_deg']))
        x, y = 380+260*math.sin(angle), 400-260*math.cos(angle)
        hue = 230-200*(r['commit_tick']-rows[0]['commit_tick'])/(rows[-1]['commit_tick']-rows[0]['commit_tick'])
        lines.append(f'<path d="M380 400 L{x} {y}" stroke="hsl({hue},70%,40%)" stroke-width="2"/>')
        if r['commit_tick'] in (444, 448, 455):
            dx, dy = {444: (-60, -8), 448: (8, 12), 455: (-35, -8)}[int(r['commit_tick'])]
            lines.append(f'<text x="{x+dx}" y="{y+dy}" font-family="sans-serif" font-size="18">{int(r["commit_tick"])}</text>')
    lines.append('<circle cx="380" cy="400" r="4" fill="black"/></svg>')
    output.write_text('\n'.join(lines), encoding='utf-8')

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('stage', choices=['stage-a', 'stage-b'])
    parser.add_argument('directory', type=Path)
    args = parser.parse_args()
    path = args.directory / 'fieldless-kinematics.csv'
    digest = hashlib.sha256(path.read_bytes()).hexdigest()
    with path.open(newline='', encoding='utf-8') as stream:
        rows = [{k: float(v) for k, v in row.items()} for row in csv.DictReader(stream)]
    n = len(rows)
    valid = [r for r in rows if r['direction_valid']]
    def count(test):
        total = sum(test(r) for r in rows)
        return f'{total}/{n}（{100*total/n:.2f}%）'
    if args.stage == 'stage-a':
        summary = f'''# Fieldless Bat Kinematics Study S0 — Stage A

僅診斷，等待 Michael＋Julia review。此階段沒有場地輸入、fair/foul 分類或校正。
CSV SHA-256：`{digest}`。

Production live-input 正整數 domain 由 intent_live 掃描；Temporal Match 與正常 fixture consumption 得到
commit **{int(rows[0]['commit_tick'])}～{int(rows[-1]['commit_tick'])}**，共 **{n}** 個 Gameplay Contact（ex=ey=0、Michael 75/85/3、Compact/Normal）。
不沿用 asset 舊 432～496 authoring bounds。CSV 全部 rows 保留，包括沒有 raw contact 的 rows。

方法：sample_manual_contact → IngameMotion::sample_barrel，使用 accepted consumed_tick、attempt tempo 與 BallResponse.contact_time_s。
Closest centerline point 固定在 barrel-local frame；±0.0001 s 中央差分沿用 bat_contact_detail 的 epsilon。
此處 velocity 是 centerline material point；raw solver 附錄則是其 surface material point，不混為同一樣本。
Geometric n 從 centerline 指向球；closing=-dot(v_ball-v_bat,n)。只在 normal 非退化且 closing>0 時計算
v_ball−2 dot(v_ball−v_bat,n)n；e=1 僅機械 probe，不是已接受物理或速度來源。1e-8 為數值退化檢查，不是 gameplay threshold。
矩陣欄位保留 production GlbMatrix 原陣列順序。NaN 表示無定義／invalid，不補方向。
Axis／velocity azimuth 只是 XZ 座標角 atan2(x,z)；主要 deflection 以 incoming velocity 的反向水平向量為零。
正號定義 atan2(ref.z*out.x−ref.x*out.z, ref.x*out.x+ref.z*out.z)，與場地無關。

## 測量

- Bat axis：{signal(rows, 'axis_azimuth_deg')}
- Bat material velocity：{signal(rows, 'bat_velocity_azimuth_deg')}
- Candidate deflection：{signal(rows, 'fieldless_deflection_deg')}
- 有效 candidate：{len(valid)}/{n}；normal 退化：{count(lambda r: not r['normal_valid'])}。
- Non-closing：{count(lambda r: r['normal_valid'] and r['closing'] <= 0)}。
- Effective time 內含於 raw envelope：{count(lambda r: bool(r['inside_envelope']))}。
- 全 temporal search 找到 raw contact：{count(lambda r: bool(r['raw_contact_exists']))}。
- 沒有 raw contact：{count(lambda r: not r['raw_contact_exists'])}。
- Separation 範圍：{min(r['separation_m'] for r in rows):.9f}～{max(r['separation_m'] for r in rows):.9f} m；envelope={rows[0]['envelope_m']:.9f} m。
- Separation 超出 envelope：{count(lambda r: r['separation_m'] > r['envelope_m'])}；超過 2× envelope：{count(lambda r: r['separation_m'] > 2*r['envelope_m'])}。
  2× 只作大距離的描述門檻，不參與任何公式。

| Commit | Effective time s | Local phase ms | Axis deg | Material velocity deg | Speed m/s | Closing m/s | Deflection deg |
|---|---:|---:|---:|---:|---:|---:|---:|
'''
        for r in rows:
            if r['commit_tick'] in (436, 441, 444, 448, 455):
                summary += '| ' + ' | '.join(f'{r[k]:.9f}' for k in
                    ('commit_tick', 'contact_time_s', 'local_phase_ms', 'axis_azimuth_deg',
                     'bat_velocity_azimuth_deg', 'bat_speed', 'closing', 'fieldless_deflection_deg')) + ' |\n'
        summary += '''
Axis 在 unwrap 後隨 commit 單調下降，但 446～450 的 125 ms phase 形成 plateau。
Material velocity 大致下降，最後一個 tick 有局部反轉；分段取樣 motion 的導數不保證平滑。
Candidate 既有 invalid 區段，也有相鄰有效 rows 的局部反轉與大角差，不能稱為完整連續、單調的 timing mapping。
這是 240 Hz 有限取樣的連續性觀察，不是數學連續性證明；跨 invalid 區段不推論 interpolation。
436／444 共用 effective world time，卻有不同 local phase、axis 與 material velocity；436 non-closing，不能補造 reflection ray。
方向訊號存在於 authored motion，但 gameplay contact 多數與 raw geometry 分離，當下這個 mechanical probe 不宜直接升為 gameplay authority。
保持 production spray、能量、flight／ground 原樣；不為預期棒球方向改 n、符號、倍率或 probe。

Native test 以同一 live preview 的 before/after bytes 檢查 tick、attempts、pitch、pose／geometry、flight、outcome 未變；
30／120 cadence 重跑全 sweep 的 CSV bytes 相同。既有 30／60／120 replay regression 保留。
'''
        (args.directory / 'stage-a-summary.md').write_text(summary, encoding='utf-8')
        plot(rows, args.directory / 'fieldless-directions.svg')
        (args.directory / 'stage-a.sha256').write_text(digest+'\n', encoding='ascii')
        print(summary)
    else:
        assert (args.directory / 'stage-a.sha256').read_text().strip() == digest, 'Stage A changed after freeze'
        projected = []
        for r in valid:
            angle = math.degrees(math.atan2(r['candidate_vx'], r['candidate_vz']))
            projected.append(dict(commit_tick=int(r['commit_tick']), world_angle_deg=angle,
                side='+X' if angle > 0 else '-X' if angle < 0 else 'center',
                inside_90_degree_sector=abs(angle) <= 45,
                production_spray_deg=r['production_spray_deg'],
                same_sign=angle*r['production_spray_deg'] > 0))
        with (args.directory / 'stage-b-projection.csv').open('w', newline='', encoding='utf-8') as stream:
            writer = csv.DictWriter(stream, fieldnames=list(projected[0]))
            writer.writeheader(); writer.writerows(projected)
        summary = f'''# Stage B — frozen Stage A 的唯讀場地投影

Stage A CSV SHA-256 已核對：`{digest}`。只讀既有 candidate XYZ；未重算或修改 probe。
World angle=atan2(candidate.x,candidate.z)，+Z 是既有 field center，正為 +X、負為 −X；
±45° 僅標示既有 90° sector 幾何，不新增 gameplay fair/foul classification。

- 有方向的 {len(valid)}/{n} rows 中，sector 內 {sum(r['inside_90_degree_sector'] for r in projected)}，外 {sum(not r['inside_90_degree_sector'] for r in projected)}。
- +X {sum(r['side']=='+X' for r in projected)}，−X {sum(r['side']=='-X' for r in projected)}。
- World angle 範圍 {min(r['world_angle_deg'] for r in projected):.6f}～{max(r['world_angle_deg'] for r in projected):.6f}°。
- 與 production spray 同號 {sum(r['same_sign'] for r in projected)}/{len(valid)}，異號或零 {sum(not r['same_sign'] for r in projected)}/{len(valid)}。
- Production 全 {n} rows 的 spray 範圍 {min(r['production_spray_deg'] for r in rows):.6f}～{max(r['production_spray_deg'] for r in rows):.6f}°，全程單調不增（含 clamp plateau）。
- Mechanical probe 有 invalid 缺口及局部反轉，角度範圍與 shape 都不等同 production timing→spray；不能用同號率當作品質或校準依據。

逐 row 比對見 stage-b-projection.csv。其他 rows 沒有 candidate，不強行對應方向。
場地分布不回饋 Stage A：不放大／clamp／換 normal／換公式。不建議 promotion，等待 Michael＋Julia 討論 geometry 與 gameplay-contact 抽象的落差。
'''
        (args.directory / 'stage-b-summary.md').write_text(summary, encoding='utf-8')
        plot(rows, args.directory / 'field-projection.svg', field=True)
        assert hashlib.sha256(path.read_bytes()).hexdigest() == digest
        print(summary)


if __name__ == '__main__':
    main()
