# Запуск в Visual Studio

Открывать нужно:

```text
HamiltonMouse\HamiltonMouse.sln
```

Исходники и файлы решения лежат в этой же папке.

## Требования

1. Visual Studio 2022 с workload **Desktop development with C++**.
2. Allegro 5.

## Установка Allegro через NuGet

В Visual Studio:

1. Открыть `Project -> Manage NuGet Packages`.
2. Найти пакет `Allegro`.
3. Установить основной пакет **Allegro** версии `5.2.11.3`.
4. Пакет `AllegroDeps` отдельно ставить обычно не нужно: он подтягивается как зависимость.

## Сборка

1. Открыть `HamiltonMouse\HamiltonMouse.sln`.
2. Выбрать `Debug | x64`.
3. Запустить `Build -> Build Solution`.
4. Запустить `Debug -> Start Without Debugging`.

Если появляется ошибка:

```text
allegro5/allegro.h: No such file or directory
```

значит пакет Allegro не восстановился или не установлен через NuGet.
