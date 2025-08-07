
#include "ofxSCLang.h"

#include <iostream> // cout
#include "ofLog.h" // ofBaseLoggerChannel
#include <algorithm> // replace
#include <regex>
#include "ofEventUtils.h" // ofNotifyEvent

#define SETUP_SUPPORTDIR_MSGID "setup-supportdir"
#define SETUP_RESSOUCREDIR_MSGID "setup-ressourcedir"

ofxScLangClient_impl::ofxScLangClient_impl(const char* _name) : SC_LanguageClient(_name){

}

// - - - - - - - - - - - - - - - - -
// SC_LanguageClient implementation

// These must be thread-safe !
void ofxScLangClient_impl::postText(const char* str, size_t len){
    if(len==0 || bIgnoreMessagesNow) return;
    if(len==1 && str[0]=='\n') return; // ignore new line only

//    auto& i = messages.emplace_back(str, len);
//    std::replace( i.begin(), i.end(), '\n', ' ');
    std::string msg = std::string(str, len);

    // Checkme: is this expected behaviour ?
    std::replace( msg.begin(), msg.end(), '\n', ' ');

    hotMessages.send(std::move(msg));
}

void ofxScLangClient_impl::postFlush(const char* str, size_t len){
    if(len==0 || bIgnoreMessagesNow) return;
    if(len==1 && str[0]=='\n') return; // ignore new line only

//    auto& i = messages.emplace_back(str, len);
//    std::replace( i.begin(), i.end(), '\n', ' ');
    std::string msg = std::string(str, len);

    // Checkme: is this expected behaviour ?
    std::replace( msg.begin(), msg.end(), '\n', ' ');

    hotMessages.send(std::move(msg));
}

void ofxScLangClient_impl::postError(const char* str, size_t len){
    if(len==0) return;

    //errors.emplace_back(str, len);
    std::string msg = std::string(str, len);

    // Checkme: is this expected behaviour ?
    std::replace( msg.begin(), msg.end(), '\n', ' ');

    hotErrors.send(std::move(msg));
}

// flush post buffer contents to screen.
//     only called from the main language thread.
// Note: Used to sync threads with incoming data
void ofxScLangClient_impl::flush(){
    static std::string receivedStr;
    while(hotErrors.tryReceive(receivedStr, 0)){
        //std::cout << "Received from thread=" << str << std::endl;
        auto& err = errors.emplace_back(receivedStr);
        ofNotifyEvent(ofxScLangClient_impl::onNewError, err);
    }
    while(hotMessages.tryReceive(receivedStr, 0)){
        //std::cout << "Received from thread=" << str << std::endl;
        // Check if message contains an ID
        // Note: ...etc... part is for retrieving truncated messages
        // Note: `, obj: ` part is missing when an error occurs --> empty message.
        static std::regex findIdRegex("^(?:->)? ?\\( ?'?name'? ?: ?(.+), ?'?u'? ?: ?([-0-9]+)(?:, ?'?obj'? ?: ?(.+))? ?(\\)|\\.\\.\\.etc\\.\\.\\.){1} ?\n?$", std::regex_constants::ECMAScript | std::regex_constants::icase);
        //receivedStr = "-> ('name': mdDemo, 'obj': ('specs': ('cutoff': [200, 16000, exp, 0, 1000], 'amp': [0, 1, db, 0, 0.5], 'freq': [50, 16000, exp, 0, 7000])))"; // tmp!
        //receivedStr = "-> ('name': mdDemo, 'u': 0, 'obj': ('specs': ('cutoff': [200, 16000, exp, 0, 1000], 'amp': [0, 1, db, 0, 0.5], 'freq': [50, 16000, exp, 0, 7000])))"; // tmp!
        auto words_begin = std::sregex_iterator(receivedStr.begin(), receivedStr.end(), findIdRegex);
        auto words_end = std::sregex_iterator();

        // None found ?
        if(words_begin == words_end){
            // Transfer message intact
            auto& msg = messages.emplace_back(receivedStr);
            ofNotifyEvent(ofxScLangClient_impl::onNewMessage, msg);
        }
        else {
            for (std::sregex_iterator i = words_begin; i != words_end; ++i){
                std::smatch match = *i;

                // Store result
                std::string message = match.size()>=3 ? match.str(3) : "";
                std::string tmp[] = {match.str(0), match.str(1), match.str(2), match.str(3), match.str(4)};
                auto& msg = messages.emplace_back(match.str(3), match.str(1), std::stoul(match.str(2)));

                // Add back missing ETC to explicitly mark it uncomplete (as received)
                if(match.str(4).compare("...etc...")==0){
                    msg.message.append("...etc...");
                    ofLogWarning("ofxScLangClient_impl::flush()") << "Warning ! SCLang returned a truncated string ! Try to make your command shorter or increase the limit in `SCClassLibrary/Common/Core/Object.sc`.";
                }

                // Transfer message
                ofNotifyEvent(ofxScLangClient_impl::onNewMessage, msg);
            }
        }
    }

    //std::cout << "flushed client!!" << std::endl;
}

