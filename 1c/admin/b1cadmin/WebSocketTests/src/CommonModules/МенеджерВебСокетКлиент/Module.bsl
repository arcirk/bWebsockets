#Область ПрограммныйИнтерфейс

Асинх Процедура ПередЗавершениемРаботыСистемы() Экспорт
	ОтключитьКлиента(Истина);
КонецПроцедуры

Асинх Процедура ПриНачалеРаботыСистемы() Экспорт
	
	Если глВебСокетКлиент = Неопределено Тогда
		глВебСокетКлиент = Новый Структура();
		глВебСокетКлиент.Вставить("НастройкиСервера", Новый Структура());
		глВебСокетКлиент.Вставить("ОбъектКомпоненты", Неопределено);
		глВебСокетКлиент.Вставить("НастройкиКлиента", МенеджерВебСокетВызовСервера.ПолучитьЛокальныеНастройкиКлиента());	
	КонецЕсли;
	
	Результат = Ждать ПодключитьКомпоненту();

	Если НЕ глВебСокетКлиент.НастройкиКлиента.ПодключатсяАвтоматически Тогда
		Возврат;
	КонецЕсли;
	
	Если Результат Тогда	
		глВебСокетКлиент.НастройкиСервера = МенеджерВебСокетВызовСервера.ПолучитьНастройкиСервера();		
		глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОПодключении", Новый ОписаниеОповещения("ОбработкаСобытияПриПодключении", ЭтотОбъект));	
		глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ОбработкаСобытияПриОшибке", ЭтотОбъект));
		глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОИзмененииСтатуса", Новый ОписаниеОповещения("ОбработкаСобытияПриИзмененииСтатуса", ЭтотОбъект));
		глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ОбработкаСобытияПриПолученииСообщения", ЭтотОбъект));
		глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОРазрывеСоединения", Новый ОписаниеОповещения("ОбработкаСобытияПриРазрывеСоединения", ЭтотОбъект));
		
		Если ПустаяСтрока(глВебСокетКлиент.НастройкиСервера.АдресСервера) ТОгда
			Возврат;
		КонецЕсли;
		
		ИмяПользователя = МенеджерВебСокетВызовСервера.ИмяТекущегоПользователя();
		ХешПользователя = МенеджерВебСокетВызовСервера.ХешТекущегоПользователя();
		Идентификатор = МенеджерВебСокетВызовСервера.ИдентификаторПользователяСтрокой();
		ИмяПриложения = "1c_websocket_client";		
		Если НЕ глВебСокетКлиент.НастройкиКлиента.ИспользоватьДанныеПользователя Тогда
			ИмяПользователя = глВебСокетКлиент.НастройкиКлиента.ИмяПользователя;
			ХешПользователя = глВебСокетКлиент.НастройкиКлиента.ХешПользователя;
		КонецЕсли;
				
		Ждать глВебСокетКлиент.ОбъектКомпоненты.УстановитьПараметрыАсинх(ИмяПользователя, ХешПользователя, Идентификатор, ИмяПриложения);
		Ждать глВебСокетКлиент.ОбъектКомпоненты.ОткрытьАсинх(глВебСокетКлиент.НастройкиСервера.АдресСервера);
		Запущен = Ждать глВебСокетКлиент.ОбъектКомпоненты.ЗапущенАсинх();	
		Если Запущен.Значение Тогда
			МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлиент::ПриНачалеРаботыСистемы", "Клиент успешно подключен к серверу.");
		КонецЕсли;	
	Иначе
		МенеджерВебСокетВызовСервера.Ошибка("МенеджерВебСокетКлиент::ПриНачалеРаботыСистемы", "Ошибка подключения компоненты.");
	КонецЕсли;
	
КонецПроцедуры

Функция ИмяМодуляКомпоненты() Экспорт
	Возврат "AddIn.WebCore.WebSocketClient";
КонецФункции

Функция ИмяКомпоненты() Экспорт
	Возврат "WebCore";
КонецФункции

Функция ПолучитьОбъектКомпоненты() Экспорт
	Попытка
		Возврат Новый (ИмяМодуляКомпоненты());
	Исключение
		Возврат Неопределено;
	КонецПопытки;
