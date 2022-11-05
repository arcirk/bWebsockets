
#Область СлужебныеПроцедурыИФункции

Функция ПустаяСсылкаСтрокой() Экспорт
	Возврат "00000000-0000-0000-0000-000000000000";	
КонецФункции

Функция ИдентификаторСтрокой(Идентификатор) Экспорт
	Возврат XMLСтрока(Идентификатор);
КонецФункции

Функция ИдентификаторИзСтроки(СтрокаИдентификатора) Экспорт
	Если СокрЛП(СтрокаИдентификатора) = "" Тогда
		Возврат Новый УникальныйИдентификатор(ПустаяСсылкаСтрокой());
	Иначе
		Возврат Новый УникальныйИдентификатор(СтрокаИдентификатора);
	КонецЕсли;
КонецФункции

Функция ПолучитьСтруктуруЛокальныхНастроек()
	Настройки = Новый Структура;
	Настройки.Вставить("РежимОтладки", Ложь); //для отладки внешней компоненты, можно использовать для явного указания местонахождения файла
	Настройки.Вставить("КаталогОтладки", ""); //каталог с бинарными файлами внешней компоененты
	Настройки.Вставить("ИспользоватьДанныеПользователя", Истина); //Будет использоваться текущее имя пользователя и идентификатор
	//в качестве данных авторизации на сервере, //sha1 пары НРЕг(ИмяПользователя) + Строка(УникальныйИдентификатор)
	Настройки.Вставить("ИмяПользователя", ""); //имя пользователя, указывается в случае если не используется данные текущего пользователя
	Настройки.Вставить("ХешПользователя", ""); //sha1 пары НРЕг(ИмяПользователя) + Пароль
	Настройки.Вставить("ПодключатсяАвтоматически", Ложь); //Автоматическое подключение при запуске
	Настройки.Вставить("ВосстановитьСоединениеПриРазрыве", Ложь); //Автоматическое подключение при запуске
	Возврат Настройки;
КонецФункции

Функция ПолучитьСтруктуруГлобальныхНастроек() Экспорт
	Настройки = Новый Структура( );
	Настройки.Вставить("АдресСервера", "ws://127.0.0.1:8080");
	Настройки.Вставить("ТребуетсяАвторизацияНаСервере", Ложь);
	Настройки.Вставить("HttpСервис", "");
	Настройки.Вставить("HttpСервисПользователь", "");
	Настройки.Вставить("HttpСервисПароль", "");
	Настройки.Вставить("РабочийКаталогСервера", "");
	Настройки.Вставить("DavСервис", "");
	Настройки.Вставить("DavСервисПользователь", "");
	Настройки.Вставить("DavСервисПароль", "");	
	Возврат Настройки;
КонецФункции

Функция ПолучитьЛокальныеНастройкиКлиента(ВыбранныйПользователь = Неопределено) Экспорт
	
	ЛокальныеНастрокйки = ПолучитьСтруктуруЛокальныхНастроек();
	
	КлючОбъекта = "WebCore";
	КлючНастроек = "Настройки";
	
	Если ВыбранныйПользователь = Неопределено Тогда
		ИмяПользователя = ИмяПользователя();
	Иначе
		ИмяПользователя = ВыбранныйПользователь.Код;
	КонецЕсли;

    _ЛокальныеНастрокйки = ХранилищеОбщихНастроек.Загрузить(КлючОбъекта,КлючНастроек,,ИмяПользователя);

	Если НЕ _ЛокальныеНастрокйки = Неопределено Тогда
		Для Каждого Элемент Из _ЛокальныеНастрокйки Цикл
			Если ЛокальныеНастрокйки.Свойство(Элемент.Ключ) Тогда
				ЛокальныеНастрокйки[Элемент.Ключ] = Элемент.Значение;
			КонецЕсли;		
		КонецЦикла;
	КонецЕсли;

	Возврат ЛокальныеНастрокйки;
	
КонецФункции

Функция ПолучитьНастройкиСервера() Экспорт
	
	мНастройки = ПолучитьСтруктуруГлобальныхНастроек();
	мНастройки.АдресСервера = Константы.АдресСервера.Получить();
	мНастройки.ТребуетсяАвторизацияНаСервере = Константы.ТребуетсяАвторизацияНаСервере.Получить();
	мНастройки.РабочийКаталогСервера = Константы.РабочийКаталогСервера.Получить();
	Возврат мНастройки;
	
КонецФункции

