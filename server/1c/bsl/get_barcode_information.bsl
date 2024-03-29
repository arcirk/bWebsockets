	//СтруктураПараметров = Новый Структура();
	//СтруктураПараметров.Вставить("Штрихкод", "2000578341114");

	Запрос = Новый Запрос;

	ТекстЗапроса ="
	|ВЫБРАТЬ
    |	Штрихкоды.Штрихкод КАК Штрихкод,
    |	Штрихкоды.Владелец КАК Номенклатура,
    |	Штрихкоды.Владелец.Наименование КАК НоменклатураНаименование,
    |	Штрихкоды.Владелец.Артикул КАК НоменклатураАртикул,
    |	Штрихкоды.Владелец.ЕдиницаХраненияОстатков.Наименование КАК НоменклатураЕдиницаИзмерения,
    |	Штрихкоды.Владелец.Родитель КАК НоменклатураРодитель,
    |	Штрихкоды.Владелец.НаименованиеПолное КАК НоменклатураНаименованиеПолное,
    |	Штрихкоды.Владелец.ТорговаяМарка.Наименование КАК ТорговаяМарка
    |ИЗ
    |	РегистрСведений.Штрихкоды КАК Штрихкоды
    |ГДЕ
    |	Штрихкоды.Штрихкод = &Штрихкод
    |	И Штрихкоды.Владелец ССЫЛКА Справочник.Номенклатура
    |";


	Запрос.УстановитьПараметр("Штрихкод", СтруктураПараметров.Штрихкод);

	Запрос.Текст = ТекстЗапроса;
	РЗ = Запрос.Выполнить();
	Выборка = РЗ.Выбрать();

	Если НЕ Выборка.Следующий() Тогда
	    СтруктураПараметров.Вставить("result", Новый Структура);
	Иначе
		СтруктураШтрихкод = Новый Структура("
						|, _id
						|, first
						|, second
						|, ref
						|, barcode
						|, vendor_code
						|, first_name
						|, parent
						|, is_group
						|, deletion_mark
						|, version");

		СтруктураШтрихкод._id = 0;
		СтруктураШтрихкод.ref = XMLСтрока(Новый УникальныйИдентификатор);
		СтруктураШтрихкод.version = 0;
		СтруктураШтрихкод.first = "";
		СтруктураШтрихкод.second = "";
		СтруктураШтрихкод.barcode = Выборка.Штрихкод;
		СтруктураШтрихкод.parent = XMLСтрока(Выборка.Номенклатура);
		СтруктураШтрихкод.is_group = 0;
		СтруктураШтрихкод.deletion_mark = 0;
		СтруктураШтрихкод.vendor_code = СокрЛП(Выборка.НоменклатураАртикул);
		СтруктураШтрихкод.first_name = СокрЛП(Выборка.НоменклатураНаименование);

		СтруктураНоменклатура = Новый Структура("
					|, _id
					|, first
					|, second
					|, ref
					|, cache
					|, parent
					|, vendor_code
					|, trademark
					|, unit
					|, is_marked
					|, is_group
					|, deletion_mark
					|, version");
		СтруктураНоменклатура._id = 0;
		СтруктураНоменклатура.ref = XMLСтрока(Выборка.Номенклатура);
		СтруктураНоменклатура.first = Выборка.НоменклатураНаименование;
		СтруктураНоменклатура.second = Выборка.НоменклатураНаименованиеПолное;
		СтруктураНоменклатура.cache = "";
		СтруктураНоменклатура.parent = XMLСтрока(Выборка.НоменклатураРодитель);
		СтруктураНоменклатура.unit = Выборка.НоменклатураЕдиницаИзмерения;
		СтруктураНоменклатура.vendor_code = Выборка.НоменклатураАртикул;
		СтруктураНоменклатура.trademark = Выборка.ТорговаяМарка;
		СтруктураНоменклатура.is_marked = ?(Выборка.Номенклатура.ВидПродукцииИС = Перечисления.ВидыПродукцииИС.ЛегкаяПромышленность, 1, 0);
		СтруктураНоменклатура.is_group = 0;
		СтруктураНоменклатура.deletion_mark = 0;
		СтруктураНоменклатура.version = 0;

		СтруктураРезультат = Новый Структура;
		СтруктураРезультат.Вставить("barcode", СтруктураШтрихкод);
		СтруктураРезультат.Вставить("nomenclature", СтруктураНоменклатура);

		СтруктураПараметров.Вставить("result", СтруктураРезультат);
	КонецЕсли;




