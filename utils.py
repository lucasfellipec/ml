import pandas as pd

days = [
    "TT_NEG_WDO_2024_05_02",
    "TT_NEG_WDO_2024_05_03",
    "TT_NEG_WDO_2024_05_06",
    "TT_NEG_WDO_2024_05_07",
    "TT_NEG_WDO_2024_05_08",
    "TT_NEG_WDO_2024_05_09",
    "TT_NEG_WDO_2024_05_10",
    "TT_NEG_WDO_2024_05_13",
    "TT_NEG_WDO_2024_05_14",
    "TT_NEG_WDO_2024_05_15",
    "TT_NEG_WDO_2024_05_16",
    "TT_NEG_WDO_2024_05_17",
    "TT_NEG_WDO_2024_05_20",
    "TT_NEG_WDO_2024_05_21",
    "TT_NEG_WDO_2024_05_22",
    "TT_NEG_WDO_2024_05_23",
    "TT_NEG_WDO_2024_05_24",
    "TT_NEG_WDO_2024_05_27",
    "TT_NEG_WDO_2024_05_28",
    "TT_NEG_WDO_2024_05_29",
    "TT_NEG_WDO_2024_05_31",
]

for day in days:
    filename = f"times_and_trades_bases/2024.05/{day}"
    splitted_filename = filename.split("_")
    date = ".".join(splitted_filename[-3:])

    if "inc" in filename:
        date = date.split(" ")[0]

    df = pd.read_csv(f"{filename}.txt", delimiter="\t", low_memory=False)
    df = df[::-1].reset_index(drop=True)
    df["Date"] = date
    df["Valor"] = df["Valor"].str.replace(".", "", regex=False).str.replace(
        ",", ".", regex=False).astype(float)

    columns = [
        "Date",
        "Data",
        "Valor",
        "Quantidade",
        "Agressor",
    ]

    df.to_csv(f"{filename}.txt", columns=columns, sep="\t", index=False)
