#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define ROW_NUM (50)
#define COLUMN_NUM (50)
#define MIN_DIS (-999)
#define TOTAL_NUM (ROW_NUM*COLUMN_NUM)
#define bool char
#define false 0
#define true 1
#define ABS(x) (x)>=0?(x):-(x)
#define ASSERT(b,str) if(!(b)){printf("assert fail :%s\nLINE:%d\n",str,__LINE__);abort();}
#define FREE(p) free(p);p=NULL; 


//实际运用的时候,node是我们数据相关的结构,如"地图节点"
typedef struct node
{
	short id;
	short x;
	short y;
	bool isObstruction;
}* pNode;


//search 用节点实现双向链表
typedef struct searchNode
{
	short x;
	short y;
	float g;
	float f;
	float h;
	short id;
	struct searchNode* parent;
	struct searchNode* next;
	struct searchNode* prev;
}* pSearchNode;

/*
typedef struct searchNodeList
{
	pSearchNode self;
	struct searchNodeList* next;
}* pSearchNodeList;*/

pNode* allNode;
pSearchNode* originalCloseMap;

pNode newNode(short id,short x,short y,bool isObstruction)
{
	pNode node= (pNode)malloc(sizeof(struct node));
	node->id = id;
	node->x = x;
	node->y = y;
	node->isObstruction = isObstruction;

	return node;
}


pSearchNode newSearchNode(short id,short x,short y)
{
	pSearchNode node= (pSearchNode)malloc(sizeof(struct searchNode));
	node->x = x;
	node->y = y;
	node->g = node->g = node->f = 0;
	node->id = id;
	node->prev = node->next = node->parent = NULL;

	return node;
}

pSearchNode addNodeToList(pSearchNode pHead,pSearchNode node)
{
	//默认链表的第一个元素是占位用的,避免这个链表被清空重置为NULL,
	//默认最小F值的 node放在链表第二个位置 即:  pHead->next

	if(pHead == NULL) //open 列表为空 还未初始化,就初始化,并且返回Head,
	{
		pHead = newSearchNode(-1,-1,-1);
		pHead->next = node;
		pHead->g = pHead->h = pHead->f = MIN_DIS;//排序用 让pHead不用加特殊判断的情况下永远放到第一个
		node->prev = pHead;
		return pHead;
	}
	else
	{
		pSearchNode p = pHead;
		do
		{	
			if(p->f >= node->f)//说明这个点比当前遍历的点p,路径更优,需要放到p前面[默然最优点放到链表顶部]
			{
				node->prev = p->prev;
				node->next = p;
				p->prev->next = node;
				p->prev = node;

				break;
			}
			else if(p->next == NULL)//已经遍历完整个链表了都没找到比node“更差”的节点
			{
				p->next = node;
				node->prev =p;
				break;
			}

			p = p->next;
		} while (p != NULL);
	}

	return pHead;
}

pSearchNode removeNodeFromList(pSearchNode pHead,short nodeId)
{
	ASSERT(pHead != NULL && pHead->next != NULL,"remove node from empty List!");
	pSearchNode p = pHead;
	do
	{
		if(p->id == nodeId)
		{
			p->prev->next = p->next;
			if(p->next!=NULL)
			{
				p->next->prev = p->prev;	
			}
			
			return p;
		}

		p = p->next;
	} while (p!= NULL);

	ASSERT(false,"can not find node form list");
	return NULL;
}


void initAllNode()
{
	allNode = (pNode*)malloc(sizeof(pNode) * TOTAL_NUM);
	originalCloseMap = (pSearchNode*)malloc(sizeof(pSearchNode) * TOTAL_NUM);
	for (int colunm = 0; colunm < COLUMN_NUM; ++colunm)
	{
		for (int row = 0; row < ROW_NUM; ++row)
		{
			short id = row + colunm * COLUMN_NUM;
			allNode[id] = newNode(id,row,colunm,false);
			if(allNode[id]->isObstruction)
			{
				originalCloseMap[id] = newSearchNode(id,row,colunm);
			}
			else
			{
				originalCloseMap[id] = NULL;
			}
		}
	}
}

bool _checkNode( short x
				,short y
				,pSearchNode targetNode
				,pSearchNode parentNode
				,pSearchNode* closeMap
				,pSearchNode pHead)
{
	short nodeId = x + y * COLUMN_NUM;
	if(nodeId == targetNode->id)
	{
		printf("find path!\n");
		return true;
	}

	if(closeMap[nodeId] == NULL)//未在关闭列表中 不然直接跳过
	{
		bool isExistInOpenList = false;//是否在openlist中

		pSearchNode p = pHead;
		do//遍历open寻找nodeId节点
		{
			if(p->id == nodeId)
			{
				isExistInOpenList = true;
				float  add = (p->x != parentNode->x &&p->y != parentNode->y) ? 1.4 : 1;//斜向的格子距离为根号2 ：1.414
				float newF = (parentNode->h + add) + p->g;
				if(newF <= p->f)//从这条路走来更优 替换节点当前的parent,重置 h f
				{
					p->parent = parentNode;
					p->h = (parentNode->h + add);
					p->f = newF;
					//排序 目前还没写,直接先移除再添加,就是有序的了
					//排序只有遍历一次 找到它 重置前后关系就OK
					removeNodeFromList(pHead,nodeId);
					addNodeToList(pHead,p);
				}
				
				break;
			}

			p = p->next;
		} while (p != NULL);

		if(!isExistInOpenList)//不存在open list中 添加进去
		{
			pSearchNode node = newSearchNode(nodeId,x,y);
			float add = (node->x != parentNode->x && node->y != parentNode->y) ? 1.4 : 1;//斜向的格子距离为根号2 ：1.414
			node->h = parentNode->h + add;
			short posX = (node->x - targetNode->x);
			short posY = (node->x - targetNode->x);
			node->g = sqrt(posX*posX + posY *posY);
			//node->g = ABS(node->x - targetNode->x) + ABS(node->x - targetNode->x);
			node->f = node->g + node->h;
			node->parent = parentNode;
			addNodeToList(pHead,node);
		}
	}
	return false;
}

