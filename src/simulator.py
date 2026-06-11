import copy
from collections import defaultdict
from typing import Callable, Dict, List

from models import Process, Resource
from dataset import Event
from metrics import MetricsCollector
from oracle import classify_timeout

TERMINAL_STATES = {"COMPLETED", "KILLED"}


class Simulator:
    def __init__(
        self,
        events: List[Event],
        strategy_name: str,
        strategy_fn: Callable,
        timeout: int,
        max_ticks: int = 300,
        retry_attempts: int = 5,
    ):
        self.strategy_name = strategy_name
        self.strategy_fn = strategy_fn
        self.timeout = timeout
        self.max_ticks = max_ticks
        self.retry_attempts = retry_attempts
        self.current_time = 0

        self.processes: Dict[str, Process] = {}
        self.resources: Dict[str, Resource] = {}

        all_pids = {e.process_id for e in events}
        self.metrics = MetricsCollector(total=len(all_pids))

        self._event_queue: Dict[int, List[Event]] = defaultdict(list)
        for e in events:
            self._event_queue[e.time].append(e)

        self._backoff_requeue: Dict[str, str] = {}

    def run(self) -> dict:
        while True:
            self._dispatch_events()
            self._wake_backoff()
            self._tick_active()
            self._check_timeouts()

            if self._all_terminal():
                break
            if self.current_time >= self.max_ticks:
                break

            self.current_time += 1

        return self.metrics.summary()

    def _dispatch_events(self):
        for event in self._event_queue.get(self.current_time, []):
            pid = event.process_id
            rid = event.resource_id

            if pid not in self.processes:
                self.processes[pid] = Process(pid)
            if rid not in self.resources:
                self.resources[rid] = Resource(rid)

            process = self.processes[pid]
            resource = self.resources[rid]

            if process.state in TERMINAL_STATES:
                continue
            if process.state == "BACKOFF":
                continue

            process.history.append(rid)

            if resource.owner is None:
                self._grant_resource(rid, pid)
                if event.duration > 0 and process.remaining_time == 0:
                    process.remaining_time = event.duration
                elif event.duration == 0:
                    process.remaining_time = 1
            else:
                if pid not in resource.wait_queue:
                    resource.wait_queue.append(pid)
                process.state = "BLOCKED"
                process.waiting_for = rid
                process.request_time = self.current_time
                process._pending_duration = event.duration

    def _wake_backoff(self):
        for pid, process in list(self.processes.items()):
            if process.state != "BACKOFF" or process.backoff_until is None:
                continue
            if self.current_time < process.backoff_until:
                continue

            process.state = "ACTIVE"
            process.backoff_until = None

            target_rid = getattr(process, "_rollback_target", None)
            if target_rid:
                process._rollback_target = None
            elif process.history:
                target_rid = process.history[0]

            if target_rid and target_rid in self.resources:
                resource = self.resources[target_rid]
                if resource.owner is None:
                    self._grant_resource(target_rid, pid)
                    pending = getattr(process, "_pending_duration", 0)
                    if process.remaining_time == 0:
                        process.remaining_time = max(pending, 1)
                elif pid not in resource.wait_queue:
                    resource.wait_queue.append(pid)
                    process.state = "BLOCKED"
                    process.waiting_for = target_rid
                    process.request_time = self.current_time

    def _tick_active(self):
        for pid, process in list(self.processes.items()):
            if process.state == "ACTIVE" and process.remaining_time > 0:
                process.remaining_time -= 1
                if process.remaining_time == 0:
                    for rid in list(process.held_resources):
                        self._release_resource(rid, pid)
                    process.state = "COMPLETED"
                    self.metrics.completed += 1

    def _check_timeouts(self):
        timed_out = [
            pid
            for pid, p in self.processes.items()
            if p.state == "BLOCKED"
            and p.request_time is not None
            and (self.current_time - p.request_time) >= self.timeout
        ]
        for pid in timed_out:
            if self.processes[pid].state != "BLOCKED":
                continue
            classification = classify_timeout(pid, self.processes, self.resources)
            if classification == "TRUE_DEADLOCK":
                self.metrics.resolved += 1
            else:
                self.metrics.false_positives += 1
            self.strategy_fn(pid, self)

    def _all_terminal(self) -> bool:
        return all(p.state in TERMINAL_STATES for p in self.processes.values())

    def _grant_resource(self, rid: str, pid: str):
        resource = self.resources[rid]
        process = self.processes[pid]
        resource.owner = pid
        process.held_resources.add(rid)
        process.waiting_for = None
        process.request_time = None
        if process.state != "ACTIVE":
            process.state = "ACTIVE"
        if not process.checkpoint:
            process.checkpoint = copy.copy(process.held_resources)
            process.checkpoint_remaining_time = process.remaining_time

    def _release_resource(self, rid: str, pid: str):
        resource = self.resources[rid]
        if resource.owner == pid:
            resource.owner = None
        self.processes[pid].held_resources.discard(rid)
        self._grant_next_in_queue(rid)

    def _grant_next_in_queue(self, rid: str):
        resource = self.resources[rid]
        while resource.wait_queue:
            next_pid = resource.wait_queue[0]
            next_proc = self.processes.get(next_pid)
            if next_proc is None or next_proc.state in TERMINAL_STATES:
                resource.wait_queue.popleft()
                continue
            resource.wait_queue.popleft()
            self._grant_resource(rid, next_pid)
            pending = getattr(next_proc, "_pending_duration", 0)
            if next_proc.remaining_time == 0:
                next_proc.remaining_time = max(pending, 1)
            break
