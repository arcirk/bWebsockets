//
// Created by admin on 22.03.2023.
//
#include <arcirk.hpp>
#include "../include/scheduled_operations.hpp"
#include <net.hpp>
#include <beast.hpp>
#include <query_builder.hpp>
#include <database_struct.hpp>

bool scheduled_operations::field_is_exists(const nlohmann::json &object, const std::string &name) {
    auto itr = object.find(name);
    return itr != object.end();
}

void scheduled_operations::add_requests(const nlohmann::json &arr,
                                     std::vector<std::string> &transaction_arr, soci::session& sql) {
    using namespace arcirk::database;
    using json = nlohmann::json;

    std::map<std::string, tables> m_types;
    m_types.emplace("CatalogRef.Пользователи", tables::tbUsers);
    m_types.emplace("CatalogRef.Номенклатура", tables::tbNomenclature);
    m_types.emplace("CatalogRef.Организации", tables::tbOrganizations);
    m_types.emplace("CatalogRef.Подразделения", tables::tbSubdivisions);
    m_types.emplace("CatalogRef.Склады", tables::tbWarehouses);
    m_types.emplace("CatalogRef.ТипыЦенНоменклатуры", tables::tbPriceTypes);
    m_types.emplace("CatalogRef.РабочиеМеста", tables::tbWorkplaces);

    std::map<std::string, std::string> m_field_matching;
    m_field_matching.emplace("Артикул", "vendor_code");
    m_field_matching.emplace("НаименованиеПолное", "second");
    m_field_matching.emplace("Родитель", "parent");
    m_field_matching.emplace("ЭтоГруппа", "is_group");
    m_field_matching.emplace("ПометкаУдаления", "deletion_mark");
    m_field_matching.emplace("Представление", "performance");
    m_field_matching.emplace("Кэш", "cache");
    m_field_matching.emplace("Сервер", "server");
    m_field_matching.emplace("ТипУстройства", "deviceType");
    m_field_matching.emplace("Адрес", "address");
    m_field_matching.emplace("РабочееМесто", "workplace");
    m_field_matching.emplace("ТипЦен", "price_type");
    m_field_matching.emplace("Склад", "warehouse");
    m_field_matching.emplace("Подразделение", "subdivision");
    m_field_matching.emplace("Организация", "organization");
    m_field_matching.emplace("Номер", "number");
    m_field_matching.emplace("ЕдиницаХраненияОстатков", "unit");
    m_field_matching.emplace("ТорговаяМарка", "trademark");
    m_field_matching.emplace("Наименование", "first");
    m_field_matching.emplace("Ссылка", "ref");
    m_field_matching.emplace("is_marked", "is_marked");
    m_field_matching.emplace("hash", "hash");

    for (auto itr = arr.begin(); itr != arr.end() ; ++itr) {
        json object = *itr;

        auto standard_attributes = object.value("СтандартныеРеквизиты", nlohmann::json::object());
        auto attributes = object.value("Реквизиты", nlohmann::json::object());

        auto type_xml = get_string_value<std::string>(standard_attributes, "Ссылка", "Тип");
        auto struct_json = table_default_json(m_types[type_xml]);
        auto table_name = arcirk::enum_synonym(m_types[type_xml]);

        for (auto it = m_field_matching.begin();  it != m_field_matching.end() ; ++it) {
            if(it->first == "Наименование" &&m_types[type_xml] == tbUsers) {
                if (field_is_exists(standard_attributes, "Код")){
                    struct_json[it->second] = get_string_value<std::string>(standard_attributes, "Код");
                    struct_json["second"] = get_string_value<std::string>(standard_attributes, "Наименование");
                }else
                    struct_json[it->second] = get_string_value<std::string>(standard_attributes, it->first);
            }else{
                if(field_is_exists(standard_attributes, it->first) && field_is_exists(struct_json, it->second)){
                    auto val = get_string_value<std::string>(standard_attributes, it->first);
                    auto t = get_string_value<std::string>(standard_attributes, it->first, "Тип");
                    if(t == "boolean")
                        struct_json[it->second] = val == "false" ? 0 : 1;
                    else
                        struct_json[it->second] = val;
                }
                else if(field_is_exists(attributes, it->first) && field_is_exists(struct_json, it->second)){
                    auto val = get_string_value<std::string>(attributes, it->first);
                    auto t = get_string_value<std::string>(attributes, it->first, "Тип");
                    if(t == "boolean")
                        struct_json[it->second] = val == "false" ? 0 : 1;
                    else
                        struct_json[it->second] = val;
                }

            }

        }

        if(m_types[type_xml] == tbUsers){
            if(struct_json.value("role", "").empty())
                struct_json["role"] = "user";
            if(field_is_exists(struct_json, "hash")){
                auto val = struct_json["hash"].get<std::string>();
                if(val.empty()){
                    auto usr = arcirk::local_8bit(struct_json["first"].get<std::string>());
                    auto pwd = arcirk::local_8bit(struct_json["ref"].get<std::string>());
                    std::string hash = arcirk::get_hash(usr, pwd);
                    struct_json["hash"] = hash;
                }
            }
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

}

template<typename T>
T scheduled_operations::get_string_value(const nlohmann::json &object, const std::string& key, const std::string& name) const {
    if(object.is_object()){
        auto value_struct = object.value(key, nlohmann::json{});
        return value_struct.value(name, T());
    }else
        return {};
}

bool scheduled_operations::perform_data_exchange() {

    using namespace arcirk::database;
    using json = nlohmann::json;

    if(sett_.ExchangePlan.empty())
        throw native_exception(__FUNCTION__, arcirk::local_8bit("Не указан идентификатор плана обмена!").c_str());

    arcirk::log(__FUNCTION__ , "Запрос информации о регистрации данных.");
    auto result = exec_http_query("ExchangePlanGetChange", nlohmann::json{{"ExchangePlan", sett_.ExchangePlan}});
    if(!result.is_array()){
        return false;
    }
    auto sql = soci_initialize();
    std::vector<std::string> transaction_arr;
    arcirk::log(__FUNCTION__, arcirk::str_sample("Информация получена. Объектов %1%", std::to_string(result.size())));

    int max_objects = 1000;
    int count = 0;
    int size_arr = (int)result.size();

    std::vector<json> obj_info;
    obj_info.push_back(json::array());

    //сначала собираем в vector объекты по 1000 штук для  http запроса
    for (auto itr = result.begin(); itr != result.end(); ++itr) {

        nlohmann::json item = *itr;
        if(!item.is_object())
            continue;

        std::string ref = item.value("Object", "");
        std::string xml_type = item.value("Type", "");

        //arcirk::log("scheduled_operations::perform_data_exchange", arcirk::str_sample("Обработка объекта: %1%", xml_type));

        //данных по штрихкодам достаточно, поэтому продолжаем
        if(xml_type == "InformationRegisterRecord.Штрихкоды"){
            std::string obj_str = item.value("Object", "");
            if(!obj_str.empty()){
                auto obj = nlohmann::json::parse(obj_str);
                add_information_register_record<barcodes>(obj, transaction_arr, *sql, arcirk::enum_synonym(tables::tbBarcodes));
            }
            continue;
        }

        count++;

        obj_info[obj_info.size() - 1] += json{
                {"Ref", ref},
                {"Type", xml_type}
        };

        if(count > max_objects){
            //Формируем массив запросов по 1000 объектов за раз
            for (auto const&  objs : obj_info) {
                auto obj_result = exec_http_query("ExchangePlanGetObjects", objs);
                if(obj_result.is_array()){
                    add_requests(obj_result, transaction_arr, *sql);
                }
            }
            count = 1;
            obj_info.clear();
            obj_info.push_back(json::array());
        }

    }

    //Догружаем остатки
    for (auto const&  objs : obj_info) {
        auto obj_result = exec_http_query("ExchangePlanGetObjects", objs);
        if(obj_result.is_array()){
            add_requests(obj_result, transaction_arr, *sql);
        }
    }

    if(!transaction_arr.empty()){
        count = 0;
        std::vector<std::string> current_queries;
        int max = (int)transaction_arr.size();
        int step = 0;
        for (auto const& query_text : transaction_arr) {
            count++;
            current_queries.push_back(query_text);
            if(count == 10000){
                auto tr = soci::transaction(*sql);
                for ( auto const& current_text : current_queries) {
                    *sql << current_text;
                }
                step += count;
                arcirk::log(__FUNCTION__, arcirk::str_sample("commit %1% elements in %2%", std::to_string(step), std::to_string(max)));
                tr.commit();
                current_queries.clear();
                count = 0;
            }
        }

        if(!current_queries.empty()){
            auto tr = soci::transaction(*sql);
            for (auto const& current_text : current_queries) {
                *sql << current_text;
            }
            tr.commit();
        }
        arcirk::log(__FUNCTION__, "Загрузка объектов завершена.");
    }else
        arcirk::log(__FUNCTION__, "Данных для загрузки не поступило.");

    //очищаем регистрацию
    arcirk::log(__FUNCTION__, "Очистка регистрации.");
    exec_http_query("ExchangePlanEraseChange", nlohmann::json{{"ExchangePlan", sett_.ExchangePlan}});

    return true;

}

scheduled_operations::scheduled_operations(const arcirk::server::server_config &sett)
: sett_(sett)
{
    //sett_ = sett;
    sql_sess = std::make_shared<soci::session>();
}

nlohmann::json scheduled_operations::exec_http_query(const std::string& command, const nlohmann::json& param) {

    //using namespace arcirk::cryptography;

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
    const std::string pwd = sett_.HSPassword;
    std::string user_pwd = arcirk::crypt(pwd, CRYPT_KEY);
    //std::string user_pwd = crypt_utils().decrypt_string(pwd);

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

    auto res_ = res.get();

    if(res_.result() == http::status::unauthorized){
        std::string s(__FUNCTION__);
        s.append(": ");
        s.append("Ошибка авторизации на http сервере!");
        throw native_exception(s.c_str());
        //throw native_exception(__FUNCTION__ , "Ошибка авторизации на http сервере!");
    }


    std::string result_body = boost::beast::buffers_to_string(res_.body().data());
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);
    if (ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};

    if(result_body == "error"){
        std::string s(__FUNCTION__);
        s.append(": ");
        s.append("Ошибка на http сервисе!");
        throw native_exception(s.c_str());
        //throw native_exception(__FUNCTION__, "Error on http service!");
    }


    nlohmann::json result{};
    try {
        //std::cout << arcirk::local_8bit(result_body) << std::endl;
        result = nlohmann::json::parse(result_body);
    } catch (const std::exception& e) {
        arcirk::fail(__FUNCTION__, e.what());
        if(!result_body.empty())
            arcirk:: fail(__FUNCTION__, result_body);
    }

    stream.close();
    //ioc.stop();

    return result;
}

template<typename T>
void scheduled_operations::add_information_register_record(const nlohmann::json &object,
                                                           std::vector<std::string> &transaction_arr,
                                                           soci::session &sql, const std::string &table_name) {

    using namespace arcirk::database;
    auto struct_n = T();
    auto struct_json = pre::json::to_json(struct_n);

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
