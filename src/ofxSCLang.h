#pragma once

#undef SC_QT // ensure QT is disabled
#include "SC_LanguageClient.h"

#include "ofFileUtils.h" // ofBuffer, ofToDataPath
#include "ofThreadChannel.h"
#include <string> // strlen, string
#include <vector>
#include <algorithm> // replace

// Of client to capture the output
class ofxScLangClient_impl : public SC_LanguageClient {
public:
    ofxScLangClient_impl()=delete;
    //CustomClient(const char* _name) : SC_TerminalClient(_name){
    ofxScLangClient_impl(const char* _name);

    // SC_LanguageClient implementation

    // These must be thread-safe !
    virtual void postText(const char* str, size_t len) override;
    virtual void postFlush(const char* str, size_t len) override;
    virtual void postError(const char* str, size_t len) override;

    // flush post buffer contents to screen.
    //     only called from the main language thread.
    // Note: Used to sync threads with incoming data
    virtual void flush() override;

    // Custom methods

	// Clears all logs. Useful for consuming data.
	void clearLogs();

	const std::vector<std::string>& getMessages();
	const std::vector<std::string>& getErrors();
	bool hasErrors() const;

	bool bIgnoreMessagesNow = false;
private:
    std::vector<std::string> messages;
    std::vector<std::string> errors;


    ofThreadChannel<std::string> hotMessages;
    ofThreadChannel<std::string> hotErrors;
};

class ofxScLangClient {
    public:
        ofxScLangClient_impl* scLangClient = nullptr;
        bool bEraseSetupMessages = false; // Fixme: maybe remove this setting as it might break thread safety.

        ofxScLangClient();
        ofxScLangClient(const char* title);
        ~ofxScLangClient();

        void destroyClient();

        bool isSetup() const;

        bool isCompiled() const;

        bool isReady() const;

        bool hasErrors() const;

        bool setup(const char* title="openframeworks-sclang");

        const char* getName() const;

        // Interprets a char array
        void interpretChars(const char* str, std::size_t len, bool printResult=false);

        // Shorthand
        void interpretChars(const char* str, bool printResult=false);

        // String shorthand
        void interpretChars(const std::string& str, bool printResult=false);

        // Interprets a char buffer
        void interpretBuffer(const ofBuffer& buf, bool printResult=false);

        // Reads a file and interprets it
        void interpretFile(const char* path, bool printResult=false);

        // Prints logs to console and clears them
        void clearLogsToConsole();

};
