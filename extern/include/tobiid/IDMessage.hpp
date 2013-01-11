/*
    Copyright (C) 2009-2011  EPFL (Ecole Polytechnique Fédérale de Lausanne)
    Michele Tavella <michele.tavella@epfl.ch>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IDMESSAGE_HPP
#define IDMESSAGE_HPP

#include "IDTypes.hpp"
#include <tobicore/TCBlock.hpp>

/*! \brief TODO
 *
 * \ingroup tobiid
 *
 */
class IDMessage : public TCBlock {
  friend class IDSerializerRapid;
  public:
    IDMessage(void);
    IDMessage(IDMessage* const other);
    IDMessage(const IDMessage& other);
    IDMessage(IDFtype familyType, IDevent event);
    virtual ~IDMessage(void);

    virtual void Copy(IDMessage* const other);
    virtual std::string GetDescription(void) const;
    virtual void SetDescription(const std::string& description);
    virtual std::string GetSource(void) const;
    virtual void SetSource(const std::string& source);
    virtual IDFvalue GetFamily(void) const;
    virtual bool SetFamilyType(const IDFtype type);
    virtual bool SetFamilyType(const std::string& type);
    virtual IDFtype GetFamilyType(void) const;
    virtual void SetEvent(const IDevent event);
    virtual IDevent GetEvent(void) const;
    virtual void SetValue( const IDvalue value );
    virtual IDvalue GetValue( ) const;
    /*! \brief Prints internal data
     */
    virtual void Dump(void) const;
    IDFtype FamilyType(IDFvalue family);
  private:
    virtual void Init(void);

  public:
    static const IDFtype FamilyUndef = -1;
    static const IDFtype FamilyBiosig = 0;
    static const IDFtype FamilyCustom = 1;

    static const IDevent EventNull = -1;

    static std::string TxtFamilyUndef;
    static std::string TxtFamilyBiosig;
    static std::string TxtFamilyCustom;
  private:
    IDFtype _familyType;
    IDevent _event;
    std::string _description;
    std::string _source;
    IDvalue _value;
};

#endif
