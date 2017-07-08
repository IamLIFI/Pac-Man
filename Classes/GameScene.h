#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include <string>
#include "Global.h"
using namespace cocos2d;
using namespace cocos2d::ui;

#include "network/HttpClient.h"
using namespace cocos2d::network;

using std::string;

class GameScene : public cocos2d::Layer
{
public:
    // there's no 'id' in cpp, so we recommend returning the class instance pointer
    static cocos2d::Scene* createScene();

    // Here's a difference. Method 'init' in cocos2d-x returns bool, instead of returning 'id' in cocos2d-iphone
    virtual bool init();

    // implement the "static create()" method manually
    CREATE_FUNC(GameScene);

	void addMap();							// ���ص�ͼ
	void addPlayer();						// �������
	void addBeans();						// �����С����
	Sprite* addBigBeans(Vec2 loaction);		// �������
	void addEnemy();						// �������
	Sprite *addEnemyByColor(int color, Vec2 locaion);	// ���벻ͬ��ɫ����

	void preloadMusic();					// ��������
	void playBgm();							// ���ű�������

	void movePlayer();						// ͨ�������ƶ����
	void enemyMove();						// �����ƶ�----��ͬ��ɫ�Ĺ����ƶ���ʽ��һ��
	void pinkEnemyMove(float time);			
	void blueEnemyMove(float time);
	void orangeEnemyMove(float time);
	void redEnemyMove(float time);

	void addKeyboardListener();				// ��Ӽ����¼�������
	void onKeyPressed(EventKeyboard::KeyCode code, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode code, Event* event);

	void getReward();		// ��һ�����й��ﾲֹ����Ľ���
	void stillEnermys();	// ���ﾲֹ
	void darkenEnermys();	// ���й�����ɫ����
	void largenPlayer();	// ����ұ��һ��
	void recoverSprites();	// �ָ����о���
	void recoverEnermys();	// ������ɫ�ָ�
	void playerRecover();	// ��һָ�������С
	

	bool collide(Sprite *s1, Sprite *s2);	// ����֮�����ײ
	bool collide(Sprite *s1, TMXObjectGroup *w);	// ��Һ�ǽ����ײ

	void update(float f);

	// ��ת������ҳ��
	void toEndScene(cocos2d::Ref *pSender, bool isWin);
	void unscheduleAll();					// ȡ�����е�����
	void quitEvent(cocos2d::Ref* pSender);
	void submitEvent();
	void onSubmitHttpCompleted(HttpClient *sender, HttpResponse* response);

private:
	Vec2 origin;				// ��Ϸ����ԭ��
	Size visibleSize;			// �����С
	Label * score_field;		// �÷ְ�
	///////////////////////////

	TMXTiledMap *map;			// ��ͼ
	TMXObjectGroup *wall;		// ��ͼ��Χ��ǽ
	// ��ͼ���½�����
	int map_x;
	int map_y;
	// ��ͼ�ĸ߿�
	int width;
	int height;
	///////////////////////////

	// ���
	Sprite *player;
	// ����ƶ����ĸ�����
	enum MOVE { UP, DOWN, LEFT, RIGHT };
	MOVE move = MOVE::LEFT;		// ����ʼ�ķ���
	bool isMove;				// ����Ƿ��ƶ�
	bool isRewarded;			// ����Ƿ��ý���
	///////////////////////////

	// ����
	Sprite *enemys[4];
	// ������ɫ
	enum ENERMYCOLOR { BLUE, RED, ORANGE, PINK };
	string enermy_color[4] = { "blue", "red", "orange", "pink" };
	Sprite *darkEnermys[4];
	//////////////////////////

	// ����С�Ͷ���
	std::vector<Sprite*> beans;
	// ����
	Sprite *bigBeans[4];

	
	
};

