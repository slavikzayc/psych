#pragma once

#include <cstdint>

// Entity — это только числовой идентификатор.
// Сам по себе он не содержит поведения и данных. Все данные сущности
// лежат отдельно в компонентах Registry: positions, renderables, patients и
// т.д.
using Entity = std::uint32_t;