КонецФункции

Асинх Функция ПодключитьКомпоненту() Экспорт
	
	РежимОтладки = глВебСокетКлиент.НастройкиКлиента.РежимОтладки;
	Местоположение = "";
	
	_ИмяКомпоненты = НРег(ИмяКомпоненты());
	Система = Новый СистемнаяИнформация;
	Расширение = "dll";
	Если Система.ТипПлатформы = ТипПлатформы.Windows_x86_64 Тогда
		_ИмяКомпоненты = _ИмяКомпоненты + "_x64";
	ИначеЕсли Система.ТипПлатформы = ТипПлатформы.Linux_x86_64 Тогда 
		_ИмяКомпоненты = "lib" + _ИмяКомпоненты;
		Расширение = "so";
	КонецЕсли;

	_ИмяКомпоненты = _ИмяКомпоненты + "." + Расширение;
	
	Если РежимОтладки ТОгда
		Местоположение = глВебСокетКлиент.НастройкиКлиента.КаталогОтладки;
		Если СокрЛП(Местоположение) = "" Тогда
			МенеджерВебСокетВызовСервера.Ошибка("МенеджерВебСокетКлент::ПодключитьКомпоненту", "Не указан каталог отладки!");
			Возврат Ложь;
		КонецЕсли;
		Местоположение = Местоположение + ПолучитьРазделительПутиКлиента() + _ИмяКомпоненты;
	Иначе
		//Код получения файла из макета
	КонецЕсли;
	
	Файл = Новый Файл(Местоположение);
	Если НЕ Ждать Файл.СуществуетАсинх() ТОгда
		МенеджерВебСокетВызовСервера.Ошибка("МенеджерВебСокетКлент::ПодключитьКомпоненту", "Файл внешней компоненты не найден!");
		Возврат Ложь;
	КонецЕсли;
	
	Результат = Ждать ПодключитьВнешнююКомпонентуАсинх(Местоположение, ИмяКомпоненты(), ТипВнешнейКомпоненты.Native);
	
	Если НЕ Результат Тогда
		МенеджерВебСокетВызовСервера.Ошибка("МенеджерВебСокетКлент::ПодключитьКомпоненту", "Ошибка подключения вшеней компоненты!");
		Возврат Ложь;	
	КонецЕсли;
	
	глВебСокетКлиент.ОбъектКомпоненты = Неопределено;
		
	Попытка
		глВебСокетКлиент.ОбъектКомпоненты = Новый (ИмяМодуляКомпоненты());
		МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлент::ПодключитьКомпоненту", "Успешное подключение внешней компоненты!");
	Исключение
		МенеджерВебСокетВызовСервера.Ошибка("МенеджерВебСокетКлент::ПодключитьКомпоненту", ОписаниеОшибки());
		Возврат Ложь;
	КонецПопытки;

	Возврат глВебСокетКлиент.ОбъектКомпоненты <> Неопределено;
	
КонецФункции

