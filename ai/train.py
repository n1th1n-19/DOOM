"""Train a PPO agent on DOOM. Works on Colab (GPU) or any Linux machine.

  python train.py --scenario defend_the_center --steps 500000
  python train.py --wad ../web/wads/freedoom1.wad --map E1M1 --steps 5000000 --out runs/e1m1

Stops and resumes safely: checkpoints are written to --out every 100k steps; run
the same command again with --resume to continue from the newest one.
"""
import argparse
import glob
import os

from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import CheckpointCallback
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.vec_env import SubprocVecEnv, VecFrameStack

from doom_env import DoomEnv


def make_env(args):
    kw = dict(wad=args.wad, map_name=args.map, skill=args.skill) if args.wad else \
        dict(scenario=args.scenario, skill=args.skill)
    vec = make_vec_env(lambda: DoomEnv(**kw), n_envs=args.envs, vec_env_cls=SubprocVecEnv)
    return VecFrameStack(vec, 4)   # the AI sees the last 4 frames, so it can sense motion


def latest_checkpoint(out):
    files = sorted(glob.glob(os.path.join(out, "doom_ppo_*_steps.zip")),
                   key=lambda f: int(f.rsplit("_", 2)[-2]))
    return files[-1] if files else None


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--scenario", default="defend_the_center", help="built-in ViZDoom scenario")
    p.add_argument("--wad", help="train on a real level from this WAD instead")
    p.add_argument("--map", default="E1M1")
    p.add_argument("--skill", type=int, default=3, help="1 (easy) .. 5 (nightmare)")
    p.add_argument("--steps", type=int, default=500_000)
    p.add_argument("--envs", type=int, default=4, help="parallel games (Colab has 2-4 CPUs)")
    p.add_argument("--out", default="runs/default")
    p.add_argument("--resume", action="store_true")
    args = p.parse_args()
    os.makedirs(args.out, exist_ok=True)

    env = make_env(args)
    ckpt = latest_checkpoint(args.out) if args.resume else None
    if ckpt:
        print("resuming from", ckpt)
        model = PPO.load(ckpt, env=env, tensorboard_log=args.out)
    else:
        model = PPO("CnnPolicy", env, n_steps=256, batch_size=256, learning_rate=2.5e-4,
                    ent_coef=0.01, gamma=0.99, verbose=1, tensorboard_log=args.out)
    save = CheckpointCallback(save_freq=max(1, 100_000 // args.envs), save_path=args.out,
                              name_prefix="doom_ppo")
    model.learn(total_timesteps=args.steps, callback=save, reset_num_timesteps=not ckpt,
                tb_log_name="ppo", progress_bar=True)
    model.save(os.path.join(args.out, "final"))
    print("saved", os.path.join(args.out, "final.zip"))


if __name__ == "__main__":
    main()
