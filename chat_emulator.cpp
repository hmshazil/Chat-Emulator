#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
using namespace std;

bool messageAdded = false;

string getTime()
{
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %I:%M:%S%p", localtime(&now));
    return string(buf);
}

struct User
{
    string username;
    string password;
    bool verified;
    string lastSeen;
};

User partsofline(string line)
{
    User u;
    string fields[4];
    int fieldIndex = 0;
    string temp = "";

    for (int i = 0; i < line.size(); i++)
    {
        if (line[i] == '|')
        {
            fields[fieldIndex] = temp;
            temp = "";
            fieldIndex++;
        }
        else
        {
            temp += line[i];
        }
    }
    fields[fieldIndex] = temp;

    u.username = fields[0];
    u.password = fields[1];
    u.verified = (fields[2] == "1");
    u.lastSeen = fields[3];

    return u;
}

void signUp()
{
    User u;
    cout << "Enter new username: ";
    getline(cin, u.username);

    
    if (u.username.empty())
    {
        cout << "Error: Username cannot be empty!\n";
        return;
    }


    ifstream check("users.txt");
    string line;
    while (getline(check, line))
    {
        User existing = partsofline(line);
        if (existing.username == u.username)
        {
            cout << "Error: Username already exists!\n";
            return;
        }
    }
    check.close();

    cout << "Enter new password: ";
    getline(cin, u.password);

   
    if (u.password.empty())
    {
        cout << "Error: Password cannot be empty!\n";
        return;
    }

    u.verified = false;
    u.lastSeen = getTime();

    ofstream fout("users.txt", ios::app);
    if (!fout)
    {
        cout << "Error: could not open users.txt!" << endl;
        return;
    }
    fout << u.username << "|" << u.password << "|"
         << (u.verified ? "1" : "0") << "|" << u.lastSeen << endl;

    fout.close();

    cout << "Account created successfully! Please verify your account." << endl;
}

bool login(string &loggedInUser)
{
    string uname, pass;
    cout << "Enter username: ";
    getline(cin, uname);
    cout << "Enter password: ";
    getline(cin, pass);

    ifstream fin("users.txt");
    if (!fin)
    {
        cout << "Error: users.txt not found!" << endl;
        return false;
    }



    string line;
    while (getline(fin, line))
    {
        User u = User(partsofline(line));

        if (u.username == uname && u.password == pass)
        {
            if (!u.verified)
            {
                cout << "Account not verified. Please verify first." << endl;
                return false;
            }
            loggedInUser = u.username;
            cout << "Login successful! Welcome " << u.username << endl;
            return true;
        }
    }
    cout << "Invalid username or password!" << endl;
    return false;
}

void verifyUser()
{
    string unameInput;
    cout << "Enter your username to verify: ";
    getline(cin, unameInput);

    ifstream fin("users.txt");
    ofstream fout("temp.txt");

    if (!fin || !fout)
    {
        cout << "Error opening files!" << endl;
        return;
    }

    string line;
    bool found = false;

    while (getline(fin, line))
    {
        User u = User(partsofline(line));

        if (u.username == unameInput)
        {
            found = true;
            if (u.verified)
            {
                cout << "User already verified!\n";
            }
            else
            {
                u.verified = true;
                u.lastSeen = getTime();
                cout << "Account verified successfully!" << endl;
            }
        }

        fout << u.username << "|" << u.password << "|"
             << (u.verified ? "1" : "0") << "|" << u.lastSeen << endl;
    }

    fin.close();
    fout.close();

    ifstream temp("temp.txt");
    ofstream users("users.txt");

    while (getline(temp, line))
    {
        users << line << "\n";
    }

    temp.close();
    users.close();

    if (!found)
    {
        cout << "No such user found!" << endl;
    }
}

