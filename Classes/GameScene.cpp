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
#include "CCCircle.h"
#include "EndScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;
using std::regex;
using std::match_results;
using std::regex_match;
using std::cmatch;
using namespace rapidjson;

#pragma execution_character_set("UTF-8")
#define WALLTHINKNESS 37	// ǽ�ĺ�� 37

USING_NS_CC;

#define database UserDefault::getInstance()

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

	visibleSize = Director::getInstance()->getVisibleSize();
	origin = Director::getInstance()->getVisibleOrigin();

	// ��Ϸ�˳���
	auto quit = MenuItemLabel::create(Label::createWithTTF("Quit", "fonts/Marker Felt.ttf", 30), CC_CALLBACK_1(GameScene::quitEvent, this));
	quit->setColor(Color3B(255, 255, 255));	// �����ɫ
	quit->setPosition(Size(origin.x + 30, origin.y + visibleSize.height - 20));
	auto menu = Menu::create(quit, NULL);
	menu->setPosition(Vec2::ZERO);
	this->addChild(menu, 2);

	// ��Ϸ�Ƿְ壨��Ϸ�տ�ʼ�÷�Ϊ0��
	Global::score = 0;
	score_field = Label::create("score: 0", "fonts/Marker Felt.TTF", 30);
	score_field->setPosition(Vec2(quit->getPositionX() + 10, quit->getPositionY() - 30));
	this->addChild(score_field, 2);

	isMove = false;
	isRewarded = false;
	preloadMusic();
	playBgm();
	addMap();
	addBeans();
	addPlayer();
	addEnemy();
	addKeyboardListener();
	schedule(schedule_selector(GameScene::update), 0.04f, kRepeatForever, 0);
	enemyMove();
	return true;
}

void GameScene::preloadMusic() {
	SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("music/bg3.wav");
	SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("music/win.wav");
	SimpleAudioEngine::sharedEngine()->preloadBackgroundMusic("music/gameover.mp3");
}

void GameScene::playBgm() {
	SimpleAudioEngine::sharedEngine()->playBackgroundMusic("music/bg3.wav", true);
}

// ���ص�ͼ
void GameScene::addMap() {

	auto label = Label::createWithTTF("Pac Man", "fonts/Marker Felt.TTF", 40);
	label->setPosition(Vec2(origin.x + visibleSize.width / 2,
		origin.y + visibleSize.height - label->getContentSize().height));
	this->addChild(label, 1);

	map = TMXTiledMap::create("wallMap1.tmx");
	map->setPosition(visibleSize.width / 2 + origin.x, visibleSize.height / 2 + origin.y);
	map->setAnchorPoint(Vec2(0.5, 0.5));
	map->setScale(640 / map->getContentSize().width);
	this->addChild(map, 0);
	width = map->getContentSize().width;
	height = map->getContentSize().height;
	map_x = map->getPosition().x - width / 2;
	map_y = map->getPosition().y - height / 2;
	wall = map->getObjectGroup("wall");
}

// ���붹��
void GameScene::addBeans() {
	// ���С����
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			auto bean = Sprite::create("sprites/bean1.png");
			bean->setPosition(Vec2(map_x + width / 10 * i + 70, map_y / 2 + height / 10 * j + 70));
			beans.push_back(bean);
			this->addChild(bean, 1);
		}
	}
	// ����ĸ����ϵĴ���
	// �ĸ����ӵ�����
	Vec2 bigBeanLocations[4] = { 
		Vec2(map_x + width / 10 * 1 + 70, map_y + height / 10 * 1 + 70),
		Vec2(map_x + width / 10 * 1 + 70, map_y +height / 10 * 7 + 70),
		Vec2(map_x + width / 10 * 7 + 70, map_y + height / 10 * 1 + 70),
		Vec2(map_x + width / 10 * 7 + 70, map_y + height / 10 * 7 + 70) };
	// ���
	for (int i = 0; i < 4; i++) {
		bigBeans[i] = addBigBeans(bigBeanLocations[i]);
	}
}

