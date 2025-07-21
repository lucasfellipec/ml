import pandas as pd

filename = "../csv/TT_NEG_WDO_24_06_21"
split = filename.split("_")
date = ""
for s in split:
    if s.isnumeric():
        date += s + "."

date = "20" + date[:-1]

df = pd.read_csv(f"{filename}.txt", delimiter="\t", low_memory=False)
df = df[::-1].reset_index(drop=True)
# df = df[df["Agressor"] != "Leilão"]
df["Date"] = date
df["Valor"] = df["Valor"].str.replace(".", "", regex=False).str.replace(",", ".", regex=False).astype(float)

columns = [
    "Date",
    "Data",
    "Valor",
    "Quantidade",
    "Agressor",
]

df.to_csv(f"{filename}-fixed.txt", columns=columns, sep="\t", index=False)
