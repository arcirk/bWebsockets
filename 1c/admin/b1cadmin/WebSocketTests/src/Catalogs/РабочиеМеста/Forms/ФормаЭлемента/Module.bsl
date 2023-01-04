
&НаКлиенте
Процедура УстройстваПередНачаломДобавления(Элемент, Отказ, Копирование, Родитель, ЭтоГруппа, Параметр)
//	Отказ = Истина;
//	ОткрытьПодборУстройств();
КонецПроцедуры



#Область ОбработчикиСобытийФормы

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	СтруктураОповещений = МенеджерВебСокетКлиент.ПолучитьСтруктуруОповещенийФормы();
	СтруктураОповещений.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ПриОшибке",  ЭтотОбъект));
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, СтруктураОповещений);
КонецПроцедуры

&НаКлиенте
Процедура ПередЗакрытием(Отказ, ЗавершениеРаботы, ТекстПредупреждения, СтандартнаяОбработка)
	МенеджерВебСокетКлиент.ФормаПриЗакрытии(УникальныйИдентификатор);
КонецПроцедуры

#КонецОбласти

#Область ОбработчикиКомандФормы

&НаКлиенте
Асинх Процедура СинхронизироватьССервером(Команда)
	Если НЕ Ждать КомпонентаДоступна(Истина) Тогда
		Возврат;
	КонецЕсли;
	
	ОбъектКомпоненты = Ждать МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты();
	Результат = Ждать ОбъектКомпоненты.ПолучитьСтруктуруЗаписиАсинх("Workplaces");
	Если Результат <> Неопределено Тогда
		СтруктураЗаписи = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.Значение);
		Если ТипЗнч(СтруктураЗаписи) = Тип("Структура") ТОгда
			УстановитьПараметрыЗаписиНаСервере(СтруктураЗаписи);
			СтруктураПараметровЗапроса = Новый Структура();
			СтруктураПараметровЗапроса.Вставить("table_name", "Workplaces");	
			СтруктураПараметровЗапроса.Вставить("query_type", "update_or_insert");	
			СтруктураПараметровЗапроса.Вставить("values", СтруктураЗаписи);			
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
	КонецЕсли;
КонецПроцедуры

&НаКлиенте
Процедура СинхронизироватьССервером3(Команда)
	СинхронизироватьССервером(Команда)
КонецПроцедуры

#КонецОбласти

#Область СлужебныеПроцедурыИФункции

//&НаКлиенте
//Процедура ДобавитьУстройство(Результат, ДополнительныеСвойства) Экспорт
//	Если Результат <> Неопределено Тогда
//	
//	КонецЕсли;
//КонецПроцедуры

//&НаСервере
//Процедура ДобавитьУстройствоНаСервере(ПараметрыУстройства)
//
//КонецПроцедуры

//&НаКлиенте
//Процедура ОткрытьПодборУстройств()
//	
//	ОписаниеОповещенияОЗакрытии = Новый ОписаниеОповещения("ДобавитьУстройство", ЭтотОбъект);
//	ПараметрыФормы = Новый Структура("РежимВыбора", Истина);
//	ОткрытьФорму("ОбщаяФорма.ФормаОборудование", ПараметрыФормы, ЭтотОбъект, , , , ОписаниеОповещенияОЗакрытии);
//	
//	
////	Если НЕ Ждать КомпонентаДоступна(Истина) Тогда
////		Возврат;
////	КонецЕсли;
////	
////	ОбъектКомпоненты = МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты();
////	Результат = Ждать ОбъектКомпоненты.ПолучитьСтруктуруЗаписиАсинх("Devices");
////	Если Результат <> Неопределено Тогда
////		СтруктураПараметровЗапроса = Новый Структура();
////		СтруктураПараметровЗапроса.Вставить("table_name", "Devices");	
////		СтруктураПараметровЗапроса.Вставить("query_type", "select");	
////		СтруктураПараметровЗапроса.Вставить("values", Новый Структура());			
////		ПараметрыЗапроса = МенеджерВебСокетВызовСервера.СтрокаВBase64(МенеджерВебСокетВызовСервера.ОбъектВJson(СтруктураПараметровЗапроса));				
////		СтруктураКомандСервера = Ждать ОбъектКомпоненты.ПолучитьСтруктуруКомандСервераАсинх();
////		Если СтруктураКомандСервера <> Неопределено Тогда
////			СтруктураКоманд = МенеджерВебСокетВызовСервера.ПрочитатьОтветСервера(СтруктураКомандСервера.Значение);
////			Если ТипЗнч(СтруктураКоманд) = Тип("Структура") Тогда					
////				//Здесь будет вызвано исключение если версия сервера не поддерживается
////				ОбъектКомпоненты.КомандаСерверуАсинх(СтруктураКоманд.ExecuteSqlQuery, 
////				МенеджерВебСокетВызовСервера.ОбъектВJson(Новый Структура("query_param", ПараметрыЗапроса)), 
////				МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(УникальныйИдентификатор));
////			КонецЕсли;
////		КонецЕсли;	
////	КонецЕсли;
//КонецПроцедуры

&НаСервере
Функция ТребуетсяЗаписатьЭлемент()
	Если Модифицированность ТОгда
		Возврат Истина;
	КонецЕсли;
	Возврат Ложь;
КонецФункции

&НаСервере
Процедура УстановитьПараметрыЗаписиНаСервере(СтруктураЗаписи)
	мОбъект = РеквизитФормыВЗначение("Объект");
	Если СтруктураЗаписи.Свойство("_id") Тогда
		СтруктураЗаписи.Удалить("_id"); //автоинкрементное поле
	КонецЕсли;
	МенеджерВебСокетВызовСервера.ОтветСервераУстановитьЗначение(СтруктураЗаписи, "ref", XMLСтрока(мОбъект.Ссылка));
	МенеджерВебСокетВызовСервера.ОтветСервераУстановитьЗначение(СтруктураЗаписи, "first", мОбъект.Наименование);
	МенеджерВебСокетВызовСервера.ОтветСервераУстановитьЗначение(СтруктураЗаписи, "second", мОбъект.Наименование);
КонецПроцедуры

&НаКлиенте
Асинх Функция КомпонентаДоступна(ПоказыватьПредупреждения = Ложь)

	Если ТребуетсяЗаписатьЭлемент() Тогда
#Если НЕ ВебКлиент Тогда		
		Если ПоказыватьПредупреждения Тогда
			Ждать ПредупреждениеАсинх("Требуется записать элемент!");
		КонецЕсли;
#КонецЕсли		
		Возврат Ложь;
	КонецЕсли;

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
//				ПараметрыЗапроса = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.param);
//				РезультатЗапроса = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.result);
//				Если РезультатЗапроса.Свойство("rows") Тогда
//					мПараметры = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(ПараметрыЗапроса.query_param);
//					Если ТипЗнч(мПараметры) = Тип("Структура") ТОгда
//						Если мПараметры.table_name = "Devices" Тогда
//							ЗаполнитьСписокОборудования(РезультатЗапроса.rows);						
//						КонецЕсли;
//					КонецЕсли;
//				КонецЕсли;
			КонецЕсли;
		КонецЕсли;
	Исключение
		//@skip-check use-non-recommended-method
		Сообщить("Не верный формат сообщения!", СтатусСообщения.Важное);
	КонецПопытки;
	
КонецПроцедуры

#КонецОбласти


