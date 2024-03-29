
&НаКлиенте
Перем ОбъектКомпоненты, НастройкиКлиента1С, ИдентификаторСессии, ТекущийПолучатель;

&НаКлиенте
Функция ИнициализироватьКомпоненту()
	ОбъектКомпоненты = МенеджерВебСокетКлиент.ПолучитьНовыйОбъектКомпоненты();
	Возврат ОбъектКомпоненты <> Неопределено;
КонецФункции

&НаКлиенте
Процедура ОтправитьСообщение(Команда)

	Если ОбъектКомпоненты = Неопределено Тогда
		Возврат;
	КонецЕсли;
	Если ТекущийПолучатель = Неопределено Тогда
		Возврат;
	КонецЕсли;

	СообщениеПользователю = Сообщение1СЧата;

	ДобавитьСообщение(СообщениеПользователю);
	Сообщение1СЧата = "";
	ОтправитьСообщениеПользователю(СообщениеПользователю);
	
КонецПроцедуры

&НаКлиенте
Процедура ОтправитьСообщениеПользователю(СообщениеПолзователю)
	
	Если ОбъектКомпоненты = Неопределено Тогда
		Возврат;
	КонецЕсли;
	
	Если ТекущийПолучатель = Неопределено Тогда
		Возврат;
	КонецЕсли;
	
	ОбъектКомпоненты.ОтправитьСообщение(ТекущийПолучатель.session_uuid, СообщениеПолзователю, "{}");	
	
КонецПроцедуры

&НаСервере
Процедура ДобавитьСообщение(СообщениеПользователю)
	
	НоваяСтрока = Чат.Добавить();
	НоваяСтрока.Дата = ТекущаяДата();
	НоваяСтрока.Автор = Пользователь1СЧат;
	НоваяСтрока.Сообщение = СообщениеПользователю;
	
КонецПроцедуры

&НаСервере
Процедура ПолзовательПриИзменении()

КонецПроцедуры

&НаКлиенте
Процедура ИмяПользователяПриИзменении(Элемент)
	
	Если НЕ ЗначениеЗаполнено(Пользователь1СЧат) Тогда
		НастройкиКлиента1С = Неопределено;
		Возврат;
	КонецЕсли;
	СтруктураОповещений = МенеджерВебСокетКлиент.ПолучитьСтруктуруОповещенийФормы();
	СтруктураОповещений.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ПриПолученииСоообщения",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ПриОшибке",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеОПодключении", Новый ОписаниеОповещения("ПриПодключении",  ЭтотОбъект));
	СтруктураОповещений.Вставить("ОповещениеОУспешнойАвторизации", Новый ОписаниеОповещения("ПриУспешнойАвторизации",  ЭтотОбъект));
	МенеджерВебСокетКлиент.ПодключитьФорму(ЭтотОбъект, СтруктураОповещений);
	
КонецПроцедуры

&НаКлиенте
Процедура ИмяПользователяНачалоВыбора(Элемент, ДанныеВыбора, СтандартнаяОбработка)
	
	СтандартнаяОбработка = Ложь;
	
	НастройкиКомпоновки = Новый НастройкиКомпоновкиДанных;
	ГруппаОтбора = НастройкиКомпоновки.Отбор.Элементы.Добавить(Тип("ГруппаЭлементовОтбораКомпоновкиДанных"));
	ГруппаОтбора.ТипГруппы = ТипГруппыЭлементовОтбораКомпоновкиДанных.ГруппаИ;
	ЭлементОтбора = ГруппаОтбора.Элементы.Добавить(Тип("ЭлементОтбораКомпоновкиДанных")); 
	ЭлементОтбора.ЛевоеЗначение  = Новый ПолеКомпоновкиДанных("Ссылка");
	ЭлементОтбора.ВидСравнения   = ВидСравненияКомпоновкиДанных.НеРавно;
	ЭлементОтбора.Использование  = Истина;
	ЭлементОтбора.ПравоеЗначение = Пользователи.ТекущийПользователь();
	
	ПараметрыФормы = Новый Структура;
	ПараметрыФормы.Вставить("ФиксированныеНастройки", НастройкиКомпоновки);
	ПараметрыФормы.Вставить("РежимВыбора",Истина);
	ПараметрыФормы.Вставить("ТолькоПросмотр",Истина);
	ОткрытьФорму("Справочник.Пользователи.Форма.ФормаВыбора", ПараметрыФормы, Элемент);
	
