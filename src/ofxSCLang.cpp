
#include "ofxSCLang.h"

#include <iostream> // cout
#include "ofLog.h" // ofBaseLoggerChannel
#include <algorithm> // replace

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
    static std::string str;
    while(hotErrors.tryReceive(str, 0)){
        //std::cout << "Received from thread=" << str << std::endl;
        errors.push_back(str);
    }
    while(hotMessages.tryReceive(str, 0)){
        //std::cout << "Received from thread=" << str << std::endl;
        messages.push_back(str);
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

const std::vector<std::string>& ofxScLangClient_impl::getMessages() {
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

    // Reset ignores
    // Note: Flush ensures all messages are delivered from threads
    scLangClient->flush();
    scLangClient->bIgnoreMessagesNow = false;

    // Erase log to ensure no messages appear there ?
    // #checkme : is this the expected behaviour ? (also erases errors)
    if(bEraseSetupMessages){
        scLangClient->clearLogs();
    }

    return isReady() && !hasErrors();
}

const char* ofxScLangClient::getName() const {
    if(!scLangClient) return "[None]";
    return scLangClient->getName();
}

// Interprets a char array
void ofxScLangClient::interpretChars(const char* str, std::size_t len, bool printResult){
    scLangClient->setCmdLine(str, len);
    printResult ? scLangClient->interpretPrintCmdLine() : scLangClient->interpretCmdLine();
}

// Shorthand
void ofxScLangClient::interpretChars(const char* str, bool printResult){
    interpretChars(str, std::strlen(str), printResult);
}

// String shorthand
void ofxScLangClient::interpretChars(const std::string& str, bool printResult){
    interpretChars(&str[0], str.length(), printResult);
}

// Interprets a char buffer
void ofxScLangClient::interpretBuffer(const ofBuffer& buf, bool printResult){
    const char* data = buf.getData();
    scLangClient->setCmdLine(data, std::strlen(data));
    printResult ? scLangClient->interpretPrintCmdLine() : scLangClient->interpretCmdLine();
}

// Reads a file and interprets it
void ofxScLangClient::interpretFile(const char* path, bool printResult){
    std::string fileName = ofToDataPath(path, true).c_str();
    // Prefer to use the original code
    if(!printResult){
        scLangClient->executeFile(fileName.c_str());
    }
    // We have to replicate the above function changing the output options.
    else {
        std::string escaped_file_name(fileName);
        int i = 0;
        while (i < escaped_file_name.size()) {
            if (escaped_file_name[i] == '\\')
                escaped_file_name.insert(++i, 1, '\\');
            ++i;
        }

        scLangClient->setCmdLinef("thisProcess.interpreter.executeFile(\"%s\")", escaped_file_name.c_str());
        scLangClient->runLibrary("interpretPrintCmdLine"); // this is the changed line : arg to char equivalent
    }
}

// Prints logs to console and clears them
void ofxScLangClient::clearLogsToConsole(){
    if(!scLangClient) return;

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