Функция ХешТекущегоПользователя() Экспорт
	
	ТекущийПользователь = Пользователи.ТекущийПользователь();
	ИмяПользователя = "";
	ИдентификаторПользователя = "";
	Если ЗначениеЗаполнено(ТекущийПользователь) ТОгда
		ИмяПользователя = ТекущийПользователь.Код;
		ИдентификаторПользователя = XMLСтрока(ТекущийПользователь);
	Иначе
		ПользовательИБ = ПользователиИнформационнойБазы.НайтиПоИмени(ИмяПользователя());
		ИмяПользователя = ИмяПользователя();
		ИдентификаторПользователя = XMLСтрока(ПользовательИБ.УникальныйИдентификатор);
	КонецЕсли;
	
	ИсходнаяСтрока = ВРег(ИмяПользователя) + ИдентификаторПользователя;
	ХешПользователя = НРЕг(ПолучитьХешСуммы(ИсходнаяСтрока));
	
	Возврат ХешПользователя;
	
КонецФункции

Функция ИдентификаторПользователяСтрокой() Экспорт
	ТекущийПользователь = Пользователи.ТекущийПользователь();
	ИдентификаторПользователя = XMLСтрока(ТекущийПользователь);
	Возврат ИдентификаторПользователя;
КонецФункции

Функция ИмяТекущегоПользователя() Экспорт
	Если ЗначениеЗаполнено(Пользователи.ТекущийПользователь()) Тогда
		Возврат СокрЛП(Пользователи.ТекущийПользователь().Наименование);
	Иначе
		Возврат ИмяПользователя();
	КонецЕсли;
КонецФункции

Функция ПолучитьХешСуммы(ИсходнаяСтрока)
	ХешированиеДанных  = Новый ХешированиеДанных (ХешФункция.SHA1) ;
 	ХешированиеДанных.Добавить(ИсходнаяСтрока);
 	Возврат ПолучитьHexСтрокуИзДвоичныхДанных(ХешированиеДанных.ХешСумма)
КонецФункции

Процедура Ошибка(ЗаголовокОшибки, ОписаниеОшибки) Экспорт
	ЗаписьЖурналаРегистрации("WebSocketClient", УровеньЖурналаРегистрации.Ошибка, , , ЗаголовокОшибки + ": " + ОписаниеОшибки)
КонецПроцедуры

Процедура Лог(ЗаголовокСообщения, ТекстСообщения) Экспорт
	ЗаписьЖурналаРегистрации("WebSocketClient", УровеньЖурналаРегистрации.Информация, , , ЗаголовокСообщения + ": " + ТекстСообщения)
КонецПроцедуры

Функция ОбработатьОтветСервера(Данные) Экспорт

	ОтветСервера_ = ПолучитьСтрокуИзДвоичныхДанных(Base64Значение(Данные));
	Если ОтветСервера_ = "" И СокрЛП(Данные) <> "" Тогда
		ОтветСервера_ = Данные;
	КонецЕсли;
		
	ОтветСервера = ПрочитатьОтветСервера(ОтветСервера_);
	Возврат ОтветСервера;

КонецФункции

Функция ПрочитатьОтветСервера(ОтветСервера) Экспорт
	СтруктураДанных = Неопределено;
	Попытка
		ЧтениеJSON = Новый ЧтениеJSON;
		ЧтениеJSON.УстановитьСтроку(ОтветСервера);
		СтруктураДанных = ПрочитатьJSON(ЧтениеJSON,,,);		
	Исключение
		СтруктураДанных = Новый Структура("Ошибка,ОписаниеОшибки", "МенеджерВебСокетВызовСервера::ПрочитатьОтветСервера", ОписаниеОшибки());
	КонецПопытки;

	Возврат СтруктураДанных;
КонецФункции

Функция ОтветСервераПолучитьЗначение(ОтветСервера, Свойство, ЗначениеПоУмолчанию = Неопределено)  Экспорт
	Если ТипЗнч(ОтветСервера) = Тип("Структура") Или ТипЗнч(ОтветСервера) = Тип("Соответствие") Тогда
		Если ТипЗнч(ОтветСервера) = Тип("Структура") Тогда
			Если НЕ ОтветСервера.Свойство(Свойство) Тогда
				Возврат ЗначениеПоУмолчанию;
			Иначе
				Возврат ОтветСервера[Свойство];
			КонецЕсли;
		Иначе 
			Возврат ?( ОтветСервера[Свойство], ЗначениеПоУмолчанию, ОтветСервера[Свойство]);
		КонецЕсли;
	Иначе
		Возврат ЗначениеПоУмолчанию;
	КонецЕсли;
КонецФункции

#КонецОбласти



