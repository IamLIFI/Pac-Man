#include "GameScene.h"
#include "json/rapidjson.h"
#include "json/document.h"
#include "json/writer.h"
#include "json/stringbuffer.h"
#include <regex>
#include <string>
#include <sstream>
#include "RankScene.h"
#include "SelectScene.h"
using std::regex;
using std::match_results;
using std::regex_match;
using std::cmatch;
using namespace rapidjson;

#pragma execution_character_set("UTF-8")

USING_NS_CC;

cocos2d::Scene* GameScene::createScene() {
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = GameScene::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool GameScene::init() {
	if (!Layer::init()) {
		return false;
	}

	Size size = Director::getInstance()->getVisibleSize();
	visibleHeight = size.height;
	visibleWidth = size.width;

	score_field = TextField::create("Score", "Arial", 30);
	score_field->setPosition(Size(visibleWidth / 4, visibleHeight / 4 * 3));
	this->addChild(score_field, 2);

	submit_button = Button::create();
	submit_button->setTitleText("Submit");
	submit_button->setTitleFontSize(30);
	submit_button->setPosition(Size(visibleWidth / 4, visibleHeight / 4));
	submit_button->addClickEventListener(CC_CALLBACK_0(GameScene::submitEvent, this));
	this->addChild(submit_button, 2);

	auto quitButton = Button::create();
	quitButton->setTitleText("<");
	quitButton->setTitleFontSize(30);
	quitButton->setPosition(Size(quitButton->getSize().width, visibleHeight - quitButton->getSize().height));
	quitButton->addClickEventListener(CC_CALLBACK_0(GameScene::quitEvent, this));
	this->addChild(quitButton, 2);

	return true;
}

// �ύ�ɼ��¼�����
void GameScene::submitEvent() {
	if (score_field->getString() == "Score" || score_field->getString() == "") {
		return;
	}

	if (atoi(score_field->getString().c_str()) > Global::maxscore) {
		HttpRequest* request = new HttpRequest();
		request->setUrl((string() + "http://" + Global::ip + ":11900/rank").c_str());
		request->setRequestType(HttpRequest::Type::POST);
		request->setResponseCallback(CC_CALLBACK_2(GameScene::onSubmitHttpCompleted, this));

		// д��POST��������
		string tmp = "username=" + Global::username + "&score=" + score_field->getString();
		const char* postData = tmp.c_str();
		request->setRequestData(postData, strlen(postData));
		request->setTag("POST test");

		cocos2d::network::HttpClient::getInstance()->send(request);
		request->release();
	} else {
		CCLOG("�����߰�");
	}
}

// �ύ�ɼ���������ص�����
void GameScene::onSubmitHttpCompleted(HttpClient *sender, HttpResponse* response) {
	if (!response) {
		return;
	}
	if (!response->isSucceed()) {
		log("response failed");
		log("error buffer: %s", response->getErrorBuffer());
		return;
	}

	score_field->setPlaceHolder("Score");
	score_field->setString("");

	auto rankScene = RankScene::createScene();
	Director::getInstance()->replaceScene(rankScene);
}

// �˳�����¼�
void GameScene::quitEvent() {
	auto selectScene = SelectScene::createScene();
	Director::getInstance()->replaceScene(selectScene);
}

