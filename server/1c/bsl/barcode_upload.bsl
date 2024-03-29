
        ИмяФайла = "%1%";

		Запрос = Новый Запрос("ВЫБРАТЬ
		                      |	0 КАК _id,
		                      |	"""" КАК first,
		                      |	"""" КАК second,
		                      |	"""" КАК ref,
		                      |	Штрихкоды.Штрихкод КАК barcode,
		                      |	Штрихкоды.Владелец КАК parent,
		                      |	0 КАК is_group,
		                      |	0 КАК deletion_mark,
		                      |	0 КАК version
		                      |ИЗ
		                      |	РегистрСведений.Штрихкоды КАК Штрихкоды
		                      |ГДЕ
		                      |	Штрихкоды.Владелец ССЫЛКА Справочник.Номенклатура
		                      |	И Штрихкоды.Владелец В ИЕРАРХИИ(&Товары)");

		мПоля = Новый Массив;
		мПоля.Добавить("_id");
		мПоля.Добавить("first");
		мПоля.Добавить("second");
		мПоля.Добавить("ref");
		мПоля.Добавить("barcode");
		мПоля.Добавить("parent");
		мПоля.Добавить("is_group");
		мПоля.Добавить("deletion_mark");
		мПоля.Добавить("version");

		Запрос.УстановитьПараметр("Товары", Константы.ГруппаТовары.Получить());
		Результат = Запрос.Выполнить();
		Выборка = Результат.Выбрать();
		мТовары = Новый Массив;
		Счетчик = 0;
		Количество = Выборка.Количество();

		ПараметрыJSON = Новый ПараметрыЗаписиJSON(ПереносСтрокJSON.Авто, " ", Истина);
		ПострочнаяЗаписьJSON = Новый ЗаписьJSON;
		ПострочнаяЗаписьJSON.ПроверятьСтруктуру = Ложь;
		ПострочнаяЗаписьJSON.ОткрытьФайл(ИмяФайла,,, ПараметрыJSON);
		ПострочнаяЗаписьJSON.ЗаписатьНачалоМассива();

		Пока Выборка.Следующий() Цикл
			ОбработкаПрерыванияПользователя();
			ПострочнаяЗаписьJSON.ЗаписатьНачалоОбъекта();
			Для Каждого Поле Из мПоля Цикл
				ПострочнаяЗаписьJSON.ЗаписатьИмяСвойства(Поле);
				Если ТипЗнч(Выборка[Поле]) = Тип("Строка") Тогда
					Если Поле = "ref" Тогда
						ПострочнаяЗаписьJSON.ЗаписатьЗначение(XMLСтрока(Новый УникальныйИдентификатор()));
					Иначе
						ПострочнаяЗаписьJSON.ЗаписатьЗначение(СокрЛП(Выборка[Поле]));
					КонецЕсли;
				ИначеЕсли ТипЗнч(Выборка[Поле]) = Тип("Число") Тогда
					ПострочнаяЗаписьJSON.ЗаписатьЗначение(Выборка[Поле]);
				Иначе
					ПострочнаяЗаписьJSON.ЗаписатьЗначение(XMLСтрока(Выборка[Поле]));
				КонецЕсли;
			КонецЦикла;
			ПострочнаяЗаписьJSON.ЗаписатьКонецОбъекта();
			Счетчик = Счетчик + 1;

			Если Счетчик % 100 = 0 Тогда
				Состояние("Выгружено объектов " + Счетчик + " из " + Количество);
			КонецЕсли;
		КонецЦикла;

		ПострочнаяЗаписьJSON.ЗаписатьКонецМассива();
		ПострочнаяЗаписьJSON.Закрыть();