КонецПроцедуры

&НаСервере
Процедура ПриСозданииНаСервере(Отказ, СтандартнаяОбработка)
	ОтладкаПутьКHtmlФайлу = "C:\src\new\html_client\index.html";
	НастройкиСервера = МенеджерВебСокетВызовСервера.ПолучитьНастройкиСервера();
	АдресСервера1СЧатДемо = НастройкиСервера.АдресСервера;
КонецПроцедуры

&НаКлиенте
Процедура ПриОткрытии(Отказ)
	HtmlЧат = ОтладкаПутьКHtmlФайлу;
	ОбъектСоздан = ИнициализироватьКомпоненту();
КонецПроцедуры

&НаКлиенте
Асинх Процедура Подключится1СЧат(Команда)
	
	Если ОбъектКомпоненты = Неопределено Тогда
		Возврат;		
	КонецЕсли;
	НастройкиКлиента1С = МенеджерВебСокетВызовСервера.ПолучитьЛокальныеНастройкиКлиента(Пользователь1СЧат);	
	Если НастройкиКлиента1С = Неопределено Тогда
		Возврат;
	КонецЕсли;
	Если СокрЛП(АдресСервера1СЧатДемо) = "" ТОгда
		Возврат;
	КонецЕсли;
	
	ИмяПользователя = ПолучитьСтандартноеИмяПользователя(Пользователь1СЧат);
	ХешПользователя = МенеджерВебСокетВызовСервера.ПолучитьХешСтандартногоПароля(Пользователь1СЧат);
	Если НЕ НастройкиКлиента1С.ИспользоватьДанныеПользователя ТОгда
		ИмяПользователя = НастройкиКлиента1С.ИмяПользователя;
		ХешПользователя = НастройкиКлиента1С.ХешПользователя;
	КонецЕсли;
	Идентификатор = МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(Пользователь1СЧат.УникальныйИдентификатор());
	ИмяПриложения = "1c chat client";
	Ждать ОбъектКомпоненты.УстановитьПараметрыАсинх(ИмяПользователя, ХешПользователя, Идентификатор, ИмяПриложения); 
	Ждать ОбъектКомпоненты.УстановитьВосстановитьСоединениеПриРазрывеАсинх(Ложь);
	Ждать ОбъектКомпоненты.УстановитьОсновнаяФормаАсинх(МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(ЭтотОбъект.УникальныйИдентификатор));
	Ждать ОбъектКомпоненты.ОткрытьАсинх(АдресСервера1СЧатДемо);

КонецПроцедуры

&НаКлиенте
Процедура ПередЗакрытием(Отказ, ЗавершениеРаботы, ТекстПредупреждения, СтандартнаяОбработка)
	Если ОбъектКомпоненты <> Неопределено Тогда
		ОбъектКомпоненты.ЗакрытьАсинх(Истина);
	КонецЕсли;
КонецПроцедуры

&НаСервере
Функция ПолучитьСтандартноеИмяПользователя(ВыбранныйПользователь)
	Возврат СокрЛП(ВыбранныйПользователь.Код);
КонецФункции

&НаКлиенте
Процедура Отключится1СЧат(Команда)
	Если ОбъектКомпоненты <> Неопределено Тогда
		ОбъектКомпоненты.ЗакрытьАсинх(Истина);
	КонецЕсли;
КонецПроцедуры

