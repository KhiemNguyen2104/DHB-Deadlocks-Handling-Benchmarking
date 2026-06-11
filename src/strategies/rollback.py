def handle(pid: str, sim) -> None:
    process = sim.processes[pid]
    contested_rid = process.waiting_for

    if process.waiting_for:
        res = sim.resources.get(process.waiting_for)
        if res and pid in res.wait_queue:
            res.wait_queue.remove(pid)

    process.retry_count += 1

    if process.retry_count > sim.retry_attempts:
        for rid in list(process.held_resources):
            sim._release_resource(rid, pid)
        process.state = "KILLED"
        process.remaining_time = 0
        process.waiting_for = None
        process.request_time = None
        sim.metrics.killed += 1
        return

    if not process.checkpoint:
        for rid in list(process.held_resources):
            sim._release_resource(rid, pid)
        process.remaining_time = 0
        delay = min(3 * (2 ** (process.retry_count - 1)), 16)
        process.backoff_until = sim.current_time + delay
        process.state = "BACKOFF"
        process.waiting_for = None
        process.request_time = None
        return

    post_checkpoint = process.held_resources - process.checkpoint
    for rid in list(post_checkpoint):
        sim._release_resource(rid, pid)

    process.remaining_time = process.checkpoint_remaining_time
    delay = min(2 * process.retry_count, 10)
    process.backoff_until = sim.current_time + delay
    process.state = "BACKOFF"
    process.waiting_for = None
    process.request_time = None

    if contested_rid:
        if contested_rid not in process.history:
            process.history.append(contested_rid)
        process._rollback_target = contested_rid
