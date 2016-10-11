
# Задаем суффикс для библиотек - для релизных пусто, для дебажных _debug или d в
# зависимости от платформы
CONFIG(debug, debug|release) {
    unix: LIBINFIX = _debug
    else: LIBINFIX = d
}

win32 {
    LIBPREFIX = lib
}

# Таргеты
include(automiq.pri)
