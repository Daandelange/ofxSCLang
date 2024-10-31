#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // Print setup messages to console
    ofSetLogLevel(OF_LOG_NOTICE);
    scLangClient.clearLogsToConsole();

    // Interpret a buffer
    ofFile f;
    f.open(ofToDataPath("mdDemo.txarcmeta"), ofFile::ReadOnly, false);
    buff.clear();
    buff = f.readToBuffer();

    // Note: Randomly, the ofBuffer sometimes contains extra non null chars, making a longer c-string with random chars, bugging sclang.
    if(buff.size() < std::strlen(buff.getData())){
        //std::cout << "Gotcha! " << buff.size() << " vs " << std::strlen(buff.getData()) << std::endl;
        buff.getData()[buff.size()+0] = '\0';
        //std::cout << "New= " << buff.size() << " vs " << std::strlen(buff.getData()) << std::endl;
    }

    // Note: SCLang needs to output something to the console which is the only way of retrieving data.
    // Otherwise the code is executed but you can't access the result.
    // Alternatively you can add a `print` statement within your expression.
    // As a shorthand, you can ask SCLang to output the latest command to the console.
    scLangClient.interpretBuffer(buff, true); // <-- Here we ask to print the result as out file doesn't contain any print statement

    // Alternative : interpret a file
    //scLangClient.interpretFile("mdDemo.txarcmeta", true);

    // Flush to wait for results
    scLangClient.scLangClient->flush();
}

//--------------------------------------------------------------
void ofApp::update(){
    // This is only needed if you constantly interpret stuff
    scLangClient.scLangClient->flush();
}

//--------------------------------------------------------------
void ofApp::draw(){
    glm::vec2 textPos = {20,20};
    const static glm::vec2 textIndent = {0,22};

    // Buffer contents
    ofDrawBitmapStringHighlight(ofToString("Buffer contents =")+buff.getData(), textPos+glm::vec2(600,0), ofColor::grey, ofColor::white);

    if(scLangClient.isSetup()){
        ofDrawBitmapStringHighlight("scLangClient is setup", textPos); textPos+=textIndent;
        ofDrawBitmapStringHighlight(ofToString("Server name=")+scLangClient.getName(), textPos); textPos+=textIndent;

        if(scLangClient.isReady()){
            ofDrawBitmapStringHighlight("scLangClient is ready", textPos); textPos+=textIndent;

            ofDrawBitmapStringHighlight("Errors:", textPos); textPos+=textIndent;
            textPos.x += 10;
            auto eBegin = scLangClient.scLangClient->getErrors().begin();
            auto eEnd = scLangClient.scLangClient->getErrors().end();
            if(eBegin==eEnd){
                ofDrawBitmapStringHighlight("[None]", textPos); textPos+=textIndent;
            }
            else for(;eBegin!=eEnd;eBegin++){
                ofDrawBitmapStringHighlight(*eBegin, textPos); textPos+=textIndent;
            }
            textPos.x -= 10;

            ofDrawBitmapStringHighlight("Messages:", textPos); textPos+=textIndent;
            textPos.x += 10;
            auto mBegin = scLangClient.scLangClient->getMessages().begin();
            auto mEnd = scLangClient.scLangClient->getMessages().end();
            if(mBegin==mEnd){
                ofDrawBitmapStringHighlight("[None]", textPos); textPos+=textIndent;
            }
            else for(;mBegin!=mEnd;mBegin++){
                ofDrawBitmapStringHighlight(*mBegin, textPos); textPos+=textIndent;
            }
            textPos.x -= 10;

        }
        else {
            ofDrawBitmapStringHighlight("not ready. (library hasn't been compiled?)", textPos); textPos+=textIndent;
        }
    }
    else {
        ofDrawBitmapStringHighlight("scLangClient isn't created", textPos); textPos+=textIndent;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
