"""Watch a trained agent: plays episodes and writes an mp4.

  python play.py --model runs/default/final.zip --scenario defend_the_center --out demo.mp4
"""
import argparse

import imageio
import numpy as np
from stable_baselines3 import PPO
from stable_baselines3.common.vec_env import DummyVecEnv, VecFrameStack

from doom_env import DoomEnv


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--model", required=True)
    p.add_argument("--scenario", default="defend_the_center")
    p.add_argument("--wad")
    p.add_argument("--map", default="E1M1")
    p.add_argument("--skill", type=int, default=3)
    p.add_argument("--episodes", type=int, default=3)
    p.add_argument("--out", default="demo.mp4")
    args = p.parse_args()

    kw = dict(wad=args.wad, map_name=args.map, skill=args.skill) if args.wad else \
        dict(scenario=args.scenario, skill=args.skill)
    env = VecFrameStack(DummyVecEnv([lambda: DoomEnv(**kw)]), 4)
    model = PPO.load(args.model)

    frames, kills, health = [], [], []
    for ep in range(args.episodes):
        obs = env.reset()
        done, total = False, 0.0
        while not done:
            action, _ = model.predict(obs, deterministic=False)
            obs, reward, dones, infos = env.step(action)
            total += float(reward[0])
            frames.append(np.repeat(np.repeat(obs[0][..., -3:], 3, axis=0), 3, axis=1))  # last frame, enlarged
            done = bool(dones[0])
        info = infos[0]
        kills.append(info.get("kills", 0)); health.append(info.get("health", 0))
        print(f"episode {ep + 1}: reward {total:.1f}, kills {kills[-1]}, health {health[-1]}")
    imageio.mimsave(args.out, frames, fps=15)
    print("wrote", args.out, f"({len(frames)} frames)")


if __name__ == "__main__":
    main()
