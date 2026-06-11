def handle(pid: str, sim) -> None:
    process = sim.processes[pid]
    if process.waiting_for:
        res = sim.resources.get(process.waiting_for)
        if res and pid in res.wait_queue:
            res.wait_queue.remove(pid)
    for rid in list(process.held_resources):
        sim._release_resource(rid, pid)
    process.state = "KILLED"
    process.remaining_time = 0
    process.waiting_for = None
    process.request_time = None
    sim.metrics.killed += 1
