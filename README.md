Alpha.ProtoRt
=============

[![Build Status](https://travis-ci.org/Automiq/alpha.protort.svg?branch=master)](https://travis-ci.org/Automiq/alpha.protort)

Alpha.ProtoRt - прототип распределённой событийно-ориентированной вычислительной среды

1.Components - В нём находится компоненты . В component.h находится класс , от которого должен наследоватся каждый компонент. В components подключаются заголовки со всеми компонентами.

2.Designer-Предполагался дизайн схемы приложения и схем депллоя, на данный момент пуст.

3.LInk-Сетевая библиотека , на данный момент не актуальна.

4.Node-Данный проект при компиляции построит узел..Который будет запущенн на сервере и будет работать с копонентами , а так же выполнять общение с другими узлами .

5.Parser-Данный проект парсит xml файлы со схемами приложений и развёртывания.На данный момент его использует только терминал для разбор данных файлов.Использует boost::property_tree для парсинга xml.

6.Protocol -описание протоколов на языке protobuf, позволяет терминалу отдавать комманды node, загружать им все схемы и собирать статистику с них, узел посредством protocol осуществляют общение между собой.

7.Protolink-Новая сетевая библиотека, использующая протокол ,описанный в соответствующем проекте , для работы.Позволяет открыть и удерживать сетевое соединение. Используется терминалом и узлом.

8.Terminal-Это клиентское приложение которое подключается к узлам,загружает на них все схемы , позволяет ими управлять (останавливать , перезагружать , запускать ) , собирает статистику.

Заметка: alpha.protort.doxygen - находится в корне репозитория , необходим для автогенерации документации по проекту как doxygen. Файл открывается в doxywizard->run->run doxygen . Документация сгенерируется в index.html в папке documentation.
