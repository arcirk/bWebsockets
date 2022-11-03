
#Область ОбработчикиСобытийФормы

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	
	Если глВебСокетКлиент = Неопределено ИЛИ глВебСокетКлиент.ОбъектКомпоненты = Неопределено Тогда
		ПоказатьПредупреждение(Неопределено, "Компонента не загружена!");
		Возврат;
	КонецЕсли;
	
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект), Новый ОписаниеОповещения("ПриОшибке", ЭтотОбъект));
	
	МенеджерВебСокетКлиент.ПолучитьСписокАктивныхПользователей(УникальныйИдентификатор);
	
КонецПроцедуры


&НаКлиенте
Процедура ПередЗакрытием(Отказ, ЗавершениеРаботы, ТекстПредупреждения, СтандартнаяОбработка)
	МенеджерВебСокетКлиент.ФормаПриЗакрытии(УникальныйИдентификатор);
КонецПроцедуры

#КонецОбласти

#Область ОбработчикиКомандФормы

#КонецОбласти 

#Область СлужебныеПроцедурыИФункции

&НаКлиенте
Процедура ПриПолученииСоообщения(Результат, ДополнителныеПараметры) Экспорт
	Попытка
		Если ТипЗнч(Результат) = Тип("Структура") Тогда
			Если Результат.command = "ServerOnlineClientsList" Тогда
				ОтветСервераТаблица = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.result);
				Если ТипЗнч(ОтветСервераТаблица) = Тип("Структура") Тогда
					Строки = ОтветСервераТаблица.rows;
					Если ТипЗнч(Строки) = Тип("Массив") тогда
						ЗаполнитьТаблицуАктивныхПользователей(Строки);
					КонецЕсли;
				КонецЕсли;
			КонецЕсли;
		КонецЕсли;
	Исключение
		//@skip-check use-non-recommended-method
		Сообщить("Не верный формат сообщения!", СтатусСообщения.Важное);
	КонецПопытки;
	
КонецПроцедуры

&НаСервере
Процедура ЗаполнитьТаблицуАктивныхПользователей(МассивСоединений)
	АктивныеПользователи.Очистить();
	Для Каждого ТекСтрока Из МассивСоединений Цикл
		НоваяСтрока = АктивныеПользователи.Добавить();
		НоваяСтрока.ВремяПодключения = ТекСтрока.start_date;
		НоваяСтрока.ИдентификаторПользователя = МенеджерВебСокетВызовСервера.ИдентификаторИзСтроки(ТекСтрока.user_uuid);
		НоваяСтрока.ИдентификаторСессии = МенеджерВебСокетВызовСервера.ИдентификаторИзСтроки(ТекСтрока.session_uuid);
		НоваяСтрока.ИмяПользователя = ТекСтрока.user_name;
		НоваяСтрока.ИмяПриложения = ТекСтрока.app_name;
	КонецЦикла;
КонецПроцедуры

&НаКлиенте 
Процедура ПриОшибке(Результат, ДополнителныеПараметры) Экспорт
	//@skip-check use-non-recommended-method
	Сообщить("ФормаАктивныеПользователи::ПриОшибке");
КонецПроцедуры


#КонецОбласти