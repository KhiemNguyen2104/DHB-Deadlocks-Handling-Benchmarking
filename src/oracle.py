from typing import Dict


def build_wfg(processes: dict, resources: dict) -> Dict[str, str]:
    wfg = {}
    for pid, process in processes.items():
        if process.state == "BLOCKED" and process.waiting_for:
            resource = resources.get(process.waiting_for)
            if resource and resource.owner:
                wfg[pid] = resource.owner
    return wfg


def has_cycle_dfs(wfg: Dict[str, str], start: str) -> bool:
    visited: set = set()
    rec_stack: set = set()

    def dfs(node: str) -> bool:
        visited.add(node)
        rec_stack.add(node)
        neighbor = wfg.get(node)
        if neighbor is None:
            rec_stack.discard(node)
            return False
        if neighbor not in visited:
            if dfs(neighbor):
                return True
        elif neighbor in rec_stack:
            return True
        rec_stack.discard(node)
        return False

    return dfs(start)


def classify_timeout(pid: str, processes: dict, resources: dict) -> str:
    wfg = build_wfg(processes, resources)
    if has_cycle_dfs(wfg, pid):
        return "TRUE_DEADLOCK"
    return "FALSE_POSITIVE"
