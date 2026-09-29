import json
from pathlib import Path
from typing import Dict, Any

CONFIG_PATH = Path(__file__).parent / "config.json"

_config: Dict[str, Any] = {}

def load_config() -> Dict[str, Any]:
    global _config
    with open(CONFIG_PATH) as f:
        _config = json.load(f)
    return _config

def get_config() -> Dict[str, Any]:
    if not _config:
        load_config()
    return _config

def save_config(config: Dict[str, Any]):
    global _config
    _config = config
    with open(CONFIG_PATH, "w") as f:
        json.dump(config, f, indent=2)

config = get_config()