bool authentication(string &loggedInUser)
{
    while (true)
    {
        cout << "\n========= Authentication Menu =========" << endl;
        cout << "1. Sign Up" << endl;
        cout << "2. Verify Account" << endl;
        cout << "3. Login" << endl;
        cout << "4. Exit" << endl;
        cout << "Enter your choice :(only digits) ";

        char choice;
        cin >> choice;
        cin.ignore();

        if (choice == '1')
        {
            signUp();
        }
        else if (choice == '2')
        {
            verifyUser();
        }
        else if (choice == '3')
        {
            if (login(loggedInUser))
            {
                return true;
            }
        }
        else if (choice == '4')
        {
            return false;
        }
        else
        {
            cout << "Invalid choice! Try again." << endl;
        }
    }
}


struct ChatSystem
{
    string *contacts;
    string **sender;
    string **text;
    string **msgtime;
    string **status;
    int *messagespercontact;
    int count;
    int maxContacts;
    int maxMessages;
};

ChatSystem chat;


void cleanupChat()
{

    for (int i = 0; i < chat.maxContacts; i++)
    {
        delete[] chat.sender[i];
        delete[] chat.text[i];
        delete[] chat.msgtime[i];
        delete[] chat.status[i];
    }
    delete[] chat.sender;
    delete[] chat.text;
    delete[] chat.msgtime;
    delete[] chat.status;
    delete[] chat.contacts;
    delete[] chat.messagespercontact;
}

void loadchats(const string filename, const string &loggedInUser)
{
    ifstream fin(filename);
    if (!fin)
        return;

    string currentUser = "";
    string str;
    int currentcontactindex = -1;
    int messagecounter = 0;

    while (getline(fin, str))
    {
        if (str.empty())
            continue;

        if (str.size() >= 10 && str.compare(0, 10, "#username:") == 0)
        {
            string uname = "";
            for (int k = 10; k < str.size(); k++)
                uname += str[k];
            currentUser = uname;
            continue;
        }

        if (currentUser != loggedInUser)
            continue;

        if (str[0] == '@')
        {
            string name = "";
            for (int k = 1; k < str.size(); k++)
                name += str[k];

            bool contactExists = false;
            for (int i = 0; i < chat.count; i++)
            {
                if (chat.contacts[i] == name)
                {
                    currentcontactindex = i;
                    contactExists = true;
                    break;
                }
            }

            
            if (!contactExists)
            {
                chat.contacts[chat.count] = name;
                currentcontactindex = chat.count;
                chat.messagespercontact[currentcontactindex] = 0;
                chat.count++;
            }

            messagecounter = chat.messagespercontact[currentcontactindex];
        }
        else if (currentcontactindex >= 0)
        {
            string tempstr = "";
            int fieldcounter = 0;
            int i = 0;

            chat.sender[currentcontactindex][messagecounter] = "";
            chat.msgtime[currentcontactindex][messagecounter] = "";
            chat.text[currentcontactindex][messagecounter] = "";
            chat.status[currentcontactindex][messagecounter] = "";

            while (str[i] != '\0')
            {
                if (str[i] != '|')
                    tempstr += str[i];
                else
                {
                    if (fieldcounter == 0)
                        chat.sender[currentcontactindex][messagecounter] = tempstr;
                    else if (fieldcounter == 1)
                        chat.msgtime[currentcontactindex][messagecounter] = tempstr;
                    else if (fieldcounter == 2)
                        chat.text[currentcontactindex][messagecounter] = tempstr;

                    tempstr = "";
                    fieldcounter++;
                }
                i++;
            }

            
            if (fieldcounter == 3)
                chat.status[currentcontactindex][messagecounter] = tempstr;
            else if (fieldcounter == 2)
                chat.status[currentcontactindex][messagecounter] = "Read";

            if (chat.sender[currentcontactindex][messagecounter] != "" &&
                chat.text[currentcontactindex][messagecounter] != "")
            {
                messagecounter++;
                chat.messagespercontact[currentcontactindex] = messagecounter;
            }
        }
    }

    fin.close();
}

