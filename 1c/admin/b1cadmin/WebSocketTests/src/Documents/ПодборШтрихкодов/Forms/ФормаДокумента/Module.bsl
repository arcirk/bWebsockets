
#Область ОбработчикиСобытийФормы

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	СтруктураОповещений = МенеджерВебСокетКлиент.ПолучитьСтруктуруОповещенийФормы();
	СтруктураОповещений.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ПриОшибке",  ЭтотОбъект));
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, СтруктураОповещений);
КонецПроцедуры

&НаКлиенте
Процедура ПослеЗаписи(ПараметрыЗаписи)
	
КонецПроцедуры

&НаКлиенте
Процедура ПриЗакрытии(ЗавершениеРаботы)
	МенеджерВебСокетКлиент.ФормаПриЗакрытии(УникальныйИдентификатор);
КонецПроцедуры


&НаКлиенте
Процедура ПередЗаписью(Отказ, ПараметрыЗаписи)
	Если НЕ ЗначениеЗаполнено(Объект.Устройство) ТОгда
		ПоказатьПредупреждение(, "Не указано устройство!");
		Отказ = Истина;
	КонецЕсли;
КонецПроцедуры

#КонецОбласти

#Область ОбработчикиКомандФормы

&НаКлиенте
Асинх Процедура ВыгрузитьНаСервер(Команда)
	Если НЕ Ждать КомпонентаДоступна(Истина) Тогда
		Возврат;
	КонецЕсли;
	
	ОбъектКомпоненты = Ждать МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты();
	Результат = Ждать ОбъектКомпоненты.ПолучитьСтруктуруЗаписиАсинх("Documents");
	Если Результат <> Неопределено Тогда
		СтруктураЗаписи = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.Значение);
		Если ТипЗнч(СтруктураЗаписи) = Тип("Структура") ТОгда
			УстановитьПараметрыЗаписиНаСервере(СтруктураЗаписи);
			СтруктураПараметровЗапроса = Новый Структура();
			СтруктураПараметровЗапроса.Вставить("table_name", "Documents");	
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
Процедура ОбновитьССервера(Команда)
	//TODO: Вставить содержимое обработчика
КонецПроцедуры

&НаСервере
Процедура УстановитьПараметрыЗаписиНаСервере(СтруктураЗаписи)
	СтруктураЗаписи.ref = XMLСтрока(Объект.Ссылка);
	СтруктураЗаписи.first = СокрЛП(Объект.Номер);
	СтруктураЗаписи.second = "Подбор штрихкодов";
	СтруктураЗаписи.date = Объект.Дата - Дата(1,1,1);
КонецПроцедуры

&НаСервере
Процедура УстановитьПараметрыЗаписиТабличнойЧастиНаСервере(СтруктураЗаписи, ИдентификаторСтроки)
	ВыбраннаяСтрока = Объект.Штрихкоды.НайтиПоИдентификатору(ИдентификаторСтроки);
	Если ВыбраннаяСтрока = Неопределено Тогда
		Возврат;
	КонецЕсли;
	Если НЕ ЗначениеЗаполнено(ВыбраннаяСтрока.КлючСтроки) Тогда
		ВыбраннаяСтрока.КлючСтроки = Новый УникальныйИдентификатор();
	КонецЕсли;
	СтруктураЗаписи.ref = XMLСтрока(ВыбраннаяСтрока.КлючСтроки);
	СтруктураЗаписи.first = "СтрокаТабличнойЧасти";
	СтруктураЗаписи.barcode = СокрЛП(ВыбраннаяСтрока.Штрихкод);
	СтруктураЗаписи.quantity = ВыбраннаяСтрока.Количество;
	СтруктураЗаписи.parent = XMLСтрока(Объект.Ссылка);
КонецПроцедуры

#КонецОбласти

#Область СлужебныеПроцедурыИФункции

&НаСервере
Функция ТребуетсяЗаписатьДокумент()
	Если Модифицированность ТОгда
		Возврат Истина;
	КонецЕсли;
	Возврат Ложь;
КонецФункции

&НаКлиенте
Асинх Функция КомпонентаДоступна(ПоказыватьПредупреждения = Ложь)
	
	Если ТребуетсяЗаписатьДокумент() Тогда
#Если НЕ ВебКлиент Тогда		
		Если ПоказыватьПредупреждения Тогда
			Ждать ПредупреждениеАсинх("Требуется записать документ!");
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

&НаКлиенте
Асинх Процедура ОбновитьТабличнуюЧастьНаСервере()
	
	Если НЕ Ждать КомпонентаДоступна(Истина) Тогда
		Возврат;
	КонецЕсли;
	
	ОбъектКомпоненты = Ждать МенеджерВебСокетКлиент.ПолучитьТекущийОбъектКомпоненты();
	Результат = Ждать ОбъектКомпоненты.ПолучитьСтруктуруЗаписиАсинх("DocumentTable");
	Если Результат <> Неопределено Тогда
		СтруктураЗаписиШаблон = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.Значение);
		Если ТипЗнч(СтруктураЗаписиШаблон) = Тип("Структура") ТОгда
			
			мСтроки = Новый Массив;
			Для Каждого ТекСтрока Из Объект.Штрихкоды Цикл
				СтруктураЗаписи = МенеджерВебСокетВызовСервера.СкопироватьКлючиСтруктуры(СтруктураЗаписиШаблон);
				УстановитьПараметрыЗаписиТабличнойЧастиНаСервере(СтруктураЗаписи, ТекСтрока.ПолучитьИдентификатор());
				мСтроки.Добавить(СтруктураЗаписи);
			КонецЦикла;
			СтруктураПараметровЗапроса = Новый Структура();
			СтруктураПараметровЗапроса.Вставить("table_name", "DocumentTable");	
			СтруктураПараметровЗапроса.Вставить("query_type", "clear_and_insert");	
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
Процедура ПриПолученииСоообщения(Результат, ДополнителныеПараметры) Экспорт
	Попытка
		Если ТипЗнч(Результат) = Тип("Структура") И Результат.message = "OK" Тогда
			Если Результат.command = "ExecuteSqlQuery" Тогда
				ПараметрыКоманды = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.param);
				Если ТипЗнч(ПараметрыКоманды) = Тип("Структура") И ПараметрыКоманды.Свойство("query_param") Тогда
					ПараметрыЗапроса = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(ПараметрыКоманды.query_param);
					Если ТипЗнч(ПараметрыЗапроса) = Тип("Структура") Тогда
						Если ПараметрыЗапроса.table_name = "Documents" И ПараметрыЗапроса.query_type = "update_or_insert" Тогда
							Если Объект.Штрихкоды.Количество() > 0 Тогда
								ОбновитьТабличнуюЧастьНаСервере();
							КонецЕсли;
							//Сообщить("Документ успешно обновлен на сервере!")
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

&НаКлиенте 
Процедура ПриОшибке(Результат, ДополнителныеПараметры) Экспорт
	//@skip-check use-non-recommended-method
	Сообщить("ПодборШтрихкодов::ПриОшибке");
КонецПроцедуры


#КонецОбласти