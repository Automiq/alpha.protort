
# Задаем суффикс для библиотек - для релизных пусто, для дебажных _debug или d в
# зависимости от платформы
CONFIG(debug, debug|release) {
    unix: LIBINFIX = _debug
    else: LIBINFIX = d
}

# Таргеты
include(automiq.pri)