void viewContacts()
{
    if (chat.count == 0)
    {
        cout << "No Contacts Found!" << endl;
    }
    else
    {
        for (int i = 0; i < chat.count; i++)
        {
            int unread = 0;
            for (int j = 0; j < chat.messagespercontact[i]; j++)
            {
                if (chat.status[i][j] == "Unread")
                {
                    unread++;
                }
            }

            cout << chat.contacts[i];
            if (unread > 0)
            {
                cout << " (" << unread << " unread)";
            }
            cout << endl;
        }
    }
}

void sendMessage(const string &loggedInUser)
{
    string contactname;
    bool found = false;
    cout << "Enter partner name " << endl;
    getline(cin, contactname);
    int currentcontactindex = -1, messagecounter;
    for (int i = 0; i < chat.count; i++)
    {
        if (chat.contacts[i] == contactname)
        {
            currentcontactindex = i;
            messagecounter = chat.messagespercontact[currentcontactindex];
            found = true;
            break;
        }
    }

    if (!found)
    {
        if (chat.count >= chat.maxContacts)
        {
            cout << "Cannot add more contacts! Limit reached." << endl;
            return;
        }

        chat.contacts[chat.count] = contactname;
        currentcontactindex = chat.count;
        messagecounter = 0;
        chat.count++;
        chat.messagespercontact[currentcontactindex] = 0;
    }
    messagecounter = chat.messagespercontact[currentcontactindex];

    string msg, msgreplier, sendername;
    bool exists = false;
    bool issent = false;

    sendername = loggedInUser;

    cout << "Enter your message:" << endl;
    getline(cin, msg);
    if (chat.messagespercontact[currentcontactindex] >= chat.maxMessages)
    {
        cout << "Cannot add more messages for this contact! Limit reached." << endl;
        return;
    }
    if (sendername != "" && msg != "")
    {
        chat.sender[currentcontactindex][messagecounter] = sendername;
        chat.msgtime[currentcontactindex][messagecounter] = getTime();
        chat.text[currentcontactindex][messagecounter] = msg;
        chat.status[currentcontactindex][messagecounter] = "Sent";
        chat.messagespercontact[currentcontactindex]++;
        cout << "Message Sent!" << endl;
        issent = true;
        messageAdded=true;
    }
    else
    {
        cout << " Message not sent! " << endl;
    }
    if (issent)
    {

        cout << contactname << " Enter reply (if you want) " << endl;
        getline(cin, msgreplier);
        if (contactname != "" && msgreplier != "")
        {
            messagecounter++;
            if (chat.messagespercontact[currentcontactindex] >= chat.maxMessages)
            {
                cout << "Cannot add more messages for this contact! Limit reached." << endl;
                return;
            }
            chat.sender[currentcontactindex][messagecounter] = contactname;
            chat.msgtime[currentcontactindex][messagecounter] = getTime();
            chat.text[currentcontactindex][messagecounter] = msgreplier;
            chat.status[currentcontactindex][messagecounter] = "Unread";
            chat.messagespercontact[currentcontactindex]++;
            cout << "reply Sent!" << endl;
            messageAdded=true;
        }

        else
        {
            cout << " Reply not sent! " << endl;
        }
    }
}

void viewChatHistory(const string &loggedInUser)
{
    string contactname;
    cout << "Enter contact name " << endl;
    getline(cin, contactname);
    int currentcontactindex = 0;
    bool found = false;
    for (int i = 0; i < chat.count; i++)
    {
        if (chat.contacts[i] == contactname)
        {
            currentcontactindex = i;
            found = true;
            break;
        }
    }
    if (!found)
    {
        cout << "No such contact." << endl;
        return;
    }
    int totalmessages = chat.messagespercontact[currentcontactindex];
    for (int i = 0; i < totalmessages; i++)
    {
        cout << chat.sender[currentcontactindex][i] << "  ["
             << chat.msgtime[currentcontactindex][i] << "]  : "
             << chat.text[currentcontactindex][i] << " ("
             << chat.status[currentcontactindex][i] << ")"
             << endl;
        if (chat.status[currentcontactindex][i] == "Unread")
        {
            chat.status[currentcontactindex][i] = "Read";
            messageAdded=true;
        }
    }
}

