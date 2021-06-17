#include "LinkRepositoryDB.hpp"

LinkRepositoryDB::LinkRepositoryDB(const MySqlConnector& obj)
{
    this->connector = obj;
}

std::vector<LinkEntry> LinkRepositoryDB::getAll()
{
    try 
    {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;  
        
         /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", connector.getName(), connector.getPassword());
        
        /* Connect to the MySQL dbname database */
        con->setSchema(connector.getDbName());

        std::vector<LinkEntry> result;
        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM Links");
        
        while (res->next()) 
        {
            // int id = res->getInt(1); // getInt(1) returns the first column
            
            std::string url = res->getString(2).asStdString();
            size_t websiteId = res->getInt(3);
            size_t status = res->getInt(4);

            result.push_back(LinkEntry(url, websiteId, status));
        }

        delete con;
        delete stmt;
        delete res;
        
        this->source = result;
        return result;
    } 
    catch (sql::SQLException &e) 
    {
        std::cout << "# ERR: SQLException in " << __FILE__;

        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;

        return std::vector<LinkEntry>();
    }
}

std::vector<LinkEntry> LinkRepositoryDB::getBy(size_t websiteId, size_t status, size_t count) const
{
    try 
    {
        sql::Driver *driver;
        sql::Connection *con;
        sql::ResultSet *res;
        sql::PreparedStatement* pstmt;
        
         /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", connector.getName(), connector.getPassword());
        
        /* Connect to the MySQL dbname database */
        con->setSchema(connector.getDbName());

        std::vector<LinkEntry> links;
        
        pstmt = con->prepareStatement("SELECT * FROM Links WHERE websiteid=(?)");     
        pstmt->setInt(1, websiteId);
        res = pstmt->executeQuery();       
        
        while (res->next() && count > 0) 
        {
            // int id = res->getInt(1); // getInt(1) returns the first column
            
            std::string url = res->getString(2).asStdString();
            size_t findWebsiteId = res->getInt(3);
            size_t status = res->getInt(4);

            if(findWebsiteId == websiteId) 
            {
                links.push_back(LinkEntry(url, websiteId, status));
            }

            --count;
        }

        delete con;
        delete pstmt;
        delete res;

        return links;
    } 
    catch (sql::SQLException &e) 
    {
        std::cout << "# ERR: SQLException in " << __FILE__;

        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;

        return std::vector<LinkEntry>();
    }
}

std::optional<LinkEntry> LinkRepositoryDB::getByUrl(const std::string& url) const
{
    LinkEntry ans;
    try 
    {
        sql::Driver* driver;
        sql::Connection* con;
        sql::ResultSet* res;
        sql::PreparedStatement* pstmt;

         /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", connector.getName(), connector.getPassword());
        
        /* Connect to the MySQL dbname database */
        con->setSchema(connector.getDbName());
        
        pstmt = con->prepareStatement("SELECT * FROM Links WHERE url=(?)");     
        pstmt->setString(1, url);
        res = pstmt->executeQuery();       
        
        while (res->next()) 
        {
            // int id = res->getInt(1); // getInt(1) returns the first column
            
            std::string findUrl = res->getString(2).asStdString();

           
            if(url == findUrl) 
            {
                size_t websiteId = res->getInt(3);
                size_t status = res->getInt(4);

                ans = LinkEntry(url, websiteId, status);
            }
        }

        delete con;
        delete pstmt;
        delete res;
        
        return std::make_optional(ans);
    } 
    catch (sql::SQLException &e) 
    {
        std::cout << "# ERR: SQLException in " << __FILE__;

        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    return {};
}

void LinkRepositoryDB::save(LinkEntry entry)
{
    try 
    {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;
        sql::PreparedStatement* pstmt;

         /* Create a connection */
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", connector.getName(), connector.getPassword());
        
        /* Connect to the MySQL dbname database */
        con->setSchema(connector.getDbName());

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM Links");
        
        bool find = false;
        int id;
        while (res->next()) 
        {
            id = res->getInt(1); // getInt(1) returns the first column
            std::string url = res->getString(2).asStdString();
            size_t websiteId = res->getInt(3);
            
            if(entry.getUrl() == url)
            {
                find = true;
                break;
            }
        }

        if(find)
        {
            pstmt = con->prepareStatement("UPDATE Links SET `status`=(?) WHERE url=(?)");            
            pstmt->setInt(1, entry.getStatus());
            pstmt->setString(2, entry.getUrl());
            pstmt->executeQuery();
        }
        else
        {
            pstmt = con->prepareStatement("INSERT INTO Links(url, websiteid, `status`) VALUES (?, ?, ?)");
            pstmt->setString(1, entry.getUrl());
            pstmt->setInt(2, entry.getWebsiteId());
            pstmt->setInt(3, entry.getStatus());
            pstmt->executeQuery();

            std::cout << "link saved\n";
            source.push_back(entry);
        }

        delete stmt;
        delete pstmt;
        delete con;
        delete res;
    }  
    catch (sql::SQLException &e) 
    {
        std::cout << "# ERR: SQLException in " << __FILE__;

        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

int LinkRepositoryDB::getSize()
{
    return source.size();
}