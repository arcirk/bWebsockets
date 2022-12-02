#Область ОписаниеПеременных

Перем глВебСокетКлиент Экспорт;
Перем глВебСокетЛокальныеКлиенты Экспорт;

#КонецОбласти

#Область ОбработчикиСобытий

Процедура ПриНачалеРаботыСистемы()	
	//@skip-check object-deprecated
	УстановитьКраткийЗаголовокПриложения("Компонента не загружена");	
	МенеджерВебСокетКлиент.ПриНачалеРаботыСистемы();
КонецПроцедуры

Процедура ОбработкаВнешнегоСобытия(Источник, Событие, Данные)

	//Сообщить(Событие);
	Результат = МенеджерВебСокетКлиент.ОбработатьВнешнееСобытиие(Источник, Событие, Данные);
	Если Результат Тогда
		Возврат;
	КонецЕсли;
	//Продолжить выполнение
	//ПолучитьСерверТО().ЗавершитьОбработкуВнешнегоСобытия(Источник, Событие, Данные);
КонецПроцедуры

Процедура ПередЗавершениемРаботыСистемы(Отказ, ТекстПредупреждения)
	МенеджерВебСокетКлиент.ПередЗавершениемРаботыСистемы();
КонецПроцедуры

Процедура ОбработкаОтключенияВнешнейКомпонентыПриОшибке(Местоположение, Имя)
	Сообщить(Местоположение + ": " + Имя);
КонецПроцедуры

#КонецОбласти

#Область ПрограмныйИнтерфейс


#КонецОбласти