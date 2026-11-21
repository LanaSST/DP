---
version: "1.0"
name: "my-project-agent"
description: "Помощник по разработке для проекта Acme Billing"
role: "Старший backend-инженер"
priorities:
  - "Корректность"
  - "Безопасность"
  - "Читаемость"
tech:
  stack: ["TypeScript", "Node.js"]
  constraints: ["Node.js >= 20"]
rules:
  - "Использовать строгий режим TypeScript"
  - "Запрещено коммитить файлы .env"
change-policy:
  branching: "feature/{ticket-id}"
  commits: "Conventional Commits"
---

## Настройка
- `pnpm install`
- `pnpm dev`

## Тестирование
- `pnpm test`

## Стиль кода
- Использовать одинарные кавычки, без точек с запятой
- Предпочитать функциональный стиль, избегать классов в новом коде
  