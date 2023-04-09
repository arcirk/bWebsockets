//
// Created by admin on 22.03.2023.
//
#include "../include/scheduled_operations.hpp"
#include <net.hpp>
#include <beast.hpp>
#include <query_builder.hpp>

bool scheduled_operations::field_is_exists(const nlohmann::json &object, const std::string &name) {
    auto itr = object.find(name);
    return itr != object.end();
}

template<typename T>
void scheduled_operations::add_query(const nlohmann::json &object,
                                     std::vector<std::string> &transaction_arr, soci::session& sql,
                                     const std::string& table_name) {
    using namespace arcirk::database;

    auto struct_n = T(); //arcirk::database::nomenclature();
    auto struct_json = pre::json::to_json(struct_n);

    nlohmann::json standard_attributes = object.value("СтандартныеРеквизиты", nlohmann::json{});
    nlohmann::json attributes = object.value("Реквизиты", nlohmann::json{});
    struct_json["ref"] = get_string_value<std::string>(standard_attributes, "Ссылка");
    if(field_is_exists(attributes, "Артикул"))
        struct_json["first"] = get_string_value<std::string>(attributes, "Артикул");
    else{
        struct_json["first"] = get_string_value<std::string>(standard_attributes, "Наименование");
    }
    if(field_is_exists(attributes, "НаименованиеПолное"))
        struct_json["second"] = get_string_value<std::string>(attributes, "НаименованиеПолное");
    else
        struct_json["second"] = get_string_value<std::string>(standard_attributes, "Наименование");

    if(field_is_exists(standard_attributes, "Родитель") && field_is_exists(struct_json, "parent"))
        struct_json["parent"] = get_string_value<std::string>(standard_attributes, "Родитель");

    if(field_is_exists(standard_attributes, "ЭтоГруппа") && field_is_exists(struct_json, "is_group")){
        auto val = get_string_value<std::string>(standard_attributes, "ЭтоГруппа");
        struct_json["is_group"] = val == "false" ? 0 : 1;
    }

    if(field_is_exists(standard_attributes, "ПометкаУдаления") && field_is_exists(struct_json, "deletion_mark")){
        auto val = get_string_value<std::string>(standard_attributes, "ПометкаУдаления");
        struct_json["deletion_mark"] = val == "false" ? 0 : 1;
    }

    if(field_is_exists(attributes, "Представление") && field_is_exists(struct_json, "performance"))
        struct_json["performance"] = get_string_value<std::string>(attributes, "Представление");

    if(field_is_exists(attributes, "Кэш") && field_is_exists(struct_json, "cache"))
        struct_json["cache"] = get_string_value<std::string>(attributes, "Кэш");

    if(field_is_exists(attributes, "Сервер") && field_is_exists(struct_json, "server"))
        struct_json["server"] = get_string_value<std::string>(attributes, "Сервер");

    if(field_is_exists(attributes, "ТипУстройства") && field_is_exists(struct_json, "deviceType"))
        struct_json["deviceType"] = get_string_value<std::string>(attributes, "ТипУстройства");

    if(field_is_exists(attributes, "Адрес") && field_is_exists(struct_json, "address"))
        struct_json["address"] = get_string_value<std::string>(attributes, "Адрес");

    if(field_is_exists(attributes, "РабочееМесто") && field_is_exists(struct_json, "workplace"))
        struct_json["workplace"] = get_string_value<std::string>(attributes, "РабочееМесто");

    if(field_is_exists(attributes, "ТипЦен") && field_is_exists(struct_json, "price_type"))
        struct_json["price_type"] = get_string_value<std::string>(attributes, "ТипЦен");

    if(field_is_exists(attributes, "Склад") && field_is_exists(struct_json, "warehouse"))
        struct_json["warehouse"] = get_string_value<std::string>(attributes, "Склад");

    if(field_is_exists(attributes, "Подразделение") && field_is_exists(struct_json, "subdivision"))
        struct_json["subdivision"] = get_string_value<std::string>(attributes, "Подразделение");

    if(field_is_exists(attributes, "Организация") && field_is_exists(struct_json, "organization"))
        struct_json["organization"] = get_string_value<std::string>(attributes, "Организация");

    if(field_is_exists(standard_attributes, "Номер") && field_is_exists(struct_json, "number"))
        struct_json["number"] = get_string_value<std::string>(standard_attributes, "Номер");

    if(struct_json["ref"].empty()){
        fail("scheduled_operations::update_nomenclature", "Ошибка в данных объекта. Обновление отменено.");
        return;
    }

    if(field_is_exists(struct_json, "hash")){
        std::string hash = arcirk::get_hash(struct_json["first"], struct_json["ref"]);
        struct_json["hash"] = hash;
    }

    soci::rowset<soci::row> rs = (sql.prepare << builder::query_builder().select(nlohmann::json{"*"}).from(table_name).where(nlohmann::json{
            {"ref", struct_json["ref"]}
    }, true).prepare());

    int count = 0;

    for (auto it = rs.begin(); it != rs.end(); it++) {
        const soci::row &row_ = *it;
        count++;
        struct_json["version"] = row_.get<int>("version") + 1;
        //вернем хеш пользователя если запись существует
        if(field_is_exists(struct_json, "hash")){
            struct_json["hash"] = row_.get<std::string>("hash");
        }
    }
    if(struct_json["version"] == 0)
        struct_json["version"] = 1;

    auto query = builder::query_builder();
    query.use(struct_json);
    if (count > 0){
        query.update(table_name, true).where(nlohmann::json{
                {"ref", struct_json["ref"]}
        }, true);
    }else
        query.insert(table_name, true);

    transaction_arr.push_back(query.prepare());

}