void deleteContact()
{
    string contactname;
    cout << "Enter Contact name to delete" << endl;
    getline(cin, contactname);

    int deleteindex = -1;
    for (int i = 0; i < chat.count; i++)
    {
        if (chat.contacts[i] == contactname)
        {
            deleteindex = i;
            break;
        }
    }

    if (deleteindex == -1)
    {
        cout << "Contact not found!" << endl;
        return;
    }

    string ch;
    cout << "Are you sure you want to delete contact? (y/n)||(Y/N)" << endl;
    while(true){
    getline(cin, ch);
    if(ch=="n"||ch=="N"||ch=="Y"||ch=="y"){
        break;
    }
    else{
        cout<<"Invalid selection! Choose only (y/n)||(Y/N)"<<endl;
        continue;
    }
}
    if (ch == "n" || ch == "N")
        return;

    for (int i = deleteindex; i < chat.count - 1; i++)
    {
        chat.contacts[i] = chat.contacts[i + 1];
        int oldMsgCount = chat.messagespercontact[i + 1];
        chat.messagespercontact[i] = oldMsgCount;

        for (int j = 0; j < oldMsgCount; j++)
        {
            chat.sender[i][j] = chat.sender[i + 1][j];
            chat.text[i][j] = chat.text[i + 1][j];
            chat.msgtime[i][j] = chat.msgtime[i + 1][j];
            chat.status[i][j] = chat.status[i + 1][j];
        }

       
        for (int j = oldMsgCount; j < chat.maxMessages; j++)
        {
            chat.sender[i][j] = "";
            chat.text[i][j] = "";
            chat.msgtime[i][j] = "";
            chat.status[i][j] = "";
        }
    }

    int last = chat.count - 1;
    chat.contacts[last] = "";
    chat.messagespercontact[last] = 0;
    for (int j = 0; j < chat.maxMessages; j++)
    {
        chat.sender[last][j] = "";
        chat.text[last][j] = "";
        chat.msgtime[last][j] = "";
        chat.status[last][j] = "";
    }

    chat.count--;
    messageAdded = true;
    cout << "Contact Deleted Successfully!" << endl;
}

void saveChats(const string &filename, const string &loggedInUser)
{
    if (!messageAdded)
        return;

    ifstream fin(filename);
    ofstream temp("temp.txt");

    if (!temp)
    {
        cout << "Error creating temp file!" << endl;
        return;
    }

    string line;
    bool skipBlock = false;

    while (getline(fin, line))
    {
        bool isUserLine = true;

        if (line.size() < 10)
            isUserLine = false;
        else
        {
            for (int i = 0; i < 10; i++)
            {
                if (line[i] != "#username:"[i])
                {
                    isUserLine = false;
                    break;
                }
            }
        }

        if (isUserLine)
        {
           
            string uname = "";
            for (int i = 10; i < line.size(); i++)
                uname += line[i];

            if (uname == loggedInUser)
            {
                skipBlock = true; 
                continue;
            }
            else
            {
                skipBlock = false;
            }
        }

        if (!skipBlock)
            temp << line << "\n";
    }

    fin.close();

    temp << "#username:" << loggedInUser << "\n\n";

    for (int i = 0; i < chat.count; i++)
    {
        temp << "@" << chat.contacts[i] << "\n";
        for (int j = 0; j < chat.messagespercontact[i]; j++)
        {
            temp << chat.sender[i][j] << "|"
                 << chat.msgtime[i][j] << "|"
                 << chat.text[i][j] << "|"
                 << chat.status[i][j] << "\n";
            if (j < chat.messagespercontact[i] - 1)
                temp << "\n";
        }
        if (i < chat.count - 1)
            temp << "\n\n";
    }

    temp.close();

    ifstream tempin("temp.txt");
    ofstream fout(filename);

    while (getline(tempin, line))
    {
        fout << line << "\n";
    }

    tempin.close();
    fout.close();

    messageAdded = false;
    cout << "Chats saved successfully!" << endl;
}


