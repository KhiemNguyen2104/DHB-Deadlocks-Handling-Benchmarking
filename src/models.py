from collections import deque
from typing import Optional, Set


class Process:
    def __init__(self, pid: str):
        self.pid = pid
        self.state = "ACTIVE"
        self.remaining_time = 0
        self.held_resources: Set[str] = set()
        self.waiting_for: Optional[str] = None
        self.request_time: Optional[int] = None
        self.history = []
        self.checkpoint: Optional[Set[str]] = None
        self.checkpoint_remaining_time: int = 0
        self.retry_count: int = 0
        self.backoff_until: Optional[int] = None
        self._rollback_target: Optional[str] = None


class Resource:
    def __init__(self, rid: str):
        self.rid = rid
        self.owner: Optional[str] = None
        self.wait_queue: deque = deque()
