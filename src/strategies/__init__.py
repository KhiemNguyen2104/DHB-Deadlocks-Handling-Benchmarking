from strategies.kill import handle as kill_handle
from strategies.retry import handle as retry_handle
from strategies.rollback import handle as rollback_handle

STRATEGY_MAP = {
    "KILL": kill_handle,
    "RETRY": retry_handle,
    "ROLLBACK": rollback_handle,
}