struct Group
{
    string id;
    string name;
    string *members;
    int memberCount;
};

bool isUserInReaders(string readers, string viewer)
{
    string tempName = "";
    for (int k = 0; k <= readers.length(); k++)
    {
        if (k == readers.length() || readers[k] == ',')
        {
            if (tempName == viewer)
            {
                return true;
            }
            tempName = "";
        }
        else
        {
            tempName += readers[k];
        }
    }
    return false;
}

bool doesGroupExist(const string &groupId)
{
    ifstream fin("groups.txt");
    if (!fin)
    {
        cout << "Could not open groups.txt" << endl;
        return false;
    }
    string line;
    while (getline(fin, line))
    {

        string currentId = "";
        for (int i = 0; i < line.length(); i++)
        {
            if (line[i] == '|')
            {
                break;
            }
            currentId += line[i];
        }
        if (currentId == groupId)
        {
            fin.close();
            return true;
        }
    }
    fin.close();
    return false;
}

bool isUserInGroup(const string &groupId, const string &username)
{
    ifstream fin("groups.txt");
    if (!fin)
    {
        cout << "Could not open groups.txt" << endl;
        return false;
    }

    string line;
    while (getline(fin, line))
    {
        string currentId = "";
        int i = 0;

        
        while (i < line.length() && line[i] != '|')
        {
            currentId += line[i];
            i++;
        }

        if (currentId != groupId)
        {
            continue;
        }

        int pipeCount = 0;
        while (i < line.length())
        {
            if (line[i] == '|')
            {
                pipeCount++;
                if (pipeCount == 2)
                {
                    i++; 
                    break;
                }
            }
            i++;
        }

        string tempName = "";
        while (i <= line.length())
        {
            if (i == line.length() || line[i] == ',')
            {
                if (tempName == username)
                {
                    fin.close();
                    return true;
                }
                tempName = "";
            }
            else
            {
                tempName += line[i];
            }
            i++;
        }
    }

    fin.close();
    return false;
}



void createGroup()
{
    Group g;
    cout << "Enter Group ID: ";
    getline(cin, g.id);

    if (g.id.empty())
    {
        cout << "Group ID cannot be empty!" << endl;
        return;
    }

    if (doesGroupExist(g.id))
    {
        cout << "Error: Group with ID '" << g.id << "' already exists. Please use a different ID." << endl;
        return;
    }

    cout << "Enter Group Name: ";
    getline(cin, g.name);

    if (g.name.empty())
    {
        cout << "Group Name cannot be empty!" << endl;
        return;
    }

    cout << "How many members to add? ";
    cin >> g.memberCount;
    cin.ignore();

    g.members = new string[g.memberCount];

    for (int i = 0; i < g.memberCount; i++)
    {
        cout << "Enter member " << i + 1 << ": ";
        getline(cin, g.members[i]);

        if (g.members[i].empty())
        {
            cout << "Member name cannot be empty! Please enter again.\n";
            i--;
            continue;
        }
    }

    ofstream fout("groups.txt", ios::app);
    if (!fout)
    {
        cout << "Error : could not open groups.txt for writing !" << endl;
        return;
    }

    fout << g.id << "|" << g.name << "|";
    for (int i = 0; i < g.memberCount; i++)
    {
        fout << g.members[i];
        if (i < g.memberCount - 1)
            fout << ",";
    }
    fout << endl;
    fout.close();

    delete[] g.members;
    cout << "Group created successfully!" << endl;
}

void listGroups()
{
    ifstream fin("groups.txt");
    if (!fin)
    {
        cout << "No groups found or failed to open groups.txt." << endl;
        return;
    }
    string line;
    cout << "\n--- All Groups ---" << endl;
    while (getline(fin, line))
    {
        cout << line << endl;
    }
    fin.close();
}

