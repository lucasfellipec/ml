import pandas as pd
import glob

folder_path = "times_and_trades_bases"
txt_files = glob.glob(f"{folder_path}/**/*.txt", recursive=True)

for file in txt_files:
    tmp = file.split(".txt")[0]
    try:
        splitted_filename = tmp.split("_")
        date = ".".join(splitted_filename[-3:])

        if "inc" in file:
            date = date.split(" ")[0]

        df = pd.read_csv(f"{file}", delimiter="\t", low_memory=False)
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

        df.to_csv(f"{file}", columns=columns, sep="\t", index=False)
    except AttributeError:
        continue
