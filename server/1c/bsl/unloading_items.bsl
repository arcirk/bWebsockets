        ИмяФайла = "%1%";
		Запрос = Новый Запрос("ВЫБРАТЬ РАЗЛИЧНЫЕ
		                      |	0 КАК _id,
		                      |	Номенклатура.Наименование КАК first,
		                      |	ВЫРАЗИТЬ(Номенклатура.НаименованиеПолное КАК СТРОКА(255)) КАК second,
		                      |	Номенклатура.Ссылка КАК ref,
		                      |	"""" КАК cache,
		                      |	Номенклатура.Родитель КАК parent,
		                      |	Номенклатура.Артикул КАК vendor_code,
		                      |	Номенклатура.ТорговаяМарка.Наименование КАК trademark,
		                      |	Номенклатура.ЕдиницаХраненияОстатков.Наименование КАК unit,
		                      |	0 КАК is_group,
		                      |	0 КАК deletion_mark,
		                      |	0 КАК version
		                      |ИЗ
		                      |	Справочник.Номенклатура КАК Номенклатура
		                      |ГДЕ
		                      |	Номенклатура.Ссылка В ИЕРАРХИИ(&Товары)
		                      |	И НЕ Номенклатура.ПометкаУдаления
		                      |	И НЕ Номенклатура.ЭтоГруппа");

		мПоля = Новый Массив;
		мПоля.Добавить("_id");
		мПоля.Добавить("first");
		мПоля.Добавить("second");
		мПоля.Добавить("ref");
		мПоля.Добавить("cache");
		мПоля.Добавить("parent");
		мПоля.Добавить("vendor_code");
		мПоля.Добавить("trademark");
		мПоля.Добавить("unit");
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
		//ПострочнаяЗаписьJSON.УстановитьСтроку(ПараметрыJSON);
		ПострочнаяЗаписьJSON.ОткрытьФайл(ИмяФайла,,, ПараметрыJSON);
		ПострочнаяЗаписьJSON.ЗаписатьНачалоМассива();

		Пока Выборка.Следующий() Цикл
			ОбработкаПрерыванияПользователя();
			ПострочнаяЗаписьJSON.ЗаписатьНачалоОбъекта();
			Для Каждого Поле Из мПоля Цикл
				ПострочнаяЗаписьJSON.ЗаписатьИмяСвойства(Поле);
				Если ТипЗнч(Выборка[Поле]) = Тип("Строка") Тогда
					ПострочнаяЗаписьJSON.ЗаписатьЗначение(СокрЛП(Выборка[Поле]));
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