// ��Ӵ���
Sprite* GameScene::addBigBeans(Vec2 locaion) {
	auto bigBean = Sprite::create("sprites/bean2.png");
	bigBean->setPosition(locaion);
	this->addChild(bigBean, 1);
	return bigBean;
}

// �������
void GameScene::addPlayer() {
	TMXObjectGroup* objectGroup = map->getObjectGroup("elements");
	ValueVector objects = objectGroup->getObjects();
	player = Sprite::create("sprites/player1.png");
	player->setPosition(Vec2(beans[27]->getPosition().x, beans[3]->getPosition().y));
	this->addChild(player, 1);
	// ���붯��
	auto playerAnimation = Animation::create();
	for (int i = 1; i < 3; i++) {
		char nameSize[100] = { 0 };
		sprintf(nameSize, "sprites/player%d.png", i);
		playerAnimation->addSpriteFrameWithFile(nameSize);
	}
	playerAnimation->setDelayPerUnit(0.1f);
	playerAnimation->setLoops(-1);
	auto playerAnimate = Animate::create(playerAnimation);
	player->runAction(playerAnimate);
}

// �������
void GameScene::addEnemy() {
	Vec2 enermyLocation[4] = { Vec2(480, 320), Vec2(500, 100), Vec2(200, 300), Vec2(300, 400) };
	enemys[ENERMYCOLOR::BLUE] = addEnemyByColor(ENERMYCOLOR::BLUE, enermyLocation[ENERMYCOLOR::BLUE]);
	enemys[ENERMYCOLOR::RED] = addEnemyByColor(ENERMYCOLOR::RED, enermyLocation[ENERMYCOLOR::RED]);
	enemys[ENERMYCOLOR::ORANGE] = addEnemyByColor(ENERMYCOLOR::ORANGE, enermyLocation[ENERMYCOLOR::ORANGE]);
	enemys[ENERMYCOLOR::PINK] = addEnemyByColor(ENERMYCOLOR::PINK, enermyLocation[ENERMYCOLOR::PINK]);
}

// ���ݹ�����ɫ��������
Sprite *GameScene::addEnemyByColor(int color, Vec2 location) {
	auto enemy = Sprite::create("sprites/" + enermy_color[color] + "1.png");
	enemy->setPosition(location);
	this->addChild(enemy, 1);

	auto animation = Animation::create();
	for (int i = 1; i < 5; i++) {
		char nameSize[100] = { 0 };
		sprintf(nameSize, ("sprites/" + enermy_color[color] + "%d.png").data(), i);
		animation->addSpriteFrameWithFile(nameSize);
	}
	animation->setDelayPerUnit(0.1f);
	animation->setLoops(-1);
	auto blueAnimate = Animate::create(animation);
	enemy->runAction(blueAnimate);
	return enemy;
}

// ���̿�������ƶ�
void GameScene::movePlayer() {
	switch (move)
	{
	case GameScene::UP:
		player->runAction(MoveBy::create(0.04f, Vec2(0, 10)));
		break;
	case GameScene::DOWN:
		player->runAction(MoveBy::create(0.04f, Vec2(0, -10)));
		break;
	case GameScene::LEFT:
		player->runAction(MoveBy::create(0.04f, Vec2(-10, 0)));
		break;
	case GameScene::RIGHT:
		player->runAction(MoveBy::create(0.04f, Vec2(10, 0)));
		break;
	default:
		break;
	}
}

// ��ͬ��ɫ�Ĺ����ƶ�λ�ò�ͬ
void GameScene::enemyMove() {
	schedule(schedule_selector(GameScene::pinkEnemyMove), 1.5f);
	schedule(schedule_selector(GameScene::blueEnemyMove), 0.5f);
	schedule(schedule_selector(GameScene::orangeEnemyMove), 0.02f);
	schedule(schedule_selector(GameScene::redEnemyMove), 0.5f);
}