void _findPath(pSearchNode pHead,pSearchNode* closeMap,pSearchNode targetNode)
{
	ASSERT(pHead!=NULL,"pHead is NULL");
	ASSERT(closeMap!=NULL,"closeMap is NULL");
	ASSERT(targetNode!=NULL,"targetNode is NULL");

	//for test 
	short startId = pHead->next->id;
	short targetId = targetNode->id;
	//for test end
	pSearchNode curNode = NULL;
	do
	{
		curNode = pHead->next;//代价最小的几点放在pHead->next 参看 addNodeToList
		if(curNode == NULL)//open list 已经为空,还未找到路径,无路可走
		{
			printf("can not find the path\n");
			return;
		}
		removeNodeFromList(pHead,curNode->id);
		ASSERT(closeMap[curNode->id] == NULL,"a node in open list && close map both");
		closeMap[curNode->id] = curNode;
		bool isFind = false;
		//左上
		if(curNode->x > 0 && curNode->y > 0)
			isFind =  _checkNode(curNode->x-1,curNode->y-1,targetNode,curNode,closeMap,pHead);

		//正上
		if(!isFind && curNode->y > 0)
			isFind =  _checkNode(curNode->x,curNode->y-1,targetNode,curNode,closeMap,pHead);

		//右上
		if(!isFind && (curNode->x < ROW_NUM - 1) &&  curNode->y > 0 )
			isFind = _checkNode(curNode->x + 1,curNode->y - 1,targetNode,curNode,closeMap,pHead);

		//右边
		if(!isFind && curNode->x < ROW_NUM - 1)
			isFind = _checkNode(curNode->x + 1,curNode->y,targetNode,curNode,closeMap,pHead);

		//右下
		if(!isFind && curNode->x< ROW_NUM - 1 && curNode->y < COLUMN_NUM - 1)
			isFind = _checkNode(curNode->x+ 1,curNode->y + 1,targetNode,curNode,closeMap,pHead);

		//下方
		if(!isFind && curNode->y < COLUMN_NUM - 1)
			isFind = _checkNode(curNode->x,curNode->y + 1,targetNode,curNode,closeMap,pHead);

		//左下
		if(!isFind &&  curNode->x> 0 &&   curNode->y < COLUMN_NUM - 1)
			isFind = _checkNode(curNode->x- 1,curNode->y + 1,targetNode,curNode,closeMap,pHead);

		//左边
		if(!isFind && curNode->x> 0)
			isFind = _checkNode(curNode->x- 1,curNode->y,targetNode,curNode,closeMap,pHead);
		if(isFind)//找到路径
			break;

	} while (true);


	//测试代码 在console下画出路径
	int mapSize = sizeof(char) * TOTAL_NUM;
	char* pathMap = (char*) malloc(mapSize);
	memset(pathMap,0,mapSize);
	int counter = 0;
	do
	{
		counter++;
		pathMap[curNode->id] = 1;
		printf("path : %d\n", curNode->id);
		curNode = curNode->parent;
	} while (curNode != NULL);

	pathMap[targetNode->id] = 1;

	printf("counter : %d\n", counter++);
	
	for (int colunm = 0; colunm < COLUMN_NUM; ++colunm)
	{
		for (int row = 0; row < ROW_NUM; ++row)
		{
			short id = row + colunm * COLUMN_NUM;
			if(allNode[id]->isObstruction)
			{
				printf("|");
			}
			else
			{
				if(pathMap[id] == 1)
				{
					if(id == startId)
					{
						printf("S");	
					}
					else if(id == targetId)
					{
						printf("E");
					}
					else
					{
						printf("*");
					}
					
				}
				else
				{
					printf("-");
				}
				
			}
		}

		printf("\n");
	}

	FREE(pathMap);
}

void findPath(short startId,short targetId)
{
	//关闭节点的MAP 空间换时间 这个MAP仅仅做查询和赋值操作
	pSearchNode* closeMap = (pSearchNode*)malloc(sizeof(pSearchNode) * TOTAL_NUM);

	//开放节点列表
	pSearchNode pOpenHead = NULL;


	for (int id = 0; id < TOTAL_NUM; ++id)
	{
		if(originalCloseMap[id] != NULL)
		{
			
			closeMap[id] = originalCloseMap[id];
		}
		else
		{
			closeMap[id] = NULL;
		}
	}

	pSearchNode curNode = newSearchNode(startId,allNode[startId]->x,allNode[startId]->y);
	pSearchNode targetNode = newSearchNode(targetId,allNode[targetId]->x,allNode[targetId]->y);
	short posX = (curNode->x - targetNode->x);
	short posY = (curNode->y - targetNode->y);
	curNode->g = sqrt(posX*posX + posY*posY);
	//curNode->g = ABS(curNode->x - targetNode->x) + ABS(curNode->y - targetNode->y);
	curNode->f = curNode->g + curNode->h;

	pOpenHead = addNodeToList(NULL,curNode);
	_findPath(pOpenHead,closeMap,targetNode);
}



int main()
{
	srand((unsigned)time(0));
	initAllNode();
	findPath(rand()%2500,rand()%2500);
	//findPath(1,94);
	return 0;
}