Асинх Процедура ПодключитьКлиента(ОповещатьООшибке = Истина) Экспорт
	
	КомпонентаПодключена = Ложь;
	
	Если глВебСокетКлиент.ОбъектКомпоненты = Неопределено ТОгда
		КомпонентаПодключена = Ждать ПодключитьКомпоненту();	
	Иначе
		КомпонентаПодключена = Истина;
	КонецЕсли;
	
	Если НЕ КомпонентаПодключена Тогда
		Возврат;
	КонецЕсли;
	
	Запущен = Ждать глВебСокетКлиент.ОбъектКомпоненты.ЗапущенАсинх();
	
	Если Запущен.Значение И ОповещатьООшибке Тогда
		#Если ТонкийКлиент ИЛИ ТолстыйКлиентОбычноеПриложение Тогда
			ПредупреждениеАсинх("Клиент уже запущен!");
		#КонецЕсли		
		Возврат;
	КонецЕсли;
	
	глВебСокетКлиент.НастройкиСервера = МенеджерВебСокетВызовСервера.ПолучитьНастройкиСервера();		
	глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОПодключении", Новый ОписаниеОповещения("ОбработкаСобытияПриПодключении", ЭтотОбъект));	
	глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеООшибке", Новый ОписаниеОповещения("ОбработкаСобытияПриОшибке", ЭтотОбъект));
	глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОИзмененииСтатуса", Новый ОписаниеОповещения("ОбработкаСобытияПриИзмененииСтатуса", ЭтотОбъект));
	глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОСообщении", Новый ОписаниеОповещения("ОбработкаСобытияПриПолученииСообщения", ЭтотОбъект));
	глВебСокетКлиент.НастройкиСервера.Вставить("ОповещениеОРазрывеСоединения", Новый ОписаниеОповещения("ОбработкаСобытияПриРазрывеСоединения", ЭтотОбъект));	
		
	Если ПустаяСтрока(глВебСокетКлиент.НастройкиСервера.АдресСервера) ТОгда
		Возврат;
	КонецЕсли;
	
	ИмяПользователя = МенеджерВебСокетВызовСервера.ИмяТекущегоПользователя();
	ХешПользователя = МенеджерВебСокетВызовСервера.ХешТекущегоПользователя();
	Идентификатор = МенеджерВебСокетВызовСервера.ИдентификаторПользователяСтрокой();
	ИмяПриложения = "1c_websocket_client";		
	Если НЕ глВебСокетКлиент.НастройкиКлиента.ИспользоватьДанныеПользователя Тогда
		ИмяПользователя = глВебСокетКлиент.НастройкиКлиента.ИмяПользователя;
		ХешПользователя = глВебСокетКлиент.НастройкиКлиента.ХешПользователя;
	КонецЕсли;
			
	Ждать глВебСокетКлиент.ОбъектКомпоненты.УстановитьПараметрыАсинх(ИмяПользователя, ХешПользователя, Идентификатор, ИмяПриложения);
	Ждать глВебСокетКлиент.ОбъектКомпоненты.ОткрытьАсинх(глВебСокетКлиент.НастройкиСервера.АдресСервера);	
	
	Запущен = Ждать глВебСокетКлиент.ОбъектКомпоненты.ЗапущенАсинх();
	
	Если Запущен.Значение Тогда
		МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлиент::ПодключитьКлиента", "Клиент успешно подключен к серверу.");
	КонецЕсли;
	
КонецПроцедуры

Асинх Процедура ОтключитьКлиента(ВыходИзПрограммы = Ложь) Экспорт
	
	Если глВебСокетКлиент.ОбъектКомпоненты = Неопределено ТОгда
		Если НЕ ВыходИзПрограммы ТОгда
			#Если ТонкийКлиент ИЛИ ТолстыйКлиентОбычноеПриложение Тогда
			ПредупреждениеАсинх("Компонента не загружена!");
			#КонецЕсли
		КонецЕсли;
		Возврат;
	КонецЕсли;
	
	Результат = Ждать глВебСокетКлиент.ОбъектКомпоненты.ЗапущенАсинх();
	Если НЕ Результат.Значение Тогда
		Возврат;
	КонецЕсли;
	
	глВебСокетКлиент.ОбъектКомпоненты.ЗакрытьАсинх(ВыходИзПрограммы);
	
КонецПроцедуры

Процедура ПодключитьФорму(Форма, ОповещениеОСообщении = Неопределено, ОповещениеООшибке = Неопределено) Экспорт
		
	ИдентификаторКлиента = Неопределено;
	
	Попытка
		ИдентификаторКлиента = Форма.УникальныйИдентификатор;
	Исключение
		ИдентификаторКлиента = Новый УникальныйИдентификатор();
	КонецПопытки;
	
	МетаданныеОбъекта = Форма.Метаданные();
	
	Если НЕ ТипЗнч(глВебСокетЛокальныеКлиенты) = Тип("Соответствие") Тогда
		глВебСокетЛокальныеКлиенты = Новый Соответствие();
	КонецЕсли;
		
	//@skip-check structure-consructor-too-many-keys
	глВебСокетЛокальныеКлиенты.Вставить(ИдентификаторКлиента, Новый Структура("ОповещениеОСообщении,ОповещениеООшибке,Форма,МетаданныеОбъекта", ОповещениеОСообщении, ОповещениеООшибке, Форма, МетаданныеОбъекта));
	