// pink ��������ƶ��������
void GameScene::pinkEnemyMove(float time) {
	if (enemys[ENERMYCOLOR::PINK] == NULL || isRewarded)
		return;
	// ��ǰλ�õ�x��y
	int pink_x = enemys[ENERMYCOLOR::PINK]->getPositionX(), pink_y = enemys[ENERMYCOLOR::PINK]->getPositionY();
	// ��ù��������֮��ľ���x��y
	int distanceX = int(player->getPositionX() - pink_x);
	int distanceY = int(player->getPositionY() - pink_y);
	int moveX = 0;
	if (distanceX != 0)
		moveX = distanceX < 0 ? -rand() % distanceX: rand() % distanceX;
	int moveY = 0;
	if (distanceY != 0)
		moveY = distanceY < 0 ? -rand() % distanceY : rand() % distanceY;
	// ����������
	Sequence *seq = Sequence::create(MoveBy::create(time / 2, Vec2(0, moveY / 2)), MoveBy::create(time / 2, Vec2(moveX / 2, 0)), NULL);
	enemys[ENERMYCOLOR::PINK]->runAction(seq);
}

// blue ������������ �ƶ� �������
void GameScene::blueEnemyMove(float time) {
	if (enemys[ENERMYCOLOR::BLUE] == NULL || isRewarded)
		return;
	int blue_x = enemys[ENERMYCOLOR::BLUE]->getPositionX(), blue_y = enemys[ENERMYCOLOR::BLUE]->getPositionY();
	int move_distance = -100 + rand() % 200;	// ��������ƶ�����
	bool isMoveX = rand() % 2 ? true : false;	// ��������Ƿ�����ƶ�
	Vec2 moveByVec2 = Vec2(0, 0);
	if (isMoveX) {
		// ����ǽ
		while (move_distance + blue_x > map_x + width - WALLTHINKNESS || move_distance + blue_x < map_x + WALLTHINKNESS) {
			move_distance = -100 + rand() % 200;
		}
		moveByVec2 = Vec2(move_distance, 0);
	}
	else {
		// ����ǽ
		while (move_distance + blue_y > map_y + height - WALLTHINKNESS || move_distance + blue_y < map_y + WALLTHINKNESS) {
			move_distance = -100 + rand() % 200;
		}
		moveByVec2 = Vec2(0, move_distance);
	}
	auto moveBy = MoveBy::create(time, moveByVec2);
	enemys[ENERMYCOLOR::BLUE]->runAction(moveBy);
}

// orange ���������
void GameScene::orangeEnemyMove(float time) {
	if (enemys[ENERMYCOLOR::ORANGE] == NULL || isRewarded)
		return;
	int moveX = enemys[ENERMYCOLOR::ORANGE]->getPositionX() > player->getPositionX() ? -1 : 1;
	int moveY = enemys[ENERMYCOLOR::ORANGE]->getPositionY() > player->getPositionY() ? -1 : 1;
	Sequence *seq = Sequence::create(MoveBy::create(time / 2, Vec2(0, moveY)), MoveBy::create(time / 2, Vec2(moveX, 0)), NULL);
	enemys[ENERMYCOLOR::ORANGE]->runAction(seq);
}

// red ������������ �ƶ� ������루��blue��ͬ��
void GameScene::redEnemyMove(float time) {
	if (enemys[ENERMYCOLOR::RED] == NULL || isRewarded)
		return;
	int red_x = enemys[ENERMYCOLOR::RED]->getPositionX(), red_y = enemys[ENERMYCOLOR::RED]->getPositionY();
	int move_distance = -100 + rand() % 200;
	bool isMoveX = rand() % 2 ? true : false;	// �Ƿ�����ƶ�
	Vec2 moveByVec2 = Vec2(0, 0);
	if (isMoveX) {
		// ����ǽ
		while (move_distance + red_x > map_x + width - WALLTHINKNESS || move_distance + red_x < map_x + WALLTHINKNESS) {
			move_distance = -100 + rand() % 200;
		}
		moveByVec2 = Vec2(move_distance, 0);
	}
	else {
		// ����ǽ
		while (move_distance + red_y > map_y + height - WALLTHINKNESS || move_distance + red_y < map_y + WALLTHINKNESS) {
			move_distance = -100 + rand() % 200;
		}
		moveByVec2 = Vec2(0, move_distance);
	}
	auto moveBy = MoveBy::create(time, moveByVec2);
	enemys[ENERMYCOLOR::RED]->runAction(moveBy);
}

