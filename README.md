### Next steps:

- Calcular a média dos N candles para o DELTA (global e por agressor);
- Calcular o desvio padrão dos N candles;
- Se o candle vigente tiver um delta >= ao delta da média e do desvio padrão, então é um gatilho de entrada;
    - A entrada vai acontecer na mínima ou na máxima (+- 0.5), dependendo do delta;
- Candle de 09:10:
    - Somar todos as agressões de compra e dividir pelo número de preços nos candles anteriores (09:00 e 09:05);
    - Se algum preço do candle de 09:10 tiver um delta de agressão (C) >= a média, considerar ele um candle de gatilho;