template<typename T>
T scheduled_operations::get_string_value(const nlohmann::json &object, const std::string& key) const {
    if(object.is_object()){
        auto value_struct = object.value(key, nlohmann::json{});
        return value_struct.value("Значение", T());
    }else
        return {};
}

bool scheduled_operations::perform_data_exchange() {

    using namespace arcirk::database;

    if(sett_.ExchangePlan.empty())
        throw native_exception("Не указан идентификатор плана обмена!");

    auto result = exec_http_query("ExchangePlanGetChange", nlohmann::json{{"ExchangePlan", sett_.ExchangePlan}});

    auto sql = soci_initialize();

    std::vector<std::string> transaction_arr;

    if(result.is_array()){

        for (auto itr = result.begin(); itr != result.end(); ++itr) {

            nlohmann::json item = *itr;
            if(item.is_object()){
                std::string ref = item.value("Object", "");
                std::string xml_type = item.value("Type", "");

                //std::cout << xml_type << std::endl;

                if(xml_type == "InformationRegisterRecord.Штрихкоды"){
                    std::string obj_str = item.value("Object", "");
                    if(!obj_str.empty()){
                        auto obj = nlohmann::json::parse(obj_str);
                        add_information_register_record<barcodes>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbBarcodes));
                    }
                    continue;
                }

                auto obj = exec_http_query("ExchangePlanGetObject", nlohmann::json{
                        {"Ref", ref},
                        {"Type", xml_type}
                });
                if(obj.is_object()){

                    if (xml_type == "CatalogRef.Номенклатура" || xml_type == "CatalogObject.Номенклатура")
                        add_query<nomenclature>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbNomenclature));
                    else if(xml_type == "CatalogRef.Организации" || xml_type == "CatalogObject.Организации")
                        add_query<organizations>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbOrganizations));
                    else if(xml_type == "CatalogRef.Пользователи" || xml_type == "CatalogObject.Пользователи")
                        add_query<user_info>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbUsers));
                    else if(xml_type == "CatalogRef.Склады" || xml_type == "CatalogObject.Склады")
                        add_query<warehouses>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbWarehouses));
                    else if(xml_type == "CatalogRef.РабочиеМеста" || xml_type == "CatalogObject.РабочиеМеста")
                        add_query<workplaces>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbWorkplaces));
                    else if(xml_type == "CatalogRef.ТипыЦенНоменклатуры" || xml_type == "CatalogObject.ТипыЦенНоменклатуры")
                        add_query<price_types>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbPriceTypes));
                    else if(xml_type == "CatalogRef.Подразделения" || xml_type == "CatalogObject.Подразделения")
                        add_query<subdivisions>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbSubdivisions));
                    else
                        continue;
                }
            }
        }

    }

    if(transaction_arr.size() > 0){
        int count = 0;
        std::vector<std::string> current_queries;
        for (auto query_text : transaction_arr) {
            //sql << query_text;
            //lenght--;
            count++;
            current_queries.push_back(query_text);
            if(count == 10000){
                auto tr = soci::transaction(*sql);
                for (const auto current_text : current_queries) {
                    *sql << current_text;
                }
                tr.commit();
                current_queries.clear();
                count = 0;
            }
        }

        if(current_queries.size() > 0){
            auto tr = soci::transaction(*sql);
            for (auto current_text : current_queries) {
                *sql << current_text;
            }
            tr.commit();
        }

    }

    //очищаем регистрацию
    exec_http_query("ExchangePlanEraseChange", nlohmann::json{{"ExchangePlan", sett_.ExchangePlan}});

    return true;

}

