//
// Created by suibin on 17-12-26.
//

#ifndef WORDSIMILARITY_MATHLIB_H
#define WORDSIMILARITY_MATHLIB_H

#include <math.h>

#define    MINZERO            (0.0000001)

class CCalculate {
public:
    CCalculate() {};

    virtual ~CCalculate() {};

    //获取最大值
    template<class T>
    static T GetMax(T *x, int nLen) {
        T Temp, Max;
        Max = *x;
        for (int i = 0; i < nLen; i++) {
            Temp = *(x + i);
            if (Temp >= Max) {
                Max = Temp;
            }
        }
        return Max;
    }

    //获取最大值序号 从0开始计算
    template<class T>
    static int GetMaxNum(T *x, int nLen) {
        T Temp, Max;
        Max = *x;
        int n = 0;
        for (int i = 0; i < nLen; i++) {
            Temp = *(x + i);
            if (Temp >= Max) {
                Max = Temp;
                n = i;
            }
        }
        return n;
    }

    //获取最小值
    template<class T>
    static T GetMin(T *x, int nLen) {
        T Temp, Min;
        Min = *x;
        for (int i = 0; i < nLen; i++) {
            Temp = *(x + i);
            if (Temp <= Min) {
                Min = Temp;
            }
        }
        return Min;
    }

    //获取最小值序号 从0开始计算
    template<class T>
    static int GetMinNum(T *x, int nLen) {
        T Temp, Min;
        Min = *x;
        int n = 0;
        for (int i = 0; i < nLen; i++) {
            Temp = *(x + i);
            if (Temp <= Min) {
                Min = Temp;
                n = i;
            }
        }
        return n;
    }

    //获取平均值
    template<class T>
    static double GetAve(T *x, int nLen) {
        double sum = 0.00;
        double Ave = 0.00;
        for (int i = 0; i < nLen; i++) {
            sum += (double) *(x + i);
        }
        Ave = sum / nLen;
        return Ave;
    }

    //获取标准差=标准偏差
    template<class T>
    static double GetSD(T *x, int nLen) {
        if (nLen <= 0) {
            return -1;
        }
        double sum = 0.00;
        double Ave = 0.00;
        Ave = GetAve(x, nLen);
        for (int i = 0; i < nLen; ++i) {
            sum += (*(x + i) - Ave) * (*(x + i) - Ave);

        }
        double SD = sqrt((sum / nLen));
        return SD;
    }

    //获取标准误差SE
    template<class T>
    static double GetSE(T *x, int nLen) {
        double SD = 0.00;
        SD = GetSD(x, nLen);
        double SE = double(SD / sqrt((double) nLen));
        return SE;
    }

    //获取变异系数CV
    template<class T>
    static bool GetCV(T *x, int nLen, double &CV) {
        double Ave = 0.00;
        double SD = 0.00;
        Ave = GetAve(x, nLen);
        if (Ave < MINZERO && Ave > -MINZERO) {
            return false;
        }
        SD = GetSD(x, nLen);
        CV = SD / (double) Ave;
        return true;
    }

    //获取平滑指数SI：Smooth Index
    template<class T>
    static bool GetSI(T *x, int nLen, double &SI) {
        double CV = 0.00;
        if (!GetCV(x, nLen, CV)) {
            return false;
        }
        if (CV < MINZERO && CV > -MINZERO) {
            return false;
        }
        SI = 1.00 / CV;
        return true;
    }

    //获取大于thres的数据的百分比
    template<class T>
    static double GetUpPercent(T *x, int nLen, T thres) {
        double dPer = 0.00;
        int nCount = 0;
        for (int i = 0; i < nLen; i++) {
            if (*(x + i) > thres) {
                nCount++;
            }
        }
        dPer = ((double) nCount) / nLen;
        return dPer;
    }

    //获取大于thres的数据的个数
    template<class T>
    static int GetUpNum(T *x, int nLen, T thres) {
        int nCount = 0;
        for (int i = 0; i < nLen; i++) {
            if (*(x + i) > thres) {
                nCount++;
            }
        }
        return nCount;
    }

    //获取小于thres的数据的百分比
    template<class T>
    static double GetDownPercent(T *x, int nLen, T thres) {
        double dPer = 0.00;
        int nCount = 0;
        for (int i = 0; i < nLen; i++) {
            if (*(x + i) < thres) {
                nCount++;
            }
        }
        dPer = ((double) nCount) / nLen;
        return dPer;
    }

    //获取小于thres的数据的个数
    template<class T>
    static int GetDownNum(T *x, int nLen, T thres) {
        int nCount = 0;
        for (int i = 0; i < nLen; i++) {
            if (*(x + i) < thres) {
                nCount++;
            }
        }
        return nCount;
    }

    //获取两组数据的相关系数，无前后关系
    template<class T>
    static bool GetCorrCoef(T *x, T *y, int nLen, double &r) {
        double xAve = GetAve(x, nLen);
        double yAve = GetAve(y, nLen);
        double Sum = 0.00;
        double xSum2 = 0.00;
        double ySum2 = 0.00;

        for (int i = 0; i < nLen; i++) {
            Sum += (*(x + i) - xAve) * (*(y + i) - yAve);
            xSum2 += (*(x + i) - xAve) * (*(x + i) - xAve);
            ySum2 += (*(y + i) - yAve) * (*(y + i) - yAve);
        }
        double res = sqrt(xSum2 * ySum2);
        if (res < MINZERO && res > -MINZERO) {
            return false;
        }
        r = Sum / res;
        return true;
    }

    //获取两组数据的回归系数，有前后关系		x自变量，y因变量  b斜率 a截距
    template<class T>
    static bool GetRegressCoef(T *x, T *y, int nLen, double &b, double &a) {
        double xAve = GetAve(x, nLen);
        double yAve = GetAve(y, nLen);
        double Sum = 0.00;
        double xSum2 = 0.00;

        for (int i = 0; i < nLen; i++) {
            Sum += (*(x + i) - xAve) * (*(y + i) - yAve);
            xSum2 += (*(x + i) - xAve) * (*(x + i) - xAve);
        }
        if (xSum2 < MINZERO && xSum2 > -MINZERO) {
            return false;
        }
        b = Sum / xSum2;
        a = yAve - xAve * b;
        return true;
    }
};

#endif //WORDSIMILARITY_MATHLIB_H
