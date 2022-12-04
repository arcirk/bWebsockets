
#Область ОбработчикиСобытийФормы

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	
	Если глВебСокетКлиент = Неопределено ИЛИ глВебСокетКлиент.ОбъектКомпоненты = Неопределено Тогда
		ПоказатьПредупреждение(Неопределено, "Компонента не загружена!");
		Возврат;
	КонецЕсли;
	
	СтруктураОповещений = МенеджерВебСокетКлиент.ПолучитьСтруктуруОповещенийФормы();
	СтруктураОповещений.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ПриОшибке",  ЭтотОбъект));
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, СтруктураОповещений);
	
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

&НаСервере
Процедура ОчиститьСписок()
	АктивныеПользователи.Очистить();
КонецПроцедуры

&НаСервере
Процедура ПриОтключенииПользователя(ИдентификаторСтрокой)
	Идентификатор = МенеджерВебСокетВызовСервера.ИдентификаторИзСтроки(ИдентификаторСтрокой);
	мАктивныеПользователи = РеквизитФормыВЗначение("АктивныеПользователи", Тип("ТаблицаЗначений"));
	НайденнаяСессия = мАктивныеПользователи.Найти(Идентификатор, "ИдентификаторСессии");
	Если НайденнаяСессия <> Неопределено Тогда
		мАктивныеПользователи.Удалить(НайденнаяСессия);	
	КонецЕсли;
	ЗначениеВРеквизитФормы(мАктивныеПользователи, "АктивныеПользователи");
КонецПроцедуры

&НаКлиенте
Процедура ПриПолученииСоообщения(Результат, ДополнителныеПараметры) Экспорт
	Попытка
		Если ТипЗнч(Результат) = Тип("Структура") Тогда
			Если Результат.command = "ServerOnlineClientsList" Тогда
				ОчиститьСписок();
				ОтветСервераТаблица = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.result);
				Если ТипЗнч(ОтветСервераТаблица) = Тип("Структура") Тогда
					Строки = ОтветСервераТаблица.rows;
					Если ТипЗнч(Строки) = Тип("Массив") тогда
						ЗаполнитьТаблицуАктивныхПользователей(Строки);
					КонецЕсли;
				КонецЕсли;
			ИначеЕсли Результат.command = "ClientLeave" Тогда 
				ПриОтключенииПользователя(Результат.sender);
			ИначеЕсли Результат.command = "ClientJoin" Тогда 
				МенеджерВебСокетКлиент.ПолучитьСписокАктивныхПользователей(УникальныйИдентификатор);
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
		НоваяСтрока.Роль = ТекСтрока.role;
	КонецЦикла;
КонецПроцедуры

&НаКлиенте 
Процедура ПриОшибке(Результат, ДополнителныеПараметры) Экспорт
	//@skip-check use-non-recommended-method
	Сообщить("ФормаАктивныеПользователи::ПриОшибке");
КонецПроцедуры


#КонецОбласти