&НаКлиенте
Процедура ПриПолученииСоообщения(Результат, ДополнительныеСвойства) Экспорт
	Отправитель = МенеджерВебСокетВызовСервера.ОтветСервераПолучитьЗначение(Результат, "sender");
	Если Отправитель = Неопределено Тогда
		Возврат;
	КонецЕсли;	
	Если Отправитель = ИдентификаторСессии Тогда
		Возврат;
	КонецЕсли;
	Событие = МенеджерВебСокетВызовСервера.ОтветСервераПолучитьЗначение(Результат, "command", "");
	Попытка
		Если ТипЗнч(Результат) = Тип("Структура") Тогда
			Если Событие = "ServerOnlineClientsList" Тогда
				ОтветСервераТаблица = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Результат.result);
				Если ТипЗнч(ОтветСервераТаблица) = Тип("Структура") Тогда
					Строки = ОтветСервераТаблица.rows;
					Если ТипЗнч(Строки) = Тип("Массив") тогда
						ЗаполнитьТаблицуАктивныхПользователей(Строки, ИдентификаторСессии);
					КонецЕсли;
				КонецЕсли;
			ИначеЕсли Событие = "ClientLeave" Тогда 
				//ПриОтключенииПользователя(Результат.sender);
			ИначеЕсли Событие = "ClientJoin" Тогда 
				//МенеджерВебСокетКлиент.ПолучитьСписокАктивныхПользователей(УникальныйИдентификатор);
			ИначеЕсли Событие = "GetMessages" Тогда 
				Сообщить("Получен список сообщений.")
			КонецЕсли;
		КонецЕсли;
	Исключение
		//@skip-check use-non-recommended-method
		Сообщить("Не верный формат сообщения!", СтатусСообщения.Важное);
	КонецПопытки;

КонецПроцедуры

&НаКлиенте
Процедура АктивныеПользователи1СЧатПриИзменении(Элемент)
	//ТекущийПолучатель
КонецПроцедуры

&НаКлиенте
Процедура ПриОшибке(Результат, ДополнительныеСвойства) Экспорт
	Сообщить("ПриОшибке Текущая форма");
КонецПроцедуры

&НаКлиенте
Процедура ПриПодключении(Результат, ДополнительныеСвойства) Экспорт
	Сообщить("ПриПодключении Текущая форма");
КонецПроцедуры

&НаКлиенте
Процедура ПриОтключении(Результат, ДополнительныеСвойства) Экспорт
	Сообщить("ПриОтключении Текущая форма");
КонецПроцедуры

&НаКлиенте
Асинх Процедура ПриУспешнойАвторизации(Результат, ДополнительныеСвойства) Экспорт
	_ИдентификаторСессии = Ждать ОбъектКомпоненты.ИдентификаторСессииАсинх();
	Если ЗначениеЗаполнено(_ИдентификаторСессии.Значение) ТОгда
		ИдентификаторСессии = _ИдентификаторСессии.Значение;
		//ToDo: не обязательная установка
		МенеджерВебСокетКлиент.УстановитьИдентификаторСессииВФорме(ЭтаФорма, Новый УникальныйИдентификатор(ИдентификаторСессии));	
	КонецЕсли;
	МенеджерВебСокетКлиент.ПолучитьСписокАктивныхПользователей(УникальныйИдентификатор);
КонецПроцедуры

&НаСервере
Процедура ЗаполнитьТаблицуАктивныхПользователей(МассивСоединений, ИдентификаторТекущейСессии)
	Элементы.СписокАктивныеПользователи.СписокВыбора.Очистить();
	Для Каждого ТекСтрока Из МассивСоединений Цикл
		Если ТекСтрока.session_uuid = ИдентификаторТекущейСессии Тогда
			Продолжить;
		КонецЕсли;
		Элементы.СписокАктивныеПользователи.СписокВыбора.Добавить(ТекСтрока, ТекСтрока.user_name);
	КонецЦикла;
КонецПроцедуры

&НаКлиенте
Асинх Функция Запущен()
	Если ОбъектКомпоненты = Неопределено Тогда
		Возврат Ложь;
	Иначе
		Результат = Ждать ОбъектКомпоненты.ЗапущенАсинх();
		Возврат Результат.Значение;
	КонецЕсли;
КонецФункции

&НаКлиенте
Асинх Процедура СписокАктивныеПользователиОбработкаВыбора(Элемент, ВыбранноеЗначение, СтандартнаяОбработка)
	Чат.Очистить();
	Если ОбъектКомпоненты <> Неопределено Тогда
		ТекущийПолучатель = ВыбранноеЗначение;
		Результат = Ждать ОбъектКомпоненты.ЗапущенАсинх();
		Если Результат.Значение Тогда
			ИдентификаторПолучателя = ВыбранноеЗначение.user_uuid;
			ОбъектКомпоненты.ПолучитьИсториюСообщенийАсинх(МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(Пользователь1СЧат), ИдентификаторПолучателя, МенеджерВебСокетВызовСервера.ИдентификаторСтрокой(УникальныйИдентификатор));	
		КонецЕсли;
	КонецЕсли;
КонецПроцедуры
