#include<iostream>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include<algorithm>

using namespace cv;
using namespace std;

void fillRunVectors(const Mat& bwImagetemp, int& NumberOfRuns, vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun)
{
	Mat bwImage;
	threshold(bwImagetemp, bwImage, 100, 255, CV_THRESH_BINARY);
	imshow("1", bwImage);
	for (int i = 0; i < bwImage.rows; i++)
	{
		//uchar* rowData = bwImage.ptr<uchar>(i);
		//uchar* rowData = new uchar[bwImage.cols];
		vector<uchar> rowData;
		for (int k = 0; k < bwImage.cols; k++)
		{
			rowData.push_back(bwImage.at<uchar>(i, k));
		}

		if (rowData[0] == 255)
		{
			NumberOfRuns++;
			stRun.push_back(0);
			rowRun.push_back(i);
		}
		for (int j = 1; j < bwImage.cols; j++)
		{
			if (rowData[j-1] == 0 && rowData[j] == 255)			//列开始处
			{
				int aa = rowData[j - 1];
				int bb = rowData[j];
				NumberOfRuns++;
				stRun.push_back(j);
				rowRun.push_back(i);
			}
			else if (rowData[j-1] == 255 && rowData[j] == 0)	//列结束处
			{
				enRun.push_back(j - 1);
			}
		}
		if (rowData[bwImage.cols - 1])
		{
			enRun.push_back(bwImage.cols - 1);
		}
	}
}

void firstPass(vector<int>& stRun, vector<int>& enRun, vector<int>& rowRun, int NumberOfRuns, vector<int>& runLabels, vector<pair<int, int>>& equicalences, int offset)
{
	runLabels.assign(NumberOfRuns, 0);   //（元素个数，要初始化的值）
	int idxLabel = 1;
	int curRowIdx = 0;
	int firstRunOnCur = 0;
	int firstRunOnPre = 0;
	int lastRunOnPre = -1;
	int maxDif = 0;
	for (int i = 0; i < NumberOfRuns; i++)
	{
		if (rowRun[i] != curRowIdx)		//如果是该行的第一个run，则更新上一行第一个run的最后一个run的序号
		{
			curRowIdx = rowRun[i];		//更新行的序号
			firstRunOnPre = firstRunOnCur;
			lastRunOnPre = i;
			firstRunOnCur = i;
		}
		for (int j = firstRunOnPre; j < lastRunOnPre; j++)		//遍历上一行的所有run，判断是否当前run有重合的区域
		{
			//区域重合且处于相邻的两行
			if (stRun[i] <= enRun[j] + offset && enRun[i] >= stRun[j] - offset && rowRun[i] == rowRun[j] + 1)	//offset：4邻接、8邻接，值为0或1
			{
				if (runLabels[i] == 0)					//没有被标号过
				{
					runLabels[i] = runLabels[j];
				}
				else if (runLabels[i] != runLabels[j])	//已经被标号
				{
					equicalences.push_back(make_pair(runLabels[i], runLabels[j]));	//保存等价对
				}
			}
			if ((lastRunOnPre - firstRunOnPre)>maxDif)
			{
				maxDif = lastRunOnPre - firstRunOnPre;
			}
		}
		if (runLabels[i] == 0)		//没有与前一列任何run重合
		{
			runLabels[i] = idxLabel++;
		}
	}
}

void replaceSameLabel(vector<int>& runLabels, vector<pair<int, int>>& equivalence)
{
	int maxLabel = *max_element(runLabels.begin(), runLabels.end());
	vector<vector<bool>> eqTab(maxLabel, vector<bool>(maxLabel, false));
	vector<pair<int, int>>::iterator vecPairIt = equivalence.begin();
	while (vecPairIt != equivalence.end())
	{
		eqTab[vecPairIt->first - 1][vecPairIt->second - 1] = true;
		eqTab[vecPairIt->second - 1][vecPairIt->first - 1] = true;
		vecPairIt++;
	}
	vector<int> labelFlag(maxLabel, 0);
	vector<vector<int>> equaList;
	vector<int> tempList;
	cout << maxLabel << endl;
	int maxOne = 0;
	int maxTwo = 0;
	for (int i = 1; i <= maxLabel; i++)
	{
		if (labelFlag[i - 1])
		{
			continue;
		}
		labelFlag[i - 1] = equaList.size() + 1;
		tempList.push_back(i);
		int tempSize = tempList.size();
		for (vector<int>::size_type j = 0; j < tempList.size(); j++)
		{
			if (tempList.size() > maxOne)
			{
				maxOne = tempList.size();
			}
			int eqSize = eqTab[tempList[j] - 1].size();
			for (vector<bool>::size_type k = 0; k < eqTab[tempList[j] - 1].size(); k++)
			{
				if (eqTab[tempList[j] - 1].size() > maxTwo)
				{
					maxTwo = eqTab[tempList[j] - 1].size();
				}
				if (eqTab[tempList[j] - 1][k] && !labelFlag[k])
				{
					tempList.push_back(k + 1);
					labelFlag[k] = equaList.size() + 1;
				}
			}
		}
		equaList.push_back(tempList);
		tempList.clear();
	}
	cout << equaList.size() << endl;
	for (vector<int>::size_type i = 0; i < runLabels.size(); i++)
	{
		runLabels[i] = labelFlag[runLabels[i] - 1];
	}
}

void main()
{
	//vector<int> para1;
	//for (int i = 1; i <= 15; i++)
	//{
	//	para1.push_back(i);
	//}
	//vector<pair<int, int>> para2;
	//para2.push_back(make_pair(1, 2));
	//para2.push_back(make_pair(1, 6));
	//para2.push_back(make_pair(3, 7));
	//para2.push_back(make_pair(9, 3));
	//para2.push_back(make_pair(8, 1));
	//para2.push_back(make_pair(8, 10));
	//para2.push_back(make_pair(11, 5));
	//para2.push_back(make_pair(11, 8));
	//para2.push_back(make_pair(11, 12));
	//para2.push_back(make_pair(11, 13));
	//para2.push_back(make_pair(11, 14));
	//para2.push_back(make_pair(15, 11));

	//replaceSameLabel(para1, para2);
	//vector<int> stRun = { 1, 9, 5, 8, 1, 6 };
	//vector<int> enRun = { 5, 12, 6, 9, 3, 7 };
	//vector<int> rowRun = { 0, 0, 1, 1, 2, 2 };
	//int NumberOfRuns = 6;
	//vector<int> runLabels = { 0, 0, 0, 0, 0, 0 };
	//vector<pair<int, int>> equicalences;
	//int offset = 1;

	//firstPass(stRun, enRun, rowRun, NumberOfRuns, runLabels, equicalences, offset);

	Mat srcImage = imread("1.JPG", 0);
	int numberOfRuns = 0;
	vector<int> stRun;
	vector<int> enRun;
	vector<int> rowRun;
	vector<int> runLabels;
	vector<pair<int, int>> equicalences;
	int offset = 1;
	fillRunVectors(srcImage, numberOfRuns, stRun, enRun, rowRun);
	firstPass(stRun, enRun, rowRun, numberOfRuns, runLabels, equicalences, offset);
	replaceSameLabel(runLabels, equicalences);
	waitKey();
}
