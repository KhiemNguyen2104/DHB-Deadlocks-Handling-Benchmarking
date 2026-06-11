class MetricsCollector:
    def __init__(self, total: int):
        self.killed = 0
        self.resolved = 0
        self.false_positives = 0
        self.completed = 0
        self.total = total

    def throughput(self) -> float:
        if self.total == 0:
            return 0.0
        return self.completed / self.total

    def false_positive_rate(self) -> float:
        denom = self.resolved + self.false_positives
        if denom == 0:
            return 0.0
        return self.false_positives / denom

    def summary(self) -> dict:
        return {
            "killed": self.killed,
            "resolved": self.resolved,
            "false_positives": self.false_positives,
            "completed": self.completed,
            "total": self.total,
            "throughput": self.throughput(),
            "false_positive_rate": self.false_positive_rate(),
        }