// - - - - - - - -
// Custom methods

// Clears all logs. Useful for consuming data.
void ofxScLangClient_impl::clearLogs() {
	messages.clear();
	errors.clear();
}

const std::vector<ofxScLangClient_impl::messageWithId>& ofxScLangClient_impl::getMessages() {
	flush();
	return messages;
}
const std::vector<std::string>& ofxScLangClient_impl::getErrors() {
	flush();
	return errors;
}
bool ofxScLangClient_impl::hasErrors() const {
	return 0 < errors.size();
}

// - - - - - - - -
// ofxScLangClient

ofxScLangClient::ofxScLangClient(){

}

ofxScLangClient::ofxScLangClient(const char* title){
    setup(title);
}

ofxScLangClient::~ofxScLangClient(){
    // handle client destruction
    destroyClient();
}

void ofxScLangClient::destroyClient(){
    if(isSetup()){
        scLangClient->shutdownLibrary();
        scLangClient->shutdownRuntime();
        destroyLanguageClient(scLangClient);
        scLangClient = nullptr;
        //delete scLangClient;
    }
}

bool ofxScLangClient::isSetup() const {
    return scLangClient != nullptr;
}

bool ofxScLangClient::isCompiled() const {
    if(!isSetup()) return false;
    return scLangClient->isLibraryCompiled();
}

bool ofxScLangClient::isReady() const {
    return isSetup() && isCompiled();
}

bool ofxScLangClient::hasErrors() const {
    if(!isSetup()) return false;
    return scLangClient->hasErrors();
}

bool ofxScLangClient::setup(const char* title){
    // Handle ownership
    if(isSetup()){
        destroyClient();
    }

    // Create object
    scLangClient = new ofxScLangClient_impl(title);

    scLangClient->bIgnoreMessagesNow = bEraseSetupMessages;

    // Note: Changing runTimeDir does nothing... :(
    // So we HAVE to put `SCClassLibrary` like `example-simple/bin/example-simple_debug.app/Contents/MacOS/SCClassLibrary`
    // Or maybe better : Symlink `example-simple/bin/example-simple_debug.app/Contents/MacOS/SCClassLibrary` to Global SC
    SC_LanguageClient::Options opts;
    //static char scClassLib[] = {"/Applications/SuperCollider.app/Contents/Resources/SCClassLibrary"};
    //static char scClassLib[] = {"/Applications/SuperCollider.app/Contents/MacOS"};
    // todo: also add ~/Library/Application Support/SuperCollider/Extensions
    //opts.mRuntimeDir = scClassLib; // Sets dir to global SuperCollider
//    static std::string runDir = "/Applications/SuperCollider.app/Contents/MacOS";
//    opts.mRuntimeDir = runDir.data(); // Sets dir to global SuperCollider

    // Init
    scLangClient->initRuntime(opts);
    scLangClient->compileLibrary(false);

    if(!scLangClient->isLibraryCompiled()){
        ofLogWarning("ofxScLangClient::setup()") << "Couldn't compile the library ! Did you forget to copy the `SCClassLibrary` folder ?";
    }

    // Reset ignores
    // Note: Flush ensures all messages are delivered from threads
    scLangClient->flush();
    scLangClient->bIgnoreMessagesNow = false;

    // Erase log to ensure no messages appear there ?
    // #checkme : is this the expected behaviour ? (also erases errors)
    if(bEraseSetupMessages){
        scLangClient->clearLogs();
    }

    // Grab some server settings
    if( isReady() && !hasErrors()) {
        // Launch commands
        interpretChars("Platform.userAppSupportDir", true, SETUP_SUPPORTDIR_MSGID);
        interpretChars("Platform.resourceDir", true, SETUP_RESSOUCREDIR_MSGID);

        // Wait for complete
        scLangClient->flush();

        // Parse data
        for(const auto& msg : scLangClient->getMessages()){
            if(!msg.hasId()) continue;

            if(msg.id.compare(SETUP_SUPPORTDIR_MSGID)==0){
                userAppSupportDir = msg.message;
            }
            else if(msg.id.compare(SETUP_RESSOUCREDIR_MSGID)==0){
                resourceDir = msg.message;
            }
        }

        // Empty again ?
        //scLangClient->clearLogs();
    }

    return isReady() && !hasErrors();
}

