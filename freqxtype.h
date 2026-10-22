#ifndef FREQXTYPE_H
#define FREQXTYPE_H


struct HiUser
{
    QString userID;
    QString userName;

    int orgID;          /// 机构ID
    QString orgName;    /// 机构名称.

    int roomID;         /// 科室/部门ID.
    QString roomName;   /// 科室/部门名称.

    int roomType;
    int deviceID;

    HiUser():
        orgID(),
        roomID(),
        roomType(),
        deviceID()
    {
    }

    HiUser(const QString& uid, const QString& uname):
        userID(uid),
        userName(uname),
        orgID(),
        roomID(),
        roomType(),
        deviceID()
    {
    }
};







#endif // FREQXTYPE_H
