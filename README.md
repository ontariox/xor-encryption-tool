# xor-encryption-tool

# XOR Cipher Key Recovery (IC + Frequency Analysis)

##  Описание / Description

**RU:**
Проект посвящён исследованию симметричного шифрования на примере XOR с повторяющимся ключом.
Реализован алгоритм автоматического восстановления ключа с использованием индекса совпадений (IC) и частотного анализа для русского и английского текста.

**EN:**
This project focuses on symmetric encryption using repeating-key XOR.
It implements an automatic key recovery algorithm based on Index of Coincidence (IC) and frequency analysis for both English and Russian texts.

---

##  Цель проекта / Project Goal

**RU:**
Исследовать эффективность статистических методов взлома XOR-шифрования и определить зависимость между длиной текста и длиной ключа.

**EN:**
To analyze the effectiveness of statistical attacks on XOR encryption and determine the relationship between text length and key length.

---

##  Функциональность / Features

* Автоматическое определение длины ключа (IC)
* Восстановление ключа (bruteforce + frequency analysis)
* Поддержка английского и русского языков
* Штрафы за непечатные символы
* Оценка качества расшифровки
* Анализ зависимости длины текста от длины ключа

---

##  Как это работает / How it works

### 1. Определение длины ключа / Key length detection

* Перебор возможных длин
* Разбиение текста на группы
* Вычисление IC
* Выбор оптимального значения

### 2. Восстановление ключа / Key recovery

* Для каждой позиции ключа:

  * перебор байтов (0–255)
  * расшифровка
  * оценка текста

### 3. Оценка результата / Scoring

* частотный анализ
* проверка читаемости
* штрафы за “мусорные” символы

---

##  Технологии / Tech Stack

* C (low-level implementation)
* Standard C libraries
* Custom statistical analysis

---

##  Запуск / Usage

```bash
gcc main.c -o xor_decoder
./xor_decoder input.txt
```

---

##  Ограничения / Limitations

* Требуется достаточная длина текста
* Чувствительность к языку текста
* Возможны ошибки при коротких данных
* Кратные длины ключа могут давать ложные результаты

---

##  What I learned

**RU:**

* Работа со статистическими методами в криптоанализе
* Реализация алгоритмов на C
* Работа с памятью и оптимизация
* Анализ данных и построение зависимостей

**EN:**

* Applied cryptanalysis techniques
* Low-level implementation in C
* Memory management
* Data analysis and visualization

---

