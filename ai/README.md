# Teach an AI to play DOOM

Reinforcement learning on DOOM with [ViZDoom](https://vizdoom.farama.org/) and
[Stable-Baselines3](https://stable-baselines3.readthedocs.io/) (PPO). The agent sees the
screen, presses buttons, and is rewarded for kills, staying alive and exploring.

| File | Purpose |
| --- | --- |
| [`doom_colab.ipynb`](doom_colab.ipynb) | Step-by-step notebook for Google Colab (free GPU). Start here. |
| [`doom_env.py`](doom_env.py) | The Gymnasium environment: actions, observations and reward shaping. |
| [`train.py`](train.py) | Trains a PPO agent; checkpoints every 100k steps and can `--resume`. |
| [`play.py`](play.py) | Plays a trained agent and writes an mp4. |

## Run on Google Colab

1. Open Colab, choose **File → Upload notebook**, and pick `doom_colab.ipynb`
   (or open it from GitHub: `https://colab.research.google.com/github/n1th1n-19/DOOM/blob/master/ai/doom_colab.ipynb`).
2. **Runtime → Change runtime type → T4 GPU**.
3. Run the cells top to bottom. The first training run (`defend_the_center`, 300k steps) takes
   roughly 10–20 minutes.

## Run on your own Linux machine

```bash
python3 -m venv ~/doom-ai && source ~/doom-ai/bin/activate
pip install -r requirements.txt
cd ai
python train.py --scenario defend_the_center --steps 300000 --out runs/dtc
python play.py --model runs/dtc/final.zip --scenario defend_the_center --out demo.mp4
```

## Training on a real level

Freedoom is included in `../web/wads/`:

```bash
python train.py --wad ../web/wads/freedoom1.wad --map E1M1 --steps 5000000 --out runs/e1m1 --envs 4
```

Move up in steps: `defend_the_center` → `deadly_corridor` → a real level. Real levels need millions
of steps and may take hours to days.

## What the reward does

Edit the reward lines in `DoomEnv.step` in `doom_env.py` to change the agent's character:

| Signal | Default |
| --- | --- |
| Kill | +5 |
| Health change | ±0.05 per point |
| Ammo picked up | +0.02 per point |
| Visiting a new 64x64 area | +0.1 |
| Death | −5 |

## Limits

- It learns "survive and shoot" well. Whole-level play (finding keys, using switches in order)
  from pixels is an open research problem and may not work.
- ViZDoom runs its own DOOM engine (ZDoom). It does not run this repo's code, but it does use the
  same Freedoom game data.
