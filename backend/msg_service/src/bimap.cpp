#include "./bimap.hpp"


#include <iostream>


namespace messenger {
    void bimap::insert(const std::string& username, const std::shared_ptr<WebSocketSession>& session_ptr) {
        this->by_usernames_.insert({username, session_ptr});
        this->by_sessions_.insert({session_ptr, username});
    }


    void bimap::erase(const std::string& username) {
        auto name_it = this->by_usernames_.find(username);
        if (name_it != nullptr) {
            auto session_it = this->by_sessions_.find(name_it->second);
            this->by_usernames_.erase(name_it);
            if (session_it != nullptr) {
                this->by_sessions_.erase(session_it);
            }
        }
    }


    void bimap::erase(const std::shared_ptr<WebSocketSession>& session_ptr) {
        auto session_it = this->by_sessions_.find(session_ptr);
        if (session_it != nullptr) {
            auto name_it = this->by_usernames_.find(session_it->second);
            this->by_sessions_.erase(session_it);
            if (name_it != nullptr) {
                this->by_usernames_.erase(name_it);
            }
        }
    }


    std::shared_ptr<WebSocketSession> bimap::get_session_ptr(const std::string& username) const {
        auto it = this->by_usernames_.find(username);
        if (it != this->by_usernames_.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }


    void bimap::print() const {
        for (const auto& [username, session_ptr] : this->by_usernames_) {
            std::cout << username << ' ' << session_ptr << '\n';
        }
    }
} // namespace messenger
