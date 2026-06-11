from typing import List

from dataset import Event
from simulator import Simulator
from strategies import STRATEGY_MAP

STRATEGIES = ["KILL", "RETRY", "ROLLBACK"]


def run_benchmark(events: List[Event], timeouts: List[int], max_ticks: int = 300, retry_attempts: int = 5) -> List[dict]:
    results = []
    for strategy_name in STRATEGIES:
        for timeout in timeouts:
            strategy_fn = STRATEGY_MAP[strategy_name]
            sim = Simulator(
                events=events,
                strategy_name=strategy_name,
                strategy_fn=strategy_fn,
                timeout=timeout,
                max_ticks=max_ticks,
                retry_attempts=retry_attempts,
            )
            metrics = sim.run()
            results.append(
                {
                    "strategy": strategy_name,
                    "timeout": timeout,
                    "max_ticks": max_ticks,
                    **metrics,
                }
            )
    return results


def run_grid_benchmark(
    events: List[Event],
    timeouts: List[int],
    max_ticks_list: List[int],
    retry_attempts: int = 5,
) -> List[dict]:
    results = []
    for max_ticks in max_ticks_list:
        batch = run_benchmark(events, timeouts, max_ticks=max_ticks, retry_attempts=retry_attempts)
        results.extend(batch)
    return results
