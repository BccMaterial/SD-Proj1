#!python

import os

import matplotlib.pyplot as plt
import pandas as pd

# Arquivos
csv_path = "./resultados.csv"
output_dir = "./output"

# Cria diretório de saída caso não exista
os.makedirs(output_dir, exist_ok=True)

# Carrega os dados
df = pd.read_csv(csv_path)

# ==========================
# Gráfico 1 - Tempo de execução
# ==========================
plt.figure(figsize=(10, 6))

for tamanho in sorted(df["Tamanho"].unique()):
    subset = df[df["Tamanho"] == tamanho]
    plt.plot(
        subset["Threads"],
        subset["TempoParaleloMedio"],
        marker="o",
        label=f"N={tamanho}",
    )

plt.xlabel("Número de Threads")
plt.ylabel("Tempo de Execução Médio")
plt.title("Tempo de Execução Paralelo vs Número de Threads")
plt.xticks(sorted(df["Threads"].unique()))
plt.grid(True)
plt.legend(title="Tamanho")
plt.tight_layout()

plt.savefig(
    os.path.join(output_dir, "tempo_execucao_vs_threads.png"),
    dpi=300,
    bbox_inches="tight",
)
plt.close()

# ==========================
# Gráfico 2 - Speedup
# ==========================
plt.figure(figsize=(10, 6))

for tamanho in sorted(df["Tamanho"].unique()):
    subset = df[df["Tamanho"] == tamanho]
    plt.plot(subset["Threads"], subset["Speedup"], marker="o", label=f"N={tamanho}")

# Linha de speedup ideal
threads = sorted(df["Threads"].unique())
plt.plot(threads, threads, linestyle="--", label="Ideal")

plt.xlabel("Número de Threads")
plt.ylabel("Speedup")
plt.title("Speedup vs Número de Threads")
plt.xticks(threads)
plt.grid(True)
plt.legend(title="Tamanho")
plt.tight_layout()

plt.savefig(
    os.path.join(output_dir, "speedup_vs_threads.png"), dpi=300, bbox_inches="tight"
)
plt.close()

print(f"Gráficos salvos em: {output_dir}")
