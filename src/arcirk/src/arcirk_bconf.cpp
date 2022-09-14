//
// Created by Борисоглебский on 06.09.2022.
//
#include "../arcirk.hpp"

namespace arcirk{
#ifdef USE_RAPIDJSON
    bConf::bConf(const boost::filesystem::path& root, const std::string &app_name, std::vector<std::string>& aliases) {

        m_root = root;
        m_file_name = root;
        m_file_name /= app_name + "_conf.json";

        m_aliases = aliases;
        m_vec.resize(aliases.size());

    }

    bVariant & bConf::operator[] (int index) {
        return m_vec[index];
    }

    bVariant const &bConf::operator[](int index) const {
        return m_vec[index];
    }

    bVariant bConf::get(int index) const {
        return m_vec[index];
    }

    void bConf::set(int index, const bVariant &value) {
        m_vec[index] = value;
    }

    std::string bConf::to_string() const{
        arcirk::json::bJson m_doc{};
        m_doc.set_object();

        for (int i = 0; i < m_aliases.size(); ++i) {
            std::string key = m_aliases[i];
            m_doc.insert(content_value(key, m_vec[i]));
        }
        return m_doc.to_string();
    }

    void  bConf::save() {

        using namespace boost::filesystem;

        if(!exists(m_root)){
            std::cerr << "bConf::save: " << arcirk::local_8bit("Ошибка сохранения настроек. Каталог не существует!") << std::endl;
            return;
        }

        arcirk::json::bJson m_doc{};
        m_doc.set_object();

        for (int i = 0; i < m_aliases.size(); ++i) {
            std::string key = m_aliases[i];
            m_doc.insert(content_value(key, m_vec[i]));
        }

        m_doc.write(m_file_name.string());
    }

    bool bConf::parse() {

        using namespace boost::filesystem;

        if(!exists(m_file_name)){
            std::cerr << "bConf::save: " << arcirk::local_8bit("Ошибка чтения настроек. Файл не существует!") << std::endl;
            return false;
        }

        arcirk::json::bJson m_doc{};
        m_doc.read(m_file_name.string());

        if(!m_doc.is_parse()){
            std::cerr << "bConf::save: " << arcirk::local_8bit("Ошибка чтения файла!") << std::endl;
            return false;
        }
        for (int i = 0; i < m_aliases.size(); ++i) {
            std::string key = m_aliases[i];
            bVariant val = m_doc.get_member(key, "");
            m_vec[i] = val;
        }
        return true;

    }
#endif
}