#pragma once
#include "utils.h"

namespace BT {
    class BlackBoard {
    public:
        BlackBoard();
        ~BlackBoard();

        void init();

        void setStateVec3( std::string key, mkVec3 vec3 );
        mkVec3 getStateVec3( std::string key, bool& isValid );
        void setStateBool( std::string key, bool val );
        bool getStateBool( std::string key, bool& isValid );
        void setStateFloat( std::string key, float val );
        float getStateFloat( std::string key, bool& isValid );
        void setStateVariant( std::string key, aiVariant val );
        aiVariant getStateVariant( std::string key );

        bool contains(const std::string& key) const;

        std::map<std::string, aiVariant>* getPtr() { return &m_state; }
    private:
        std::map<std::string, aiVariant> m_state;
    };
}