const char* ofxScLangClient::getName() const {
    if(!scLangClient) return "[None]";
    return scLangClient->getName();
}

//const char* ofxScLangClient::wrapCommandWithID(const char* orig, const char* cmdID) {
//    if(cmdID==nullptr){
//        return orig;
//    }

//    // Wrap the message !

//}

// Interprets a char array
void ofxScLangClient::interpretChars(const char* str, std::size_t len, bool printResult, const char* cmdId, unsigned int universe){
    if(!isSetup()) return;
    setCmdLineWithMsgID(str, len, cmdId, universe);
    printResult ? scLangClient->interpretPrintCmdLine() : scLangClient->interpretCmdLine();
}

// Shorthand
void ofxScLangClient::interpretChars(const char* str, bool printResult, const char* cmdId, unsigned int universe){
    if(!isSetup()) return;
    interpretChars(str, std::strlen(str), printResult, cmdId, universe);
}

// String shorthand
void ofxScLangClient::interpretChars(const std::string& str, bool printResult, const char* cmdId, unsigned int universe){
    if(!isSetup()) return;
    interpretChars(&str[0], str.length(), printResult, cmdId, universe);
}

// Interprets a char buffer
void ofxScLangClient::interpretBuffer(const ofBuffer& buf, bool printResult, const char* cmdId, unsigned int universe){
    if(!isSetup()) return;
    const char* data = buf.getData();
    setCmdLineWithMsgID(data, std::strlen(data), cmdId, universe);
    printResult ? scLangClient->interpretPrintCmdLine() : scLangClient->interpretCmdLine();
}

// Reads a file and interprets it. cmdId injects an extra ID which lets you retrieve the result corresponding to a given command
void ofxScLangClient::interpretFile(const char* path, bool toDataPath, bool printResult, const char* cmdId, unsigned int universe){
    if(!isSetup()) return;

    std::string fileName = toDataPath?ofToDataPath(path, true):path;

    // Prefer to use the original code
    if(!printResult && cmdId==nullptr){
        scLangClient->executeFile(fileName.c_str());
    }
    // We have to replicate the above function changing the output options.
    else {
        // Original behaviour
        std::string escaped_file_name(fileName);
        int i = 0;
        while (i < escaped_file_name.size()) {
            if (escaped_file_name[i] == '\\')
                escaped_file_name.insert(++i, 1, '\\');
            ++i;
        }

        // Original behaviour
        if(cmdId==nullptr){
            scLangClient->setCmdLinef("thisProcess.interpreter.executeFile(\"%s\")", escaped_file_name.c_str());
        }
        // Modified behaviour
        else {
            std::string escaped_id(cmdId);
            #if false
            int i = 0;
            while (i < escaped_id.size()) {
                if (escaped_id[i] == '"')
                    escaped_id.insert(++i, 1, '\\');
                ++i;
            }
            #endif
            //scLangClient->setCmdLinef("(name:\"%s\", u: %u, obj: thisProcess.interpreter.executeFile(\"%s\"))", escaped_id.c_str(), universe, escaped_file_name.c_str());
            scLangClient->setCmdLinef("a=thisProcess.interpreter.executeFile(\"%s\");(name:\"%s\", u: %u, obj: a)", escaped_file_name.c_str(), escaped_id.c_str(), universe);

            //printf("Running cmd = (name:\"%s\", obj: thisProcess.interpreter.executeFile(\"%s\"))\n", escaped_id.c_str(), escaped_file_name.c_str());
        }
        //scLangClient->runLibrary(printResult?"interpretPrintCmdLine":"interpretCmdLine"); // this is the changed line : arg to char equivalent
        printResult ? scLangClient->interpretPrintCmdLine() : scLangClient->interpretCmdLine();
    }
}

