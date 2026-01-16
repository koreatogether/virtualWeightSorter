from fastapi import FastAPI
from pydantic import BaseModel


class HealthResponse(BaseModel):
    status: str


app = FastAPI(title="DS18B20 / DHT22 Backend API")


@app.get("/health", response_model=HealthResponse)
def health() -> dict[str, str]:
    """Health endpoint returning a simple status object."""
    return {"status": "ok"}


@app.get("/sensors")
def sensors() -> dict[str, list]:
    """Return current sensors list (empty by default)."""
    return {"sensors": []}