КонецПроцедуры

Процедура ФормаПриЗакрытии(ИдентификаторФормы) Экспорт
	Если НЕ ТипЗнч(глВебСокетЛокальныеКлиенты) = Тип("Соответствие") Тогда
		Возврат;
	КонецЕсли;
	Если глВебСокетЛокальныеКлиенты.Получить(ИдентификаторФормы) <> Неопределено Тогда
		глВебСокетЛокальныеКлиенты.Удалить(ИдентификаторФормы);
	КонецЕсли;	
КонецПроцедуры

Функция Подключен() Экспорт
	Если глВебСокетКлиент = Неопределено ИЛИ  глВебСокетКлиент.ОбъектКомпоненты = Неопределено ТОгда 
		Возврат Ложь;
	Иначе
		Возврат глВебСокетКлиент.ОбъектКомпоненты.Запущен();
	КонецЕсли;
КонецФункции

Функция ОбработатьВнешнееСобытиие(Источник, Событие, Данные) Экспорт	
	Результат = Ложь;
	Если Источник = "WebSocketClient" Тогда
		мСтандартныеСобытия = ПолучитьСтруктуруСтандартныхСобытий();
		ПараметрыСообщения = ОбработатьОтветСервера(Событие, Данные);
		
		Если Событие = мСтандартныеСобытия.ПриПолученииСообщения Тогда
//			Если НЕ ПараметрыСообщения = Неопределено Тогда
				//Если ПараметрыСообщения.Команда = "set_client_param" И ПараметрыСообщения.Результат = "success" Тогда				
					
				//ИначеЕсли ПараметрыСообщения.Команда = "get_list_forms" Тогда 
	//				ПараметрыКоманды = МенеджерВебСокетВызовСервера.ПрочитатьОтветСервера(ПараметрыСообщения.ОтветСервера.param);
	//				Если ТипЗнч(ПараметрыКоманды) = Тип("Структура") Тогда
	//					АктивныеФормы = ПодключенныеФормы();
	//					КомандаКлиенту(мВебСокет.ИдентификаторСессии(), ПараметрыКоманды.recipient, "list_forms", арс_РаботаСJSON.СформироватьСтрокуJSON(АктивныеФормы));
	//				КонецЕсли;
				//ИначеЕсли ПараметрыСообщения.Команда = "get_document_info" Тогда
					//ОбработатьЗапросИнформацияООбъекте(ПараметрыСообщения.ОтветСервера);
//				Иначе 
//					МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлиентОбработатьВнешнееСобытиие", "Получена необработанная команда: " + ПараметрыСообщения.Команда); 
//				КонецЕсли;			
//			КонецЕсли;
			ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОСообщении, ПараметрыСообщения);
		ИначеЕсли Событие = мСтандартныеСобытия.ПриПодключении Тогда
//			 ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОПодключении, 
//			 Новый Структура("command, result, message", "set_client_param", "success", 
//			 СтрШаблон("Успешное подключение к серверу! %1 %2", глВебСокетКлиент.ОбъектКомпоненты.url, ТекущаяДата())));
			ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОПодключении, ПараметрыСообщения);	
		ИначеЕсли Событие = мСтандартныеСобытия.ПриРазрывеСоединения Тогда
//			 ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОРазрывеСоединения, 
//			 Новый Структура("command, result, message", "close", "success", 
//			 СтрШаблон("Клиент отключился от севрвера %1 %2", глВебСокетКлиент.ОбъектКомпоненты.url, ТекущаяДата())));
			 ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОРазрывеСоединения, ПараметрыСообщения);			
		ИначеЕсли Событие = мСтандартныеСобытия.ПриИзмененииСтатуса ТОгда
			ВыполнитьОбработкуОповещения(глВебСокетКлиент.НастройкиСервера.ОповещениеОИзмененииСтатуса, ПараметрыСообщения);
		ИначеЕсли Событие = мСтандартныеСобытия.ПриОшибке Тогда
						 		
		КонецЕсли;
		
		Результат = Истина;
	Иначе
		Если СтрДлина(Источник) <> 36 Тогда //длина идентификатора
			Возврат Ложь;
		КонецЕсли;
		Результат = Истина;
	КонецЕсли;
	
	Возврат Результат;