scheduled_operations::scheduled_operations(const arcirk::server::server_config &sett) {
    sett_ = sett;
    sql_sess = std::make_shared<soci::session>();
}

nlohmann::json scheduled_operations::exec_http_query(const std::string& command, const nlohmann::json& param) {

    auto url = arcirk::Uri::Parse(sett_.HSHost);
    auto const host = url.Host;
    auto const port = url.Port;//"80";
    auto const target = url.Path + "/info";
    int version = 10;

    net::io_context ioc;
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    auto const results = resolver.resolve(host, port);
    stream.connect(results);

    http::request<http::string_body> req{http::verb::post, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    std::string user_name = sett_.HSUser;
    std::string user_pwd = sett_.HSPassword;
//    if(!sett_.HSPassword.empty())
//        user_pwd = arcirk::crypt(sett_.HSPassword, CRYPT_KEY);

    std::string auth = user_name;
    auth.append(":");
    auth.append(user_pwd);

    req.set(http::field::authorization, "Basic " + arcirk::base64::base64_encode(auth));
    req.set(http::field::content_type, "application/json");

    nlohmann::json body{
            {"command", command},
            {"param", param}
    };

    req.body() = body.dump();
    req.prepare_payload();
    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response_parser<http::dynamic_body> res;
    res.body_limit((std::numeric_limits<std::uint64_t>::max)());
    http::read(stream, buffer, res);

    //std::cout << res << std::endl;
    //auto req_status = res.result();

    auto res_ = res.get();

    if(res_.result() == http::status::unauthorized)
        throw native_exception(arcirk::local_8bit("Ошибка авторизации на http сервисе!").c_str());

    std::string result_body = boost::beast::buffers_to_string(res_.body().data());
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};


    auto result = nlohmann::json::parse(result_body);

    return result;
}

template<typename T>
void scheduled_operations::add_information_register_record(const nlohmann::json &object,
                                                           std::vector<std::string> &transaction_arr,
                                                           soci::session &sql, const std::string &table_name) {

    using namespace arcirk::database;
    auto struct_n = T();
    auto struct_json = pre::json::to_json(struct_n);

    std::cout << arcirk::local_8bit(object.dump()) << std::endl;
    std::cout << arcirk::local_8bit(struct_json.dump()) << std::endl;

    if(field_is_exists(object, "barcode") && field_is_exists(struct_json, "barcode"))
        struct_json["barcode"] = get_string_value<std::string>(object, "barcode");
    if(field_is_exists(object, "parent") && field_is_exists(struct_json, "parent"))
        struct_json["parent"] = get_string_value<std::string>(object, "parent");

    int count = 0;

    if(table_name == arcirk::enum_synonym(tables::tbBarcodes)){
        if (std::string(struct_json["barcode"]).empty())
            return;
        soci::rowset<soci::row> rs = (sql.prepare << builder::query_builder().select(nlohmann::json{"version", "ref"}).from(table_name).where(nlohmann::json{
                {"barcode", struct_json["barcode"]}
        }, true).prepare());

        for (auto it = rs.begin(); it != rs.end(); it++) {
            const soci::row &row_ = *it;
            count++;
            struct_json["version"] = row_.get<int>(0) + 1;
            struct_json["ref"] = row_.get<std::string>(1);
        }
        if(struct_json["version"] == 0)
            struct_json["version"] = 1;

        if(count == 0)
            struct_json["ref"] = arcirk::uuids::uuid_to_string(arcirk::uuids::random_uuid());
    }

    auto query = builder::query_builder();
    query.use(struct_json);
    if (count > 0){
        query.update(table_name, true).where(nlohmann::json{
                {"ref", struct_json["ref"]}
        }, true);
    }else
        query.insert(table_name, true);

    transaction_arr.push_back(query.prepare());
}
