
&НаКлиенте
Процедура Заполнить(Команда)
	Оповестить("ДанныеСТСД", Товары, ЭтотОбъект);
	Закрыть();
КонецПроцедуры

&НаСервере
Процедура ПриСозданииНаСервере(Отказ, СтандартнаяОбработка)
		
	Устройство = Параметры.Устройство;	
	Заголовок = "Товары: " + Устройство.НаименованиеПолное;
	ИдентификаторДокумента = Параметры.ИдентификаторДокумента;
	Если ЗначениеЗаполнено(Устройство) ТОгда
		ПараметрыКоманды = ПолучитьПараметрыКоманды(XMLСтрока(Параметры.ИдентификаторДокумента));
		ТекстЗапроса = "api/info";
		СоответствиеДанные = Новый Соответствие;
		СоответствиеДанные.Вставить("command", "ExecuteSqlQuery");
		СоответствиеДанные.Вставить("param", ПараметрыКоманды); 
		ЗаписьJSON = Новый ЗаписьJSON;
		ЗаписьJSON.УстановитьСтроку();
		ЗаписатьJSON(ЗаписьJSON, СоответствиеДанные);
		СтрокаДанные = ЗаписьJSON.Закрыть();
		HTTPСоединение = Новый HTTPСоединение("192.168.10.80", 8080);
		HTTPЗапрос = Новый HTTPЗапрос(ТекстЗапроса);
		HTTPЗапрос.Заголовки.Вставить("Content-Type", "application/json");
		HTTPЗапрос.Заголовки.Вставить("Accept", "application/json");
		Токен = МенеджерВебСокетВызовСервера.СтрокаВBase64("admin:admin");
		HTTPЗапрос.Заголовки.Вставить("Authorization", "Basic " + Токен);
		HTTPЗапрос.УстановитьТелоИзСтроки(СтрокаДанные, "UTF-8", ИспользованиеByteOrderMark.НеИспользовать);
		Попытка
			Результат = HTTPСоединение.ОтправитьДляОбработки(HTTPЗапрос);
		Исключение
			Сообщить(ОписаниеОшибки());
			Возврат;
		КонецПопытки;
	КонецЕсли;
	
	РезультатТело = Результат.ПолучитьТелоКакСтроку();
	
	Если Результат.КодСостояния = 200 ТОгда
		ОтветСервера = МенеджерВебСокетВызовСервера.ПрочитатьОтветСервера(РезультатТело);
		Если ОтветСервера.message = "OK" И СокрЛП(ОтветСервера.result) <> "" Тогда
			СписокДокументов = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(ОтветСервера.result);
			Если ТипЗнч(СписокДокументов.rows) = Тип("Массив") Тогда
				ЗаполнитьСписокСтрокДокумента(СписокДокументов.rows);
			КонецЕсли;
		КонецЕсли;
	Иначе
		Сообщить(РезультатТело);
	КонецЕсли;
	
КонецПроцедуры


&НаСервереБезКонтекста
Функция ПолучитьПараметрыКоманды(Идентификатор)
	
	Параметры = Новый Структура;
	Параметры.Вставить("table_name", "DocumentsTables");
	Параметры.Вставить("query_type", "select");
	Параметры.Вставить("values", Новый Структура());
	Параметры.Вставить("where_values", Новый Структура("parent", Идентификатор));
	Параметры_ = МенеджерВебСокетВызовСервера.СтрокаВBase64(МенеджерВебСокетВызовСервера.ОбъектВJson(Параметры));
    query_param = МенеджерВебСокетВызовСервера.СтрокаВBase64(МенеджерВебСокетВызовСервера.ОбъектВJson(Новый Структура("query_param", Параметры_)));
	Возврат query_param;

КонецФункции

&НаСервере
Процедура ЗаполнитьСписокСтрокДокумента(МассивСтрок)
	Товары.Очистить();
	Для Каждого ТекСтрока Из МассивСтрок Цикл
		НоваяСтрока = Товары.Добавить();
		НоваяСтрока.Штрихкод = ТекСтрока.barcode;
		НоваяСтрока.Количество = ТекСтрока.quantity;
	КонецЦикла;
КонецПроцедуры