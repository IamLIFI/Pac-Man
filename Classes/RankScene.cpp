#include "RankScene.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include <regex>
using std::regex;
using std::match_results;
using std::regex_match;
using std::cmatch;
using namespace rapidjson;

USING_NS_CC;

cocos2d::Scene* RankScene::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = RankScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool RankScene::init() {
	if (!Layer::init()) {
		return false;
	}

	Size size = Director::getInstance()->getVisibleSize();
	visibleHeight = size.height;
	visibleWidth = size.width;

	username_field = TextField::create("", "Arial", 30);
	username_field->setPosition(Size(visibleWidth / 4 * 3, visibleHeight / 6 * 5));
	this->addChild(username_field, 2);

	score_field = TextField::create("", "Arial", 30);
	score_field->setPosition(Size(visibleWidth / 4 * 3, visibleHeight / 6 * 4));
	this->addChild(score_field, 2);

	rank_field = TextField::create("", "Arial", 30);
	rank_field->setPosition(Size(visibleWidth / 4 * 3, visibleHeight / 6 * 3));
	this->addChild(rank_field, 2);

	rank_button = Button::create();
	rank_button->setTitleText("Rank");
	rank_button->setTitleFontSize(30);
	rank_button->setPosition(Size(visibleWidth / 4, visibleHeight / 4));
	rank_button->addClickEventListener(CC_CALLBACK_0(RankScene::rankEvent, this));
	this->addChild(rank_button, 2);

	head_field = TextField::create("", "Arial", 15);
	head_field->setPosition(Size(visibleWidth / 4, visibleHeight / 4 * 3));
	head_field->setString(Global::loginHead);
	this->addChild(head_field, 2);

	body_field = TextField::create("", "Arial", 20);
	body_field->setPosition(Size(visibleWidth / 4, visibleHeight / 4 * 2));
	body_field->setString(Global::loginBody);
	this->addChild(body_field, 2);

	return true;
}

// �鿴�����¼�����
void RankScene::rankEvent() {

	HttpRequest* request = new HttpRequest();

	// ����GET���󼴿�
	request->setUrl("http://localhost:11900/rank");
	request->setRequestType(HttpRequest::Type::GET);
	request->setResponseCallback(CC_CALLBACK_2(RankScene::onRankHttpCompleted, this));
	request->setTag("GET test");

	// д��GET����ͷ����
	vector<string> headers;
	headers.push_back("Cookie: SESSION=" + Global::gameSessionId);
	request->setHeaders(headers);

	cocos2d::network::HttpClient::getInstance()->send(request);
	request->release();
}

// �鿴������������ص�����
void RankScene::onRankHttpCompleted(HttpClient *sender, HttpResponse* response) {
	if (!response) {
		return;
	}
	if (!response->isSucceed()) {
		log("response failed");
		log("error buffer: %s", response->getErrorBuffer());
		return;
	}

	std::vector<char>* headBuffer = response->getResponseHeader();
	std::vector<char>* bodyBuffer = response->getResponseData();
	head_field->setString(string(Global::toString(headBuffer)));
	body_field->setString(string(Global::toString(bodyBuffer)));
	rapidjson::Document d;
	d.Parse<0>(Global::toString(bodyBuffer).c_str());
	if (d.HasParseError()) {
		log("GetParseError " + d.GetParseError());
		return;
	}
	if (d.IsObject() && d.HasMember("info")) {
		string temp = d["result"].GetString();
		string result = "";
		for (int i = 1; i < temp.length(); i++) {
			if (temp[i] == '|') {
				result += "\n";
			} else {
				result += temp[i];
			}
		}
		rank_field->setString(result);
	}
}


