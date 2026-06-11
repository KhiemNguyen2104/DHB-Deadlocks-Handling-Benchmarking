import csv
from dataclasses import dataclass
from typing import List


@dataclass
class Event:
    time: int
    process_id: str
    action: str
    resource_id: str
    duration: int


def load_events(path: str) -> List[Event]:
    events = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            events.append(
                Event(
                    time=int(row["time"]),
                    process_id=row["process_id"],
                    action=row["action"],
                    resource_id=row["resource_id"],
                    duration=int(row["duration"]),
                )
            )
    events.sort(key=lambda e: e.time)
    return events