void sendGroupMessage(string groupId, string from, string text)
{

    if (!doesGroupExist(groupId))
    {
        cout << "Error: Group ID '" << groupId << "' does not exist. Please create the group first." << endl;
        return;
    }

    if (!isUserInGroup(groupId, from))
    {
        cout << "Error: User '" << from << "' is not a member of group '" << groupId << "'." << endl;
        return;
    }

    if (text.empty())
    {
        cout << "Message cannot be empty." << endl;
        return;
    }
    string filename = "group_" + groupId + ".txt";

    ofstream fout(filename, ios::app);

    if (!fout)
    {
        cout << "Failed to open " << filename << " for writing." << endl;
        return;
    }

    fout << from << "|" << getTime() << "|" << text << "|READ_BY:" << from << endl;
    fout.close();
    cout << "Message sent to group " << groupId << endl;
}

void viewGroupConversation(string groupId, string viewer)
{

    if (!doesGroupExist(groupId))
    {
        cout << "Error: Group ID '" << groupId << "' does not exist. Please create the group first." << endl;
        return;
    }


    if (!isUserInGroup(groupId, viewer)) {
    cout << "Error: User '" << viewer << "' is not a member of group '" << groupId << "'." << endl;
    return;
}


    string filename = "group_" + groupId + ".txt";
    ifstream fin(filename);
    if (!fin)
    {
        cout << "No chat history for group " << groupId << endl;
        return;
    }

    ofstream temp("temp.txt");

    if (!temp)
    {
        cout << " Error : could not reopen temp.txt " << endl;
        return;
    }
    string line;
    cout << "\n--- Group " << groupId << " Chat ---" << endl;

    while (getline(fin, line))
    {
        cout << line << endl;

        int idx = -1;
        for (int i = 0; i < line.length(); i++)
        {
            if (line[i] == 'R' && line.substr(i, 7) == "READ_BY")
            {
                idx = i + 8;
                break;
            }
        }

        if (idx != -1)
        {
            string readers = "";
            for (int j = idx; j < line.length(); j++)
            {
                readers += line[j];
            }
            if (!isUserInReaders(readers, viewer))
            {
                line += "," + viewer;
            }
        }
        temp << line << endl;
    }

    fin.close();
    temp.close();

    ifstream tempin("temp.txt");
    if (!tempin)
    {
        cout << " Error : could not reopen temp.txt " << endl;
        return;
    }

    ofstream groupChatFile(filename);
    if (!groupChatFile)
    {
        cout << " Error : could not right back to  " << filename << endl;
        return;
    }

    while (getline(tempin, line))
    {
        groupChatFile << line << "\n";
    }

    tempin.close();
    groupChatFile.close();
}

void groupUnreadCount(string groupId, string viewer)
{

    if (!isUserInGroup(groupId, viewer)) {
    cout << "Error: User '" << viewer << "' is not a member of group '" << groupId << "'." << endl;
    return;
}

    string filename = "group_" + groupId + ".txt";
    ifstream fin(filename);
    if (!fin)
    {
        cout << "No chat history for group " << groupId << endl;
        return;
    }

    string line;
    int unread = 0;
    while (getline(fin, line))
    {
        int idx = -1;
        for (int i = 0; i < line.length(); i++)
        {
            if (line[i] == 'R' && line.substr(i, 7) == "READ_BY")
            {
                idx = i + 8;
                break;
            }
        }
        if (idx != -1)
        {
            string readers = "";
            for (int j = idx; j < line.length(); j++)
            {
                readers += line[j];
            }
            if (!isUserInReaders(readers, viewer))
            {
                unread++;
            }
        }
    }
    fin.close();

    cout << viewer << " has " << unread
         << " unread messages in group " << groupId << endl;
}