// Prints logs to console and clears them
void ofxScLangClient::clearLogsToConsole(){
    if(!isSetup()) return;

    auto& errors = scLangClient->getErrors();
    if(errors.size()>0){
        ofLogWarning("ofxScLangClient") << "Errors ("<< errors.size() << ") :";
        auto eBegin = errors.begin();
        auto eEnd = errors.end();
        for(;eBegin!=eEnd;eBegin++){
            ofLogWarning("ofxScLangClient") << "•" << &eBegin;
        }
    }

    auto& messages = scLangClient->getMessages();
    if(messages.size()>0){
        ofLogNotice("ofxScLangClient") << "Messages ("<< messages.size() << ") :";
        auto mBegin = messages.begin();
        auto mEnd = messages.end();
        for(;mBegin!=mEnd;mBegin++){
            ofLogNotice("ofxScLangClient") << "•" << *mBegin;
        }
    }

    scLangClient->clearLogs();
}

void ofxScLangClient::setCmdLineWithMsgID(const char* cmd, std::size_t len, const char* cmdID, unsigned int universe){
    if(cmd == nullptr) return;
    if(!isSetup()) return;

    // Set without ID ?
    const char* cmdReal;
    if(cmdID==nullptr){
        scLangClient->setCmdLine(cmd, std::strlen(cmd));
        return;
    }

    // Set with ID ?
    else {
        // Set wrapped command
        // Todo: use same syntax as interpretExecuteFile to more error-friendly (ensure getting returned object).
        scLangClient->setCmdLinef("(name:\"%s\", u: %u, obj: %s)", cmdID, universe, cmd);
    }
}

// Helper to get the SuperCollider.app synthdef folder
std::string ofxScLangClient::getSCUserSynthdefLibFolder(const char* extraPath) {
    std::string ret;

#if defined(TARGET_OSX) || defined(TARGET_OF_IOS) || defined(TARGET_LINUX)
    const char* homeDir = getenv("HOME");

    #ifdef TARGET_OSX
    if(homeDir){
        ret = homeDir;
        ret += "/Library/Application Support/SuperCollider/synthdefs/";
    }
    #elif defined(TARGET_LINUX)
    assert("Linux support hasn't been implemented yet !");
    #else
    assert("iOS support hasn't been implemented yet !");
    #endif
#elif defined(TARGET_WIN32)
    // Windows & non-Unix
    // todo : implement windows env getter
    assert("Windows support hasn't been implemented yet !");
#else
    assert("Support for this target platform hasn't been implemented yet !");
#endif

    if(extraPath != nullptr) ret += extraPath;
    return ret;
}

// Helper to get the current synthdef lib folder
std::string ofxScLangClient::getUserSynthdefLibFolder(const char* extraPath) const {
    std::string ret;
    if(!isSetup()) return ret;

    ret = getUserAppSupportDir() + "/synthdefs";

    if(extraPath != nullptr) ret += extraPath;
    return ret;
}

const std::string& ofxScLangClient::getUserAppSupportDir() const {
    return userAppSupportDir;
}
const std::string& ofxScLangClient::getResourceDir() const {
    return resourceDir;
}

std::ostream& operator<< (
  std::ostream& stream,
  const ofxScLangClient_impl::messageWithId& msg
) {
    return stream << "["<< msg.universe << "/`" << (msg.hasId()?msg.id:"*") << "`]=`" << msg.message << "`";
}
