
#Область ОбработчикиСобытийФормы

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	СтруктураОповещений = МенеджерВебСокетКлиент.ПолучитьСтруктуруОповещенийФормы();
	СтруктураОповещений.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ПриОшибке",  ЭтотОбъект));
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, СтруктураОповещений);
	ПолучитьСписокУстройств();
КонецПроцедуры

&НаКлиенте
Процедура ПередЗакрытием(Отказ, ЗавершениеРаботы, ТекстПредупреждения, СтандартнаяОбработка)
	МенеджерВебСокетКлиент.ФормаПриЗакрытии(УникальныйИдентификатор);
КонецПроцедуры

&НаСервере
Процедура ПриСозданииНаСервере(Отказ, СтандартнаяОбработка)
	Если Параметры.Свойство("РежимВыбора") ТОгда
		Элементы.СписокОборудования.РежимВыбора = Истина;
	КонецЕсли;
КонецПроцедуры

&НаКлиенте
Процедура СписокОборудованияВыбор(Элемент, ВыбраннаяСтрока, Поле, СтандартнаяОбработка)
	//TODO: Вставить содержимое обработчика
КонецПроцедуры

&НаКлиенте
Процедура СписокОборудованияВыборЗначения(Элемент, Значение, СтандартнаяОбработка)
	//TODO: Вставить содержимое обработчика
КонецПроцедуры


#КонецОбласти

#Область ОбработчикиКомандФормы


#КонецОбласти

#Область СлужебныеПроцедурыИФункции

//&НаКлиенте
//Асинх Функция Подключен() Экспорт
//	Если глВебСокетКлиент = Неопределено ИЛИ  глВебСокетКлиент.ОбъектКомпоненты = Неопределено ТОгда 
//		Возврат Ложь;
//	Иначе
//		Результат = Ждать глВебСокетКлиент.ОбъектКомпоненты.ЗапущенАсинх();
//		Возврат ?(Результат = Неопределено, Ложь, Результат.Значение);
//	КонецЕсли;
//КонецФункции

&НаКлиенте
Асинх Процедура ПолучитьСписокУстройств()
	
	Если НЕ Ждать КомпонентаДоступна() Тогда
		Возврат;
	КонецЕсли;
	
	ОбъектКомпоненты = МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты();
	Результат = Ждать ОбъектКомпоненты.ПолучитьСтруктуруЗаписиАсинх("Devices");
	Если Результат <> Неопределено Тогда
		СтруктураПараметровЗапроса = Новый Структура();
		СтруктураПараметровЗапроса.Вставить("table_name", "Devices");	
		СтруктураПараметровЗапроса.Вставить("query_type", "select");	
		СтруктураПараметровЗапроса.Вставить("values", Новый Структура());			
		ПараметрыЗапроса = МенеджерВебСокетВызовСервера.СтрокаВBase64(МенеджерВебСокетВызовСервера.ОбъектВJson(СтруктураПараметровЗапроса));				
		СтруктураКомандСервера = Ждать ОбъектКомпоненты.ПолучитьСтруктуруКомандСервераАсинх();
		Если СтруктураКомандСервера <> Неопределено Тогда
			СтруктураКоманд = МенеджерВебСокетВызовСервера.ПрочитатьОтветСервера(СтруктураКомандСервера.Значение);
			Если ТипЗнч(СтруктураКоманд) = Тип("Структура") Тогда					
				//Здесь будет вызвано исключение если версия сервера не поддерживается
				ОбъектКомпоненты.КомандаСерверуАсинх(СтруктураКоманд.ExecuteSqlQuery, 
				МенеджерВебСокетВызовСервера.ОбъектВJson(Новый Структура("query_param", ПараметрыЗапроса)), 
				МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(УникальныйИдентификатор));
			КонецЕсли;
		КонецЕсли;	
	КонецЕсли;
КонецПроцедуры

&НаКлиенте
Асинх Функция КомпонентаДоступна(ПоказыватьПредупреждения = Ложь)

	Если МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты() = Неопределено Тогда
#Если НЕ ВебКлиент Тогда		
		Если ПоказыватьПредупреждения Тогда
			Ждать ПредупреждениеАсинх("Компонента не загружена!");
		КонецЕсли;
#КонецЕсли	
		Возврат Ложь;
	КонецЕсли;
	
	Если НЕ Ждать МенеджерВебСокетКлиент.Подключен() Тогда
#Если НЕ ВебКлиент Тогда		
		Если ПоказыватьПредупреждения Тогда
			Ждать ПредупреждениеАсинх("Сервер не доступен!");
		КонецЕсли;
#КонецЕсли	
		Возврат Ложь;				
	КонецЕсли;
	
	Возврат Истина;
	
КонецФункции

Процедура ПриОшибке(Результат, ДополнителныеПараметры) Экспорт
	//@skip-check use-non-recommended-method
	Сообщить(Результат.command + "::Error: " + Результат.message, СтатусСообщения.Важное);
КонецПроцедуры

&НаКлиенте
Процедура ПриПолученииСоообщения(Результат, ДополнителныеПараметры) Экспорт
	Попытка
		Если ТипЗнч(Результат) = Тип("Структура") Тогда
			Если Результат.command = "ExecuteSqlQuery" Тогда
				ПараметрыЗапроса = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.param);
				РезультатЗапроса = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.result);
				Если РезультатЗапроса.Свойство("rows") Тогда
					мПараметры = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(ПараметрыЗапроса.query_param);
					Если ТипЗнч(мПараметры) = Тип("Структура") ТОгда
						Если мПараметры.table_name = "Devices" Тогда
							ЗаполнитьСписокОборудования(РезультатЗапроса.rows);						
						КонецЕсли;
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
Процедура ЗаполнитьСписокОборудования(МассивОборудования)
	СписокОборудования.Очистить();
	Для Каждого ТекСтрока Из МассивОборудования Цикл
		НоваяСтрока = СписокОборудования.Добавить();
		НоваяСтрока.Идентификатор = Новый УникальныйИдентификатор(ТекСтрока.ref);
		НоваяСтрока.Наименование = ТекСтрока.first;
		НоваяСтрока.ТипУстройсва = ТекСтрока.deviceType;	
	КонецЦикла;
КонецПроцедуры


#КонецОбласти