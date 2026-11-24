"""A Gymnasium environment for training an AI to play DOOM with ViZDoom.

Two modes:
  * a built-in ViZDoom scenario (small, learns in minutes to hours):
        DoomEnv(scenario="defend_the_center")
  * a real level from a WAD file such as Freedoom (hard, learns in days):
        DoomEnv(wad="freedoom1.wad", map_name="E1M1")

Reward shaping (the part that decides what the AI learns): kills, health kept,
ammo found, a small bonus for moving through new places, a penalty for dying.
"""
import itertools
import os

import gymnasium as gym
import numpy as np
import vizdoom as vzd

SCENARIOS = ("basic", "defend_the_center", "deadly_corridor", "health_gathering")

# The buttons the AI may press. Each action is one combination of them.
BUTTONS = [
    vzd.Button.MOVE_FORWARD,
    vzd.Button.TURN_LEFT,
    vzd.Button.TURN_RIGHT,
    vzd.Button.ATTACK,
    vzd.Button.USE,
    vzd.Button.MOVE_LEFT,
    vzd.Button.MOVE_RIGHT,
]
# Each action is a combination of the buttons above, in the same order:
#           forward left right attack use strafe-left strafe-right
ACTIONS = [
    [0, 0, 0, 0, 0, 0, 0],   # do nothing
    [1, 0, 0, 0, 0, 0, 0],   # forward
    [0, 1, 0, 0, 0, 0, 0],   # turn left
    [0, 0, 1, 0, 0, 0, 0],   # turn right
    [1, 1, 0, 0, 0, 0, 0],   # forward + turn left
    [1, 0, 1, 0, 0, 0, 0],   # forward + turn right
    [0, 0, 0, 1, 0, 0, 0],   # fire
    [1, 0, 0, 1, 0, 0, 0],   # forward + fire
    [0, 1, 0, 1, 0, 0, 0],   # turn left + fire
    [0, 0, 1, 1, 0, 0, 0],   # turn right + fire
    [0, 0, 0, 0, 1, 0, 0],   # use (doors, switches)
    [1, 0, 0, 0, 1, 0, 0],   # forward + use
    [0, 0, 0, 0, 0, 1, 0],   # strafe left
    [0, 0, 0, 0, 0, 0, 1],   # strafe right
]


class DoomEnv(gym.Env):
    metadata = {"render_modes": ["rgb_array"]}

    def __init__(self, scenario="defend_the_center", wad=None, map_name="E1M1",
                 skill=3, frame_skip=4, size=(84, 84), episode_timeout=2100, seed=None,
                 shaping=True):
        super().__init__()
        self.frame_skip, self.size, self.shaping = frame_skip, size, shaping
        g = vzd.DoomGame()
        if wad:
            # a real level: the WAD is both the game data and the "scenario"
            g.set_doom_game_path(os.path.abspath(wad))
            g.set_doom_scenario_path(os.path.abspath(wad))
            g.set_doom_map(map_name)
        else:
            g.set_doom_scenario_path(os.path.join(vzd.scenarios_path, scenario + ".wad"))
            g.set_doom_map("map01")
        g.set_doom_skill(skill)
        g.set_screen_resolution(vzd.ScreenResolution.RES_160X120)
        g.set_screen_format(vzd.ScreenFormat.RGB24)
        g.set_window_visible(False)
        g.set_episode_timeout(episode_timeout)
        g.set_available_buttons(BUTTONS)
        g.set_available_game_variables([
            vzd.GameVariable.HEALTH, vzd.GameVariable.AMMO2, vzd.GameVariable.KILLCOUNT,
            vzd.GameVariable.POSITION_X, vzd.GameVariable.POSITION_Y,
        ])
        g.set_mode(vzd.Mode.PLAYER)
        g.set_doom_skill(skill)
        if seed is not None:
            g.set_seed(seed)
        g.init()
        self.game = g
        self.action_space = gym.spaces.Discrete(len(ACTIONS))
        self.observation_space = gym.spaces.Box(0, 255, (size[0], size[1], 3), np.uint8)
        self._visited = set()
        self._last = None

    # -- helpers -----------------------------------------------------------
    def _frame(self):
        state = self.game.get_state()
        if state is None:
            return np.zeros(self.observation_space.shape, np.uint8)
        img = np.transpose(state.screen_buffer, (1, 2, 0)) if state.screen_buffer.shape[0] == 3 else state.screen_buffer
        # nearest-neighbour resize to the observation size, no extra dependencies
        h, w = img.shape[:2]
        ys = (np.arange(self.size[0]) * h // self.size[0])
        xs = (np.arange(self.size[1]) * w // self.size[1])
        return img[ys][:, xs].astype(np.uint8)

    def _vars(self):
        if self.game.get_state() is None:
            return self._last
        v = self.game.get_state().game_variables
        return dict(health=v[0], ammo=v[1], kills=v[2], x=v[3], y=v[4])

    # -- gymnasium API -----------------------------------------------------
    def reset(self, seed=None, options=None):
        super().reset(seed=seed)
        self.game.new_episode()
        self._visited.clear()
        self._last = self._vars()
        return self._frame(), {}

    def step(self, action):
        raw = self.game.make_action(ACTIONS[int(action)], self.frame_skip)
        done = self.game.is_episode_finished()
        now = self._vars() if not done else self._last
        reward = float(raw)                      # the scenario's own reward, if any
        if self.shaping and self._last is not None and now is not None and not done:
            reward += 5.0 * (now["kills"] - self._last["kills"])          # kills
            reward += 0.05 * (now["health"] - self._last["health"])       # health lost / gained
            reward += 0.02 * max(0, now["ammo"] - self._last["ammo"])     # ammo picked up
            cell = (int(now["x"] // 64), int(now["y"] // 64))             # new 64x64 areas
            if cell not in self._visited:
                self._visited.add(cell)
                reward += 0.1
        if done and self._last is not None and self._last["health"] <= 0:
            reward -= 5.0                                                  # died
        if now is not None:
            self._last = now
        obs = self._frame() if not done else np.zeros(self.observation_space.shape, np.uint8)
        info = {}
        if now is not None:
            info = {"kills": now["kills"], "health": now["health"]}
        return obs, reward, done, False, info

    def render(self):
        return self._frame()

    def close(self):
        self.game.close()