int main()
{

    cout << R"(

[ System Booting... ]
===============================
||   OFFLINE CHAT EMULATOR   ||
===============================
>> Initializing modules...
>> Loading user interface...
>> Establishing secure link... -
>> Ready.

Welcome to the Offline Chat Emulator *-^
Login or create an account to continue.

)" << endl;

    string loggedInUser;

    if (!authentication(loggedInUser))
    {
        cout << "Exiting program..." << endl;
        return 0;
    }

    cout << "\nWelcome " << loggedInUser << "! You are now logged in." << endl;

    chat.maxContacts = 100;
    chat.maxMessages = 100;
    chat.count = 0;

    chat.contacts = new string[chat.maxContacts];
    chat.sender = new string *[chat.maxContacts];
    chat.text = new string *[chat.maxContacts];
    chat.msgtime = new string *[chat.maxContacts];
    chat.status = new string *[chat.maxContacts];
    chat.messagespercontact = new int[chat.maxContacts];

    for (int i = 0; i < chat.maxContacts; i++)
    {
        chat.sender[i] = new string[chat.maxMessages];
        chat.text[i] = new string[chat.maxMessages];
        chat.msgtime[i] = new string[chat.maxMessages];
        chat.status[i] = new string[chat.maxMessages];
        chat.messagespercontact[i] = 0;
    }

    char choice;
    string filename = "chats.txt";
    if (!ifstream(filename))
    {
        cout << "ERROR: File NOT FOUND!" << endl;
    }
    else
    {
        loadchats(filename, loggedInUser);

        while (true)
        {
            cout << "1. View Contacts" << endl;
            cout << "2. Send Message" << endl;
            cout << "3. View Chat History" << endl;
            cout << "4. Delete Contact" << endl;
            cout << "5. Exit" << endl;
            cout << "Enter your choice (1||2||3||4||5)" << endl;
            cin >> choice;
            cin.ignore();
            if (choice == '1')
            {
                viewContacts();
            }
            else if (choice == '2')
            {
                sendMessage(loggedInUser);
            }
            else if (choice == '3')
            {
                viewChatHistory(loggedInUser);
            }
            else if (choice == '4')
            {
                deleteContact();
            }
            else if (choice == '5')
            {
                cout << "Your thanks!" << endl;
                break;
            }
            else
            {
                cout << "Invalid Choice! Try again." << endl;
                continue;
            }
        }
        saveChats(filename, loggedInUser);
    }

    cleanupChat();

    while (true)
    {
        cout << "\n==== Group Chat Menu ====" << endl;
        cout << "1. Create Group" << endl;
        cout << "2. List Groups" << endl;
        cout << "3. Send Group Message" << endl;
        cout << "4. View Group Conversation" << endl;
        cout << "5. Group Unread Count" << endl;
        cout << "6. Exit" << endl;
        cout << "Enter choice: ";

        char choice;
        cin >> choice;
        cin.ignore();

        if (choice == '1')
        {
            createGroup();
        }
        else if (choice == '2')
        {
            listGroups();
        }
        else if (choice == '3')
        {
            string gid, from, msg;
            cout << "Enter Group ID: ";
            getline(cin, gid);
            cout << "Enter your name: ";
            getline(cin, from);
            cout << "Enter message: ";
            getline(cin, msg);
            sendGroupMessage(gid, from, msg);
        }
        else if (choice == '4')
        {
            string gid, viewer;
            cout << "Enter Group ID: ";
            getline(cin, gid);
            cout << "Enter your name: ";
            getline(cin, viewer);
            viewGroupConversation(gid, viewer);
        }
        else if (choice == '5')
        {
            string gid, viewer;
            cout << "Enter Group ID: ";
            getline(cin, gid);
            cout << "Enter your name: ";
            getline(cin, viewer);
            groupUnreadCount(gid, viewer);
        }
        else if (choice == '6')
        {
            cout << "Exiting..." << endl;
            break;
        }
        else
        {
            cout << "Invalid choice!" << endl;
        }
    }

    return 0;
}