math.randomseed(os.time())
--------------------------------------------
-- test code:
--------------------------------------------
local NODE_OBSTRUCTION = -1;
local ROW_NUM = 50;
local COLUNM_NUM = 50;
local allNode = {};
local originalCloseMap = {};


local searchNodeCounter = 0;
local usingNodePool = {};
local unuseNodePool = {};
function newSearchNode(id,x,y) -- 这里可以实现一个pool 不知道是否能提速
	
	if(#unuseNodePool > 0)then
		--print("using cache node");
		local node = unuseNodePool[#unuseNodePool];
		unuseNodePool[#unuseNodePool] = nil;
		usingNodePool[#usingNodePool + 1] = node;
		node.id = id;
		node.x = x;
		node.y = y;
		node.g = 0;
		node.f = 0;
		node.g = 0;
		node.parent_id = -1;
		
		return node;
	end

	--print("create SearchNode",searchNodeCounter);
	--searchNodeCounter = searchNodeCounter + 1;

	local node = 
	{
		x = x;
		y = y;
		h = 0;
		g = 0;
		f = 0;
		parent_id = -1;
		id = id;
	};

	usingNodePool[#usingNodePool + 1] = node;
	return node;
end

function clearSearchNode()
	for i=1,#usingNodePool do
		unuseNodePool[#unuseNodePool + 1] = usingNodePool[i];
	end
	usingNodePool = {};
end

function newNode(id,x,y)
	local node = 
	{
		id = id;
		x = x;
		y = y;
		isObstruction = false;--(math.random(1,8) % 5 == 0) and id ~= searchStartID and id ~= searchFinishID;
	}
	return node;
end


function initAllNode()
	for colunm = 0 , COLUNM_NUM - 1 do
		for row = 0, ROW_NUM - 1  do
			local id = row + colunm * COLUNM_NUM;
			allNode[id] = newNode(id,row,colunm);
			---这里预先记录下阻挡的点 避免每一次计算 (NODE_OBSTRUCTION有阻挡,nil无阻挡)
			if(allNode[id].isObstruction)then
				originalCloseMap[id] = NODE_OBSTRUCTION;
			end
		end
	end

end


function _checkNode(nodeX,nodeY,targetNode,parentNode,closeMap,openList)
	local nodeId = nodeX + nodeY * COLUNM_NUM;
	if(nodeId == targetNode.id)then
		print("find path! ");
		return true;
	end
	if(closeMap[nodeId] == nil)then --这个点没在关闭列表中
		local isExistInOpenList = false;
		for k,v in pairs(openList) do
			if(v.id == nodeId)then -- 已经在开放列表中 判断F值是否改变
				isExistInOpenList = true;
				local newF = (parentNode.h + 1) + v.g;
				if(newF < v.f)then --更好的路径
					v.parent_id = parentNode.id;
					v.h = parentNode.h + 1;
					v.f = newF;
					table.sort(openList,function(n1,n2)
						return n1.f > n2.f;
					end)
				end
				
				break;
			end
		end

		if(not isExistInOpenList)then--没在开放列表中
			local node = newSearchNode(nodeId,nodeX,nodeY);
			node.h = parentNode.h + 1;
			node.g = math.abs(node.x - targetNode.x) + math.abs(node.y - targetNode.y);
			node.f = node.h + node.g;
			node.parent_id = parentNode.id;
			table.insert(openList,node)
			table.sort(openList,function(n1,n2)
				return n1.f > n2.f;
			end)
		end
	end
	return false;
end

local pathMap = {};

function _findPath(openList,closeMap,targetId)
	local curNode;--= openList[#openList];--默认openlist 最后那个是代价最低的
	while(true)do
		curNode = openList[#openList];--默认openlist 最后那个是代价最低的
		if(curNode == nil)then
			print("can not find the path");
			return nil;
		end
		local targetNode = allNode[targetId];
		assert(closeMap[curNode.id] == nil,"curNode.id : " .. curNode.id);
		closeMap[curNode.id] = curNode.parent_id;

		for i,v in pairs(openList) do
			if(v.id == curNode.id)then
				table.remove(openList,i);
				break;
			end
		end
		local isFind = false;

		--左上
		if(curNode.x > 0 and curNode.y > 0)then
			isFind =  _checkNode(curNode.x-1,curNode.y-1,targetNode,curNode,closeMap,openList);
		end

		--正上
		if(not isFind and curNode.y > 0)then
			isFind = _checkNode(curNode.x,curNode.y-1,targetNode,curNode,closeMap,openList);
		end

		--右上
		if(not isFind and (curNode.x < ROW_NUM - 1) and  curNode.y > 0 )then
			isFind = _checkNode(curNode.x + 1,curNode.y - 1,targetNode,curNode,closeMap,openList);
		end

		--右边
		if(not isFind and curNode.x < ROW_NUM - 1)then
			isFind = _checkNode(curNode.x + 1,curNode.y,targetNode,curNode,closeMap,openList);
		end

		--右下
		if(not isFind and curNode.x < ROW_NUM - 1 and curNode.y < COLUNM_NUM - 1)then
			isFind = _checkNode(curNode.x + 1,curNode.y + 1,targetNode,curNode,closeMap,openList);
		end

		--下方
		if(not isFind and curNode.y < COLUNM_NUM - 1)then
			isFind = _checkNode(curNode.x,curNode.y + 1,targetNode,curNode,closeMap,openList);
		end

		--左下
		if(not isFind and  curNode.x > 0 and   curNode.y < COLUNM_NUM - 1)then
			isFind = _checkNode(curNode.x - 1,curNode.y + 1,targetNode,curNode,closeMap,openList);
		end

		--左边
		if(not isFind and curNode.x > 0)then
			isFind = _checkNode(curNode.x - 1,curNode.y,targetNode,curNode,closeMap,openList);
		end
		
		if(isFind)then
			break;
		end
	end

	pathMap = {};
	pathMap[curNode.id] = true;
	local parent_id = closeMap[curNode.id];
	while(parent_id ~= nil and parent_id ~= NODE_OBSTRUCTION)do
		print("path:",parent_id);
		pathMap[parent_id] = true;
		parent_id = closeMap[parent_id];
	end
	pathMap[targetId] = true;
end




function findPath(startId,targetId)
	--起始点加入open队列
	local closeMap = {};
	local openList = {};

	for k,v in pairs(originalCloseMap) do
		closeMap[k] = v; --这里add node 的时候会保存当前点得父节点ID 用于回溯
	end
	
	local currentNode = newSearchNode(startId,allNode[startId].x,allNode[startId].y);
	local targetNode = allNode[targetId];
	
	currentNode.h = 0;
	currentNode.g = math.abs(currentNode.x - targetNode.x) + math.abs(currentNode.y - targetNode.y);
	currentNode.f = currentNode.h + currentNode.g;
	openList[1] = currentNode;
	_findPath(openList,closeMap,targetId);


	
	for colunm = 0 , COLUNM_NUM - 1 do
		printStr = "";
		for row = 0, ROW_NUM - 1  do
			local id = row + colunm * COLUNM_NUM;
			if(pathMap[id])then
				printStr = printStr .. "*"
			else
				if(allNode[id].isObstruction)then
					printStr = printStr .. "|"
				else
					printStr = printStr .. "-"
				end
			end
		end
		print(printStr);
	end

	clearSearchNode();
end


initAllNode();
local a = os.clock();

findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
-- findPath(math.random(0,2500),math.random(0,2500))
print("use time :",os.clock() - a);

