КонецФункции

Функция ОбработатьОтветСервера(Событие, Данные) Экспорт
	
	ОтветСервера = МенеджерВебСокетВызовСервера.ОбработатьОтветСервера(Событие, Данные);
	
	Если ОтветСервера.Свойство("Ошибка") Тогда
		ВыполнитьОбработкуОповещения(глВебСокетКлиент.ОповещениеООшибке, Новый Структура("command, result", "Ошибка парсинга", ОтветСервера.ОписаниеОшибки));
		Возврат Неопределено;
	КонецЕсли;

	Возврат ОтветСервера;
	
КонецФункции

Процедура ОбработкаСобытияПриОшибке(Результат, ДополнительныеПараметры) Экспорт
	Если ТипЗнч(Результат) = Тип("Структура") ТОгда
		Если Результат.result = "success" Тогда
			МенеджерВебСокетВызовСервера.Ошибка(Результат.command, Результат.message);		
		КонецЕсли;	
	КонецЕсли;
КонецПроцедуры

Процедура ОбработкаСобытияПриПодключении(Результат, ДополнительныеПараметры) Экспорт
	Если ТипЗнч(Результат) = Тип("Структура") ТОгда
		Если Результат.result = "success" Тогда
			МенеджерВебСокетВызовСервера.Лог(Результат.command, Результат.message);		
		КонецЕсли;	
	КонецЕсли;
КонецПроцедуры

Процедура ОбработкаСобытияПриИзмененииСтатуса(Результат, ДополнительныПараметры) Экспорт
	Если ТипЗнч(Результат) = Тип("Структура") Тогда
		СтатусПодключения = Неопределено;
		Результат.Свойство("result", СтатусПодключения);
		Если СтатусПодключения = Неопределено Тогда
			СтатусПодключения = Ложь;
		КонецЕсли;
		//@skip-check object-deprecated
		УстановитьКраткийЗаголовокПриложения(?(СтатусПодключения, "Подключен: " + глВебСокетКлиент.НастройкиСервера.АдресСервера, "Не подключен"));	
	КонецЕсли;	
КонецПроцедуры

Процедура ОбработкаСобытияПриРазрывеСоединения(Результат, ДополнительныеПараметры) Экспорт
	//@skip-check use-non-recommended-method
	МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлиент::ОбработкаСобытияПриРазрывеСоединения", "Подключение разорвано. " + СокрЛП(ТекущаяДата()));	
КонецПроцедуры

Процедура ОбработкаСобытияПриПолученииСообщения(Результат, ДополнительныеПараметры) Экспорт
	Если ТипЗнч(Результат) = Тип("Структура") Тогда
		МенеджерВебСокетВызовСервера.Лог("МенеджерВебСокетКлиент::ОбработкаСобытияПриПолученииСообщения", "Получено сообщение.");	
	КонецЕсли;
КонецПроцедуры


#КонецОбласти

#Область СлужебныеПроцедурыИФункции

Функция ПолучитьСтруктуруСтандартныхСобытий()
	мСтандартныеСобытия = Новый Структура();
	мСтандартныеСобытия.Вставить("ПриПолученииСообщения", "WebCore::Message");
	мСтандартныеСобытия.Вставить("ПриПодключении", "WebCore::Connect");
	мСтандартныеСобытия.Вставить("ПриРазрывеСоединения", "WebCore::Close");
	мСтандартныеСобытия.Вставить("ПриИзмененииСтатуса", "WebCore::StatusChanged");
	мСтандартныеСобытия.Вставить("ПриОшибке", "WebCore::Error");
	Возврат мСтандартныеСобытия;
КонецФункции

Процедура ПолучитьСписокАктивныхПользователей() Экспорт
	
	Если глВебСокетКлиент = Неопределено ИЛИ глВебСокетКлиент.ОбъектКомпоненты = Неопределено Тогда
		Возврат;
	КонецЕсли;
	
	глВебСокетКлиент.ОбъектКомпоненты.АктивныеПользователи();
	
КонецПроцедуры

#КонецОбласти