// ��Ӽ�����Ӧ�¼�--�ĸ�����+������ֱ��
void GameScene::addKeyboardListener() {
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(GameScene::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(GameScene::onKeyReleased, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, this);	// ע��ַ���
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode code, Event* event) {
	switch (code) {
		case EventKeyboard::KeyCode::KEY_A:
		case EventKeyboard::KeyCode::KEY_CAPITAL_A:
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		{
			if (move == MOVE::RIGHT)
				player->setFlipX(true);
			else if (move == MOVE::UP) {
				player->setRotation(0);
				player->setFlipX(true);
			}
			else if (move == MOVE::DOWN) {
				player->setRotation(0);
				player->setFlipX(true);
			}
			if (move != MOVE::LEFT || player->getPosition().x > origin.x) {
				move = MOVE::LEFT;
				isMove = true;
			}
			break;
		}
		case EventKeyboard::KeyCode::KEY_D:
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		{
			if (move == MOVE::LEFT)
				player->setFlipX(false);
			else if (move == MOVE::UP)
				player->setRotation(0);
			else if (move == MOVE::DOWN)
				player->setRotation(0);
			if (move != MOVE::RIGHT || player->getPosition().x < origin.x + visibleSize.width) {
				move = MOVE::RIGHT;
				isMove = true;
			}
			break;
		}
		case EventKeyboard::KeyCode::KEY_W:
		case EventKeyboard::KeyCode::KEY_CAPITAL_W:
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		{
			if (move == MOVE::LEFT) {
				player->setFlipX(false);
				player->setRotation(-90);
			}
			else if (move == MOVE::RIGHT) {
				player->setRotation(-90);
			}
			else if (move == MOVE::DOWN)
				player->setRotation(-90);
			if (move != MOVE::UP || player->getPosition().x < origin.x + visibleSize.width) {
				move = MOVE::UP;
				isMove = true;
			}
			break;
		}
		case EventKeyboard::KeyCode::KEY_S:
		case EventKeyboard::KeyCode::KEY_CAPITAL_S:
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW:
		{
			if (move == MOVE::LEFT) {
				player->setFlipX(false);
				player->setRotation(90);
			}
			else if (move == MOVE::RIGHT) {
				player->setRotation(90);
			}
			else if (move == MOVE::UP)
				player->setRotation(90);
			if (move != MOVE::DOWN || player->getPosition().x < origin.x + visibleSize.width) {
				move = MOVE::DOWN;
				isMove = true;
			}
			break;
		}
	}
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode code, Event* event) {
	switch (code) {
		case EventKeyboard::KeyCode::KEY_A:
		case EventKeyboard::KeyCode::KEY_CAPITAL_A:
		case EventKeyboard::KeyCode::KEY_LEFT_ARROW:
		case EventKeyboard::KeyCode::KEY_D:
		case EventKeyboard::KeyCode::KEY_CAPITAL_D:
		case EventKeyboard::KeyCode::KEY_RIGHT_ARROW:
		case EventKeyboard::KeyCode::KEY_W:
		case EventKeyboard::KeyCode::KEY_CAPITAL_W:
		case EventKeyboard::KeyCode::KEY_UP_ARROW:
		case EventKeyboard::KeyCode::KEY_S:
		case EventKeyboard::KeyCode::KEY_CAPITAL_S:
		case EventKeyboard::KeyCode::KEY_DOWN_ARROW: {
			isMove = false;
		}
	}
}

void GameScene::update(float f) {
	if (isMove) {
		this->movePlayer();
	}
	if (collide(player, wall)) {
		toEndScene(NULL, false); // lose
	}
	// ��û�л�ý�����״̬�£��������lose
	if (!isRewarded) {
		for (int i = 0; i < 4; i++) {
			if (enemys[i] != NULL && collide(player, enemys[i])) {
				toEndScene(NULL, false);
			}
		}
	}
	else {	// �ڻ�ý�����״̬�£���ҿ��ԳԵ�����
		for (int i = 0; i < 4; i++) {
			if (enemys[i] != NULL && collide(player, enemys[i])) {
				enemys[i]->removeFromParentAndCleanup(true);
				darkEnermys[i]->removeFromParentAndCleanup(true);
				darkEnermys[i] = NULL;
				enemys[i] = NULL;
			}
		}
	}
	
	// �ж�����Ƿ�Ե����Ӳ���ý���
	for (int i = 0; i < 4; i++) {
		if (bigBeans[i] != NULL && collide(player, bigBeans[i])) {
			bigBeans[i]->removeFromParentAndCleanup(true);
			bigBeans[i] = NULL;
			if (!isRewarded) {
				isRewarded = true;
				getReward();
			}
		}
	}
	// �ж�����Ƿ�Ե�С����
	for (vector<Sprite*>::iterator bean = beans.begin(); bean != beans.end(); bean++) {
		if (collide(player, *bean)) {
			 ++ Global::score;
			(*bean)->removeFromParentAndCleanup(true);
			beans.erase(bean);
			break;
		}
	}
	// ���¼Ƿְ�
	char *score_str = new char[12];
	sprintf(score_str, "score: %d", Global::score);
	score_field->setString(score_str);
	// �Թ����еĶ�
	if (beans.size() == 0)
		toEndScene(NULL, true); // ��Ϸʤ��
}

// ��һ�����й��޾�ֹ����Ľ���
void GameScene::getReward() {
	stillEnermys();		// ʹ���ﾲֹ
	darkenEnermys();	// ʹ������ɫ
	largenPlayer();		// ��Ҵ�С���
	// �ӳ�3s�󣬹��￪ʼ�ƶ�
	auto func1 = CallFuncN::create(CC_CALLBACK_0(GameScene::recoverSprites, this));
	auto func2 = CallFuncN::create(CC_CALLBACK_0(GameScene::enemyMove, this));
	auto seq = Sequence::create(DelayTime::create(3.0f), func1, func2, NULL);
	this->runAction(seq);
}

// ����ұ��һ��
void GameScene::largenPlayer() {
	player->setScale(2);
}

// ȡ�������ƶ��ĵ������������й��ﾲֹ
void GameScene::stillEnermys() {
	for (int i = 0; i < 4; i++) {
		if (enemys[i] != NULL) {
			enemys[i]->stopAllActions();	// ֹͣ��������ж���
		}
	}
	// ȡ�����е�����
	unschedule(schedule_selector(GameScene::pinkEnemyMove));
	unschedule(schedule_selector(GameScene::blueEnemyMove));
	unschedule(schedule_selector(GameScene::orangeEnemyMove));
	unschedule(schedule_selector(GameScene::redEnemyMove));
}

// ���й�����ɫ�������ɫ����ͼƬ�����ڹ������棩
void GameScene::darkenEnermys() {
	for (int i = 0; i < 4; i++) {
		if (enemys[i] != NULL && darkEnermys[i] == NULL) {
			darkEnermys[i] = Sprite::create("sprites/sprite1.png");
			darkEnermys[i]->setPosition(enemys[i]->getPosition());
			this->addChild(darkEnermys[i], 1);
			auto animation = Animation::create();
			for (int i = 1; i < 5; i++) {
				char nameSize[100] = { 0 };
				sprintf(nameSize, "sprites/sprite%d.png", i);
				animation->addSpriteFrameWithFile(nameSize);
			}
			animation->setDelayPerUnit(0.1f);
			animation->setLoops(-1);
			auto animate = Animate::create(animation);
			darkEnermys[i]->runAction(animate);	
		}
	}
}

// �ָ���������
void GameScene::recoverSprites() {
	recoverEnermys();
	playerRecover();
}

// ������ɫ�ָ�����
void GameScene::recoverEnermys() {
	isRewarded = false;
	for (int i = 0; i < 4; i++) {
		if (enemys[i] != NULL && darkEnermys[i] != NULL) {
			darkEnermys[i]->removeFromParentAndCleanup(true);
			darkEnermys[i] = NULL;
		}
	}
}

// ��һָ�������С
void GameScene::playerRecover() {
	player->setScale(1);
}

// ����֮�����ײ
bool GameScene::collide(Sprite *s1, Sprite *s2) {
	if (s2 == NULL || s1 == NULL) {
		return false;
	}
	if (!isRewarded) {
		if (CCCircle(CCPoint(Vec2(s2->getPosition().x, s2->getPosition().y)), s2->getContentSize().width / 2 - 5).intersectsCircle(CCCircle(CCPoint(Vec2(s1->getPosition().x,
			s1->getPosition().y)), s1->getContentSize().height / 2 - 5)))
			return true;
		return false;
	}
	else {
		if (CCCircle(CCPoint(Vec2(s2->getPosition().x, s2->getPosition().y)), s2->getContentSize().width / 2 + 5).intersectsCircle(CCCircle(CCPoint(Vec2(s1->getPosition().x,
			s1->getPosition().y)), s1->getContentSize().height / 2 + 5)))
			return true;
		return false;
	}
}

// ������ǽ
bool GameScene::collide(Sprite *s1, TMXObjectGroup *w) {
	auto container = w->getObjects();
	for (auto obj : container) {
		ValueMap values = obj.asValueMap();
		int x = values.at("x").asInt();
		int y = values.at("y").asInt();
		int width = values.at("width").asInt();
		int height = values.at("height").asInt();
		if (Rect(map_x + x, map_y + y, width, height).intersectsCircle(Vec2(s1->getPosition().x,
			s1->getPosition().y), s1->getContentSize().height / 2 - 3)) {
			return true;
		}
	}
	return false;
}


// ��ת���������
void GameScene::toEndScene(cocos2d::Ref *pSender, bool isWin) {
	SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
	if (isWin) {
		SimpleAudioEngine::getInstance()->playEffect("music/win.wav", false);
	}
	else {
		SimpleAudioEngine::getInstance()->playEffect("music/gameover.mp3", false);
	}
	auto scene = EndScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(0.5, scene, Color3B(0, 0, 0)));
}


