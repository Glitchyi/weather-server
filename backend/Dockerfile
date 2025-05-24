FROM ghcr.io/astral-sh/uv:0.7.8-python3.9-bookworm-slim
WORKDIR /app

COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

COPY . .

EXPOSE 8765

CMD ["python", "app.py"]