void GameScene::submitEvent() {
	if (Global::score > Global::maxscore) {
		//HttpRequest* request = new HttpRequest();
		//request->setUrl((string() + "http://" + Global::ip + ":11900/submit").c_str());
		//request->setRequestType(HttpRequest::Type::POST);
		//request->setResponseCallback(CC_CALLBACK_2(GameScene::onSubmitHttpCompleted, this));

		//std::stringstream ss;
		//ss << Global::score;
		//string temp;
		//ss >> temp;

		//// д��POST��������
		//string tmp = "username=" + Global::username + "&score=" + temp;
		//const char* postData = tmp.c_str();
		//request->setRequestData(postData, strlen(postData));
		//request->setTag("POST test");

		//Global::maxscore = Global::score;
		//database->setIntegerForKey("maxscore", Global::maxscore);

		//cocos2d::network::HttpClient::getInstance()->send(request);
		//request->release();
	}
	else {
		auto endScene = EndScene::createScene();
		Director::getInstance()->replaceScene(endScene);
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
	auto endScene = EndScene::createScene();
	Director::getInstance()->replaceScene(endScene);
}


// �˳�����¼����˵�ѡ����棩
void GameScene::quitEvent(cocos2d::Ref* pSender) {
	SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
	auto selectScene = SelectScene::createScene();
	Director::getInstance()->replaceScene(selectScene);
}

